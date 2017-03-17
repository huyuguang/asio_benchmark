#include <algorithm>
#include <iostream>
#include <list>
#include <string>
#include <mutex>
#include <assert.h>
#include <vector>
#include <memory>
#include <thread>
#include "io_pool.h"

namespace {

#pragma pack(push,1)
struct Header {
    uint32_t body_size_; // net order
    uint32_t packet_count_; // net order
    uint32_t get_body_size() const {
        return ntohl(body_size_);
    }
    uint32_t get_full_size() const {
        return sizeof(Header) + ntohl(body_size_);
    }
    void inc_packet_count() {
        uint32_t count = ntohl(packet_count_) + 1;
        packet_count_ = htonl(count);
    }
};
#pragma pack(pop)

uint64_t clock_us() {
    return std::chrono::steady_clock::now().time_since_epoch().count() / 1000;
}

class session : noncopyable
{
public:
    session(asio::io_service& ios, size_t total_count)
        : io_service_(ios)
        , socket_(ios)
        , total_count_(total_count) {
    }

    ~session() {
    }

    void write() {
        Header* header = (Header*)buffer_;
        uint32_t full_size = header->get_full_size();
        asio::async_write(socket_, asio::buffer(buffer_, full_size),
            [this](const asio::error_code& err, size_t cb) {
            if (!err) {
                read_head();
            }
        });
    }

    void read_head() {
        asio::async_read(socket_, asio::buffer(buffer_, sizeof(Header)),
            [this](const asio::error_code& err, size_t cb) {
            if (!err) {
                read_body();
            }
        });
    }

    void read_body() {
        Header* header = (Header*)buffer_;
        size_t body_size = header->get_body_size();
        asio::async_read(socket_,
            asio::buffer(buffer_ + sizeof(Header), body_size),
            [this](const asio::error_code& err, size_t cb) {
            if (!err) {
                if (check_count()) {
                    finished_ = true;
                    stop_time_ = clock_us();
                    socket_.close();
                } else {
                    Header* header = (Header*)buffer_;
                    header->body_size_ = htonl(get_body_len());
                    header->inc_packet_count();

                    write();
                }
            }
        });
    }

    void start(asio::ip::tcp::endpoint endpoint) {
        socket_.async_connect(endpoint, [this](const asio::error_code& err) {
            if (!err)
            {
                asio::ip::tcp::no_delay no_delay(true);
                socket_.set_option(no_delay);

                Header* header = (Header*)buffer_;
                header->body_size_ = htonl(get_body_len());
                header->packet_count_ = htonl(1);

                start_time_ = clock_us();
                write();
            }
        });
    }

    bool finished() const {
        return finished_;
    }

    uint64_t use_time() const {
        return stop_time_ - start_time_;
    }
private:
    uint32_t get_body_len() {
        if (body_len_ > 10000)
            body_len_ = 100;
        return body_len_++;
    }
    
    bool check_count() {
        Header* header = (Header*)buffer_;
        return (ntohl(header->packet_count_) >= total_count_);
    }
private:
    asio::io_service& io_service_;
    asio::ip::tcp::socket socket_;
    static size_t const max_block_size_ = 10000 + sizeof(Header);
    char buffer_[max_block_size_];
    bool finished_ = false;
    uint32_t const total_count_;
    uint64_t start_time_ = 0;
    uint64_t stop_time_ = 0;
    uint32_t body_len_ = 100;
};

class client : noncopyable
{
public:
    client(int thread_count,
        char const* host, char const* port,
        uint32_t total_count, size_t session_count)
        : thread_count_(thread_count) {
        io_services_.resize(thread_count_);
        threads_.resize(thread_count_);

        for (auto i = 0; i < thread_count_; ++i) {
            io_services_[i].reset(new asio::io_service);
        }

        resolver_.reset(new asio::ip::tcp::resolver(*io_services_[0]));
        asio::ip::tcp::resolver::iterator iter =
            resolver_->resolve(asio::ip::tcp::resolver::query(host, port));
        endpoint_ = *iter;

        for (size_t i = 0; i < session_count; ++i)
        {
            auto& io_service = *io_services_[i % thread_count_];
            std::unique_ptr<session> new_session(new session(io_service,
                total_count));
            sessions_.emplace_back(std::move(new_session));
        }
    }

    ~client() {
        uint32_t finished_count = 0;
        uint32_t error_count = 0;
        uint64_t total_time = 0;
        for (auto& session : sessions_) {
            if (session->finished()) {
                ++finished_count;
                total_time += session->use_time();
            } else {
                ++error_count;
            }
        }
        std::cout << "failed count: " << error_count << "\n";
        std::cout << "finished count: " << finished_count << "\n";
        std::cout << "average time(us): " <<
            (uint64_t)((double)total_time / finished_count) << "\n";
    }

    void start() {
        for (auto& session : sessions_) {
            session->start(endpoint_);
        }
        for (auto i = 0; i < thread_count_; ++i) {
            threads_[i].reset(new std::thread([this, i]() {
                auto& io_service = *io_services_[i];
                try {
                    io_service.run();
                } catch (std::exception& e) {
                    std::cerr << "Exception: " << e.what() << "\n";
                }
            }));
        }
    }

    void wait() {
        for (auto& thread : threads_) {
            thread->join();
        }
    }

private:
    int const thread_count_;
    std::vector<std::unique_ptr<asio::io_service>> io_services_;
    std::unique_ptr<asio::ip::tcp::resolver> resolver_;
    std::vector<std::unique_ptr<std::thread>> threads_;
    std::vector<std::unique_ptr<session>> sessions_;
    asio::ip::tcp::endpoint endpoint_;
};

} // namespace 

void client_test3(int thread_count, char const* host, char const* port,
    size_t total_count, size_t session_count) {
    try {
        client c(thread_count, host, port, total_count, session_count);
        c.start();
        c.wait();
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}

