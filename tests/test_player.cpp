#include <gtest/gtest.h>
#include "player.hpp"
#include "consts.hpp"

using namespace Player;

class PlayerTest : public ::testing::Test {
protected:
    void SetUp() override {
        player = makeNewPlayer();
    }

    PackedPlayer player;
};

struct ResourceTestData {
    Resource resource;
    uint8_t value;
};

class ResourcePackTest : public PlayerTest, public ::testing::WithParamInterface<ResourceTestData> {};

struct DevCardTestData {
    DevType devType;
    uint8_t value;
};

class DevCardPackTest : public PlayerTest, public ::testing::WithParamInterface<DevCardTestData> {};

struct BuyableTestData {
    BuyableType buyableType;
    bool shouldHaveResources;
    std::array<uint8_t, 5> resources;
};

class HasEnoughResourcesTest : public PlayerTest, public ::testing::WithParamInterface<BuyableTestData> {};

struct ValueTestData {
    uint8_t value;
};

class GameStateTest : public PlayerTest, public ::testing::WithParamInterface<ValueTestData> {};

struct StructureTestData {
    StructureType structureType;
    uint8_t value;
};

class StructurePackTest : public PlayerTest, public ::testing::WithParamInterface<StructureTestData> {};

struct BuyTestData {
    BuyableType buyableType;
    std::array<uint8_t, 5> initialResources;
    std::array<uint8_t, 5> expectedResources;
    uint8_t expectedSettlements;
    uint8_t expectedCities;
    uint8_t expectedRoads;
    uint8_t expectedVP;
    DevType devType;
};

class BuyTest : public PlayerTest, public ::testing::WithParamInterface<BuyTestData> {};

TEST_F(PlayerTest, InitialState) {
    for(int r = Resource::Brick; r <= Resource::Ore; ++r) {
        EXPECT_EQ(unpackResource(player, static_cast<Resource>(r)), 0);
    }
    for (uint8_t i = 0; i < static_cast<uint8_t>(DevType::NoDev); ++i) {
        auto d = static_cast<DevType>(i);
        EXPECT_EQ(unpackDevCard(player, d), 0);
    }
    EXPECT_EQ(unpackUsedKnights(player), 0);
    EXPECT_EQ(unpackLongestRoadLength(player), 0);
    EXPECT_FALSE(unpackLongestRoadFlag(player));
    EXPECT_FALSE(unpackLargestArmyFlag(player));
    EXPECT_EQ(unpackAvailableStructures(player, StructureType::Settlement), 5);
    EXPECT_EQ(unpackAvailableStructures(player, StructureType::City), 4);
    EXPECT_EQ(unpackAvailableStructures(player, StructureType::Road), 15);
}

TEST_P(ResourcePackTest, PackUnpackSingleResource) {
    auto param = GetParam();
    player = packResource(player, param.resource, param.value);
    EXPECT_EQ(unpackResource(player, param.resource), param.value);
}

INSTANTIATE_TEST_SUITE_P(
    AllResources,
    ResourcePackTest,
    ::testing::Values(
        ResourceTestData{Resource::Brick, 10},
        ResourceTestData{Resource::Lumber, 15},
        ResourceTestData{Resource::Wool, 8},
        ResourceTestData{Resource::Grain, 12},
        ResourceTestData{Resource::Ore, 5},
        ResourceTestData{Resource::Brick, 0},
        ResourceTestData{Resource::Lumber, 31},
        ResourceTestData{Resource::Wool, 16}
    )
);

TEST_P(DevCardPackTest, PackUnpackSingleDevCard) {
    auto param = GetParam();
    player = packDevCard(player, param.devType, param.value);
    EXPECT_EQ(unpackDevCard(player, param.devType), param.value);
}

