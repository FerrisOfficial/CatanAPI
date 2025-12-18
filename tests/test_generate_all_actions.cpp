#include <gtest/gtest.h>
#include <iostream>
#include "board.hpp"
#include "consts.hpp"
#include "player.hpp"
#include "actions.hpp"

using namespace Board;

class ActionTest : public ::testing::Test {
protected:
    Board::BoardState boardState;

    void SetUp() override {
        boardState.generateRandomBoard();
    }

    void setBankResourcesTen() {
        for (size_t res = 0; res < 5; ++res) {
            boardState.packedBank = Bank::packResource(boardState.packedBank, static_cast<Resource>(res), 10);
        }
    }

    void setPlayersResourcesSeven() {
        for (size_t p = 0; p < 2; ++p) {
            for (size_t res = 0; res < 5; ++res) {
                boardState.packedPlayers[p] = Player::packResource(
                    boardState.packedPlayers[p],
                    static_cast<Resource>(res),
                    7
                );
            }
        }
    }

};

TEST_F(ActionTest, ExpectTradeActionsWithWoolPort){
    setBankResourcesTen();
    setPlayersResourcesSeven();

    PlayerId tradingPlayer = PlayerId::Player0;

    // Give player a settlement on a wool port
    NodeId woolPortNodeId = 7;
    boardState.nodes[woolPortNodeId] = Node::packStructure(boardState.nodes[woolPortNodeId], StructureType::Settlement);
    boardState.nodes[woolPortNodeId] = Node::packOwner(boardState.nodes[woolPortNodeId], tradingPlayer);

    std::vector<Action::PackedAction> tradeActions;

    tradeActions = boardState.generateTwoToOnePortTradeActions(tradingPlayer);

    for (const auto& action : tradeActions) {
        EXPECT_EQ(ActionType::TradeBank, Action::unpackType(action));
        EXPECT_EQ(tradingPlayer, Action::unpackPlayerID(action));
        EXPECT_EQ(Resource::Wool, static_cast<Resource>(Action::unpackArg1(action)));
        EXPECT_EQ(2, Action::unpackArg3(action));
    }
}

TEST_F(ActionTest, ExpectTradeActionsThreeToOnePort){
    setBankResourcesTen();
    setPlayersResourcesSeven();

    PlayerId tradingPlayer = PlayerId::Player0;

    // Give player a settlement on a 3:1 port
    NodeId threeForOnePortNodeId = 2;
    boardState.nodes[threeForOnePortNodeId] = Node::packStructure(boardState.nodes[threeForOnePortNodeId], StructureType::Settlement);
    boardState.nodes[threeForOnePortNodeId] = Node::packOwner(boardState.nodes[threeForOnePortNodeId], tradingPlayer);

    std::vector<Action::PackedAction> tradeActions;

    tradeActions = boardState.generateThreeToOnePortTradeActions(tradingPlayer);

    for (const auto& action : tradeActions) {
        EXPECT_EQ(ActionType::TradeBank, Action::unpackType(action));
        EXPECT_EQ(tradingPlayer, Action::unpackPlayerID(action));
        EXPECT_EQ(3, Action::unpackArg3(action));
    }

    EXPECT_EQ(40, tradeActions.size());
}

TEST_F(ActionTest, ExpectBankTradeActions){
    setBankResourcesTen();
    setPlayersResourcesSeven();

    PlayerId tradingPlayer = PlayerId::Player0;

    std::vector<Action::PackedAction> tradeActions;
    boardState.packedPlayers[static_cast<uint8_t>(tradingPlayer)] = Player::packResource(
        boardState.packedPlayers[static_cast<uint8_t>(tradingPlayer)],
        Resource::Brick,
        16
    );

    tradeActions = boardState.generateBankTradeActions(tradingPlayer);

    for (const auto& action : tradeActions) {
        EXPECT_EQ(ActionType::TradeBank, Action::unpackType(action));
        EXPECT_EQ(tradingPlayer, Action::unpackPlayerID(action));
        EXPECT_EQ(4, Action::unpackArg3(action));
    }

    EXPECT_EQ(32, tradeActions.size());
}

TEST_F(ActionTest, ExpectNoTradeActionsWithoutResources){
    setBankResourcesTen();
    // Players have zero resources

    PlayerId tradingPlayer = PlayerId::Player0;

    // Give player a settlement on a brick port
    NodeId brickPortNodeId = 15;
    boardState.nodes[brickPortNodeId] = Node::packStructure(boardState.nodes[brickPortNodeId], StructureType::Settlement);
    boardState.nodes[brickPortNodeId] = Node::packOwner(boardState.nodes[brickPortNodeId], tradingPlayer);

    std::vector<Action::PackedAction> tradeActions;

    tradeActions = boardState.generateTwoToOnePortTradeActions(tradingPlayer);

    EXPECT_EQ(0, tradeActions.size());
}

