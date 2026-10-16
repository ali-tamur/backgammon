#pragma once

// A minimal writer for NumPy's .npy format (version 1.0), so the training data
// loads in Python with np.load and no parsing. Format:
// https://numpy.org/doc/stable/reference/generated/numpy.lib.format.html

#include <bit>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <vector>

#include "assertion.h"

namespace bg {

static_assert(std::endian::native == std::endian::little,
              "the .npy descriptors below are little-endian");

template <typename T>
constexpr auto npy_descr() -> const char*;
template <>
constexpr auto npy_descr<int8_t>() -> const char* {
  return "|i1";
}
template <>
constexpr auto npy_descr<uint8_t>() -> const char* {
  return "|u1";
}
template <>
constexpr auto npy_descr<int16_t>() -> const char* {
  return "<i2";
}
template <>
constexpr auto npy_descr<int32_t>() -> const char* {
  return "<i4";
}
template <>
constexpr auto npy_descr<float>() -> const char* {
  return "<f4";
}

// The header for a C-order array of T with the given shape.
template <typename T>
auto npy_header(const std::vector<size_t>& shape) -> std::string {
  static constexpr size_t ALIGNMENT = 64;
  static constexpr size_t PREAMBLE = 10;  // magic (6) + version (2) + header length (2)
  std::string dims;
  for (size_t d : shape) {
    dims += std::to_string(d) + ", ";
  }
  if (shape.size() > 1) {
    dims.resize(dims.size() - 2);  // "(3, 28)" but "(3,)"
  } else {
    dims.resize(dims.size() - 1);
  }
  std::string dict = std::string("{'descr': '") + npy_descr<T>() +
                     "', 'fortran_order': False, 'shape': (" + dims + "), }";
  size_t total = PREAMBLE + dict.size() + 1;  // + '\n'
  dict.append((ALIGNMENT - (total % ALIGNMENT)) % ALIGNMENT, ' ');
  dict += '\n';
  std::string header = "\x93NUMPY";
  header += '\x01';
  header += '\x00';
  header += static_cast<char>(dict.size() & 0xFFU);
  header += static_cast<char>(dict.size() >> 8U);
  return header + dict;
}

template <typename T>
void write_npy(const std::filesystem::path& path, std::span<const T> data,
               const std::vector<size_t>& shape) {
  size_t count = 1;
  for (size_t d : shape) {
    count *= d;
  }
  ASSERT(count == data.size(), "shape does not match the data");
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  ASSERT(out.is_open(), "cannot open the .npy file for writing");
  std::string header = npy_header<T>(shape);
  out.write(header.data(), static_cast<std::streamsize>(header.size()));
  out.write(reinterpret_cast<const char*>(data.data()),
            static_cast<std::streamsize>(data.size_bytes()));
  ASSERT(out.good(), "failed writing the .npy file");
}

}  // namespace bg
