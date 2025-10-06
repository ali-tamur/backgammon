from python.src.dice_source import Dice
from python.src.logger import DummyLogWriter
from python.src.player import *
from python.src.pretty_print import pretty_print
from python.src.state import score_state


class GameManager:
    def __init__(
        self,
        white: Player,
        black: Player,
        dice_source: Dice,
        logger=DummyLogWriter(),
        print_debug=False,
    ):
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
            d1, d2 = self.dice_source.throw_dice()
            state = self.players[on_turn].move(state, d1, d2)
            if self.print_debug:
                print(f"{d1}-{d2}")
                print(pretty_print(state))
                print("\n\n")
                input("...")
            self.logger.move([d1, d2], state)
            if score_state(state.a) is not None:
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
