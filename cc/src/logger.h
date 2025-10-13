#pragma once

#include "configs.h"
#include "dice.h"
#include "proto/game.pb.h"
#include <filesystem>
#include <utility>

namespace bg {

class LogWriter {
public:
  explicit LogWriter(std::ostream &out, bool is_dummy = false)
      : out_(out), is_dummy_{is_dummy} {}
  ~LogWriter();

  void start_game(uint64_t game_id, std::string white_name,
                  std::string black_name);
  void add_move(Dice dice, const Board &b);
  void end_game(int32_t score);
  void flush();

private:
  std::ostream &out_;
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
