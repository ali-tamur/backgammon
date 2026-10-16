import pytest

from bg.db import close_db, open_db


@pytest.fixture
def db(tmp_path):
    """A fresh, empty database for one test."""
    yield open_db(tmp_path / "test.db")
    close_db()
