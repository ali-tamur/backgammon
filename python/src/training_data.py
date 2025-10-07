from itertools import islice
from pathlib import Path

import numpy as np
import tensorflow as tf

from python.src.logger import LogReader
from python.src.state import BLACK_BAG, BLACK_BAR, COLLECT_END, WHITE_BAG, WHITE_BAR


def state_to_tensor(state_arr: list[int]) -> tf.Tensor:
    lst = [0] * 330
    for i in range(COLLECT_END):
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


def prepare_training_data(log_file: Path, player_name: str, data_size: int):
    log_reader = LogReader(log_file)
    result = []
    for log in islice(log_reader.get_player_moves(player_name), data_size):
        board, move_n, n_moves, score = log
        result.append((state_to_tensor(board), move_n, n_moves, score))
    return result