INSTANTIATE_TEST_SUITE_P(
    AllDevCards,
    DevCardPackTest,
    ::testing::Values(
        DevCardTestData{DevType::Knight, 7},
        DevCardTestData{DevType::RoadBuilding, 2},
        DevCardTestData{DevType::YearOfPlenty, 1},
        DevCardTestData{DevType::Monopoly, 3},
        DevCardTestData{DevType::VictoryPoint, 5},
        DevCardTestData{DevType::Knight, 0},
        DevCardTestData{DevType::Knight, 15},
        DevCardTestData{DevType::RoadBuilding, 3},
        DevCardTestData{DevType::VictoryPoint, 7}
    )
);

TEST_P(GameStateTest, PackUnpackUsedKnights) {
    auto param = GetParam();
    player = packUsedKnights(player, param.value);
    EXPECT_EQ(unpackUsedKnights(player), param.value);
}

TEST_P(GameStateTest, PackUnpackLongestRoadLength) {
    auto param = GetParam();
    player = packLongestRoadLength(player, param.value);
    EXPECT_EQ(unpackLongestRoadLength(player), param.value);
}

INSTANTIATE_TEST_SUITE_P(
    GameStateValues,
    GameStateTest,
    ::testing::Values(
        ValueTestData{0},
        ValueTestData{5},
        ValueTestData{10},
        ValueTestData{15}
    )
);

TEST_F(PlayerTest, PackUnpackFlags) {
    player = packLongestRoadFlag(player, true);
    EXPECT_TRUE(unpackLongestRoadFlag(player));

    player = packLongestRoadFlag(player, false);
    EXPECT_FALSE(unpackLongestRoadFlag(player));

    player = packLargestArmyFlag(player, true);
    EXPECT_TRUE(unpackLargestArmyFlag(player));

    player = packLargestArmyFlag(player, false);
    EXPECT_FALSE(unpackLargestArmyFlag(player));
}

TEST_P(StructurePackTest, PackUnpackStructures) {
    auto param = GetParam();
    player = packAvailableStructures(player, param.structureType, param.value);
    EXPECT_EQ(unpackAvailableStructures(player, param.structureType), param.value);
}

INSTANTIATE_TEST_SUITE_P(
    AllStructures,
    StructurePackTest,
    ::testing::Values(
        StructureTestData{StructureType::Settlement, 0},
        StructureTestData{StructureType::Settlement, 3},
        StructureTestData{StructureType::Settlement, 5},
        StructureTestData{StructureType::City, 2},
        StructureTestData{StructureType::City, 4},
        StructureTestData{StructureType::Road, 10},
        StructureTestData{StructureType::Road, 15}
    )
);

TEST_F(PlayerTest, PackUnpackMaxValues) {
    player = packResource(player, Resource::Brick, 19);
    player = packResource(player, Resource::Lumber, 19);
    player = packResource(player, Resource::Wool, 19);
    player = packResource(player, Resource::Grain, 19);
    player = packResource(player, Resource::Ore, 19);

    player = packDevCard(player, DevType::Knight, 14);
    player = packDevCard(player, DevType::RoadBuilding, 2);
    player = packDevCard(player, DevType::YearOfPlenty, 2);
    player = packDevCard(player, DevType::Monopoly, 2);
    player = packDevCard(player, DevType::VictoryPoint, 5);

    player = packUsedKnights(player, 14);
    player = packLongestRoadLength(player, 15);
    player = packLongestRoadFlag(player, true);
    player = packLargestArmyFlag(player, true);

    player = packVictoryPoints(player, 16);

    EXPECT_EQ(unpackResource(player, Resource::Brick), 19);
    EXPECT_EQ(unpackResource(player, Resource::Lumber), 19);
    EXPECT_EQ(unpackResource(player, Resource::Wool), 19);
    EXPECT_EQ(unpackResource(player, Resource::Grain), 19);
    EXPECT_EQ(unpackResource(player, Resource::Ore), 19);

    EXPECT_EQ(unpackDevCard(player, DevType::Knight), 14);
    EXPECT_EQ(unpackDevCard(player, DevType::RoadBuilding), 2);
    EXPECT_EQ(unpackDevCard(player, DevType::YearOfPlenty), 2);
    EXPECT_EQ(unpackDevCard(player, DevType::Monopoly), 2);
    EXPECT_EQ(unpackDevCard(player, DevType::VictoryPoint), 5);

    EXPECT_EQ(unpackUsedKnights(player), 14);
    EXPECT_EQ(unpackLongestRoadLength(player), 15);
    EXPECT_TRUE(unpackLongestRoadFlag(player));
    EXPECT_TRUE(unpackLargestArmyFlag(player));
    EXPECT_EQ(unpackVictoryPoints(player), 16);
}

