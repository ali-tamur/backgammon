from typing import Tuple

import pytest

from src.state import *


@pytest.fixture
def collecting_state() -> GameState:
    s = GameState.game_start()
    a = [0] * COLLECT_START
    for i in range(COLLECT_START, COLLECT_END):
        a.append(2)
    a += [0, 0, 0, 0]
    a[WHITE_BAG] = 3
    a[0] = -15
    return GameState(tuple(a))


@pytest.fixture
def on_bar_state() -> GameState:
    s = GameState.game_start()
    a = list(s.a)
    a[0] -= 1
    a[WHITE_BAR] = 1
    return GameState(tuple(a))


@pytest.fixture(params=
                [(b, st) for b in [True, False] for st in ["normal", "gammon", "backgammon-1", "backgammon-2"]])
def game_score(request) -> Tuple[GameState, int]:
    a = [0] * 28
    a[WHITE_BAG] = 15
    b, st = request.param
    should_score = None
    if st == "normal":
        should_score = 1
        a[0] = -1
        a[BLACK_BAG] = 14
    elif st == "gammon":
        should_score = 2
        a[0] = -15
        a[BLACK_BAG] = 0
    elif st == "backgammon-1":
        should_score = 3
        a[0] = -14
        a[COLLECT_START] = -1
    elif st == "backgammon-2":
        should_score = 3
        a[0] = -14
        a[BLACK_BAR] = 1
    if b:
        return GameState(a), should_score
    else:
        return GameState(a).opponent_state(), -should_score


@pytest.fixture
def game_lost(game_won):
    s_opp = game_won.opponent_state()
    return s_opp


class TestGameState:
    def test_game_start(self):
        s = GameState.game_start()
        player_stones = sum([n for n in s.a[:24] if n > 0])
        player_stones += s.player_on_bar()
        player_stones += s.player_collected()
        assert player_stones == 15
        assert s.score() is None

        opponent_stones = abs(sum([n for n in s.a[:24] if n < 0]))
        opponent_stones += s.opponent_on_bar()
        opponent_stones += s.opponent_collected()
        assert opponent_stones == 15

    def test_opponent_state(self):
        s = GameState.game_start()
        s_opp = s.opponent_state()
        assert s_opp.score() is None
        assert s == s_opp

    def test_opponent_state_2(self):
        a = [0] * 28
        a[COLLECT_START] = 15
        a[0] = -15
        state = GameState(tuple(a))
        s_opp = state.opponent_state()
        s_opp_2 = s_opp.opponent_state()
        assert state == s_opp_2

    def test_win_lose(self, game_score):
        state, should_score = game_score
        assert state.score() == should_score

    def test_no_score(self):
        assert GameState.game_start().score() is None

    def test_move_state(self, collecting_state, on_bar_state):
        s = GameState.game_start()
        assert s.move_state() == STATE.NORMAL

        assert collecting_state.move_state() == STATE.COLLECTING
        assert on_bar_state.move_state() == STATE.ON_BAR
