#include "game_manager.h"

#include <iostream>
#include <limits>
#include <tuple>

#include "assertion.h"
#include "configs.h"
#include "model.h"
#include "player.h"
#include "pretty_print.h"
#include "state.h"

namespace bg {

auto play_turn(const MOVE& player, const Board& board, Dice dice, bool white_moves)
    -> std::pair<Board, int32_t> {
  if (white_moves) {
    Board after = player(board, dice);
    return {after, score(after)};
  }
  Board after = player(flip(board), dice);
  return {flip(after), -score(after)};
}

// Seeds the game's generator from the match seed and the game's index, so any
// game can be replayed alone. std::seed_seq's algorithm is fixed by the standard.
static void seed_game(std::mt19937& rng, uint64_t seed, int32_t game_index) {
  static constexpr uint32_t LOW_32_BITS = 0xFFFF'FFFFU;
  static constexpr uint32_t SHIFT_32 = 32;
  std::seed_seq seq{static_cast<uint32_t>(seed & LOW_32_BITS),
                    static_cast<uint32_t>(seed >> SHIFT_32), static_cast<uint32_t>(game_index)};
  rng.seed(seq);
}

auto GameManager::run_once(LogWriter* logger, int32_t game_index) -> int32_t {
  seed_game(*rng_, seed_, game_index);
  std::array<MOVE, 2> players{white_, black_};
  int32_t this_score;
  int32_t on_turn = 0;
  if (first_move_ == FirstMove::Always_Black ||
      (first_move_ == FirstMove::Alternate && (game_index % 2 == 1))) {
    on_turn = 1;
  }
  logger->start_game(on_turn == 0);

  Board board = GAME_START;

  while (true) {
    Dice dice = dice_throw_.throw_dice();
    if (is_interactive_) {
      std::cout << print_board(board);
      auto dice_values = dice.values(false);
      for (auto d = dice_values.begin(); d != bg::DiceRolls::end(); ++d) {
        std::cout << std::format("{}", *d) << " ";
      }
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    std::tie(board, this_score) = play_turn(players[on_turn], board, dice, on_turn == 0);
    logger->add_move(dice, board);

    if (this_score != 0) {
      break;
    }
    on_turn = 1 - on_turn;
  }
  logger->end_game(this_score);
  return this_score;
}

auto GameManager::run() -> std::tuple<int32_t, int32_t> {
  int32_t white_score = 0;
  int32_t black_score = 0;
  for (int32_t i = 0; i < n_games_; ++i) {
    int32_t this_score = run_once(&logger_, i);
    if (print_score_) {
      std::cout << i << "th game: " << this_score << "\n";
    }
    if (this_score > 0) {
      white_score += this_score;
    } else {
      black_score -= this_score;
    }
  }
  return std::make_tuple(white_score, black_score);
}

auto game_manager_factory(const GameOptions& options) -> GameManager {
  static constexpr size_t BUFFER_SIZE = 50000;
  auto rng = std::make_shared<std::mt19937>();
  uint64_t seed =
      options.seed.value_or((uint64_t{std::random_device{}()} << 32U) | std::random_device{}());
  MOVE white_move;
  if (options.white_model == "random") {
    white_move = construct_random_player_move(rng);
  } else {
    auto white_path =
        (std::filesystem::path(PROJECT_ROOT) / "models" / options.white_model).string();
    white_move = construct_nn_player_move(rng, white_path, options.white_temperature);
  }
  MOVE black_move;
  if (options.black_model == "random") {
    black_move = construct_random_player_move(rng);
  } else {
    auto black_path =
        (std::filesystem::path(PROJECT_ROOT) / "models" / options.black_model).string();
    black_move = construct_nn_player_move(rng, black_path, options.black_temperature);
  }
  LogWriter logger;
  if (!options.log_path.empty() && !options.is_unittest) {
    auto logger_path = (std::filesystem::path(PROJECT_ROOT) / "games" / options.log_path).string();
    logger = LogWriter(logger_path, BUFFER_SIZE);
  } else if (options.is_unittest) {
    logger = LogWriter(options.log_path, BUFFER_SIZE);
  }
  return {white_move,
          black_move,
          rng,
          seed,
          logger,
          options.n_games,
          options.first_move,
          options.is_interactive,
          options.print_score};
}

}  // namespace bg
