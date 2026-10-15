#include "pretty_print.h"

#include <format>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

#include "assertion.h"

// NOLINTBEGIN(readability-magic-numbers)
namespace bg {

namespace {

inline auto utf8_char_len(char ch) -> size_t {
  const auto c = static_cast<unsigned char>(ch);
  if ((c & 0x80U) == 0) {  // 0xxxxxxx
    return 1;
  }
  if ((c & 0xE0U) == 0xC0) {  // 110xxxxx
    return 2;
  }
  if ((c & 0xF0U) == 0xE0) {  // 1110xxxx
    return 3;
  }
  if ((c & 0xF8U) == 0xF0) {  // 11110xxx
    return 4;
  }
  return 1;
}

auto split_utf8(std::string_view st) -> std::vector<std::string> {
  std::vector<std::string> result;
  for (size_t i = 0; i < st.size();) {
    size_t len = utf8_char_len(st[i]);
    result.emplace_back(st.substr(i, len));
    i += len;
  }
  return result;
}

class BoardPrinter {
  static constexpr std::array<size_t, 24> CELL_TO_SCREEN_COOR = {
      27, 25, 23, 21, 19, 17, 12, 10, 8, 6, 4, 2, 2, 4, 6, 8, 10, 12, 17, 19, 21, 23, 25, 27};
  static constexpr std::string_view top_row{"┌─────────────┬┬─────────────┐"};
  static constexpr std::string_view mid_row{"│             ││             │"};
  static constexpr std::string_view bot_row{"└─────────────┴┴─────────────┘"};
  static constexpr size_t TOP_ROW = 1;
  static constexpr size_t BOT_ROW = 13;
  static constexpr size_t WIDTH = 30;
  static constexpr size_t HEIGHT = 15;

 public:
  explicit BoardPrinter(const Board& board) : board_(board) {
    cells_.push_back(split_utf8(top_row));
    for (size_t row = 1; row < HEIGHT - 1; ++row) {
      cells_.push_back(split_utf8(mid_row));
    }
    cells_.push_back(split_utf8(bot_row));
  }

  void update2_(size_t cell, size_t start_row,
                const std::function<size_t(size_t, size_t)>& row_calc) {
    size_t col = CELL_TO_SCREEN_COOR[cell];
    int8_t val = board_[cell];
    if (val == 0) {
      cells_[start_row][col] = ".";
      return;
    }
    std::string ch = (val > 0) ? "●" : "○";
    val = static_cast<int8_t>(std::abs(val));
    for (size_t v = 0; std::cmp_less(v, val); ++v) {
      if (std::cmp_greater_equal(v, val)) {
        break;
      }
      if (v == 5) {
        ch = std::to_string(val - 5);
      }
      cells_[row_calc(start_row, v)][col] = ch;
    }
  }

  void update_() {
    for (size_t cell = 0; cell < 12; ++cell) {
      update2_(cell, TOP_ROW, [](auto a, auto b) -> auto { return a + b; });
    }
    for (size_t cell = 12; cell < 24; ++cell) {
      update2_(cell, BOT_ROW, [](auto a, auto b) -> auto { return a - b; });
    }
  }

  auto build() -> std::string {
    update_();
    std::ostringstream out;
    for (size_t row = 0; row < HEIGHT; ++row) {
      for (size_t col = 0; col < WIDTH; ++col) {
        out << cells_[row][col];
      }
      out << "\n";
    }
    return out.str();
  }

 private:
  std::vector<std::vector<std::string>> cells_;
  const Board& board_;
};
}  // namespace

auto print_board(const Board& b) -> std::string {
  BoardPrinter printer(b);
  return printer.build();
}

}  // namespace bg
// NOLINTEND(readability-magic-numbers)