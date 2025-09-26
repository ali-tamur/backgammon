from src.dice_source import Dice
from src.logger import DummyLogWriter
from src.player import *
from src.pretty_print import pretty_print


class GameManager:
    def __init__(self, white: Player, black: Player, dice_source: Dice,
                 logger=DummyLogWriter(), print_debug=False):
        self.players = [white, black]
        self.dice_source = dice_source
        self.logger = logger
        self.print_debug = print_debug

    def _run_once(self):
        state = GameState.game_start()
        on_turn = 0
        self.logger.start_game("123", self.players[0].name, self.players[1].name)
        if self.print_debug:
            print(pretty_print(state))
            input("...")
        while True:
            dice = self.dice_source.throw_dice()
            state = self.players[on_turn].move(state, dice)
            if self.print_debug:
                print(f"{dice[0]} {dice[1]}")
                print(pretty_print(state))
                print(f"\n\n")
                input("...")
            self.logger.move([dice[0], dice[1]], state)
            if state.score() is not None:
                break
            on_turn = 1 - on_turn

        result = state.score()
        if self.print_debug:
            print(f" Score: {result}")
            input("enter to end")
        self.logger.end_game(result)
        return result

    def run(self, n_times):
        with self.logger:
            for _ in range(n_times):
                self._run_once()
