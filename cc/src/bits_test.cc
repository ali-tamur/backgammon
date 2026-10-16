
#include "bits.h"

#include "configs.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::ElementsAreArray;

// NOLINTBEGIN(readability-magic-numbers)
namespace bg {

auto to_collection(Bits b) -> std::vector<int32_t> {
  auto indices = std::vector<int32_t>{};
  for (auto i : b) {
    indices.push_back(i);
  }
  return indices;
}

TEST(BitsTest, SetUnset) {
  Bits w{};
  EXPECT_TRUE(to_collection(w).empty());

  w.set(7);
  EXPECT_THAT(to_collection(w), ElementsAreArray({7}));

  w.set(9);
  EXPECT_THAT(to_collection(w), ElementsAreArray({7, 9}));

  w.set(5);
  EXPECT_THAT(to_collection(w), ElementsAreArray({5, 7, 9}));

  w.unset(7);
  EXPECT_THAT(to_collection(w), ElementsAreArray({5, 9}));

  w.unset(9);
  EXPECT_THAT(to_collection(w), ElementsAreArray({5}));

  w.unset(5);
  EXPECT_TRUE(to_collection(w).empty());
}

}  // namespace bg
// NOLINTEND(readability-magic-numbers)
