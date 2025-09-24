from abc import ABC, abstractmethod
from typing import Set, Tuple

from src.state import *


class ResultKeeper:
    def __init__(self, initial_state: GameState, remaining_dice: int) -> None:
        self.d = {initial_state}
        self.remaining_dice = remaining_dice

    def add(self, state, remaining_dice) -> None:
        if remaining_dice > self.remaining_dice:
            return
        elif remaining_dice < self.remaining_dice:
            self.remaining_dice = remaining_dice
            self.d = {state}
        else:
            self.d.add(state)

    def moves(self) -> Set[GameState]:
        return self.d


def on_board(index):
    return 0 <= index < COLLECT_END


class MoveGenerator(ABC):
    def __init__(self):
        self.player_bag = None
        self.player_bar = None

    @abstractmethod
    def destination(self, index, dice) -> int:
        pass

    @abstractmethod
    def is_present_at(self, state: GameState, index: int) -> bool:
        pass

    @abstractmethod
    def can_land_on(self, state: GameState, dst_index: int) -> bool:
        pass

    @abstractmethod
    def land_index_from_bar(self, dice) -> int:
        pass

    @abstractmethod
    def update_board(self, state: GameState, src_index: int, dst_index: int) -> GameState:
        pass

    @abstractmethod
    def collect_range(self) -> range:
        pass

    @abstractmethod
    def perfect_collect(self, dice) -> int:
        pass

    @abstractmethod
    def best_for_collect(self, src_indexes) -> int:
        pass

    @abstractmethod
    def num_stones_at_home(self, state) -> int:
        pass

    def can_move_on_board(self, state:GameState, src_index, dice: int) -> bool:
        dst_index = self.destination(src_index, dice)
        return 0 <= dst_index < COLLECT_END and self.can_land_on(state, dst_index)

    def playable_indexes_for_normal(self, state:GameState, dice: int) -> List[int]:
        return [i for i in range(COLLECT_END) if
                self.is_present_at(state, i) and self.can_move_on_board(state, i, dice)]

    def moves_for_on_bar(self, state: GameState, dice: List[int]) -> List[Tuple[GameState, List[int]]]:
        r = []
        dice_list = [dice[-1]] if dice[0] == dice[-1] else dice
        for d in dice_list:
            land_index = self.land_index_from_bar(d)
            if self.can_land_on(state, land_index):
                new_state = self.update_board(state, self.player_bar, land_index)
                new_dice = dice[:]
                new_dice.remove(d)
                r.append((new_state, new_dice))
        return r

    def playable_indexes_for_collect(self, state:GameState, dice: int) -> List[int]:
        src_indexes = [index for index in self.collect_range() if self.is_present_at(state, index)]
        can_collect_with_waste = all(
            [not on_board(self.destination(index, dice)) for index in src_indexes])
        if can_collect_with_waste:
            return [self.best_for_collect(src_indexes)]
        cands = [src_index for src_index in src_indexes if self.can_move_on_board(state, src_index,
                                                                                  dice)]
        perfect = self.perfect_collect(dice)
        if perfect in src_indexes:
            cands.append(perfect)
        return cands

    def move_state(self, state) -> STATE:
        if state.a[self.player_bar] > 0:
            return STATE.ON_BAR
        if self.num_stones_at_home(state) + state.a[self.player_bag] == 15:
            return STATE.COLLECTING
        return STATE.NORMAL

    def generate_moves(self, initial_state: GameState, initial_dice: List[int]) -> Set[GameState]:
        partial_results = [(initial_state, initial_dice)]
        result = ResultKeeper(initial_state, len(initial_dice))
        i = 0
        while i < len(partial_results):
            state, dice = partial_results[i]
            i += 1
            if not dice:
                result.add(state, 0)
                continue
            move_state = self.move_state(state)
            if move_state == STATE.ON_BAR:
                candidates = self.moves_for_on_bar(state, dice)
                if not candidates:
                    result.add(state, len(dice))
                    continue
                for new_state, new_dice in candidates:
                    if not new_dice:
                        result.add(new_state, 0)
                    else:
                        partial_results.append((new_state, new_dice))
            else:
                if move_state == STATE.COLLECTING or dice[-1] == dice[0]:
                    dices = [dice[-1]]
                else:
                    dices = dice
                for d in dices:
                    if move_state == STATE.NORMAL:
                        playable = self.playable_indexes_for_normal(state, d)
                    else:
                        playable = self.playable_indexes_for_collect(state,d)
                    if not playable:
                        result.add(state, len(dice))
                    for src_index in playable:
                        dst_index = self.destination(src_index, d)
                        if not on_board(dst_index):
                            new_state = self.update_board(state, src_index, self.player_bag)
                            if new_state.score() is not None:
                                return {new_state}
                        else:
                            new_state = self.update_board(state, src_index, dst_index)
                        new_dice = dice[:]
                        new_dice.remove(d)
                        if not new_dice:
                            result.add(new_state, 0)
                        else:
                            partial_results.append((new_state, new_dice))
        return result.moves()


class BlackGenerator(MoveGenerator):
    def __init__(self):
        super().__init__()
        self.player_bag = BLACK_BAG
        self.player_bar = BLACK_BAR

    def update_board(self, state, src_index: int, dst_index: int):
        a = list(state.a)
        if src_index == self.player_bar:
            a[src_index] -= 1
        else:
            a[src_index] += 1
        if dst_index == self.player_bag:
            a[dst_index] += 1
        elif a[dst_index] == 1:
            a[WHITE_BAR] += 1
            a[dst_index] = -1
        else:
            a[dst_index] -= 1
        return GameState(tuple(a))

    def destination(self, index, dice) -> int:
        return index - dice

    def is_present_at(self, state, index) -> bool:
        return state.a[index] < 0

    def can_land_on(self, state, dst_index) -> bool:
        return state.a[dst_index] <= 1

    def land_index_from_bar(self, dice) -> int:
        return COLLECT_END - dice

    def collect_range(self):
        return range(6)

    def perfect_collect(self, dice) -> int:
        return dice - 1

    def best_for_collect(self, src_indexes) -> int:
        return max(src_indexes)

    def num_stones_at_home(self, state) -> int:
       return -sum(state.a[i] for i in self.collect_range() if self.is_present_at(state, i))


class WhiteGenerator(MoveGenerator):
    def __init__(self):
        super().__init__()
        self.player_bag = WHITE_BAG
        self.player_bar = WHITE_BAR

    def update_board(self, state, src_index: int, dst_index: int):
        a = list(state.a)
        a[src_index] -= 1
        if a[dst_index] == -1:
            a[BLACK_BAR] += 1
            a[dst_index] = 1
        else:
            a[dst_index] += 1
        return GameState(tuple(a))

    def destination(self, index, dice) -> int:
        return index + dice

    def is_present_at(self, state, index):
        return state.a[index] > 0

    def can_land_on(self, state, dst_index):
        return state.a[dst_index] >= -1

    def land_index_from_bar(self, dice) -> int:
        return dice - 1

    def collect_range(self):
        return range(COLLECT_START, COLLECT_END)

    def perfect_collect(self, dice) -> int:
        return COLLECT_END - dice

    def best_for_collect(self, src_indexes) -> int:
        return min(src_indexes)

    def num_stones_at_home(self, state) -> int:
        return sum(state.a[i] for i in self.collect_range() if self.is_present_at(state, i))


WhiteGeneratorFactory = lambda: WhiteGenerator()
BlackGeneratorFactory = lambda: BlackGenerator()
