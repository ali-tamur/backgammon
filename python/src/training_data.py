from pathlib import Path
from typing import Any

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


def state_to_numpy(state_arr: list[int]) -> np.typing.NDArray[Any]:
    numpy_array = np.array(state_to_lst(state_arr))
    return numpy_array


def prepare_training_data(
    log_file: Path, player_name: str, opponent_name: str | None = None, batch_size: int = 128
) -> tf.data.Dataset:
    with tf.device("/CPU:0"):

        def data_generator():
            log_reader = LogReader(log_file)
            iterator = log_reader.get_player_moves(player_name, opponent_name)
            for d in iterator:
                x = state_to_numpy(d.board)
                y = np.array([d.move_number, d.num_moves, d.score], dtype=np.float32)
                yield x, y

        dataset = tf.data.Dataset.from_generator(
            data_generator,
            output_signature=(
                tf.TensorSpec(shape=(250,), dtype=tf.float32),  # type: ignore
                tf.TensorSpec(shape=(3,), dtype=tf.float32),  # type: ignore
            ),
        )
        result = dataset.batch(batch_size).prefetch(2)
        print(f"Returning: {result}")
        return result
