from src.move_generator import *


def generate_all_dice():
    return [Dice.dice_throw(i, j) for i in [1, 2, 3, 4, 5, 6] for j in [1, 2, 3, 4, 5, 6]]


def HOME(dice):
    return COLLECT_END - dice


class TestMoveGenerator:
    def test_stone_on_bar_cannot_land(self):
        # The opponent home is full and the stone on the bar cannot land.
        a = [0] * 28
        for i in range(6):
            a[i] = -2
        a[10] = -3
        a[HOME(6)] = 14
        a[WHITE_BAR] = 1
        state = GameState(tuple(a))

        for d in generate_all_dice():
            r = WhiteGenerator().generate_moves(state, d)
            assert len(r) == 1
            assert r.pop() == state

    def test_stone_on_bar_cannot_land_2(self):
        # All the dice values point to opponent barrier and the stone on the bar cannot land.
        a = [0] * 28
        a[1] = -2
        a[3] = -2
        a[10] = -11
        a[HOME(6)] = 14
        a[WHITE_BAR] = 1
        state = GameState(tuple(a))

        for d1, d2 in [(2, 2), (2, 4), (4, 4)]:
            dice = Dice.dice_throw(d1, d2)
            r = WhiteGenerator().generate_moves(state, dice)
            assert len(r) == 1
            assert r.pop() == state

    def test_stone_cannot_move(self):
        # All stones are at the end except for a couple at the start completely trapped.
        a = [0] * 28
        a[0] = 2
        for i in range(1, 7):
            a[i] = -2
        a[10] = -3
        a[HOME(1)] = 13
        state = GameState(tuple(a))

        for d in generate_all_dice():
            r = WhiteGenerator().generate_moves(state, d)
            assert len(r) == 1
            assert r.pop() == state

    def test_stone_cannot_move_2(self):
        # The stone at the start is trapped and the others cannot move
        a = [0] * 28
        a[0] = 3
        a[6] = -15
        for i in range(HOME(6), COLLECT_END):
            a[i] = 2
        state = GameState(tuple(a))

        dice = Dice.dice_throw(6, 6)
        r = WhiteGenerator().generate_moves(state, dice)
        assert len(r) == 1
        assert r.pop() == state

    def test_remaining_stone_can_neither_move_nor__get_collected(self):
        a = [0] * 28
        a[HOME(6)] = 2
        a[HOME(1)] = -2
        a[WHITE_BAG] = 13
        a[BLACK_BAG] = 13
        state = GameState(tuple(a))

        dice = Dice.dice_throw(5, 5)
        r = WhiteGenerator().generate_moves(state, dice)
        assert len(r) == 1
        assert r.pop() == state

    def test_no_stone_can_move(self):
        a = [0] * 28
        a[5] = 2
        a[7] = -2
        a[8] = -2
        a[10] = 2
        a[12] = -2
        a[13] = -2
        a[WHITE_BAG] = 11
        a[BLACK_BAG] = 7
        state = GameState(tuple(a))

        dice = Dice.dice_throw(2, 3)
        r = WhiteGenerator().generate_moves(state, dice)
        assert len(r) == 1
        assert r.pop() == state

    def test_no_stone_can_move_2(self):
        a = [0] * 28
        a[HOME(5)] = 2
        a[WHITE_BAG] = 13
        a[COLLECT_END - 1] = -2
        a[COLLECT_END - 2] = -2
        a[BLACK_BAG] = 11
        state = GameState(tuple(a))

        dice = Dice.dice_throw(3, 4)
        r = WhiteGenerator().generate_moves(state, dice)
        assert len(r) == 1
        assert r.pop() == state

    def test_land_one_stone(self):
        a = [0] * 28
        a[HOME(5)] = 13
        a[WHITE_BAR] = 2
        for i in [0, 1, 3, 4, 5]:
            a[i] = -2
        a[BLACK_BAG] = 5
        state = GameState(tuple(a))

        dice = Dice.dice_throw(2, 3)
        r = WhiteGenerator().generate_moves(state, dice)
        assert len(r) == 1

        expected = a[:]
        expected[2] = 1
        expected[WHITE_BAR] = 1
        assert r.pop().a == tuple(expected)

    def test_land_two_stones(self):
        a = [0] * 28
        a[HOME(1)] = 13
        a[WHITE_BAR] = 2
        for i in [0, 1, 3, 4, 5]:
            a[i] = -2
        a[BLACK_BAG] = 5
        state = GameState(tuple(a))

        dice = Dice.dice_throw(3, 3)
        r = WhiteGenerator().generate_moves(state, dice)
        assert len(r) == 1

        expected = a[:]
        expected[2] = 2
        expected[WHITE_BAR] = 0
        assert r.pop().a == tuple(expected)

    def test_escape_1(self):
        a = [0] * 28
        a[HOME(1)] = 14
        a[WHITE_BAR] = 1
        for i in [0, 1, 2, 3, 4]:
            a[i] = -2
        a[BLACK_BAG] = 5
        state = GameState(tuple(a))

        dice = Dice.dice_throw(6, 6)
        r = WhiteGenerator().generate_moves(state, dice)
        assert len(r) == 1

        expected = a[:]
        expected[HOME(1)] = 15
        expected[WHITE_BAR] = 0
        assert r.pop().a == tuple(expected)

    def test_escape_2_have_to_use_smaller_dice_first(self):
        a = [0] * 28
        a[HOME(1)] = 14
        a[0] = 1
        a[5] = -2
        a[BLACK_BAR] = 13
        state = GameState(tuple(a))

        dice = Dice.dice_throw(5, 2)
        r = WhiteGenerator().generate_moves(state, dice)
        assert len(r) == 1

        expected = a[:]
        expected[0] = 0
        expected[7] = 1
        assert r.pop().a == tuple(expected)

    def test_single_stone_movement(self):
        a = [0] * 28
        a[HOME(1)] = 14
        a[0] = 1
        a[BLACK_BAR] = 15
        state = GameState(tuple(a))

        for i in range(1, 7):
            for j in range(i + 1, 7):
                dice = Dice.dice_throw(i, j)
                r = WhiteGenerator().generate_moves(state, dice)
                assert len(r) == 1
                expected = a[:]
                expected[0] = 0
                expected[i + j] = 1
                assert r.pop().a == tuple(expected)

        for i in range(1, 6):
            dice = Dice.dice_throw(i, i)
            r = WhiteGenerator().generate_moves(state, dice)
            assert len(r) == 1
            expected = a[:]
            expected[0] = 0
            expected[4 * i] = 1
            assert r.pop().a == tuple(expected)

        dice = Dice.dice_throw(6, 6)
        r = WhiteGenerator().generate_moves(state, dice)
        assert len(r) == 1
        expected = a[:]
        expected[0] = 0
        expected[WHITE_BAG] = 1
        assert r.pop().a == tuple(expected)

    def test_single_stone_move_and_bolt(self):
        a = [0] * 28
        a[HOME(1)] = 14
        a[0] = 1
        a[BLACK_BAR] = 14
        a[7] = -1
        state = GameState(tuple(a))

        for i in range(1, 7):
            j = 7 - i
            dice = Dice.dice_throw(i, j)
            r = WhiteGenerator().generate_moves(state, dice)
            assert len(r) == 1

            expected = a[:]
            expected[0] = 0
            expected[7] = 1
            expected[BLACK_BAR] = 15
            assert r.pop().a == tuple(expected)

    def test_double_stone_movement(self):
        a = [0] * 28
        a[HOME(1)] = 13
        a[0] = 1
        a[10] = 1
        a[BLACK_BAR] = 15
        state = GameState(tuple(a))

        for i in range(1, 7):
            for j in range(i + 1, 7):
                dice = Dice.dice_throw(i, j)
                r = WhiteGenerator().generate_moves(state, dice)
                assert len(r) == 4

                expected_set = set()
                for src in [0, 10]:
                    e = a[:]
                    e[src] = 0
                    e[src + i + j] += 1
                    expected_set.add(tuple(e))
                for src1, src2 in [(0, 10), (10, 0)]:
                    e = a[:]
                    e[src1] = 0
                    e[src2] = 0
                    e[src1 + i] = 1
                    e[src2 + j] = 1
                    expected_set.add(tuple(e))

                assert {state.a for state in r} == expected_set

    def test_double_stone_movement_with_blocks(self):
        a = [0] * 28
        a[HOME(1)] = 13
        a[0] = 1
        a[1] = 1
        a[2] = -2
        a[7] = -2
        a[6] = -2
        a[8] = -2
        a[BLACK_BAR] = 7
        state = GameState(tuple(a))

        dice = Dice.dice_throw(2, 5)
        r = WhiteGenerator().generate_moves(state, dice)
        assert len(r) == 1

        expected = a[:]
        expected[0] = 0
        expected[5] = 1
        expected[1] = 0
        expected[3] = 1
        assert r.pop().a == tuple(expected)

    def test_double_stone_movement_with_blocks_2(self):
        a = [0] * 28
        a[HOME(1)] = 13
        a[0] = 1
        a[1] = 1
        a[5] = -2
        a[6] = -2
        a[8] = -2
        a[BLACK_BAR] = 9
        state = GameState(tuple(a))

        dice = Dice.dice_throw(2, 5)
        r = WhiteGenerator().generate_moves(state, dice)
        assert len(r) == 1

        expected = a[:]
        expected[0] = 0
        expected[7] = 1
        assert r.pop().a == tuple(expected)

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
        state = GameState(tuple(a))

        dice = Dice.dice_throw(6, 5)
        r = WhiteGenerator().generate_moves(state, dice)
        assert len(r) == 2

        expected_set = set()
        e = a[:]
        e[0] = 0
        e[5] = 1
        expected_set.add((tuple(e)))
        e = a[:]
        e[4] = 0
        e[9] = 1
        expected_set.add(tuple(e))
        assert {state.a for state in r} == expected_set

    def test_double_stone_double_dice(self):
        a = [0] * 28
        a[HOME(1)] = 13
        a[0] = 1
        a[1] = 1
        a[BLACK_BAR] = 15
        state = GameState(tuple(a))

        dice = Dice.dice_throw(3, 3)
        r = WhiteGenerator().generate_moves(state, dice)
        assert len(r) == 5

        expected_set = set()
        for move_first in range(5):
            move_second = 4 - move_first
            e = a[:]
            e[0] = 0
            e[move_first * 3] = 1
            e[1] = 0
            e[1 + move_second * 3] = 1
            expected_set.add(tuple(e))
        assert {state.a for state in r} == expected_set

    def test_double_stone_double_dice_2(self):
        a = [0] * 28
        a[HOME(1)] = 13
        a[0] = 1
        a[4] = 1
        a[BLACK_BAR] = 15
        state = GameState(tuple(a))

        dice = Dice.dice_throw(4, 4)
        r = WhiteGenerator().generate_moves(state, dice)
        assert len(r) == 3

        expected_set = set()
        for move_first in range(5):
            move_second = 4 - move_first
            e = a[:]
            e[0] = 0
            e[move_first * 4] += 1
            e[4] -= 1
            e[4 + move_second * 4] += 1
            expected_set.add(tuple(e))
        assert {state.a for state in r} == expected_set

    def test_four_stones_double_dice(self):
        a = [0] * 28
        a[HOME(1)] = 11
        for i in range(4):
            a[i] = 1
        a[BLACK_BAR] = 15
        state = GameState(tuple(a))

        dice = Dice.dice_throw(4, 4)
        r = WhiteGenerator().generate_moves(state, dice)
        assert len(r) == 35

        expected_set = set()
        for move_a in range(5):
            for move_b in range(5 - move_a):
                for move_c in range(5 - move_a - move_b):
                    move_d = 4 - move_a - move_b - move_c
                    e = a[:]
                    for src, moves in [(0, move_a), (1, move_b), (2, move_c), (3, move_d)]:
                        e[src] -= 1
                        e[src + 4 * moves] += 1
                    expected_set.add(tuple(e))
        assert {state.a for state in r} == expected_set

    def test_collect_with_waste(self):
        a = [0] * 28
        for i in range(HOME(3), COLLECT_END):
            a[i] = 1
        a[WHITE_BAG] = 12
        a[BLACK_BAR] = 15
        state = GameState(tuple(a))

        dice = Dice.dice_throw(4, 5)
        r = WhiteGenerator().generate_moves(state, dice)
        assert len(r) == 1

        e = a[:]
        e[WHITE_BAG] = 14
        e[HOME(3)] = 0
        e[HOME(2)] = 0
        assert r.pop().a == tuple(e)

    def test_collect_double(self):
        a = [0] * 28
        for i in range(COLLECT_START + 1, COLLECT_END):
            a[i] = 1
        a[HOME(6)] = 10
        a[BLACK_BAR] = 15
        state = GameState(tuple(a))

        dice = Dice.dice_throw(6, 6)
        r = WhiteGenerator().generate_moves(state, dice)
        assert len(r) == 1

        e = a[:]
        e[WHITE_BAG] = 4
        e[HOME(6)] = 6
        assert r.pop().a == tuple(e)

    def test_move_in_collect_phase(self):
        a = [0] * 28
        a[HOME(1)] = 1
        a[HOME(6)] = 1
        a[WHITE_BAG] = 13
        a[BLACK_BAR] = 15
        state = GameState(tuple(a))

        dice = Dice.dice_throw(3, 2)
        r = WhiteGenerator().generate_moves(state, dice)
        assert len(r) == 1

        e = a[:]
        e[HOME(1)] = 2
        e[HOME(6)] = 0
        assert r.pop().a == tuple(e)

    def test_partial_collect(self):
        a = [0] * 28
        a[HOME(1)] = 2
        a[HOME(2)] = -2
        a[HOME(3)] = 1
        a[HOME(5)] = 1
        a[WHITE_BAG] = 11
        a[BLACK_BAR] = 13
        state = GameState(tuple(a))

        dice = Dice.dice_throw(3, 3)
        r = WhiteGenerator().generate_moves(state, dice)
        assert len(r) == 1

        e = a[:]
        e[WHITE_BAG] = 12
        e[HOME(3)] = 0
        assert r.pop().a == tuple(e)

    def test_move_and_collect(self):
        a = [0] * 28
        a[HOME(3)] = 1
        a[HOME(6)] = 1
        a[WHITE_BAG] = 13
        a[BLACK_BAR] = 15
        state = GameState(tuple(a))

        dice = Dice.dice_throw(5, 3)
        r = WhiteGenerator().generate_moves(state, dice)
        assert len(r) == 1

        e = a[:]
        e[WHITE_BAG] = 14
        e[HOME(3)] = 0
        e[HOME(6)] = 0
        e[HOME(1)] = 1
        assert r.pop().a == tuple(e)

    def test_move_all_no_collect(self):
        a = [0] * 28
        a[HOME(3)] = 5
        a[HOME(1)] = -2
        a[WHITE_BAG] = 10
        a[BLACK_BAR] = 13
        state = GameState(tuple(a))

        dice = Dice.dice_throw(1, 1)
        r = WhiteGenerator().generate_moves(state, dice)
        assert len(r) == 1

        e = a[:]
        e[HOME(3)] = 1
        e[HOME(2)] = 4
        assert r.pop().a == tuple(e)

    def test_game_end(self):
        a = [0] * 28
        a[HOME(5)] = 1
        a[HOME(3)] = 1
        a[HOME(1)] = 1
        a[WHITE_BAG] = 12
        a[BLACK_BAR] = 15
        state = GameState(tuple(a))

        dice = Dice.dice_throw(5, 5)
        r = WhiteGenerator().generate_moves(state, dice)
        assert len(r) == 1

        e = a[:]
        e[WHITE_BAG] = 15
        e[HOME(5)] = 0
        e[HOME(3)] = 0
        e[HOME(1)] = 0
        actual = r.pop()
        assert actual.a == tuple(e)
        assert actual.score() == 3

    def test_count_moves_at_start(self):
        state = GameState.game_start()

        dice = Dice.dice_throw(6, 6)
        r = WhiteGenerator().generate_moves(state, dice)
        assert len(r) == 11

        dice = Dice.dice_throw(5, 5)
        r = WhiteGenerator().generate_moves(state, dice)
        assert len(r) == 4

        dice = Dice.dice_throw(1, 5)
        r = WhiteGenerator().generate_moves(state, dice)
        assert len(r) == 8
