#include <algorithm>
#include <iostream>
#include <list>
#include <string>
#include <mutex>
#include <assert.h>
#include <vector>
#include <memory>
#include <queue>
#include <stack>
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

class session : public std::enable_shared_from_this<session>, noncopyable
{
public:
    session(asio::io_service& ios, size_t total_count)
        : io_service_(ios)
        , socket_(ios)
        , total_count_(total_count) {
    }

    ~session() {
    }

    asio::ip::tcp::socket& socket() {
        return socket_;
    }

    void start() {
        asio::ip::tcp::no_delay no_delay(true);
        socket_.set_option(no_delay);
        read_head();
    }

    void write() {
        Header* header = (Header*)buffer_;
        uint32_t full_size = header->get_full_size();
        header->inc_packet_count();

        asio::async_write(socket_, asio::buffer(buffer_, full_size),
            [this, self = shared_from_this()](
                const asio::error_code& err, size_t cb) {
            if (!err) {
                if (check_count()) {
                    socket_.close();
                } else {
                    read_head();
                }
            }
        });
    }

    void read_head() {
        asio::async_read(socket_, asio::buffer(buffer_, sizeof(Header)),
            [this, self = shared_from_this()](
                const asio::error_code& err, size_t cb) {
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
            [this, self = shared_from_this()](
                const asio::error_code& err, size_t cb) {
            if (!err) {
                if (check_count()) {
                    socket_.close();
                } else {
                    write();
                }
            }
        });
    }

    bool check_count() {
        Header* header = (Header*)buffer_;
        return (ntohl(header->packet_count_) >= total_count_);
    }
private:
    asio::io_service& io_service_;
    asio::ip::tcp::socket socket_;    
    uint32_t const total_count_;
    static size_t const max_block_size_ = 10000 + sizeof(Header);
    char buffer_[max_block_size_];
};

class server : noncopyable
{
public:
    server(int thread_count, const asio::ip::tcp::endpoint& endpoint,
        uint32_t total_count)
        : thread_count_(thread_count)
        , total_count_(total_count)
        , service_pool_(thread_count)
        , acceptor_(service_pool_.get_io_service())
    {
        acceptor_.open(endpoint.protocol());
        acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(1));
        acceptor_.bind(endpoint);
        acceptor_.listen();
    }

    void start() {
        accept();
    }

    void wait() {
        service_pool_.run();
    }
private:
    void accept() {
        std::shared_ptr<session> new_session(new session(
            service_pool_.get_io_service(), total_count_));
        auto& socket = new_session->socket();
        acceptor_.async_accept(socket,
            [this, new_session = std::move(new_session)](
                const asio::error_code& err) {
            if (!err) {
                new_session->start();
                accept();
            }
        });
    }

private:
    int const thread_count_;
    uint32_t const total_count_;
    io_service_pool service_pool_;
    asio::ip::tcp::acceptor acceptor_;
};

} // namespace 

void server_test3(int thread_count, char const* host, char const* port,
    size_t total_count) {
    auto endpoint = asio::ip::tcp::endpoint(
        asio::ip::address::from_string(host), atoi(port));
    server s(thread_count, endpoint, total_count);
    s.start();
    s.wait();
}