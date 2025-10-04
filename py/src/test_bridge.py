import sys
import os

def try_load():
    try:
        print(f"TRY LOAD STARTS")
        cc_module_path = "/tmp/bazelbin/cc/src"
        sys.path.insert(0, cc_module_path)
    except Exception:
        print(f"EXCEPT")
    try:
        import backgammon as bg
        print("Successfully loaded C++ module: backgammon_core")
    except ImportError as e:
        print(f"Error loading C++ module: {e}")


    #     successor_boards = bg.white_move(initial_board, 3, 1)

    sys.exit(1)

if __name__ == '__main__':
    try_load()
