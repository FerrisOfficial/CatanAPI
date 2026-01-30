#include <gtest/gtest.h>

#include "utils/randomDevice.hpp"

TEST(RandomDeviceTest, UniformU32RangeWithinBounds) {
    for (int i = 0; i < 1000; ++i) {
        uint32_t val = RandomDevice::uniform_u32_range(5, 10);
        EXPECT_GE(val, 5u);
        EXPECT_LE(val, 10u);
    }
}

TEST(RandomDeviceTest, UniformU32ExclusiveUpperBound) {
    for (int i = 0; i < 1000; ++i) {
        uint32_t val = RandomDevice::uniform_u32(10);
        EXPECT_GE(val, 0u);
        EXPECT_LT(val, 10u);
    }
}

TEST(RandomDeviceTest, RollDicesRange) {
    for (int i = 0; i < 1000; ++i) {
        int roll = RandomDevice::rollDices();
        EXPECT_GE(roll, 2);
        EXPECT_LE(roll, 12);
    }
}

TEST(RandomDeviceTest, DifferentCallsProduceDifferentResults) {
    std::set<uint32_t> results;
    for (int i = 0; i < 100; ++i) {
        results.insert(RandomDevice::uniform_u32(1000));
    }

    EXPECT_GE(results.size(), 80u);
}

TEST(RandomDeviceTest, RollDicesDistribution) {
    std::map<int, int> rollCounts;
    const int numRolls = 10'000'000;
    for (int i = 0; i < numRolls; ++i) {
        int roll = RandomDevice::rollDices();
        rollCounts[roll]++;
    }

    for (int roll = 2; roll <= 12; ++roll) {
        EXPECT_GT(rollCounts[7], rollCounts[2]);
        EXPECT_GT(rollCounts[7], rollCounts[3]);
        EXPECT_GT(rollCounts[7], rollCounts[11]);
        EXPECT_GT(rollCounts[7], rollCounts[12]);
    }
}
