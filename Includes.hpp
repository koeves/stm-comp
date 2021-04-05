#ifndef INCLUDES_HPP
#define INCLUDES_HPP

#include <iostream>
#include <atomic>
#include <unordered_map>
#include <vector>
#include <utility>
#include <cstdint>
#include <setjmp.h>
#include "ThreadStream.hpp"

#define DEBUG 0

#if DEBUG

#include <syncstream>
#define TRACE std::osyncstream(std::cout)

#endif

#endif