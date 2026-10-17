import pytest

from bg.db import create_db, open_db


@pytest.fixture
def db(tmp_path):
    """A fresh database for one test."""
    path = tmp_path / "test.db"
    create_db(path)
    with open_db(path) as db:
        yield db
