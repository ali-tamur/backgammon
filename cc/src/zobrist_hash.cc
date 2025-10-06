#include "zobrist_hash.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <random>

namespace bg {
const ZobristTableType &get_zobrist_table() {
  static const ZobristTableType INSTANCE = [] {
    size_t table_size = 32;
    ZobristTableType table{};
    std::random_device rd{};
    std::mt19937_64 generator(rd());
    std::uniform_int_distribution<uint64_t> dist;
    for (size_t i = 0; i < table_size; ++i) {
      table[i] = dist(generator);
    }
    return table;
  }();
  return INSTANCE;
}

}  // namespace bg