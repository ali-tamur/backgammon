#pragma once

#include <cstdlib>
#include <format>
#include <iostream>
#include <string_view>

[[noreturn]] inline void assert_fail(const char* file, int line, const char* expr,
                                     const char* msg) {
  std::cerr << "ASSERTION FAILED: " << file << ":" << line << "\n";
  std::cerr << expr << " : " << msg << "\n";
  std::abort();
}

#define PING() std::cout << "# " << __FILE__ << " : " << __LINE__ << "\n";

#define FAIL(expr, msg) assert_fail(__FILE__, __LINE__, #expr, msg);

#define ASSERT(expr, msg) \
  if (!(expr)) {          \
    FAIL(expr, msg)       \
  }
