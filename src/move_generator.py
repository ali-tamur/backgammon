from abc import ABC, abstractmethod
from typing import Set, Tuple

from src.state import *


def on_board(index):
    return 0 <= index < COLLECT_END


class MoveGenerator:
    def __init__(self, player_bag:int, player_bar:int, color: COLOR):
        self.player_bag = player_bag
        self.player_bar = player_bar
        self.color = color

    def can_move_on_board(self, state: GameState, src_index, dice: int) -> bool:
        if self.color == COLOR.WHITE:
            dst_index = src_index + dice
            return dst_index < COLLECT_END and state.a[dst_index] >= -1
        else:
            dst_index = src_index - dice
            return dst_index >= 0 and state.a[dst_index] <= 1

    def playable_indexes_for_normal(self, state: GameState, dice: int) -> List[int]:
        if self.color == COLOR.WHITE:
            return [i for i in range(COLLECT_END)
                    if state.a[i] > 0 and i + dice < COLLECT_END and state.a[i + dice] >= -1]
        else:
            return [i for i in range(COLLECT_END)
                    if state.a[i] < 0 <= i - dice and state.a[i - dice] <= 1]

    def moves_for_on_bar(self, state: GameState, dice: List[int]) -> List[
        Tuple[GameState, List[int]]]:
        r = []
        dice_list = [dice[-1]] if dice[0] == dice[-1] else dice
        for d in dice_list:
            land_index = d - 1 if self.color==COLOR.WHITE else COLLECT_END-d
            if ((self.color == COLOR.WHITE and state.a[land_index] >= -1) or
                    (self.color == COLOR.BLACK and state.a[land_index] <= 1)):
                new_state = state.update(self.player_bar, land_index, self.color)
                new_dice = dice[:]
                new_dice.remove(d)
                r.append((new_state, new_dice))
        return r

    def playable_indexes_for_collect(self, state: GameState, dice: int) -> List[int]:
        if self.color == COLOR.WHITE:
            src_indexes = [i for i in range(COLLECT_START, COLLECT_END) if state.a[i] > 0]
        else:
            src_indexes = [i for i in range(6)  if state.a[i] < 0]
        if self.color == COLOR.WHITE and min(src_indexes) + dice >= COLLECT_END:
            return [min(src_indexes)]
        elif self.color == COLOR.BLACK and max(src_indexes) - dice < 0:
            return [max(src_indexes)]
        cands = [src_index for src_index in src_indexes if self.can_move_on_board(state, src_index,
                                                                                  dice)]
        perfect = COLLECT_END - dice if self.color == COLOR.WHITE else dice - 1
        if perfect in src_indexes:
            cands.append(perfect)
        return cands

    def move_state(self, state) -> STATE:
        if state.a[self.player_bar] > 0:
            return STATE.ON_BAR
        if self.color == COLOR.WHITE:
            related_stones = sum([state.a[i] for i in range(COLLECT_START, COLLECT_END) if state.a[
                i]>0])
            related_stones += state.a[WHITE_BAG]
        else:
            related_stones = -sum([state.a[i] for i in range(6) if state.a[i]<0])
            related_stones += state.a[BLACK_BAG]
        return STATE.COLLECTING if related_stones == 15 else STATE.NORMAL

    def generate_moves(self, initial_state: GameState, initial_dice: List[int]) -> Set[GameState]:
        partial_results = [(initial_state, initial_dice)]
        result_list = [initial_state]
        result_dice = len(initial_dice)
        i = 0
        while i < len(partial_results):
            state, dice = partial_results[i]
            i += 1
            if not dice:
                if result_dice > 0:
                    result_list, result_dice = [state], 0
                else:
                    result_list.append(state)
                continue
            board_state = self.move_state(state)
            if board_state == STATE.ON_BAR:
                candidates = self.moves_for_on_bar(state, dice)
                if not candidates:
                    this_dice = len(dice)
                    if this_dice < result_dice:
                        result_list, result_dice = [state], this_dice
                    elif this_dice == result_dice:
                        result_list.append(state)
                    continue
                for new_state, new_dice in candidates:
                    if not new_dice:
                        if result_dice > 0:
                            result_list, result_dice = [state], 0
                        else:
                            result_list.append(state)
                    else:
                        partial_results.append((new_state, new_dice))
            else:
                if board_state == STATE.COLLECTING or dice[-1] == dice[0]:
                    dices = [dice[-1]]
                else:
                    dices = dice
                for d in dices:
                    if board_state == STATE.NORMAL:
                        playable = self.playable_indexes_for_normal(state, d)
                    else:
                        playable = self.playable_indexes_for_collect(state, d)
                    if not playable:
                        this_dice = len(dice)
                        if this_dice < result_dice:
                            result_list, result_dice = [state], this_dice
                        elif this_dice == result_dice:
                            result_list.append(state)
                    for src_index in playable:
                        dst_index = src_index + d if self.color == COLOR.WHITE else src_index - d
                        if not on_board(dst_index):
                            new_state = state.update(src_index, self.player_bag, self.color)
                            if new_state.score() is not None:
                                return {new_state}
                        else:
                            new_state = state.update(src_index, dst_index, self.color)
                        new_dice = dice[:]
                        new_dice.remove(d)
                        if not new_dice:
                            if result_dice > 0:
                                result_list, result_dice = [new_state], 0
                            else:
                                result_list.append(new_state)
                        else:
                            partial_results.append((new_state, new_dice))
        return set(result_list)


WhiteGeneratorFactory = lambda: MoveGenerator(WHITE_BAG, WHITE_BAR, COLOR.WHITE)
BlackGeneratorFactory = lambda: MoveGenerator(BLACK_BAG, BLACK_BAR, COLOR.BLACK)
