from src.move_generator import *
from src.dice_source import Dice


def generate_all_dice():
    return [Dice.to_list(i, j) for i in [1, 2, 3, 4, 5, 6] for j in [1, 2, 3, 4, 5, 6]]


def HOME(dice):
    return COLLECT_END - dice

def construct_state(d:dict, r = None, rval = None)->GameState:
    a = [0] * 28
    for key in d:
        a[key] = d[key]
    if r is not None:
        for i in r:
            a[i] = rval
    state = GameState(a)
    return state

def change_state(state, d) -> GameState:
    a = list(state.a)
    for key in d:
        a[key] = d[key]
    state = GameState(a)
    return state

def opposite_perspective(state:GameState)->GameState:
    a = [-state.a[i] for i in range(COLLECT_END - 1, -1, -1)]
    for i in [BLACK_BAR, WHITE_BAR, BLACK_BAG, WHITE_BAG]:
        a.append(state.a[i])
    return GameState(a)

class Expectation:
    def __init__(self, length=None, expected_state=None, state_set=None, *, score=None):
        self.length = length
        self.expected_state = expected_state
        self.state_set = state_set
        self.score = score

    def check(self, result_set:Set[GameState]):
       if self.length is not None:
           assert len(result_set) == self.length
       if self.state_set is not None:
           assert result_set == self.state_set
       if self.expected_state is not None:
           singe_result = result_set.pop()
           assert singe_result == self.expected_state
           if self.score is not None:
               assert singe_result.score() == self.score


class MyTestCase:
    def __init__(self, state:GameState, dice:list[int], expected:Expectation):
        self.state = state
        self.dice = dice
        self.expected = expected

    def check(self):
        r = WhiteGeneratorFactory().generate_moves(self.state, self.dice)
        self.expected.check(r)

        black_state = opposite_perspective(self.state)
        r = BlackGeneratorFactory().generate_moves(black_state, self.dice)
        black_r:Set[GameState] = set()
        for st in r:
            black_r.add(opposite_perspective(st))
        self.expected.check(black_r)


