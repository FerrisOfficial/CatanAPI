#include <gtest/gtest.h>

#include "game_simulation/packedBank.hpp"

using namespace Bank;

TEST(BankTest, MakeNewBankCounts) {
    PackedBank b = makeNewBank();

    // Resources
    EXPECT_EQ(unpackResource(b, Resource::Brick), 19);
    EXPECT_EQ(unpackResource(b, Resource::Lumber), 19);
    EXPECT_EQ(unpackResource(b, Resource::Wool), 19);
    EXPECT_EQ(unpackResource(b, Resource::Grain), 19);
    EXPECT_EQ(unpackResource(b, Resource::Ore), 19);

    // Dev cards
    EXPECT_EQ(unpackDevCard(b, DevType::Knight), 14);
    EXPECT_EQ(unpackDevCard(b, DevType::RoadBuilding), 2);
    EXPECT_EQ(unpackDevCard(b, DevType::YearOfPlenty), 2);
    EXPECT_EQ(unpackDevCard(b, DevType::Monopoly), 2);
    EXPECT_EQ(unpackDevCard(b, DevType::VictoryPoint), 5);

    // Cached total
    EXPECT_EQ(unpackTotalDevCount(b), 25);
    EXPECT_EQ(computeTotalDevCards(b), 25);
}

TEST(BankTest, PackUnpackResource) {
    PackedBank b = makeNewBank();
    b = packResource(b, Resource::Brick, 5);
    EXPECT_EQ(unpackResource(b, Resource::Brick), 5);
    b = packResource(b, Resource::Ore, 0);
    EXPECT_EQ(unpackResource(b, Resource::Ore), 0);
}

TEST(BankTest, PackUnpackDevCardAndCachedTotal) {
    PackedBank b = makeNewBank();
    // Decrease knights from 14 to 10
    b = packDevCard(b, DevType::Knight, 10);
    // recompute cached total and write it
    uint8_t newTotal = static_cast<uint8_t>(computeTotalDevCards(b));
    b = packTotalDevCount(b, newTotal);

    EXPECT_EQ(unpackDevCard(b, DevType::Knight), 10);
    EXPECT_EQ(unpackTotalDevCount(b), newTotal);
    EXPECT_EQ(computeTotalDevCards(b), newTotal);
}
