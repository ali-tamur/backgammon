from itertools import islice
from pathlib import Path

import numpy as np
import tensorflow as tf

from python.src.logger import LogReader, TrainData
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


def prepare_training_data(
    log_file: Path, player_name: str, opponent_name: str | None = None, data_size: int | None = None
) -> tuple[tf.Tensor, tf.Tensor]:
    """Prepare training data from game logs.

    Returns:
        x_train: Board state tensors
        y_train: (move_number, num_moves, score) for each position
    """
    log_reader = LogReader(log_file)

    train_data = []
    iterator = log_reader.get_player_moves(player_name, opponent_name)
    if data_size:
        iterator = islice(iterator, data_size)

    for d in iterator:
        if d is None:
            break
        train_data.append(d)

    x_train: tf.Tensor = tf.stack([state_to_tensor(d.board) for d in train_data])
    y_train: tf.Tensor = tf.cast(
        tf.constant([[d.move_number, d.num_moves, d.score] for d in train_data]), dtype=tf.float32
    )

    return x_train, y_train