TEST_F(ActionTest, ExpectNoTradeActionsWithoutPorts){
    setBankResourcesTen();
    setPlayersResourcesSeven();

    PlayerId tradingPlayer = PlayerId::Player0;

    std::vector<Action::PackedAction> tradeActions;

    tradeActions = boardState.generateTwoToOnePortTradeActions(tradingPlayer);

    EXPECT_EQ(0, tradeActions.size());
}

TEST_F(ActionTest, ExpectBuyDevCardActions){
    setBankResourcesTen();
    setPlayersResourcesSeven();

    std::vector<Action::PackedAction> buyDevCardActions;

    buyDevCardActions = boardState.generateDevCardActions(PlayerId::Player0);

    EXPECT_EQ(7, buyDevCardActions.size());
    for (const auto& action : buyDevCardActions) {
        ActionType type = Action::unpackType(action);
        EXPECT_EQ(ActionType::BuyDevCard, type);
    }
}

class ActionTestEdgeParam : public ::testing::TestWithParam<std::tuple<EdgeId, EdgeId, EdgeId, EdgeId, EdgeId>> {
protected:
    Board::BoardState boardState;

    void SetUp() override {
        boardState.generateRandomBoard();
    }

    void setPlayersResourcesSeven() {
        for (size_t p = 0; p < 2; ++p) {
            for (size_t res = 0; res < 5; ++res) {
                boardState.packedPlayers[p] = Player::packResource(
                    boardState.packedPlayers[p],
                    static_cast<Resource>(res),
                    7
                );
            }
        }
    }

};


TEST_P(ActionTestEdgeParam, ExpectBuildRoadFromRoadActions){
    setPlayersResourcesSeven();

    std::vector<Action::PackedAction> buildRoadActions;
    std::tuple<EdgeId, EdgeId, EdgeId, EdgeId, EdgeId> givenEdgeId = GetParam();
    EdgeId roadEdgeId = std::get<0>(givenEdgeId);
    std::vector<EdgeId> expectedEdgeId = {
        std::get<1>(givenEdgeId),
        std::get<2>(givenEdgeId),
        std::get<3>(givenEdgeId),
        std::get<4>(givenEdgeId)
    };

    // Give player a starting road to build off of
    // boardState
    boardState.edges[roadEdgeId] = Edge::packHasRoad(boardState.edges[roadEdgeId], true);
    boardState.edges[roadEdgeId] = Edge::packOwner(boardState.edges[roadEdgeId], PlayerId::Player0);

    buildRoadActions = boardState.generateBuildRoadActions(PlayerId::Player0);

    EXPECT_LE(buildRoadActions.size(), 4);
    EXPECT_GE(buildRoadActions.size(), 1);
    int i = 0;
    for (const auto& action : buildRoadActions) {
        ActionType type = Action::unpackType(action);
        EXPECT_EQ(ActionType::BuildRoad, type);
        EXPECT_EQ(expectedEdgeId[i], Action::unpackArg1(action));
        ++i;
    }
}

INSTANTIATE_TEST_SUITE_P(
    BuildRoadFromRoadTests,
    ActionTestEdgeParam,
    ::testing::Values(
        std::make_tuple(0, 1, 6, EdgeIdNone, EdgeIdNone),
        std::make_tuple(30, 21, 29, 31, 37),
        std::make_tuple(19, 11, 12, 25, 26),
        std::make_tuple(49, 39, 40, 54, EdgeIdNone),
        std::make_tuple(66, 62, 67, EdgeIdNone, EdgeIdNone)
    )
);

TEST_F(ActionTest, ExpectBuildRoadFromTwoRoadsActions){
    setPlayersResourcesSeven();

    std::vector<Action::PackedAction> buildRoadActions;

    // Give player two starting roads to build off of
    boardState.edges[10] = Edge::packHasRoad(boardState.edges[10], true);
    boardState.edges[10] = Edge::packOwner(boardState.edges[10], PlayerId::Player0);
    boardState.edges[52] = Edge::packHasRoad(boardState.edges[52], true);
    boardState.edges[52] = Edge::packOwner(boardState.edges[52], PlayerId::Player0);

    buildRoadActions = boardState.generateBuildRoadActions(PlayerId::Player0);

    std::vector<EdgeId> expectedEdgeIds = {6, 11, 18, 45, 46, 59, 60};

    EXPECT_EQ(buildRoadActions.size(), expectedEdgeIds.size());
    int i = 0;
    for (const auto& action : buildRoadActions) {
        ActionType type = Action::unpackType(action);
        EXPECT_EQ(ActionType::BuildRoad, type);
        EXPECT_EQ(expectedEdgeIds[i], Action::unpackArg1(action));
        ++i;
    }
}

