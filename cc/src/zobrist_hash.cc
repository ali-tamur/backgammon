#include "zobrist_hash.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <random>

namespace bg {

auto get_zobrist_table() -> ZobristTableType {
  ZobristTableType table{};
  std::random_device rd{};
  std::mt19937_64 generator(rd());
  std::uniform_int_distribution<uint32_t> dist;
  for (auto& entry : table) {
    entry = dist(generator);
  }
  return table;
}

// A global, not a function-local static: it is read on every hash update. If
// std::random_device throws at startup, terminating is the right outcome.
// NOLINTNEXTLINE(bugprone-throwing-static-initialization)
const ZobristTableType ZOBRIST_TABLE = get_zobrist_table();

}  // namespace bg
