
#include "bits.h"

#include "configs.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace ::testing;

namespace bg {

template <typename Config> std::vector<int32_t> to_collection(Bits<Config> b) {
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

#if 0
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
  EXPECT_THAT(to_collection<WhiteConfig>(b), ElementsAreArray({7}));

  b.set(9);
  EXPECT_THAT(to_collection<WhiteConfig>(b), ElementsAreArray({9, 7}));

  b.set(5);
  EXPECT_THAT(to_collection<WhiteConfig>(b), ElementsAreArray({9, 7, 5}));

  b.unset(7);
  EXPECT_THAT(to_collection<WhiteConfig>(b), ElementsAreArray({9, 5}));

  b.unset(9);
  EXPECT_THAT(to_collection<WhiteConfig>(b), ElementsAreArray({5}));

  b.unset(5);
  EXPECT_TRUE(to_collection<BlackConfig>(b).empty());
#endif
}

} // namespace bg
