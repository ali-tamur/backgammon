
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include "configs.h"

namespace py = pybind11;
using namespace bg;

PYBIND11_MODULE(backgammon, m) {
  m.doc() = "Pybind11 bridge for the zero-cost C++ Backgammon Move Generator.";

  m.def("white_moves", &white_move,
        "Generates all unique, valid resulting boards for the White player.",
        py::arg("current_board"), py::arg("d1"), py::arg("d2"));

  m.def("black_moves", &black_move,
        "Generates all unique, valid resulting boards for the Black player.",
        py::arg("current_board"), py::arg("d1"), py::arg("d2"));
}
