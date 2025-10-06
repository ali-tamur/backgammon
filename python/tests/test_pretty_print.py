from python.src.pretty_print import pretty_print
from python.src.state import GameState


class TestPrettyPrint:
    def test_1(self):
        st = GameState.game_start()
        result = pretty_print(st)
        expected = """\
┌─────────────┬┬─────────────┐
│ ○ . . . ● . ││ ● . . . . ○ │
│ ○       ●   ││ ●         ○ │
│ ○       ●   ││ ●           │
│ ○           ││ ●           │
│ ○           ││ ●           │
│             ││             │
│ ●           ││ ○           │
│ ●           ││ ○           │
│ ●       ○   ││ ○           │
│ ●       ○   ││ ○         ● │
│ ● . . . ○ . ││ ○ . . . . ● │
└─────────────┴┴─────────────┘"""
        expected.strip("\n")
        expected.strip(" ")
        assert result == expected
