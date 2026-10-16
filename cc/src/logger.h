#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <utility>

#include "assertion.h"
#include "configs.h"
#include "dice.h"
#include "proto/game.pb.h"

namespace bg {

template <typename P>
class ProtoWriter {
 public:
  ProtoWriter(std::filesystem::path path, size_t buffer_size)
      : path_(std::move(path)), buffer_size_(buffer_size) {}

  ~ProtoWriter() { flush(); }

  void flush() {
    std::ofstream out{path_, std::ios::binary | std::ios::app};
    for (const auto& item : buffer_) {
      std::string serialized = item.SerializeAsString();
      uint32_t size = serialized.size();
      out.write(reinterpret_cast<const char*>(&size), sizeof(size));
      out.write(serialized.data(), size);
    }
    out.flush();
    buffer_.clear();
  }

  void add(P&& item) {
    buffer_.emplace_back(item);
    if (buffer_.size() >= buffer_size_) [[unlikely]] {
      flush();
    }
  }

 private:
  std::filesystem::path path_;
  size_t buffer_size_;
  std::vector<P> buffer_;
};

class LogWriter {
 public:
  LogWriter() = default;
  LogWriter(const std::filesystem::path& path, size_t buffer_size);

  // Records GAME_START as the game's start position.
  void start_game(bool white_first);
  void add_move(Dice dice, const Board& b);
  void end_game(int32_t score);
  void flush();

 private:
  std::optional<ProtoWriter<Game>> writer_;
  Game game_in_construction_;
  bool in_game_ = false;

  static void set_position(Position* position, const Board& board);
};

class LogReader {
 public:
  explicit LogReader(std::istream& in) : in_(in) {}
  // Reads the next game; false at the end of the input. Aborts on a truncated
  // or corrupt record.
  auto next(Game* game) -> bool;
  auto read_all() -> std::vector<Game>;

 private:
  std::istream& in_;
};

}  // namespace bg