class ActionTestNodeParam : public ::testing::TestWithParam<std::tuple<NodeId, EdgeId, EdgeId, EdgeId, StructureType>> {
protected:
    Board::BoardState boardState;

    void SetUp() override {
        boardState.generateRandomBoard();
    }

    void setPlayersResourcesSeven() {
        for (size_t p = 0; p < 2; ++p) {
            for (size_t res = 0; res < 5; ++res) {
                boardState.packedPlayers[p] = Player::packResource(
                    boardState.packedPlayers[p],
                    static_cast<Resource>(res),
                    7
                );
            }
        }
    }

};

TEST_P(ActionTestNodeParam, ExpectBuildRoadFromSettlementActions){
    setPlayersResourcesSeven();

    std::vector<Action::PackedAction> buildRoadActions;
    std::tuple<NodeId, EdgeId, EdgeId, EdgeId, StructureType> givenId = GetParam();
    NodeId settlementNodeId = std::get<0>(givenId);
    StructureType structureType = std::get<4>(givenId);
    std::vector<EdgeId> expectedEdgeId = {
        std::get<1>(givenId),
        std::get<2>(givenId),
        std::get<3>(givenId)
    };

    // Give player a settlement to build off of
    boardState.nodes[settlementNodeId] = Node::packStructure(boardState.nodes[settlementNodeId], structureType);
    boardState.nodes[settlementNodeId] = Node::packOwner(boardState.nodes[settlementNodeId], PlayerId::Player0);

    buildRoadActions = boardState.generateBuildRoadActions(PlayerId::Player0);

    EXPECT_LE(buildRoadActions.size(), 3);
    EXPECT_GE(buildRoadActions.size(), 1);
    int i = 0;
    for (const auto& action : buildRoadActions) {
        ActionType type = Action::unpackType(action);
        EXPECT_EQ(ActionType::BuildRoad, type);
        EXPECT_EQ(expectedEdgeId[i], Action::unpackArg1(action));
        ++i;
    }
}

INSTANTIATE_TEST_SUITE_P(
    BuildRoadFromSettlementTests,
    ActionTestNodeParam,
    ::testing::Values(
        std::make_tuple(0, 0, 6, EdgeIdNone, StructureType::Settlement),
        std::make_tuple(14, 9, 16, 17, StructureType::Settlement),
        std::make_tuple(27, 33, 39, EdgeIdNone, StructureType::Settlement),
        std::make_tuple(36, 47, 48, 53, StructureType::City),
        std::make_tuple(52, 70, 71, EdgeIdNone, StructureType::City)
    )
);

class ActionTestStructureTypeParam : public ::testing::TestWithParam<StructureType> {
protected:
    Board::BoardState boardState;

    void SetUp() override {
        boardState.generateRandomBoard();
    }

    void setPlayersResourcesSeven() {
        for (size_t p = 0; p < 2; ++p) {
            for (size_t res = 0; res < 5; ++res) {
                boardState.packedPlayers[p] = Player::packResource(
                    boardState.packedPlayers[p],
                    static_cast<Resource>(res),
                    7
                );
            }
        }
    }
};

// TEST_P(ActionTestStructureTypeParam, ExpectNoBuildWithNoAvaliableStructures){
//     setPlayersResourcesSeven();

//     StructureType structureType = GetParam();

//     // Remove all available structures of the given type from Player0
//     boardState.packedPlayers[static_cast<uint8_t>(PlayerId::Player0)] = Player::packAvailableStructures(
//         boardState.packedPlayers[static_cast<uint8_t>(PlayerId::Player0)],
//         structureType,
//         0
//     );

//     std::vector<Action::PackedAction> buildActions;

//     if (structureType == StructureType::Road) {
//         buildActions = boardState.generateBuildRoadActions(PlayerId::Player0);
//     }
//     // else if (structureType == StructureType::Settlement) {
//     //     buildActions = boardState.generateBuildSettlementActions(PlayerId::Player0);
//     // }
//     // else if (structureType == StructureType::City) {
//     //     buildActions = boardState.generateBuildCityActions(PlayerId::Player0);
//     // }

//     EXPECT_EQ(0, buildActions.size());
// }

