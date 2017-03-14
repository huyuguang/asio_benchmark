#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void client_test1(int thread_count, char const* host, char const* port,
    size_t block_size, size_t session_count, int timeout);

void client_test2(int thread_count, char const* host, char const* port,
    size_t block_size, size_t session_count, int timeout);

void server_test1(int thread_count, char const* host, char const* port,
    size_t block_size);

void server_test2(int thread_count, char const* host, char const* port,
    size_t block_size);

int usage() {
    std::cerr << "Usage: asio_test client <host> <port> <threads>"
        " <blocksize> <sessions> <time> <type>[1/2]\n";
    std::cerr << "Usage: asio_test server <address> <port> <threads>"
        " <blocksize> <type>[1/2]\n";
    std::cerr << "type1: single buffer\n";
    std::cerr << "type2: multiple buffers\n";

    return 1;
}

int main(int argc, char* argv[])
{
    if (argc != 9 && argc != 7) {
        return usage();
    }

    if (argc == 9 && strcmp(argv[1], "client")) {
        return usage();
    }

    if (argc == 7 && strcmp(argv[1], "server")) {
        return usage();
    }

    if (argc == 9) {
        using namespace std; // For atoi.
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
        using namespace std; // For atoi.
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