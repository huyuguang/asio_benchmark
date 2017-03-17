#include <iostream>
#include <algorithm>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <functional>
#include "io_pool.h"


namespace {

typedef std::function<void()> Task;

uint64_t clock_us() {
    return std::chrono::steady_clock::now().time_since_epoch().count() / 1000;
}

class post_task : noncopyable {
public:
    post_task(uint64_t post_count)
        : post_count_(post_count) {
        io_work_.reset(new asio::io_service::work(io_service_));
        thread_ = std::thread([this]() {
            io_service_.run();
        });
        pending_tasks_.reserve((uint32_t)post_count);
        temp_tasks_.reserve((uint32_t)post_count);
    }

    void start() {
        start_time_ = clock_us();
        for (size_t i = 0; i < post_count_; ++i) {
            post();
        }
    }

    void wait() {
        io_work_.reset();
        thread_.join();
    }

    uint64_t use_time() const {
        return stop_time_ - start_time_;
    }
private:
    void post() {
        bool need_post = false;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            need_post = pending_tasks_.empty();
            pending_tasks_.emplace_back([this]() {
                count_ += 1;
            });
        }

        if (need_post) {
            io_service_.post([this]() {
                temp_tasks_.clear();

                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    temp_tasks_.swap(pending_tasks_);
                }

                for (auto& task :  temp_tasks_) {
                    task();
                }

                if (count_ == post_count_) {
                    stop_time_ = clock_us();
                }
            });
        }
    }
private:
    uint64_t const post_count_;
    asio::io_service io_service_;
    std::unique_ptr<asio::io_service::work> io_work_;
    std::thread thread_;
    uint64_t count_{ 0 };
    uint64_t start_time_;
    uint64_t stop_time_;
    std::mutex mutex_;
    std::vector<Task> pending_tasks_;
    std::vector<Task> temp_tasks_;
};
} // namespace 

void posttask_test5(uint64_t post_count) {
    post_task p(post_count);
    p.start();
    p.wait();
    std::cout << "use time(us): " << p.use_time() << "\n";
}