TEST_P(HasEnoughResourcesTest, CheckResourceRequirements) {
    auto param = GetParam();
    
    player = packResource(player, Resource::Brick, param.resources[0]);
    player = packResource(player, Resource::Lumber, param.resources[1]);
    player = packResource(player, Resource::Wool, param.resources[2]);
    player = packResource(player, Resource::Grain, param.resources[3]);
    player = packResource(player, Resource::Ore, param.resources[4]);
    
    EXPECT_EQ(hasEnoughResources(player, param.buyableType), param.shouldHaveResources);
}

INSTANTIATE_TEST_SUITE_P(
    AllBuyableTypes,
    HasEnoughResourcesTest,
    ::testing::Values(
        BuyableTestData{BuyableType::Road, false, {0, 0, 0, 0, 0}},
        BuyableTestData{BuyableType::Road, false, {1, 0, 0, 0, 0}},
        BuyableTestData{BuyableType::Road, true, {1, 1, 0, 0, 0}},
        BuyableTestData{BuyableType::Road, true, {5, 3, 0, 0, 0}},
        
        BuyableTestData{BuyableType::Settlement, false, {0, 0, 0, 0, 0}},
        BuyableTestData{BuyableType::Settlement, false, {1, 1, 1, 0, 0}},
        BuyableTestData{BuyableType::Settlement, true, {1, 1, 1, 1, 0}},
        BuyableTestData{BuyableType::Settlement, true, {5, 3, 2, 4, 0}},
        
        BuyableTestData{BuyableType::City, false, {0, 0, 0, 0, 0}},
        BuyableTestData{BuyableType::City, false, {0, 0, 0, 2, 0}},
        BuyableTestData{BuyableType::City, false, {0, 0, 0, 2, 2}},
        BuyableTestData{BuyableType::City, true, {0, 0, 0, 2, 3}},
        BuyableTestData{BuyableType::City, true, {0, 0, 0, 5, 8}},
        
        BuyableTestData{BuyableType::DevCard, false, {0, 0, 0, 0, 0}},
        BuyableTestData{BuyableType::DevCard, false, {0, 0, 1, 1, 0}},
        BuyableTestData{BuyableType::DevCard, true, {0, 0, 1, 1, 1}},
        BuyableTestData{BuyableType::DevCard, true, {0, 0, 3, 2, 4}}
    )
);

TEST_P(BuyTest, BuyItems) {
    auto param = GetParam();
    
    player = packResource(player, Resource::Brick, param.initialResources[0]);
    player = packResource(player, Resource::Lumber, param.initialResources[1]);
    player = packResource(player, Resource::Wool, param.initialResources[2]);
    player = packResource(player, Resource::Grain, param.initialResources[3]);
    player = packResource(player, Resource::Ore, param.initialResources[4]);
    
    if (param.buyableType == BuyableType::DevCard) {
        buy(player, param.buyableType, param.devType);
        EXPECT_EQ(unpackDevCard(player, param.devType), 1);
    } else {
        buy(player, param.buyableType);
    }
    
    EXPECT_EQ(unpackResource(player, Resource::Brick), param.expectedResources[0]);
    EXPECT_EQ(unpackResource(player, Resource::Lumber), param.expectedResources[1]);
    EXPECT_EQ(unpackResource(player, Resource::Wool), param.expectedResources[2]);
    EXPECT_EQ(unpackResource(player, Resource::Grain), param.expectedResources[3]);
    EXPECT_EQ(unpackResource(player, Resource::Ore), param.expectedResources[4]);
    
    EXPECT_EQ(unpackAvailableStructures(player, StructureType::Settlement), param.expectedSettlements);
    EXPECT_EQ(unpackAvailableStructures(player, StructureType::City), param.expectedCities);
    EXPECT_EQ(unpackAvailableStructures(player, StructureType::Road), param.expectedRoads);
    EXPECT_EQ(unpackVictoryPoints(player), param.expectedVP);
}

