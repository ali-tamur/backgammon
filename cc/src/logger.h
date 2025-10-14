#pragma once

#include <filesystem>
#include <iostream>
#include <utility>
#include <optional>
#include <fstream>

#include "configs.h"
#include "dice.h"
#include "proto/game.pb.h"

namespace bg {

class LogWriter {
public:
    LogWriter(LogWriter&&) = default;
    auto operator=(LogWriter&&) -> LogWriter& = default;
    LogWriter(const LogWriter&) = delete;
    auto operator=(const LogWriter&) -> LogWriter& = delete;
    LogWriter() = default;

  explicit LogWriter(
    const std::filesystem::path& path, 
    std::string white_name, 
    std::string black_name, 
    bool is_dummy = false) : 
      white_name_(std::move(white_name)),
      black_name_(std::move(black_name)),
      is_dummy_(is_dummy) {
      if (!is_dummy_) {
        file_stream_.emplace(path, std::ios::binary | std::ios::app);
        out_ = &file_stream_.value();
      }
  }
  
  explicit LogWriter(std::ostream* out, 
    std::string white_name, 
    std::string black_name, 
    bool is_dummy = false) : out_(out), 
    white_name_(std::move(white_name)),
    black_name_(std::move(black_name)),
    is_dummy_{is_dummy} {}
  ~LogWriter();

  void start_game(uint64_t game_id);
  void add_move(Dice dice, const Board &b);
  void end_game(int32_t score);
  void flush();

private:
  std::optional<std::ofstream> file_stream_;
  std::ostream* out_;
  std::string white_name_;
  std::string black_name_;
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

} // namespace bg
