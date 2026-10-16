import pytest

from bg.db import open_db


@pytest.fixture
def db(tmp_path):
    """A fresh, empty database for one test."""
    with open_db(tmp_path / "test.db") as db:
        yield db
