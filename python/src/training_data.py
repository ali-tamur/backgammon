from itertools import islice
from pathlib import Path

import numpy as np
import tensorflow as tf

from python.src.logger import LogReader
from python.src.state import BLACK_BAG, BLACK_BAR, COLLECT_END, WHITE_BAG, WHITE_BAR


class OneHotEncoder:
    def __init__(self, size):
        self.lst = [0] * size
        self.i = 0

    def add(self, value, cap_at, can_be_negative):
        advance = cap_at + 1 if not can_be_negative else 2 * cap_at + 1
        value = min(value, cap_at)
        value = max(value, -cap_at)
        if can_be_negative:
            value += cap_at
        self.lst[self.i + value] = 1
        self.i += advance


def state_to_lst(state_arr: list[int]) -> list[int]:
    one_hot = OneHotEncoder(250)
    for i in range(COLLECT_END):
        one_hot.add(state_arr[i], 4, True)
    one_hot.add(state_arr[WHITE_BAR], 4, False)
    one_hot.add(-state_arr[BLACK_BAR], 4, False)
    one_hot.add(state_arr[WHITE_BAG], 10, False)
    one_hot.add(-state_arr[BLACK_BAG], 10, False)
    return one_hot.lst


def state_to_tensor(state_arr: list[int]) -> tf.Tensor:
    numpy_array = np.array(state_to_lst(state_arr))
    tensor = tf.constant(numpy_array)
    return tensor


def prepare_training_data(log_file: Path, player_name: str, data_size: int):
    log_reader = LogReader(log_file)
    result = []
    for log in islice(log_reader.get_player_moves(player_name), data_size):
        board, move_n, n_moves, score = log
        result.append((state_to_tensor(board), move_n, n_moves, score))
    return result
