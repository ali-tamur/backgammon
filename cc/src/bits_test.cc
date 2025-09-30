
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "bits.h"

using namespace ::testing;

namespace bg {

std::vector<int32_t> to_collection(Bits b) {
    auto indices = std::vector<int32_t>{};
    for (auto i : b) {
        indices.push_back(i);
    }
    return indices;
}

TEST(BitsTest, SetUnset) { 
    Bits b{};
    EXPECT_TRUE(to_collection(b).empty());

    b.set(7);
    EXPECT_THAT(b, ElementsAreArray({7}));

    b.set(9);
    EXPECT_THAT(b, ElementsAreArray({7, 9}));

    b.set(5);
    EXPECT_THAT(b, ElementsAreArray({5, 7, 9}));

    b.unset(7);
    EXPECT_THAT(b, ElementsAreArray({5, 9}));

    b.unset(9);
    EXPECT_THAT(b, ElementsAreArray({5}));

    b.unset(5);
    EXPECT_TRUE(to_collection(b).empty());
}



}  // namespace bg