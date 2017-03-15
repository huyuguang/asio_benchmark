#include <iostream>
#include <algorithm>
#include <memory>
#include <atomic>
#include "io_pool.h"

namespace {

uint64_t clock_us() {
    return std::chrono::steady_clock::now().time_since_epoch().count() / 1000;
}

class post_task : noncopyable {
public:
    post_task(int thread_count, uint64_t post_count)
        : thread_count_(thread_count)
        , post_count_(post_count)
        , service_pool_(thread_count) {
    }

    void start() {
        start_time_ = clock_us();

        for (int i = 0; i < thread_count_ / 2; ++i) {
            post(i*2);
        }
        
    }

    void wait() {
        service_pool_.run();
    }

    uint64_t use_time() const {
        return stop_time_ - start_time_;
    }
private:

    void post(int thread_index) {
        service_pool_.get_io_service(thread_index).post(
            [this, thread_index]() mutable {
            ++count_;
            if (count_ >= post_count_) {
                stop();
            } else {
                if (thread_index % 2) {
                    thread_index -= 1;
                } else {
                    thread_index += 1;
                }

                post(thread_index);
            }
        });
    }

    void stop() {
        stop_time_ = clock_us();
        service_pool_.stop();
    }
private:
    int const thread_count_;
    uint64_t const post_count_;
    io_service_pool service_pool_;
    std::atomic<uint64_t> count_{ 0 };
    uint64_t start_time_;
    uint64_t stop_time_;
};
} // namespace 

void posttask_test2(int thread_count, uint64_t post_count) {
    post_task p(thread_count, post_count);
    if (thread_count < 2 || thread_count % 2)
        throw std::runtime_error("thread_count must >2 && %2 == 0");
    p.start();
    p.wait();
    std::cout << "use time(us): " << p.use_time() << "\n";
}