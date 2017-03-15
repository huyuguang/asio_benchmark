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

int usage() {
    std::cerr << "Usage: asio_test socketpair <pair_count> <active_count>"
        " <write_count>\n";
    std::cerr << "Usage: asio_test client <host> <port> <threads>"
        " <blocksize> <sessions> <time> <type>[1/2]\n";
    std::cerr << "Usage: asio_test server <address> <port> <threads>"
        " <blocksize> <type>[1/2]\n";
    std::cerr << "type1: single buffer\n";
    std::cerr << "type2: multiple buffers\n";

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

    if (argc != 5 && argc != 9 && argc != 7) {
        return usage();
    }

    if (argc == 5 && strcmp(argv[1], "socketpair")) {
        return usage();
    }

    if (argc == 9 && strcmp(argv[1], "client")) {
        return usage();
    }

    if (argc == 7 && strcmp(argv[1], "server")) {
        return usage();
    }

    if (argc == 5) {
        size_t pair_count = atoi(argv[2]);
        size_t active_count = atoi(argv[3]);
        size_t write_count = atoi(argv[4]);
        socket_pair_test(pair_count, active_count, write_count);
    } else if (argc == 9) {        
        const char* host = argv[2];
        const char* port = argv[3];
        int thread_count = atoi(argv[4]);
        size_t block_size = atoi(argv[5]);
        size_t session_count = atoi(argv[6]);
        int timeout = atoi(argv[7]);
        int type = atoi(argv[8]);
        if (type == 1) {
            client_test1(thread_count, host, port, block_size, session_count, timeout);
        } else if (type == 2) {
            client_test2(thread_count, host, port, block_size, session_count, timeout);
        }
    } else {
        const char* host = argv[2];
        const char* port = argv[3];
        int thread_count = atoi(argv[4]);
        size_t block_size = atoi(argv[5]);
        int type = atoi(argv[6]);
        if (type == 1) {
            server_test1(thread_count, host, port, block_size);
        } else if (type == 2) {
            server_test2(thread_count, host, port, block_size);
        }
    }

    return 0;
}