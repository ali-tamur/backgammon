#include "training_data.h"

#include <fstream>

#include "assertion.h"
#include "logger.h"
#include "model.h"

namespace bg {
auto game_to_training_data(const Game& game, const std::string& model_path /*currently ignored*/)
    -> std::vector<TrainingData> {
  int32_t number_of_moves = game.positions_size();
  std::vector<TrainingData> result;
  for (int32_t i = game.player() == FirstPlayer::WHITE ? 1 : 2; i < number_of_moves; i += 2) {
    TrainingData data{};
    auto input_vector = position_to_model_input(game.positions(i));
    data.mutable_input()->mutable_cell()->Assign(input_vector.begin(), input_vector.end());
    data.set_number_of_moves(number_of_moves);
    data.set_move_number(i);
    switch (game.final_score()) {
      case 1:
        data.mutable_score()->set_white_win(1.0);
        break;
      case 2:
      case 3:
        data.mutable_score()->set_white_gammon(1.0);
        break;
      case -1:
        data.mutable_score()->set_black_win(1.0);
        break;
      case -2:
      case -3:
        data.mutable_score()->set_black_gammon(1.0);
        break;
      default:
        FAIL(false, "bad final score");
    }
    result.emplace_back(std::move(data));
  }
  return result;
}

void generate_training_data(const std::string& white_model, const std::string& log_path,
                            const std::string& output_path) {
  std::ifstream log_file(log_path);
  ASSERT(log_file.is_open(), "cannot open the log_file");
  auto log_reader = LogReader(log_file);
  auto games = log_reader.read_all();
  auto data_writer = ProtoWriter<TrainingData>(output_path, 1000);
  for (auto& game : games) {
    for (TrainingData& training_data : game_to_training_data(game, "")) {
      data_writer.add(std::move(training_data));
    }
  }
  data_writer.flush();
}

}  // namespace bg