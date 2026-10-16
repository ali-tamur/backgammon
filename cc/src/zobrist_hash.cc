#include "zobrist_hash.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <random>

namespace bg {

// Fixed seed: the hash decides the order in which candidate moves come out of
// the unordered_set, so a per-process seed would make seeded games differ
// between runs. 32-bit values on purpose: the unordered_set reduces the hash
// modulo its bucket count, and a hash that fits in 32 bits takes the faster
// 32-bit division (measured: 64-bit values made random games ~8% slower).
auto get_zobrist_table() -> ZobristTableType {
  static constexpr uint64_t ZOBRIST_SEED = 0x5EED'BAC6'A770'0001ULL;
  ZobristTableType table{};
  std::mt19937_64 generator(ZOBRIST_SEED);
  for (auto& entry : table) {
    entry = static_cast<uint32_t>(generator());
  }
  return table;
}

// A global, not a function-local static: it is read on every hash update.
const ZobristTableType ZOBRIST_TABLE = get_zobrist_table();

}  // namespace bg
