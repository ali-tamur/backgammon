import random
from ctypes import HRESULT

from state import GameState
from player import *
from move_generator import generate_moves
from dice_source import DiceSource
from logger import LogWriter
from typing import Optional

class GameManager:
    def __init__(self, alice:Player, bob:Player, dice_source:DiceSource, logger:Optional[LogWriter] = None):
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
                self.logger.move([d1,d2], state)
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

if __name__ == "__main__":
    for i in range(20):
        log_file = 'mylog.avro'
        my_schema = 'log.avsc'
        mylogger = LogWriter(log_file, my_schema)
        seed_a = random.Random(18+i)
        seed_b = random.Random(456+i)
        alice = RandomPlayer("alice", seed_a)
        bob = RandomPlayer("bob", seed_b)
        dice_source = DiceSource(789+i)
        game_manager = GameManager(alice, bob, dice_source, mylogger)
        score = game_manager.run_once()
        #print(score)
