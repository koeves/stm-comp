#ifndef UTIL_HPP
#define UTIL_HPP

#include <iostream>
#include <fstream>
#include <atomic>
#include <unordered_map>
#include <map>
#include <vector>
#include <unordered_set>
#include <utility>
#include <string>
#include <cstdint>
#include <cstring>
#include <setjmp.h>
#include <any>
#include <assert.h>
#include <chrono>
#include <thread>
#include <random>
#include "ThreadStream.hpp"

#define DEBUG 0

#if defined(DEBUG) && DEBUG > 0
    #define TRACE(_x)   do { TOUT << _x << std::endl; } while(0)
#else
    #define TRACE(_x)   do {} while(0)
#endif

#if __GNUC__ > 9
    #define ATOMIC_LOAD(_T, _x) std::atomic_ref<_T>(*_x).load(std::memory_order_acquire)
    #define ATOMIC_STORE(_T, _x, _y) std::atomic_ref<_T>(*_x).store(_y, std::memory_order_release)  
#else
    #define ATOMIC_LOAD(_T, _x) AtomicRef<_T>(_x).load()
    #define ATOMIC_STORE(_T, _x, _y) AtomicRef<_T>(_x).store(_y)
#endif

#endif