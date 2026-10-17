from compression import zstd

import pytest

from bg.__main__ import main
from bg.db import Network, Player, open_db, to_text
from proto import config_pb2


def test_new_db_creates_the_database_and_its_directory(tmp_path):
    db = tmp_path / "data" / "backgammon.db"
    assert main(["--db", str(db), "new-db"]) == 0
    with open_db(db):
        assert [p.type for p in Player.select()] == ["RANDOM"]
    assert not (tmp_path / "data" / "archive").exists()


def test_new_db_archives_the_old_one_and_clears_the_cache(tmp_path, capsys):
    db = tmp_path / "backgammon.db"
    main(["--db", str(db), "new-db"])
    with open_db(db):
        Network.create(
            onnx=b"old", config=to_text(config_pb2.NetworkConfig()), input_encoding="", output_layout="", git_commit=""
        )
    old = db.read_bytes()
    (tmp_path / "cache" / "abc").mkdir(parents=True)

    main(["--db", str(db), "new-db"])
    main(["--db", str(db), "new-db"])  # twice in the same second: two archives

    with open_db(db):
        assert Network.select().count() == 0
    assert not (tmp_path / "cache").exists()
    archives = [zstd.decompress(a.read_bytes()) for a in (tmp_path / "archive").iterdir()]
    assert len(archives) == 2
    assert old in archives
    assert "to undo" in capsys.readouterr().out


def test_unknown_command_is_an_error(capsys):
    with pytest.raises(SystemExit):
        main(["frobnicate"])
    assert "invalid choice" in capsys.readouterr().err
