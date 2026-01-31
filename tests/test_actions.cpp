#include <gtest/gtest.h>

#include "game_simulation/actions.hpp"

using namespace Action;

class ActionPackTest : public ::testing::TestWithParam<
                           std::tuple<ActionType, PlayerId, uint8_t, uint8_t>> {
};

TEST_P(ActionPackTest, TypeAndPlayerIDPacking) {
    auto [actionType, playerId, expectedType, expectedPlayer] = GetParam();

    PackedAction action = 0;
    action = packType(action, actionType);
    action = packPlayerID(action, playerId);

    EXPECT_EQ(unpackType(action), actionType);
    EXPECT_EQ(unpackPlayerID(action), playerId);
}

INSTANTIATE_TEST_SUITE_P(
    ActionPackTests, ActionPackTest,
    ::testing::Values(
        std::make_tuple(ActionType::RollDice, PlayerId::Player0, 1, 0),
        std::make_tuple(ActionType::EndTurn, PlayerId::Player1, 2, 1),
        std::make_tuple(ActionType::MoveRobber, PlayerId::Player0, 3, 0),
        std::make_tuple(ActionType::BuildRoad, PlayerId::Player1, 5, 1),
        std::make_tuple(ActionType::BuyDevCard, PlayerId::Player0, 8, 0)));

class ResourcePackTest
    : public ::testing::TestWithParam<std::tuple<Resource, uint8_t>> {};

TEST_P(ResourcePackTest, ResourcePacking) {
    auto [resource, value] = GetParam();

    PackedAction action = 0;
    action = packResource(action, resource, value);

    EXPECT_EQ(unpackResource(action, resource), value);

    // Ensure other resources are not affected
    for (int r = 0; r <= 4; r++) {
        if (r != static_cast<int>(resource)) {
            EXPECT_EQ(unpackResource(action, static_cast<Resource>(r)), 0);
        }
    }
}

INSTANTIATE_TEST_SUITE_P(
    ResourcePackTests, ResourcePackTest,
    ::testing::Values(std::make_tuple(Resource::Brick, 5),
                      std::make_tuple(Resource::Lumber, 10),
                      std::make_tuple(Resource::Wool, 15),
                      std::make_tuple(Resource::Grain, 20),
                      std::make_tuple(Resource::Ore, 25),
                      std::make_tuple(Resource::Brick, 31),  // Max value
                      std::make_tuple(Resource::Ore, 0)      // Min value
                      ));

class ArgumentPackTest
    : public ::testing::TestWithParam<std::tuple<uint8_t, uint8_t>> {};

TEST_P(ArgumentPackTest, ArgumentPacking) {
    auto [arg1, arg2] = GetParam();

    PackedAction action = 0;
    action = packArg1(action, arg1);
    action = packArg2(action, arg2);

    EXPECT_EQ(unpackArg1(action), arg1);
    EXPECT_EQ(unpackArg2(action), arg2);
}

INSTANTIATE_TEST_SUITE_P(ArgumentPackTests, ArgumentPackTest,
                         ::testing::Values(std::make_tuple(0, 0),
                                           std::make_tuple(127, 128),
                                           std::make_tuple(255, 255),
                                           std::make_tuple(42, 200),
                                           std::make_tuple(1, 254)));

TEST(ActionTest, ComplexActionCreation) {
    PackedAction action = 0;

    // Pack action with multiple resources for DiscardResources action
    action = packType(action, ActionType::DiscardResources);
    action = packPlayerID(action, PlayerId::Player0);
    action = packResource(action, Resource::Brick, 2);
    action = packResource(action, Resource::Lumber, 1);
    action = packResource(action, Resource::Wool, 3);
    action = packResource(action, Resource::Grain, 0);
    action = packResource(action, Resource::Ore, 1);

    EXPECT_EQ(unpackType(action), ActionType::DiscardResources);
    EXPECT_EQ(unpackPlayerID(action), PlayerId::Player0);
    EXPECT_EQ(unpackResource(action, Resource::Brick), 2);
    EXPECT_EQ(unpackResource(action, Resource::Lumber), 1);
    EXPECT_EQ(unpackResource(action, Resource::Wool), 3);
    EXPECT_EQ(unpackResource(action, Resource::Grain), 0);
    EXPECT_EQ(unpackResource(action, Resource::Ore), 1);
}

TEST(ActionTest, BitBoundaries) {
    PackedAction action = 0;

    // Test maximum values for each field
    action = packType(action, static_cast<ActionType>(15));  // 4 bits max
    EXPECT_EQ(static_cast<uint8_t>(unpackType(action)), 15);

    action = 0;
    action = packPlayerID(action, static_cast<PlayerId>(3));  // 2 bits max
    EXPECT_EQ(static_cast<uint8_t>(unpackPlayerID(action)), 3);

    action = 0;
    action = packResource(action, Resource::Brick, 31);  // 5 bits max
    EXPECT_EQ(unpackResource(action, Resource::Brick), 31);

    action = 0;
    action = packArg1(action, 255);  // 8 bits max
    EXPECT_EQ(unpackArg1(action), 255);

    action = 0;
    action = packArg2(action, 255);  // 8 bits max
    EXPECT_EQ(unpackArg2(action), 255);
}

TEST(ActionTest, ZeroInitialization) {
    PackedAction action = 0;

    EXPECT_EQ(static_cast<uint8_t>(unpackType(action)), 0);
    EXPECT_EQ(static_cast<uint8_t>(unpackPlayerID(action)), 0);
    EXPECT_EQ(unpackResource(action, Resource::Brick), 0);
    EXPECT_EQ(unpackResource(action, Resource::Lumber), 0);
    EXPECT_EQ(unpackResource(action, Resource::Wool), 0);
    EXPECT_EQ(unpackResource(action, Resource::Grain), 0);
    EXPECT_EQ(unpackResource(action, Resource::Ore), 0);
    EXPECT_EQ(unpackArg1(action), 0);
    EXPECT_EQ(unpackArg2(action), 0);
}

TEST(ActionTest, MultiplePacking) {
    PackedAction action = 0;

    // Pack all fields with different values
    action = packType(action, ActionType::PlayDevCardKnight);
    action = packPlayerID(action, PlayerId::Player1);
    action = packResource(action, Resource::Brick, 5);
    action = packResource(action, Resource::Lumber, 10);
    action = packResource(action, Resource::Wool, 15);
    action = packResource(action, Resource::Grain, 20);
    action = packResource(action, Resource::Ore, 25);
    action = packArg1(action, 100);
    action = packArg2(action, 200);

    // Verify all fields maintained their values
    EXPECT_EQ(unpackType(action), ActionType::PlayDevCardKnight);
    EXPECT_EQ(unpackPlayerID(action), PlayerId::Player1);
    EXPECT_EQ(unpackResource(action, Resource::Brick), 5);
    EXPECT_EQ(unpackResource(action, Resource::Lumber), 10);
    EXPECT_EQ(unpackResource(action, Resource::Wool), 15);
    EXPECT_EQ(unpackResource(action, Resource::Grain), 20);
    EXPECT_EQ(unpackResource(action, Resource::Ore), 25);
    EXPECT_EQ(unpackArg1(action), 100);
    EXPECT_EQ(unpackArg2(action), 200);
}