#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <utility>

#include "configs.h"
#include "dice.h"
#include "proto/game.pb.h"

namespace bg {

class LogWriter {
 public:
  LogWriter(LogWriter &&) = default;
  auto operator=(LogWriter &&) -> LogWriter & = default;
  LogWriter() = default;

  LogWriter(const std::filesystem::path &path, bool is_dummy = false) : is_dummy_(is_dummy) {
    if (!is_dummy_) {
      file_stream_.emplace(path, std::ios::binary | std::ios::app);
      out_ = &file_stream_.value();
    }
  }

  LogWriter(std::ostream *out, bool is_dummy = false) : out_(out), is_dummy_{is_dummy} {}
  ~LogWriter();

  void start_game(uint64_t game_id);
  void add_move(Dice dice, const Board &b);
  void end_game(int32_t score);
  void flush();

 private:
  std::optional<std::ofstream> file_stream_;
  std::ostream *out_;
  bool is_dummy_;
  std::vector<Game> buffer_;
};

class LogReader {
 public:
  explicit LogReader(std::istream &in) : in_(in) {}
  auto read_all() -> std::vector<Game>;

 private:
  std::istream &in_;
};

}  // namespace bg
