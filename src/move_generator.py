from abc import ABC, abstractmethod
from typing import Set, Tuple, List
import bitarray as bt

from src.state import *


def on_board(index):
    return 0 <= index < COLLECT_END


def to_bits(lst):
    total = 0
    for index in lst:
        total += 1 << index
    return total

def add_bit(n, index):
    n |= 1<<index

POW_TO_INDEX = dict()
for i in range(BLACK_BAR+1):
    POW_TO_INDEX[1<<i] = i

class MoveGenerator:
    def __init__(self, player_bag:int, player_bar:int, color: COLOR):
        self.player_bag = player_bag
        self.player_bar = player_bar
        self.color = color

    def can_move_on_board(self, a: List[int], src_index, dice: int) -> bool:
        if self.color == COLOR.WHITE:
            dst_index = src_index + dice
            return dst_index < COLLECT_END and a[dst_index] >= -1
        else:
            dst_index = src_index - dice
            return dst_index >= 0 and a[dst_index] <= 1

    def playable_indexes_for_normal(self, a: List[int], dice: int, bits:int) -> List[int]:
        r = []
        while bits != 0:
            x = bits - (bits & (bits-1))
            index = POW_TO_INDEX[x]
            bits = bits & (bits-1)
            if self.color == COLOR.WHITE and a[index] > 0 and index+dice < COLLECT_END and a[i+dice] >= -1:
                    r.append(index)
            elif self.color == COLOR.BLACK and a[index] < 0 <= index - dice and a[index - dice] <= 1:
                r.append(index)
        return r

    def moves_for_on_bar(self, a: List[int], dice: List[int]) -> List[ Tuple[GameState, List[int]]]:
        r = []
        dice_list = [dice[-1]] if dice[0] == dice[-1] else dice
        for d in dice_list:
            land_index = d - 1 if self.color==COLOR.WHITE else COLLECT_END-d
            if ((self.color == COLOR.WHITE and a[land_index] >= -1) or
                    (self.color == COLOR.BLACK and a[land_index] <= 1)):
                new_state = update_state(a[:], self.player_bar, land_index, self.color)
                new_dice = dice[:]
                new_dice.remove(d)
                r.append((new_state, new_dice))
        return r

    def playable_indexes_for_collect(self, a: List[int], dice: int) -> List[int]:
        if self.color == COLOR.WHITE:
            src_indexes = [i for i in range(COLLECT_START, COLLECT_END) if a[i] > 0]
        else:
            src_indexes = [i for i in range(6)  if a[i] < 0]
        if self.color == COLOR.WHITE and min(src_indexes) + dice >= COLLECT_END:
            return [min(src_indexes)]
        elif self.color == COLOR.BLACK and max(src_indexes) - dice < 0:
            return [max(src_indexes)]
        cands = [src_index for src_index in src_indexes if self.can_move_on_board(a, src_index,
                                                                                  dice)]
        perfect = COLLECT_END - dice if self.color == COLOR.WHITE else dice - 1
        if perfect in src_indexes:
            cands.append(perfect)
        return cands

    def move_state(self, a, home_stones) -> STATE:
        if a[self.player_bar] > 0:
            return STATE.ON_BAR
        return STATE.COLLECTING if home_stones == 15 else STATE.NORMAL

    def generate_moves(self, initial_state: GameState, initial_dice: List[int]) -> List[List[int]]:
        a = initial_state.a
        if self.color == COLOR.WHITE:
            bits = to_bits([a[j] > 0 for j in range(COLLECT_END)])
            if a[WHITE_BAR] > 0:
                bits += 1<<WHITE_BAR
            home_stones = sum([a[i] for i in range(COLLECT_START, COLLECT_END) if a[i] > 0])
        else:
            bits = to_bits([a[j] < 0 for j in range(COLLECT_END)])
            if a[BLACK_BAR]:
                bits += 1<<BLACK_BAR
            home_stones = -sum([a[i] for i in range(COLLECT_START, COLLECT_END) if a[i] > 0])

        partial_results = [(initial_state.a, bits, home_stones, initial_dice)]
        result_list = [initial_state.a]
        result_dice = len(initial_dice)
        i = 0
        while i < len(partial_results):
            a, bits, home_stones, dice = partial_results[i]
            i += 1
            if not dice:
                if result_dice > 0:
                    result_list, result_dice = [a], 0
                else:
                    result_list.append(a)
                continue
            board_state = self.move_state(a,home_stones)
            if board_state == STATE.ON_BAR:
                candidates = self.moves_for_on_bar(a, dice)
                if not candidates:
                    this_dice = len(dice)
                    if this_dice < result_dice:
                        result_list, result_dice = [a], this_dice
                    elif this_dice == result_dice:
                        result_list.append(a)
                    continue
                for new_state, new_dice in candidates:
                    if not new_dice:
                        if result_dice > 0:
                            result_list, result_dice = [a], 0
                        else:
                            result_list.append(a)
                    else:
                        partial_results.append((new_state, bits, home_stones, new_dice))
            else:
                if board_state == STATE.COLLECTING or dice[-1] == dice[0]:
                    dices = [dice[-1]]
                else:
                    dices = dice
                for d in dices:
                    if board_state == STATE.NORMAL:
                        playable = self.playable_indexes_for_normal(a, d)
                    else:
                        playable = self.playable_indexes_for_collect(a, d)
                    if not playable:
                        this_dice = len(dice)
                        if this_dice < result_dice:
                            result_list, result_dice = [a], this_dice
                        elif this_dice == result_dice:
                            result_list.append(a)
                    for src_index in playable:
                        dst_index = src_index + d if self.color == COLOR.WHITE else src_index - d
                        if not on_board(dst_index):
                            new_state = update_state(a[:], src_index, self.player_bag,
                                                     self.color)
                            if score_state(new_state) is not None:
                                return [new_state]
                        else:
                            new_state = update_state(a[:], src_index, dst_index, self.color)
                        new_dice = dice[:]
                        new_dice.remove(d)
                        if not new_dice:
                            if result_dice > 0:
                                result_list, result_dice = [new_state], 0
                            else:
                                result_list.append(new_state)
                        else:
                            new_home_stones = home_stones
                            new_bits = bits
                            if dst_index != WHITE_BAG and dst_index != BLACK_BAG:
                                new_bits |= 1 << dst_index
                            if self.color == COLOR.WHITE and COLLECT_START <= src_index < COLLECT_END:
                                new_home_stones -= 1
                            elif self.color == COLOR.BLACK and 0 <= src_index < 6:
                                new_home_stones -= 1
                            if self.color == COLOR.WHITE and COLLECT_START <= dst_index < COLLECT_END:
                                new_home_stones += 1
                            elif self.color == COLOR.BLACK and 0 <= dst_index < 6:
                                new_home_stones += 1
                            partial_results.append((new_state, new_bits, new_home_stones, new_dice))
        return result_list
