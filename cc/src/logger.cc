#include "logger.h"

#include <cassert>
#include <vector>

#include "assertion.h"
#include "configs.h"
#include "proto/game.pb.h"

namespace bg {

LogWriter::LogWriter(const std::filesystem::path& path, size_t buffer_size) {
  writer_.emplace(path, buffer_size);
}

void LogWriter::start_game(uint64_t game_id, bool is_white_the_first_player) {
  if (!writer_) {
    return;
  }
  game_in_construction_.Clear();
  game_in_construction_.set_game_id(game_id);
  if (is_white_the_first_player) {
    game_in_construction_.set_player(::bg::FirstPlayer::WHITE);
  } else {
    game_in_construction_.set_player(::bg::FirstPlayer::BLACK);
  }
}

void LogWriter::add_move(Dice dice, const Board& board) {
  if (!writer_) {
    return;
  }
  assert(game_in_construction_.game_id() > 0 && "call start_game before add_move");
  auto* cells = game_in_construction_.add_positions()->mutable_cell();
  cells->Reserve(BOARD_SIZE);
  for (int8_t val : board) {
    cells->Add(val);
  }
  game_in_construction_.add_dice(dice.dice_);
}

void LogWriter::end_game(int32_t score) {
  if (!writer_) {
    return;
  }
  game_in_construction_.set_final_score(score);
  writer_->add(std::move(game_in_construction_));
}

void LogWriter::flush() {
  if (!writer_) {
    return;
  }
  writer_->flush();
}

auto LogReader::read_all() -> std::vector<Game> {
  std::vector<Game> games;

  while (in_) {
    uint32_t size;
    in_.read(reinterpret_cast<char*>(&size), sizeof(size));
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

}  // namespace bg