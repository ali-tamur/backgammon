from pathlib import Path

import numpy as np
import tensorflow as tf

from py.src.logger import PlayerLog, read_for_player
from py.src.state import BLACK_BAG, BLACK_BAR, COLLECT_START, WHITE_BAG, WHITE_BAR


def state_to_tensor(state_arr: list[int]) -> tf.Tensor:
    lst = [0] * 330
    for i in range(COLLECT_START):
        n = state_arr[i]
        index = 12 * i
        if n >= 0:
            n = max(n, 5)
            lst[index + n] = 1
        else:
            n = min(-n, 5)
            lst[index + 6 + n] = 1
    start_bar_index = 12 * 24
    n = max(state_arr[WHITE_BAR], 5)
    lst[start_bar_index + n] = 1
    n = max(state_arr[BLACK_BAR], 5)
    lst[start_bar_index + 6 + n] = 1
    bear_off_index = start_bar_index + 12
    lst[bear_off_index + state_arr[WHITE_BAG]] += 1
    lst[bear_off_index + 15 + state_arr[BLACK_BAG]] += 1

    numpy_array = np.array(lst)
    tensor = tf.constant(numpy_array)
    return tensor


def game_to_training_data(game: PlayerLog, accumulate_records):
    states = game.game_states()
    score = game.score()
    num_moves = len(states)
    for i in range(num_moves):
        accumulate_records.append((score, i + 1, num_moves, state_to_tensor(states[i])))


def prepare_training_data(log_file: Path, player_name: str):
    result = []
    for player_log in read_for_player(log_file, player_name):
        game_to_training_data(player_log, result)
    return result