class TestMoveGenerator:
    def test_stone_on_bar_cannot_land(self):
        # The opponent home is full and the stone on the bar cannot land.
        state = construct_state({10:-3,HOME(6):14, WHITE_BAR:1}, range(6), -2)
        exp = Expectation(length=1, expected_state=state)

        for d in generate_all_dice():
            MyTestCase(state, d, exp).check()

    def test_stone_on_bar_cannot_land_2(self):
        # All the dice values point to opponent barrier and the stone on the bar cannot land.
        state = construct_state({1:-2, 3:-2,10:-11,HOME(6):14,WHITE_BAR:1})
        exp = Expectation(length=1, expected_state=state)

        for d1, d2 in [(2, 2), (2, 4), (4, 4)]:
            MyTestCase(state, Dice.to_list(d1, d2), exp).check()

    def test_stone_cannot_move(self):
        # All stones are at the end except for a couple at the start completely trapped.
        state = construct_state({10:-3,HOME(1):13, 0:2}, range(1,7), -2)
        exp = Expectation(length=1, expected_state=state)

        for d in generate_all_dice():
            MyTestCase(state, d, exp).check()

    def test_stone_cannot_move_2(self):
        # The stone at the start is trapped and the others cannot move
        state = construct_state({0:3, 6:-15}, range(HOME(6), COLLECT_END), 2)
        exp = Expectation(length=1, expected_state=state)
        dice = Dice.to_list(6, 6)
        MyTestCase(state, dice, exp).check()

    def test_remaining_stone_can_neither_move_nor__get_collected(self):
        state = construct_state({HOME(6):2,HOME(1):-2,WHITE_BAG:13, BLACK_BAG:13})
        exp = Expectation(length=1, expected_state=state)
        dice = Dice.to_list(5, 5)
        MyTestCase(state, dice, exp).check()

    def test_no_stone_can_move(self):
        state = construct_state({5:2,7:-2,8:-2,10:2,12:-2,13:-2,WHITE_BAG:11,BLACK_BAG:7})
        exp = Expectation(length=1, expected_state=state)
        dice = Dice.to_list(2, 3)
        MyTestCase(state, dice, exp).check()

    def test_no_stone_can_move_2(self):
        state = construct_state({HOME(5):2, WHITE_BAG:13,COLLECT_END-1:-2,COLLECT_END-2:-2,
                                 BLACK_BAG:11})
        exp = Expectation(length=1, expected_state=state)
        dice = Dice.to_list(3, 4)
        MyTestCase(state, dice, exp).check()

    def test_land_one_stone(self):
        state = construct_state({HOME(5):13,WHITE_BAR:2,0:-2,1:-2,3:-2,4:-2,5:-2,BLACK_BAG:5})
        expected = change_state(state, {2:1,WHITE_BAR:1})
        exp = Expectation(length=1, expected_state=expected)
        dice = Dice.to_list(2, 3)
        MyTestCase(state, dice, exp).check()

    def test_land_two_stones(self):
        state = construct_state({HOME(1):13, WHITE_BAR:2, 0:-2,1:-2,BLACK_BAG:5}, range(3,6), -2)
        expected = change_state(state, {2: 2, WHITE_BAR: 0})
        exp = Expectation(length=1, expected_state=expected)
        dice = Dice.to_list(3, 3)
        MyTestCase(state, dice, exp).check()

    def test_escape_1(self):
        state = construct_state({HOME(1):14, WHITE_BAR:1, BLACK_BAG:5}, range(5), -2)
        expected = change_state(state, {HOME(1): 15, WHITE_BAR: 0})
        exp = Expectation(length=1, expected_state=expected)
        dice = Dice.to_list(6, 6)
        MyTestCase(state, dice, exp).check()

    def test_escape_2_have_to_use_smaller_dice_first(self):
        state = construct_state({HOME(1):14, 0:1, 5:-2, BLACK_BAR:13})
        expected = change_state(state, {0:0, 7:1})
        exp = Expectation(length=1, expected_state=expected)
        dice = Dice.to_list(5, 2)
        MyTestCase(state, dice, exp).check()

    def test_single_stone_movement(self):
        state = construct_state({HOME(1):14, 0:1, BLACK_BAR:15})
        for i in range(1, 7):
            for j in range(i + 1, 7):
                expected = change_state(state, {0:0, i+j:1})
                exp = Expectation(length=1, expected_state=expected)
                dice = Dice.to_list(i, j)
                MyTestCase(state, dice, exp).check()

        for i in range(1, 6):
            expected = change_state(state, {0: 0, 4 * i: 1})
            exp = Expectation(length=1, expected_state=expected)
            dice = Dice.to_list(i, i)
            MyTestCase(state, dice, exp).check()

        dice = Dice.to_list(6, 6)
        expected = change_state(state, {0: 0, WHITE_BAG: 1})
        exp = Expectation(length=1, expected_state=expected)
        MyTestCase(state, dice, exp).check()

    def test_single_stone_move_and_bolt(self):
        state = construct_state({HOME(1):14, 0:1,BLACK_BAR:14,7:-1})

        for i in range(1, 7):
            j = 7 - i
            dice = Dice.to_list(i, j)
            expected = change_state(state, {0: 0, 7:1, BLACK_BAR: 15})
            exp = Expectation(length=1, expected_state=expected)
            MyTestCase(state, dice, exp).check()

    def test_double_stone_movement(self):
        state = construct_state({HOME(1):13, 0:1, 10:1, BLACK_BAR:15})

        for i in range(1, 7):
            for j in range(i + 1, 7):
                dice = Dice.to_list(i, j)

                expected_set = set()
                for src in [0, 10]:
                    e = change_state(state, {src:0, src+i+j:state.a[src+i+j]+1})
                    expected_set.add(e)
                for src1, src2 in [(0, 10), (10, 0)]:
                    e = change_state(state, {src1:0, src2:0,src1+i:1,src2+j:1})
                    expected_set.add(e)

                exp = Expectation(length=4, state_set=expected_set)
                MyTestCase(state, dice, exp).check()

    def test_double_stone_movement_with_blocks(self):
        state = construct_state({HOME(1):13, 0:1,1:1,2:-2,BLACK_BAR:7}, range(6,9),-2)
        expected = change_state(state, {0: 0, 5: 1, 1:0, 3:1})
        exp = Expectation(length=1, expected_state=expected)
        dice = Dice.to_list(5, 2)
        MyTestCase(state, dice, exp).check()

    def test_double_stone_movement_with_blocks_2(self):
        a = [0] * 28
        a[HOME(1)] = 13
        a[0] = 1
        a[1] = 1
        a[5] = -2
        a[6] = -2
        a[8] = -2
        a[BLACK_BAR] = 9
        state = GameState(a)

        dice = Dice.to_list(2, 5)
        expected = change_state(state, {0: 0, 7: 1})
        exp = Expectation(length=1, expected_state=expected)
        MyTestCase(state, dice, exp).check()


    def test_only_one_dice_can_move_multiple_results(self):
        a = [0] * 28
        a[HOME(6)] = 13
        a[HOME(1)] = -2
        a[0] = 1
        a[4] = 1
        a[6] = -2
        a[10] = -2
        a[11] = -2
        a[15] = -2
        a[BLACK_BAR] = 5
        state = GameState(a)

        dice = Dice.to_list(6, 5)
        expected_set = set()
        expected_set.add(change_state(state, {0:0, 5:1}))
        expected_set.add(change_state(state, {4: 0, 9: 1}))
        exp = Expectation(length=2, state_set=expected_set)
        MyTestCase(state, dice, exp).check()

    def test_double_stone_double_dice(self):
        a = [0] * 28
        a[HOME(1)] = 13
        a[0] = 1
        a[1] = 1
        a[BLACK_BAR] = 15
        state = GameState(a)

        dice = Dice.to_list(3, 3)
        expected_set = set()
        for move_first in range(5):
            move_second = 4 - move_first
            e = change_state(state, {0:0,move_first*3:1,1:0,1+move_second*3:1})
            expected_set.add(e)
        exp = Expectation(length=5, state_set=expected_set)
        MyTestCase(state, dice, exp).check()

    def test_double_stone_double_dice_2(self):
        a = [0] * 28
        a[HOME(1)] = 13
        a[0] = 1
        a[4] = 1
        a[BLACK_BAR] = 15
        state = GameState(a)

        dice = Dice.to_list(4, 4)
        expected_set = set()
        for move_first in range(5):
            move_second = 4 - move_first
            e = a[:]
            e[0] = 0
            e[move_first * 4] += 1
            e[4] -= 1
            e[4 + move_second * 4] += 1
            expected_set.add(GameState(e))
        exp = Expectation(length=3, state_set=expected_set)
        MyTestCase(state, dice, exp).check()

    def test_four_stones_double_dice(self):
        a = [0] * 28
        a[HOME(1)] = 11
        for i in range(4):
            a[i] = 1
        a[BLACK_BAR] = 15
        state = GameState(a)

        dice = Dice.to_list(4, 4)
        expected_set = set()
        for move_a in range(5):
            for move_b in range(5 - move_a):
                for move_c in range(5 - move_a - move_b):
                    move_d = 4 - move_a - move_b - move_c
                    e = a[:]
                    for src, moves in [(0, move_a), (1, move_b), (2, move_c), (3, move_d)]:
                        e[src] -= 1
                        e[src + 4 * moves] += 1
                    expected_set.add(GameState(e))

        exp = Expectation(length=35, state_set=expected_set)
        MyTestCase(state, dice, exp).check()

    def test_collect_with_waste(self):
        a = [0] * 28
        for i in range(HOME(3), COLLECT_END):
            a[i] = 1
        a[WHITE_BAG] = 12
        a[BLACK_BAR] = 15
        state = GameState(a)

        dice = Dice.to_list(4, 5)
        e = change_state(state, {WHITE_BAG:14, HOME(3):0, HOME(2):0})
        exp = Expectation(length=1, expected_state=e)
        MyTestCase(state, dice, exp).check()

    def test_collect_double(self):
        a = [0] * 28
        for i in range(COLLECT_START + 1, COLLECT_END):
            a[i] = 1
        a[HOME(6)] = 10
        a[BLACK_BAR] = 15
        state = GameState(a)

        dice = Dice.to_list(6, 6)

        e = change_state(state, {WHITE_BAG: 4, HOME(6): 6})
        exp = Expectation(length=1, expected_state=e)
        MyTestCase(state, dice, exp).check()

    def test_move_in_collect_phase(self):
        a = [0] * 28
        a[HOME(1)] = 1
        a[HOME(6)] = 1
        a[WHITE_BAG] = 13
        a[BLACK_BAR] = 15
        state = GameState(a)

        dice = Dice.to_list(3, 2)

        e = change_state(state, {HOME(1):2, HOME(6):0})
        exp = Expectation(length=1, expected_state=e)
        MyTestCase(state, dice, exp).check()

    def test_partial_collect(self):
        a = [0] * 28
        a[HOME(1)] = 2
        a[HOME(2)] = -2
        a[HOME(3)] = 1
        a[HOME(5)] = 1
        a[WHITE_BAG] = 11
        a[BLACK_BAR] = 13
        state = GameState(a)

        dice = Dice.to_list(3, 3)

        e = change_state(state, {WHITE_BAG: 12, HOME(3): 0})
        exp = Expectation(length=1, expected_state=e)
        MyTestCase(state, dice, exp).check()

    def test_move_and_collect(self):
        a = [0] * 28
        a[HOME(3)] = 1
        a[HOME(6)] = 1
        a[WHITE_BAG] = 13
        a[BLACK_BAR] = 15
        state = GameState(a)

        dice = Dice.to_list(5, 3)
        e = change_state(state, {WHITE_BAG: 14, HOME(3): 0, HOME(6):0, HOME(1):1})
        exp = Expectation(length=1, expected_state=e)
        MyTestCase(state, dice, exp).check()

    def test_move_all_no_collect(self):
        a = [0] * 28
        a[HOME(3)] = 5
        a[HOME(1)] = -2
        a[WHITE_BAG] = 10
        a[BLACK_BAR] = 13
        state = GameState(a)

        dice = Dice.to_list(1, 1)
        e = change_state(state, {HOME(3):1, HOME(2):4})
        exp = Expectation(length=1, expected_state=e)
        MyTestCase(state, dice, exp).check()

    def test_game_end(self):
        a = [0] * 28
        a[HOME(5)] = 1
        a[HOME(3)] = 1
        a[HOME(1)] = 1
        a[WHITE_BAG] = 12
        a[BLACK_BAR] = 15
        state = GameState(a)

        dice = Dice.to_list(5, 5)
        e = change_state(state, {HOME(5): 0, HOME(3): 0, HOME(1):0, WHITE_BAG:15})
        exp = Expectation(length=1, expected_state=e, score=3)
        MyTestCase(state, dice, exp).check()

    def test_count_moves_at_start(self):
        state = GameState.game_start()

        dice = Dice.to_list(6, 6)
        exp = Expectation(length=11)
        MyTestCase(state, dice, exp).check()

        dice = Dice.to_list(5, 5)
        exp = Expectation(length=4)
        MyTestCase(state, dice, exp).check()

        dice = Dice.to_list(1, 5)
        exp = Expectation(length=8)
        MyTestCase(state, dice, exp).check()
