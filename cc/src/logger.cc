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

void LogWriter::start_game(bool white_first) {
  if (!writer_) {
    return;
  }
  game_in_construction_.Clear();
  game_in_construction_.set_white_first(white_first);
  set_position(game_in_construction_.mutable_start(), GAME_START);
  in_game_ = true;
}

void LogWriter::add_move(Dice dice, const Board& board) {
  if (!writer_) {
    return;
  }
  assert(in_game_ && "call start_game before add_move");
  Turn* turn = game_in_construction_.add_turns();
  turn->set_dice(dice.encode());
  set_position(turn->mutable_after(), board);
}

void LogWriter::set_position(Position* position, const Board& board) {
  auto* cells = position->mutable_cell();
  cells->Reserve(BOARD_SIZE);
  for (int8_t val : board) {
    cells->Add(val);
  }
}

void LogWriter::end_game(int32_t score) {
  if (!writer_) {
    return;
  }
  game_in_construction_.set_final_score(score);
  writer_->add(std::move(game_in_construction_));
  in_game_ = false;
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