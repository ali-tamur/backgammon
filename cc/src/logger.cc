#include "logger.h"

#include <cassert>
#include <vector>

#include "configs.h"
#include "proto/game.pb.h"

namespace bg {
LogWriter::LogWriter(const std::filesystem::path &path, size_t buffer_size, bool is_dummy)
    : is_dummy_(is_dummy) {
  if (!is_dummy_) {
    writer_ = ProtoWriter<Game>(path, buffer_size);
  }
}

LogWriter::LogWriter(std::ostream *out, size_t buffer_size, bool is_dummy) : is_dummy_{is_dummy} {
  if (!is_dummy_) {
    writer_ = ProtoWriter<Game>(out, buffer_size);
  }
}

LogWriter::~LogWriter() { flush(); }

// NOLINTNEXTLINE(readability-make-member-function-const)
void LogWriter::start_game(uint64_t game_id) {
  if (is_dummy_) {
    return;
  }
  game_in_construction_.Clear();
  game_in_construction_.set_game_id(game_id);
}

// NOLINTNEXTLINE(readability-make-member-function-const)
void LogWriter::add_move(Dice dice, const Board &board) {
  if (is_dummy_) {
    return;
  }
  assert(game_in_construction_.game_id() > 0 && "call start_game before add_move");
  auto *cells = game_in_construction_.add_positions()->mutable_cell();
  cells->Reserve(BOARD_SIZE);
  for (int8_t val : board) {
    cells->Add(val);
  }
  game_in_construction_.add_dice(dice.dice_);
}

void LogWriter::end_game(int32_t score) {
  if (is_dummy_) {
    return;
  }
  game_in_construction_.set_final_score(score);
}

void LogWriter::flush() {
  if (is_dummy_) {
    return;
  }
  writer_.flush();
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

}  // namespace bg