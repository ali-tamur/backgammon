#include "logger.h"

#include "proto/game.pb.h"
#include <cassert>
#include <fstream>
#include <vector>

namespace bg {

LogWriter::~LogWriter() { flush(); }

void LogWriter::start_game(uint64_t game_id, std::string white_name,
                           std::string black_name) {
  if (is_dummy_)
    return;
  Game game;
  game.set_game_id(game_id);
  game.set_white(white_name);
  game.set_black(black_name);
  buffer_.emplace_back(game);
}

void LogWriter::add_move(Dice dice, const Board &board) {
  if (is_dummy_) return;
  assert(buffer_.size() > 0 && "call start_game before add_move");
  auto &game = buffer_[buffer_.size() - 1];
  auto *cells = game.add_moves()->mutable_cell();
    cells->Reserve(28);
    for (int8_t val : board) {
        cells->Add(val);
    }
    game.add_dice(dice.dice_);
}

void LogWriter::end_game(int32_t score) {
  if (is_dummy_)
    return;
  auto &game = buffer_[buffer_.size() - 1];
    game.set_score(score);
    if (buffer_.size() >= 1000) {
        flush();
    }
}

void LogWriter::flush() {
  if (is_dummy_)
        return;
  std::ofstream outs(db_path_, std::ios::binary | std::ios::app);
  for (const auto &game : buffer_) {
        std::string serialized = game.SerializeAsString();
        uint32_t size = serialized.size();
    outs.write(reinterpret_cast<const char *>(&size), sizeof(size));
        outs.write(serialized.data(), size);
      }
      buffer_.clear();
};

} // namespace bg