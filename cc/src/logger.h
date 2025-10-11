#pragma once

#include "configs.h"
#include "dice.h"
#include "proto/game.pb.h"
#include <filesystem>

namespace bg {

class LogWriter {
public:
  LogWriter(std::filesystem::path db_path, bool is_dummy = false)
      : db_path_(db_path), is_dummy_{is_dummy} {}
  ~LogWriter();

  void start_game(uint64_t game_id, std::string white_name,
                  std::string black_name);
  void add_move(Dice dice, const Board &b);
  void end_game(int32_t score);
  void flush();

private:
  std::filesystem::path db_path_;
  bool is_dummy_;
  std::vector<Game> buffer_{};
};

class LogReader {
public:
  LogReader(const std::string &db_path) : db_path_(db_path) {}

private:
  std::string db_path_;
};

} // namespace bg