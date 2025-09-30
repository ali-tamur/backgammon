
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "state.h"

using namespace ::testing;

namespace bg {

std::vector<int32_t> to_collection(Dice d, bool will_use_max) {
    auto rolls = std::vector<int32_t>{};
    for (auto d : d.values(will_use_max)) {
        rolls.push_back(d);
    }
    return rolls;
}

TEST(StateTest, Hello) { 
    EXPECT_EQ(2, 2);
}

} // namespace bg