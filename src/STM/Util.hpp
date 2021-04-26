#ifndef UTIL_HPP
#define UTIL_HPP

#include <iostream>
#include <atomic>
#include <unordered_map>
#include <map>
#include <vector>
#include <unordered_set>
#include <utility>
#include <string>
#include <cstdint>
#include <setjmp.h>
#include <any>
#include <assert.h>
#include "ThreadStream.hpp"

#define DEBUG 0

#if defined(DEBUG) && DEBUG > 0
#define TRACE(_x)   do { TOUT << _x << std::endl; } while(0)
#else
#define TRACE(_x)   do {} while(0)
#endif

#endif