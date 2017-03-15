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

const size_t kMaxStackSize = 10;

class session : public std::enable_shared_from_this<session>, noncopyable
{
public:
    session(asio::io_service& ios, size_t block_size)
        : io_service_(ios)
        , socket_(ios)
        , block_size_(block_size) {
        for (size_t i = 0; i < kMaxStackSize; ++i) {
            buffers_.push(new char[block_size_]);
        }
    }

    ~session() {
        while (!buffers_.empty()) {
            auto buffer = buffers_.top();
            delete[] buffer;
            buffers_.pop();
        }
    }

    asio::ip::tcp::socket& socket() {
        return socket_;
    }

    void start() {
        //asio::ip::tcp::no_delay no_delay(true);
        //socket_.set_option(no_delay);
        read();
    }

    void write(char* buffer, size_t len) {
        asio::async_write(socket_, asio::buffer(buffer, len),
            [this, self = shared_from_this(), buffer](
                const asio::error_code& err, size_t cb) {
            bool need_read = buffers_.empty();
            buffers_.push(buffer);
            if (need_read && !err) {
                read();
            }
            if (err) {
                close();
            }
        });
    }

    void read() {
        if (buffers_.empty()) {
            std::cout << "the cached buffer used out\n";
            return;
        }

        auto buffer = buffers_.top();
        buffers_.pop();

        socket_.async_read_some(asio::buffer(buffer, block_size_),
            [this, self = shared_from_this(), buffer](
                const asio::error_code& err, size_t cb) {
            if (!err) {
                write(buffer, cb);
                read();
            } else {
                close();
            }
        });
    }
private:
    void close() {
        socket_.close();
    }
private:
    asio::io_service& io_service_;
    asio::ip::tcp::socket socket_;
    size_t const block_size_;
    std::stack<char*> buffers_;
};

class server : noncopyable
{
public:
    server(int thread_count, const asio::ip::tcp::endpoint& endpoint,
        size_t block_size)
        : thread_count_(thread_count)
        , block_size_(block_size)
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
            service_pool_.get_io_service(), block_size_));
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
    size_t const block_size_;
    io_service_pool service_pool_;
    asio::ip::tcp::acceptor acceptor_;    
};

} // namespace 

void server_test2(int thread_count, char const* host, char const* port,
    size_t block_size) {
    auto endpoint = asio::ip::tcp::endpoint(
        asio::ip::address::from_string(host), atoi(port));
    server s(thread_count, endpoint, block_size);
    s.start();
    s.wait();
}