
#include <iostream>
#include "base.h"
#include "clock.h"

#ifdef ASIO_HAS_LOCAL_SOCKETS
using asio::local::stream_protocol;

namespace {

uint64_t clock_us() {
    return std::chrono::steady_clock::now().time_since_epoch().count()/1000;
}

struct pair : noncopyable {
    pair(asio::io_service& io_service) : s1_(io_service), s2_(io_service) {
        asio::local::connect_pair(s1_, s2_);
    }
    stream_protocol::socket s1_;
    stream_protocol::socket s2_;
    char data_ = '\0';
};

typedef std::unique_ptr<pair> pair_ptr;

class pairs : noncopyable {
public:
    pairs(asio::io_service& io_service, size_t pair_count, size_t active_count,
        size_t write_count)
        : io_service_(io_service)
        , pair_count_(pair_count)
        , active_count_(active_count)
        , write_count_(write_count) {
        ptrs_.resize(pair_count_);
        for (size_t i = 0; i < pair_count_; ++i) {
            ptrs_[i].reset(new pair(io_service_));
            pair_read(i);
        }
    }

    void start() {
        start_time_ = clock_us();// high_res_clock();
        size_t space = pair_count_ / active_count_;
        for (size_t i = 0; i < active_count_; ++i) {
            size_t index = i;// i * space;
            pair_write(index, &first_ch_);
        }
    }

    uint64_t use_time() {
        return stop_time_ - start_time_;
    }
private:
    void pair_read(size_t i) {
        auto ptr = ptrs_[i].get();
        asio::async_read(ptr->s2_, asio::buffer(&ptr->data_, 1),
            [this, ptr, i](const asio::error_code& err, size_t cb) {
            if (!err) {
                ++real_read_count_;
                pair_read(i);

                if (real_read_count_ < write_count_) {
                    size_t next_i = i + 1;
                    if (next_i >= pair_count_)
                        next_i -= pair_count_;
                    pair_write(next_i, &ptr->data_);                    
                } else if (real_read_count_ == real_write_count_) {
                    stop();
                }
            }
        });
    }

    void pair_write(size_t i, char const* data) {
        auto ptr = ptrs_[i].get();
        //asio::async_write(ptr->s1_, asio::buffer(data, 1),
        //    [this](const asio::error_code& err, size_t cb) {
        //    if (!err) {
        //        ++real_write_count_;
        //    }
        //});

        asio::write(ptr->s1_, asio::buffer(data, 1));
        ++real_write_count_;
    }

    void stop() {
        stop_time_ = clock_us();// high_res_clock();
        //std::cout << "real read count: " << real_read_count_ << "\n";
        //std::cout << "real write count: " << real_write_count_ << "\n";
        std::cout << "time: " << stop_time_ - start_time_ << "\n";
        for (auto& i : ptrs_) {
            i->s1_.close();
            i->s2_.close();
        }
    }
private:
    asio::io_service& io_service_;
    size_t const pair_count_;
    size_t const active_count_;
    size_t const write_count_;
    std::vector<pair_ptr> ptrs_;
    size_t real_read_count_ = 0;
    size_t real_write_count_ = 0;
    char first_ch_ = 'm';
    uint64_t start_time_ = 0;
    uint64_t stop_time_ = 0;
};

} // namespace

void socket_pair_test(size_t pair_count, size_t active_count,
    size_t write_count) {
    size_t test_times = 25;
    uint64_t total_time = 0;
    for (size_t i = 0; i < test_times; ++i) {
        asio::io_service io_service;
        pairs p(io_service, pair_count, active_count, write_count);
        p.start();
        io_service.run();
        if (i) { // ignore the first time
            total_time += p.use_time();
        }
    }
    uint64_t average_use_time = total_time / (test_times - 1);
    std::cout << "average time: " << average_use_time << "us\n";
}
#else // #ifdef ASIO_HAS_LOCAL_SOCKETS
void socket_pair_test(size_t pair_count, size_t active_count,
    size_t write_count) {
    std::cout << "windows does not support socket pair\n";
}
#endif // #ifdef ASIO_HAS_LOCAL_SOCKETS