
load("@hedron_compile_commands//:refresh_compile_commands.bzl", "refresh_compile_commands")

refresh_compile_commands(
    name = "refresh_cc_for_backgammon",
    targets = {
        "//cc/src:game_manager_benchmark": "",
        "//cc/src:play_games_main": "", },
    exclude_external_sources = True,
)

