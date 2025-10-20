#include "pretty_print.h"

#include <iostream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace ::testing;

namespace bg {

TEST(PrettyPrintTest, Basic) {
  std::string result = "\n" + print_board(GAME_START);
  std::cout << result << "\n";
  std::string expected = R"(
┌─────────────┬┬─────────────┐
│ ● . . . ○ . ││ ○ . . . . ● │
│ ●       ○   ││ ○         ● │
│ ●       ○   ││ ○           │
│ ●           ││ ○           │
│ ●           ││ ○           │
│             ││             │
│             ││             │
│             ││             │
│ ○           ││ ●           │
│ ○           ││ ●           │
│ ○       ●   ││ ●           │
│ ○       ●   ││ ●         ○ │
│ ○ . . . ● . ││ ● . . . . ○ │
└─────────────┴┴─────────────┘
)";
  EXPECT_EQ(result, expected);
}

}  // namespace bg