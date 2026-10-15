
#include "bits.h"

#include "configs.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::ElementsAreArray;

// NOLINTBEGIN(readability-magic-numbers)
namespace bg {

template <typename Config>
auto to_collection(Bits<Config> b) -> std::vector<int32_t> {
  auto indices = std::vector<int32_t>{};
  for (auto i : b) {
    indices.push_back(i);
  }
  return indices;
}

TEST(BitsTest, SetUnset) {
  Bits<WhiteConfig> w{};
  EXPECT_TRUE(to_collection<WhiteConfig>(w).empty());

  w.set(7);
  EXPECT_THAT(to_collection<WhiteConfig>(w), ElementsAreArray({7}));

  w.set(9);
  EXPECT_THAT(to_collection<WhiteConfig>(w), ElementsAreArray({7, 9}));

  w.set(5);
  EXPECT_THAT(to_collection<WhiteConfig>(w), ElementsAreArray({5, 7, 9}));

  w.unset(7);
  EXPECT_THAT(to_collection<WhiteConfig>(w), ElementsAreArray({5, 9}));

  w.unset(9);
  EXPECT_THAT(to_collection<WhiteConfig>(w), ElementsAreArray({5}));

  w.unset(5);
  EXPECT_TRUE(to_collection<WhiteConfig>(w).empty());

  Bits<BlackConfig> b{};
  EXPECT_TRUE(to_collection<BlackConfig>(b).empty());

  b.set(7);
  EXPECT_THAT(to_collection<BlackConfig>(b), ElementsAreArray({7}));

  b.set(9);
  EXPECT_THAT(to_collection<BlackConfig>(b), ElementsAreArray({9, 7}));

  b.set(5);
  EXPECT_THAT(to_collection<BlackConfig>(b), ElementsAreArray({9, 7, 5}));

  b.unset(7);
  EXPECT_THAT(to_collection<BlackConfig>(b), ElementsAreArray({9, 5}));

  b.unset(9);
  EXPECT_THAT(to_collection<BlackConfig>(b), ElementsAreArray({5}));

  b.unset(5);
  EXPECT_TRUE(to_collection<BlackConfig>(b).empty());
}

}  // namespace bg
// NOLINTEND(readability-magic-numbers)
