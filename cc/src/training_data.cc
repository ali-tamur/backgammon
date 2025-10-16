#include "training_data.h"

#include "model.h"

namespace bg {
void game_to_training_data(const Game& game, const std::string& model_path /*currently ignored*/,
                           std::vector<TrainingData>& output) {
  int32_t number_of_moves = game.positions_size();
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
      case -1:
        data.mutable_score()->set_black_win(1.0);
      case -2:
      case -3:
        data.mutable_score()->set_black_gammon(1.0);
      default:
        return;
        output.emplace_back(data);
    }
  }
}

}  // namespace bg