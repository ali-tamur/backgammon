#include "logger.h"

#include "proto/game.pb.h"
#include <cassert>
#include <vector>

namespace bg {

LogWriter::~LogWriter() { flush(); }

// NOLINTNEXTLINE(readability-make-member-function-const)
void LogWriter::start_game(uint64_t game_id, std::string white_name,
                           std::string black_name) {
  if (is_dummy_) {
    return;
  }
  Game game;
  game.set_game_id(game_id);
  game.set_white(white_name);
  game.set_black(black_name);
  buffer_.emplace_back(game);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void LogWriter::add_move(Dice dice, const Board &board) {
  if (is_dummy_) {
    return;
  }
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
  if (is_dummy_) {
    return;
  }
  auto &game = buffer_[buffer_.size() - 1];
  game.set_score(score);
  if (buffer_.size() >= 1000) {
    flush();
  }
}

void LogWriter::flush() {
  if (is_dummy_) {
    return;
  }

  for (const auto &game : buffer_) {
    std::string serialized = game.SerializeAsString();
    uint32_t size = serialized.size();
    out_.write(reinterpret_cast<const char *>(&size), sizeof(size));
    out_.write(serialized.data(), size);
  }
  out_.flush();
  buffer_.clear();
}

auto LogReader::read_all() -> std::vector<Game> {
  std::vector<Game> games;

  while (in_) {
    uint32_t size;
    in_.read(reinterpret_cast<char *>(&size), sizeof(size));
    if (!in_ || size == 0) {
      break;
    }

    std::string data(size, '\0');
    in_.read(data.data(), size);
    if (!in_) {
      break;
    }

    Game game;
    if (!game.ParseFromString(data)) {
      std::cerr << "Failed to parse game\n";
      continue;
    }
    games.push_back(game);
  }

  return games;
}

} // namespace bg