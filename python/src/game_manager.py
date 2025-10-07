from random import Random, randint

from python.src.dice_source import Dice
from python.src.logger import DummyLogWriter, LogWriter
from python.src.player import Player
from python.src.pretty_print import pretty_print
from python.src.state import GameState, score_state


class GameManager:
    def __init__(
        self,
        white: Player,
        black: Player,
        dice_source: Dice,
        logger: DummyLogWriter | LogWriter = DummyLogWriter(),  # noqa: E501
        print_debug=False,
    ):
        self.players = [white, black]
        self.dice_source = dice_source
        self.logger = logger
        self.print_debug = print_debug
        self.r = Random()

    def _run_once(self) -> int:
        game_id = self.r.randint(0, 1 << 64 - 1)
        state = GameState.game_start()
        on_turn = 0
        self.logger.start_game(game_id, self.players[0].name, self.players[1].name)
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
            self.logger.move(10 * d1 + d2, state.a)
            if score_state(state.a) is not None:
                break
            on_turn = 1 - on_turn

        result = state.score()
        if self.print_debug:
            print(f" Score: {result}")
            input("enter to end")
        self.logger.end_game(result)
        return result or 0

    def run(self, n_times):
        white_score = 0
        black_score = 0
        with self.logger:
            for i in range(n_times):
                this_score = self._run_once()
                if self.print_debug:
                    print(f"{i}th game: {this_score} ", flush=True)
                if this_score > 0:
                    white_score += this_score
                else:
                    black_score -= this_score
        print(f"Games: {n_times} White: {white_score} Black: {black_score}", flush=True)
