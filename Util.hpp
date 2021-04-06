#ifndef UTIL_HPP
#define UTIL_HPP

#include <iostream>
#include <atomic>
#include <unordered_map>
#include <map>
#include <vector>
#include <set>
#include <utility>
#include <string>
#include <cstdint>
#include <setjmp.h>
#include <any>
#include <assert.h>
#include "ThreadStream.hpp"

#define DEBUG 1

#ifdef DEBUG
#define TRACE(_x)   do {                                        \
                        TOUT << _x << std::endl; \
                    } while(0)
#else
#define TRACE(_x) (void)(_x)
#endif

#endif