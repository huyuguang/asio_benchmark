#pragma once

#include "base.h"

/// A pool of io_service objects.
class io_service_pool : noncopyable {
public:
    /// Construct the io_service pool.
    explicit io_service_pool(std::size_t pool_size);

    /// Run all io_service objects in the pool.
    void run();

    /// Stop all io_service objects in the pool.
    void stop();

    /// Get an io_service to use.
    asio::io_service& get_io_service();

    asio::io_service& get_io_service(size_t index);
private:
    typedef std::shared_ptr<asio::io_service> io_service_ptr;
    typedef std::shared_ptr<asio::io_service::work> work_ptr;

    /// The pool of io_services.
    std::vector<io_service_ptr> io_services_;

    /// The work that keeps the io_services running.
    std::vector<work_ptr> work_;

    /// The next io_service to use for a connection.
    std::size_t next_io_service_;
};
