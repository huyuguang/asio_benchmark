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
        uint32_t* thread_index = new uint32_t(0);
        uint64_t* count = new uint64_t(0);
        start_time_ = clock_us();
        post(thread_index, count);        
    }

    void wait() {
        service_pool_.run();
    }

    uint64_t use_time() const {
        return stop_time_ - start_time_;
    }
private:

    void post(uint32_t* thread_index, uint64_t* count) {
        service_pool_.get_io_service(*thread_index).post(
            [this, thread_index, count]() {
            ++(*count);
            ++(*thread_index);
            if (*count == post_count_) {
                delete thread_index;
                delete count;
                stop();
            } else {
                post(thread_index, count);
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
    uint64_t start_time_;
    uint64_t stop_time_;
};
} // namespace 

void posttask_test1(int thread_count, uint64_t post_count) {
    post_task p(thread_count, post_count);
    p.start();
    p.wait();
    std::cout << "use time(us): " << p.use_time() << "\n";
}