INSTANTIATE_TEST_SUITE_P(
    AllPurchases,
    BuyTest,
    ::testing::Values(
        BuyTestData{BuyableType::Road, {3,2,1,0,0}, {2,1,1,0,0}, 5,4,14, 0, DevType::Knight},
        BuyTestData{BuyableType::Settlement, {2,3,2,1,0}, {1,2,1,0,0}, 4,4,15, 1, DevType::Knight},
        BuyTestData{BuyableType::City, {0,0,0,4,5}, {0,0,0,2,2}, 6,3,15, 1, DevType::Knight},
        BuyTestData{BuyableType::DevCard, {0,0,2,3,1}, {0,0,1,2,0}, 5,4,15, 0, DevType::Knight}
    )
);

TEST_F(PlayerTest, BuyMultipleDevCards) {
    player = packResource(player, Resource::Wool, 5);
    player = packResource(player, Resource::Grain, 5);
    player = packResource(player, Resource::Ore, 5);
    
    buy(player, BuyableType::DevCard, DevType::Knight);
    buy(player, BuyableType::DevCard, DevType::RoadBuilding);
    buy(player, BuyableType::DevCard, DevType::VictoryPoint);
    
    EXPECT_EQ(unpackResource(player, Resource::Wool), 2);
    EXPECT_EQ(unpackResource(player, Resource::Grain), 2);
    EXPECT_EQ(unpackResource(player, Resource::Ore), 2);
    
    EXPECT_EQ(unpackDevCard(player, DevType::Knight), 1);
    EXPECT_EQ(unpackDevCard(player, DevType::RoadBuilding), 1);
    EXPECT_EQ(unpackDevCard(player, DevType::VictoryPoint), 1);
    EXPECT_EQ(unpackDevCard(player, DevType::YearOfPlenty), 0);
    EXPECT_EQ(unpackDevCard(player, DevType::Monopoly), 0);
}

TEST_F(PlayerTest, IntegratedPurchaseScenario) {
    player = packResource(player, Resource::Brick, 10);
    player = packResource(player, Resource::Lumber, 10);
    player = packResource(player, Resource::Wool, 10);
    player = packResource(player, Resource::Grain, 10);
    player = packResource(player, Resource::Ore, 10);
    
    buy(player, BuyableType::Road);
    buy(player, BuyableType::Road);
    buy(player, BuyableType::Settlement);
    buy(player, BuyableType::Settlement);
    buy(player, BuyableType::City);
    buy(player, BuyableType::DevCard, DevType::VictoryPoint);
    
    EXPECT_EQ(unpackResource(player, Resource::Brick), 6);
    EXPECT_EQ(unpackResource(player, Resource::Lumber), 6);
    EXPECT_EQ(unpackResource(player, Resource::Wool), 7);
    EXPECT_EQ(unpackResource(player, Resource::Grain), 5);
    EXPECT_EQ(unpackResource(player, Resource::Ore), 6);
    
    EXPECT_EQ(unpackAvailableStructures(player, StructureType::Road), 13);
    EXPECT_EQ(unpackAvailableStructures(player, StructureType::Settlement), 4);
    EXPECT_EQ(unpackAvailableStructures(player, StructureType::City), 3);
    
    EXPECT_EQ(unpackVictoryPoints(player), 4);
    EXPECT_EQ(unpackDevCard(player, DevType::VictoryPoint), 1);
}



