from typing import List, Tuple

import numpy as np
import tensorflow as tf

from src.state import GameState, OPPONENT_BAR, COLLECT_START, PLAYER_BAR, PLAYER_BAG, OPPONENT_BAG


def state_to_tensor(state_arr: List[int]) -> tf.Tensor:
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
    n = max(state_arr[PLAYER_BAR], 5)
    lst[start_bar_index + n] = 1
    n = max(state_arr[OPPONENT_BAR], 5)
    lst[start_bar_index + 6 + n] = 1
    bear_off_index = start_bar_index + 12
    lst[bear_off_index + state_arr[PLAYER_BAG]] += 1
    lst[bear_off_index + 15 + state_arr[OPPONENT_BAG]] += 1

    numpy_array = np.array(lst)
    tensor = tf.constant(numpy_array)
    return tensor


def game_to_training_data(state_list: List[GameState], player_index) -> tf.Tensor:
    player_tensor = [state_to_tensor(st.a) for st in state_list[player_index:len(state_list):2]]
    return tf.stack(player_tensor)

def prepare_training_data(log_file, player_name):
    pass
