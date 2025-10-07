import click

from python.src.commands import c_benchmark, c_interactive, c_load_data, c_play


@click.group()
def cli():
    pass


def player_options(f):
    f = click.option(
        "--white", default="random", help="The player that will play the white pieces"
    )(f)
    f = click.option("--white-model", default="", help="The path to the model file to load")(f)
    f = click.option(
        "--black", default="random", help="The player that will play the black pieces"
    )(f)
    f = click.option("--black-model", default="", help="The path to the model file to load")(f)
    return f


@cli.command()
@player_options
@click.option("--count", default=1000, help="Number of games to play")
@click.option("--db-path", default="/usr/data/backgammon/db.sqlite")
def play(white, white_model, black, black_model, count, db_path):
    c_play(white, white_model, black, black_model, count, db_path)


@cli.command()
@player_options
@click.option("--count", default=1000, help="Number of games")
@click.option("--seed", default=31415, help="Random seed to use")
def benchmark(white, white_model, black, black_model, count, seed):
    c_benchmark(white, white_model, black, black_model, count, seed)


@cli.command()
@player_options
def interactive(white, white_model, black, black_model):
    c_interactive(white, white_model, black, black_model)


@cli.command()
@click.argument("player-name")
@click.option("--limit", default=1000, help="Number of games to load")
@click.option("--db-path", default="/usr/data/backgammon/db.sqlite")
def load_data(player_name, limit, db_path):
    c_load_data(player_name, limit, db_path)
