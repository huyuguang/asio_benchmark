#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32
#include <sys/time.h>
#include <sys/resource.h>
#endif

void client_test1(int thread_count, char const* host, char const* port,
    size_t block_size, size_t session_count, int timeout);

void client_test2(int thread_count, char const* host, char const* port,
    size_t block_size, size_t session_count, int timeout);

void server_test1(int thread_count, char const* host, char const* port,
    size_t block_size);

void server_test2(int thread_count, char const* host, char const* port,
    size_t block_size);

void socket_pair_test(size_t pair_count, size_t active_count,
    size_t write_count);

void client_test3(int thread_count, char const* host, char const* port,
    size_t total_count, size_t session_count);

void server_test3(int thread_count, char const* host, char const* port,
    size_t total_count);

void posttask_test1(int thread_count, uint64_t post_count);

void posttask_test2(int thread_count, uint64_t post_count);

void posttask_test3(uint64_t post_count);

void posttask_test4(uint64_t post_count);

void posttask_test5(uint64_t post_count);

int usage() {
    std::cerr << "Usage: asio_test socketpair <pair_count> <active_count>"
        " <write_count>\n";
    std::cerr << "Usage: asio_test client1 <host> <port> <threads>"
        " <blocksize> <sessions> <time>\n";
    std::cerr << "Usage: asio_test server1 <address> <port> <threads>"
        " <blocksize>\n";
    std::cerr << "Usage: asio_test client2 <host> <port> <threads>"
        " <blocksize> <sessions> <time>\n";
    std::cerr << "Usage: asio_test server2 <address> <port> <threads>"
        " <blocksize>\n";
    std::cerr << "Usage: asio_test client3 <host> <port> <threads>"
        " <totalcount> <sessions>\n";
    std::cerr << "Usage: asio_test server3 <address> <port> <threads>"
        " <totalcount>\n";
    std::cerr << "Usage: asio_test posttask1 <threads> <totalcount>\n";
    std::cerr << "Usage: asio_test posttask2 <threads> <totalcount>\n";
    std::cerr << "Usage: asio_test posttask3 <totalcount>\n";
    std::cerr << "Usage: asio_test posttask4 <totalcount>\n";
    std::cerr << "Usage: asio_test posttask5 <totalcount>\n";
    return 1;
}


#ifdef _WIN32
void change_limit() {
}
#else
void change_limit() {
    rlimit rlim;
    if (getrlimit(RLIMIT_NOFILE, &rlim) < 0) {
        std::cout << "getrlimit failed, error: " << strerror(errno) << "\n";
        return;
    }
    if (rlim.rlim_cur < rlim.rlim_max) {
        auto old_cur = rlim.rlim_cur;
        rlim.rlim_cur = rlim.rlim_max;
        if (setrlimit(RLIMIT_NOFILE, &rlim) < 0) {
            std::cout << "setrlimit failed, error: " << strerror(errno) <<
                " " << std::to_string(old_cur) + " to " <<
                std::to_string(rlim.rlim_max) << "\n";
        } else {
            std::cout << "setrlimit success: " << std::to_string(old_cur) <<
                " to " << std::to_string(rlim.rlim_max) << "\n";
        }
    }
}
#endif

int main(int argc, char* argv[])
{
    change_limit();

    using namespace std; // For atoi.

    if (argc < 2)
        return usage();

    if (!strcmp(argv[1], "socketpair") && argc != 5)
        return usage();
    if (!strcmp(argv[1], "client1") && argc != 8)
        return usage();
    if (!strcmp(argv[1], "server1") && argc != 6)
        return usage();
    if (!strcmp(argv[1], "client2") && argc != 8)
        return usage();
    if (!strcmp(argv[1], "server2") && argc != 6)
        return usage();
    if (!strcmp(argv[1], "client3") && argc != 7)
        return usage();
    if (!strcmp(argv[1], "server3") && argc != 6)
        return usage();
    if (!strcmp(argv[1], "posttask1") && argc != 4)
        return usage();
    if (!strcmp(argv[1], "posttask2") && argc != 4)
        return usage();
    if (!strcmp(argv[1], "posttask3") && argc != 3)
        return usage();
    if (!strcmp(argv[1], "posttask4") && argc != 3)
        return usage();
    if (!strcmp(argv[1], "posttask5") && argc != 3)
        return usage();

    if (!strcmp(argv[1], "socketpair")) {
        size_t pair_count = atoi(argv[2]);
        size_t active_count = atoi(argv[3]);
        size_t write_count = atoi(argv[4]);
        socket_pair_test(pair_count, active_count, write_count);
    } else if (!strcmp(argv[1], "client1")) {
        const char* host = argv[2];
        const char* port = argv[3];
        int thread_count = atoi(argv[4]);
        size_t block_size = atoi(argv[5]);
        size_t session_count = atoi(argv[6]);
        int timeout = atoi(argv[7]);
        client_test1(thread_count, host, port, block_size, session_count, timeout);
    } else if (!strcmp(argv[1], "client2")) {
        const char* host = argv[2];
        const char* port = argv[3];
        int thread_count = atoi(argv[4]);
        size_t block_size = atoi(argv[5]);
        size_t session_count = atoi(argv[6]);
        int timeout = atoi(argv[7]);
        client_test2(thread_count, host, port, block_size, session_count, timeout);
    } else if (!strcmp(argv[1], "server1")) {
        const char* host = argv[2];
        const char* port = argv[3];
        int thread_count = atoi(argv[4]);
        size_t block_size = atoi(argv[5]);
        server_test1(thread_count, host, port, block_size);
    } else if (!strcmp(argv[1], "server2")) {
        const char* host = argv[2];
        const char* port = argv[3];
        int thread_count = atoi(argv[4]);
        size_t block_size = atoi(argv[5]);
        server_test2(thread_count, host, port, block_size);
    } else if (!strcmp(argv[1], "client3")) {
        const char* host = argv[2];
        const char* port = argv[3];
        int thread_count = atoi(argv[4]);
        uint32_t total_count = atoi(argv[5]);
        size_t session_count = atoi(argv[6]);
        client_test3(thread_count, host, port, total_count, session_count);
    } else if (!strcmp(argv[1], "server3")) {
        const char* host = argv[2];
        const char* port = argv[3];
        int thread_count = atoi(argv[4]);
        uint32_t total_count = atoi(argv[5]);
        server_test3(thread_count, host, port, total_count);
    } else if (!strcmp(argv[1], "posttask1")) {
        int thread_count = atoi(argv[2]);
        uint64_t total_count = strtoull(argv[3], nullptr, 10);
        posttask_test1(thread_count, total_count);
    } else if (!strcmp(argv[1], "posttask2")) {
        int thread_count = atoi(argv[2]);
        uint64_t total_count = strtoull(argv[3], nullptr, 10);
        posttask_test2(thread_count, total_count);
    } else if (!strcmp(argv[1], "posttask3")) {
        uint64_t total_count = strtoull(argv[2], nullptr, 10);
        posttask_test3(total_count);
    } else if (!strcmp(argv[1], "posttask4")) {
        uint64_t total_count = strtoull(argv[2], nullptr, 10);
        posttask_test4(total_count);
    } else if (!strcmp(argv[1], "posttask5")) {
        uint64_t total_count = strtoull(argv[2], nullptr, 10);
        posttask_test5(total_count);
    } else {
        return usage();
    }

    return 0;
}