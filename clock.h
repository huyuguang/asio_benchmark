#pragma once


#if defined(ASIO_WINDOWS)

inline uint64_t high_res_clock()
{
    LARGE_INTEGER i;
    QueryPerformanceCounter(&i);
    return i.QuadPart;
}

#elif defined(__GNUC__) && defined(__x86_64__)

inline uint64_t high_res_clock()
{
    unsigned long low, high;
    __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high));
    return (((uint64_t)high) << 32) | low;
}

#else

#include <chrono>

inline uint64_t high_res_clock()
{
    return std::chrono::steady_clock::now().time_since_epoch().count();
}

#endif

