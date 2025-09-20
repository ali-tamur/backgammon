from typing import Optional

from src.dice_source import DiceSource
from src.logger import LogWriter
from src.player import *


class GameManager:
    def __init__(self, alice: Player, bob: Player, dice_source: DiceSource, logger: Optional[LogWriter] = None):
        self.players = [alice, bob]
        self.dice_source = dice_source
        self.logger = logger

    def run_once(self):
        state = GameState.game_start()
        on_turn = 0
        if self.logger:
            self.logger.start_game("123", self.players[0].name, self.players[1].name)
        while True:
            d1, d2 = self.dice_source.throw_dice()
            dice = Dice.dice_throw(d1, d2)
            state = self.players[on_turn].move(state, dice)
            if self.logger:
                self.logger.move([d1, d2], state)
            if state.score() is not None:
                break
            on_turn = 1 - on_turn
            state = state.opponent_state()

        result = state.score()
        if on_turn == 1:
            result = -result
            state = state.opponent_state()
        if self.logger:
            self.logger.end_game(result)
        return result
