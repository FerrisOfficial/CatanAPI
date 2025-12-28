#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <iostream>
#include <algorithm>
#include "board.hpp"
#include "consts.hpp"
#include "player.hpp"
#include "actions.hpp"
using ::testing::UnorderedElementsAre;

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

    buyDevCardActions = boardState.generateBuyDevCardActions(PlayerId::Player0);

    EXPECT_EQ(7, buyDevCardActions.size());
    for (const auto& action : buyDevCardActions) {
        ActionType type = Action::unpackType(action);
        EXPECT_EQ(ActionType::BuyDevCard, type);
    }
}

TEST_F(ActionTest, ExpectNoBuyDevCardActionsWithoutResources){
    setBankResourcesTen();
    // Players have zero resources

    std::vector<Action::PackedAction> buyDevCardActions;

    buyDevCardActions = boardState.generateBuyDevCardActions(PlayerId::Player0);

    EXPECT_EQ(0, buyDevCardActions.size());
}

TEST_F(ActionTest, ExpectNoBuyDevCardActionsWhenDeckEmpty){
    setBankResourcesTen();
    setPlayersResourcesSeven();

    // Empties the development card deck
    for ( DevType devType : {
        DevType::Knight,
        DevType::RoadBuilding,
        DevType::YearOfPlenty,
        DevType::Monopoly,
        DevType::VictoryPoint
    }) {
        boardState.packedBank = Bank::packDevCard(
            boardState.packedBank,
            devType,
            0
        );
    }

    std::vector<Action::PackedAction> buyDevCardActions;

    buyDevCardActions = boardState.generateBuyDevCardActions(PlayerId::Player0);

    EXPECT_EQ(0, buyDevCardActions.size());
}

TEST_F(ActionTest, ExpectPlayDevCardKnightActions){
    setBankResourcesTen();
    setPlayersResourcesSeven();

    PlayerId playingPlayer = PlayerId::Player0;
    HexId robberPosition = boardState.robberPosition;

    // Give player a knight development card
    boardState.packedPlayers[static_cast<size_t>(playingPlayer)] = Player::packDevCard(
        boardState.packedPlayers[static_cast<size_t>(playingPlayer)],
        DevType::Knight,
        1
    );

    std::vector<Action::PackedAction> playDevCardActions;

    playDevCardActions = boardState.generatePlayDevCardKnightActions(playingPlayer);

    EXPECT_EQ(18, playDevCardActions.size());
    for (const auto& action : playDevCardActions) {
        ActionType type = Action::unpackType(action);
        EXPECT_EQ(ActionType::PlayDevCardKnight, type);
        EXPECT_NE(robberPosition, Action::unpackArg1(action));
    }
}

TEST_F(ActionTest, ExpectPlayDevCardRoadBuildingFromRoadActions){
    PlayerId playingPlayer = PlayerId::Player1;

    // Give player a road building development card
    boardState.packedPlayers[static_cast<size_t>(playingPlayer)] = Player::packDevCard(
        boardState.packedPlayers[static_cast<size_t>(playingPlayer)],
        DevType::RoadBuilding,
        1
    );

    // Place some roads to limit available options
    for (EdgeId edgeId = 0; edgeId <= 6; ++edgeId) {
        boardState.edges[edgeId] = Edge::packHasRoad(boardState.edges[edgeId], true);
        boardState.edges[edgeId] = Edge::packOwner(boardState.edges[edgeId], playingPlayer);
    }

    std::vector<Action::PackedAction> playDevCardActions;

    playDevCardActions = boardState.generatePlayDevCardRoadBuildingActions(playingPlayer);

    std::vector<std::pair<EdgeId, EdgeId>> expectedEdgeId = {
        {7, 8}, {7, 9}, {7, 10}, {7, 11}, {7,12}, {7, 13},
        {8, 7}, {8, 9}, {8, 10}, {8, 11}, {8, 14}, {8, 15},
        {9, 7}, {9, 8}, {9, 10}, {9, 11}, {9, 16}, {9, 17},
        {10, 7}, {10, 8}, {10, 9}, {10, 11}, {10, 18},
        {11, 7}, {11, 8}, {11, 9}, {11, 10}, {11, 12}, {11, 19}
    };
    std::vector<std::pair<EdgeId, EdgeId>> resultEdgeId;

    EXPECT_EQ(playDevCardActions.size(), 29);
    for (const auto& action : playDevCardActions) {
        ActionType type = Action::unpackType(action);
        EXPECT_EQ(ActionType::PlayDevCardRoadBuilding, type);
        resultEdgeId.push_back(std::make_pair(
            static_cast<EdgeId>(Action::unpackArg1(action)),
            static_cast<EdgeId>(Action::unpackArg2(action))
        ));
    }
    EXPECT_THAT(resultEdgeId, ::testing::UnorderedElementsAreArray(expectedEdgeId));
}

TEST_F(ActionTest, ExpectPlayDevCardRoadBuildingFromSettlementActions){
    PlayerId playingPlayer = PlayerId::Player0;

    // Give player a road building development card
    boardState.packedPlayers[static_cast<size_t>(playingPlayer)] = Player::packDevCard(
        boardState.packedPlayers[static_cast<size_t>(playingPlayer)],
        DevType::RoadBuilding,
        1
    );

    // Give player a settlement to build roads from
    NodeId settlementNodeId = 11;
    boardState.nodes[settlementNodeId] = Node::packStructure(boardState.nodes[settlementNodeId], StructureType::Settlement);
    boardState.nodes[settlementNodeId] = Node::packOwner(boardState.nodes[settlementNodeId], playingPlayer);

    std::vector<Action::PackedAction> playDevCardActions;

    playDevCardActions = boardState.generatePlayDevCardRoadBuildingActions(playingPlayer);

    std::vector<std::pair<EdgeId, EdgeId>> expectedEdgeId = {
        {13, 7}, {13, 12}, {14, 8}, {14, 15},
        {20, 27}, {20, 28}, {13, 14}, {13, 20},
        {14, 13}, {14, 20}, {20, 13}, {20, 14}
    };
    std::vector<std::pair<EdgeId, EdgeId>> resultEdgeId;

    EXPECT_EQ(playDevCardActions.size(), 12);
    for (const auto& action : playDevCardActions) {
        ActionType type = Action::unpackType(action);
        EXPECT_EQ(ActionType::PlayDevCardRoadBuilding, type);
        resultEdgeId.push_back(std::make_pair(
            static_cast<EdgeId>(Action::unpackArg1(action)),
            static_cast<EdgeId>(Action::unpackArg2(action))
        ));
    }
    EXPECT_THAT(resultEdgeId, ::testing::UnorderedElementsAreArray(expectedEdgeId));
}

TEST_F(ActionTest, ExpectPlayDevCardRoadBuildingActionWhenOneRoadAvailable){
    PlayerId playingPlayer = PlayerId::Player1;

    // Give player a road building development card
    boardState.packedPlayers[static_cast<size_t>(playingPlayer)] = Player::packDevCard(
        boardState.packedPlayers[static_cast<size_t>(playingPlayer)],
        DevType::RoadBuilding,
        1
    );

    boardState.packedPlayers[static_cast<size_t>(playingPlayer)] = Player::packAvailableStructures(
        boardState.packedPlayers[static_cast<size_t>(playingPlayer)],
        StructureType::Road,
        1
    );

    // Place roads to limit available options
    for (EdgeId edgeId = 0; edgeId <= 6; ++edgeId) {
        boardState.edges[edgeId] = Edge::packHasRoad(boardState.edges[edgeId], true);
        boardState.edges[edgeId] = Edge::packOwner(boardState.edges[edgeId], playingPlayer);
    }

    std::vector<Action::PackedAction> playDevCardActions;

    std::vector<EdgeId> expectedEdgeId = {7, 8, 9, 10, 11};
    std::vector<EdgeId> resultEdgeId;

    playDevCardActions = boardState.generatePlayDevCardRoadBuildingActions(playingPlayer);

    EXPECT_EQ(5, playDevCardActions.size());
    for (const auto& action : playDevCardActions) {
        ActionType type = Action::unpackType(action);
        EXPECT_EQ(ActionType::PlayDevCardRoadBuilding, type);
        EXPECT_EQ(EdgeIdNone, Action::unpackArg2(action));
        resultEdgeId.push_back(static_cast<EdgeId>(Action::unpackArg1(action)));
    }
    EXPECT_THAT(resultEdgeId, ::testing::UnorderedElementsAreArray(expectedEdgeId));
}

TEST_F(ActionTest, ExpectPlayDevCardYearOfPlentyActions){
    setBankResourcesTen();
    setPlayersResourcesSeven();

    PlayerId playingPlayer = PlayerId::Player1;

    // Give player a year of plenty development card
    boardState.packedPlayers[static_cast<size_t>(playingPlayer)] = Player::packDevCard(
        boardState.packedPlayers[static_cast<size_t>(playingPlayer)],
        DevType::YearOfPlenty,
        1
    );

    std::vector<Action::PackedAction> playDevCardActions;

    playDevCardActions = boardState.generatePlayDevCardYearOfPlentyActions(playingPlayer);

    EXPECT_EQ(25, playDevCardActions.size());
    for (const auto& action : playDevCardActions) {
        ActionType type = Action::unpackType(action);
        EXPECT_EQ(ActionType::PlayDevCardYearOfPlenty, type);
    }
}

TEST_F(ActionTest, ExpectPlayDevCardMonopolyActions){
    setBankResourcesTen();
    setPlayersResourcesSeven();

    PlayerId playingPlayer = PlayerId::Player0;

    // Give player a monopoly development card
    boardState.packedPlayers[static_cast<size_t>(playingPlayer)] = Player::packDevCard(
        boardState.packedPlayers[static_cast<size_t>(playingPlayer)],
        DevType::Monopoly,
        1
    );

    std::vector<Action::PackedAction> playDevCardActions;

    playDevCardActions = boardState.generatePlayDevCardMonopolyActions(playingPlayer);

    std::vector<Resource> expectedResources = {
        Resource::Brick, Resource::Lumber, Resource::Wool,
        Resource::Grain, Resource::Ore
    };
    std::vector<Resource> resultResources;

    EXPECT_EQ(5, playDevCardActions.size());
    for (const auto& action : playDevCardActions) {
        ActionType type = Action::unpackType(action);
        EXPECT_EQ(ActionType::PlayDevCardMonopoly, type);
        resultResources.push_back(static_cast<Resource>(Action::unpackArg1(action)));
    }
    EXPECT_THAT(resultResources, ::testing::UnorderedElementsAreArray(expectedResources));
}

class ActionTestDevCardParam
    : public ActionTest,
      public ::testing::WithParamInterface<std::tuple<DevType, int>> { };

TEST_P(ActionTestDevCardParam, ExpectGeneratePlayDevCardActions){
    setBankResourcesTen();
    setPlayersResourcesSeven();

    PlayerId playingPlayer = PlayerId::Player0;
    DevType devType = std::get<0>(GetParam());
    int expectedActionCount = std::get<1>(GetParam());

    boardState.packedPlayers[static_cast<size_t>(playingPlayer)] = Player::packDevCard(
        boardState.packedPlayers[static_cast<size_t>(playingPlayer)],
        devType,
        1
    );

    if (devType == DevType::RoadBuilding) {
        for (EdgeId edgeId = 0; edgeId < 6; ++edgeId) {
            boardState.edges[edgeId] = Edge::packHasRoad(boardState.edges[edgeId], true);
            boardState.edges[edgeId] = Edge::packOwner(boardState.edges[edgeId], playingPlayer);
        }
    }

    std::vector<Action::PackedAction> playDevCardActions;
    playDevCardActions = boardState.generatePlayDevCardActions(playingPlayer);
    EXPECT_EQ(expectedActionCount, playDevCardActions.size());
}

INSTANTIATE_TEST_SUITE_P(
    GeneratePlayDevCardActionsTests,
    ActionTestDevCardParam,
    ::testing::Values(
        std::make_tuple(DevType::Knight, 18),
        std::make_tuple(DevType::RoadBuilding, 20),
        std::make_tuple(DevType::YearOfPlenty, 25),
        std::make_tuple(DevType::Monopoly, 5)
    )
);

TEST_F(ActionTest, ExpectNoPlayDevCardActionsWithoutDevCards){
    setBankResourcesTen();
    setPlayersResourcesSeven();

    PlayerId playingPlayer = PlayerId::Player0;

    std::vector<Action::PackedAction> playDevCardActions;

    playDevCardActions = boardState.generatePlayDevCardActions(playingPlayer);

    EXPECT_EQ(0, playDevCardActions.size());
}

class ActionTestEdgeParam
    : public ActionTest,
      public ::testing::WithParamInterface<std::tuple<EdgeId, EdgeId, EdgeId, EdgeId, EdgeId>> { };

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
    std::vector<EdgeId> resultEdgeId;
    expectedEdgeId.erase(
        std::remove(expectedEdgeId.begin(), expectedEdgeId.end(), EdgeIdNone),
        expectedEdgeId.end()
    );

    // Give player a starting road to build off of
    // boardState
    boardState.edges[roadEdgeId] = Edge::packHasRoad(boardState.edges[roadEdgeId], true);
    boardState.edges[roadEdgeId] = Edge::packOwner(boardState.edges[roadEdgeId], PlayerId::Player0);

    buildRoadActions = boardState.generateBuildRoadActions(PlayerId::Player0);

    EXPECT_LE(buildRoadActions.size(), 4);
    EXPECT_GE(buildRoadActions.size(), 1);
    for (const auto& action : buildRoadActions) {
        ActionType type = Action::unpackType(action);
        EXPECT_EQ(ActionType::BuildRoad, type);
        resultEdgeId.push_back(static_cast<EdgeId>(Action::unpackArg1(action)));
    }
    EXPECT_THAT(resultEdgeId, ::testing::UnorderedElementsAreArray(expectedEdgeId));
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

    std::vector<EdgeId> expectedEdgeId = {6, 11, 18, 45, 46, 59, 60};
    std::vector<EdgeId> resultEdgeId;

    EXPECT_EQ(buildRoadActions.size(), expectedEdgeId.size());
    int i = 0;
    for (const auto& action : buildRoadActions) {
        ActionType type = Action::unpackType(action);
        EXPECT_EQ(ActionType::BuildRoad, type);
        resultEdgeId.push_back(static_cast<EdgeId>(Action::unpackArg1(action)));
    }

    EXPECT_THAT(resultEdgeId, ::testing::UnorderedElementsAreArray(expectedEdgeId));
}

class ActionTestNodeParam
    : public ActionTest,
      public ::testing::WithParamInterface<std::tuple<NodeId, EdgeId, EdgeId, EdgeId, StructureType>> { };

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

    expectedEdgeId.erase(
        std::remove(expectedEdgeId.begin(), expectedEdgeId.end(), EdgeIdNone),
        expectedEdgeId.end()
    );

    std::vector<EdgeId> resultEdgeId;

    // Give player a settlement to build off of
    boardState.nodes[settlementNodeId] = Node::packStructure(boardState.nodes[settlementNodeId], structureType);
    boardState.nodes[settlementNodeId] = Node::packOwner(boardState.nodes[settlementNodeId], PlayerId::Player0);

    buildRoadActions = boardState.generateBuildRoadActions(PlayerId::Player0);

    EXPECT_LE(buildRoadActions.size(), 3);
    EXPECT_GE(buildRoadActions.size(), 1);

    for (const auto& action : buildRoadActions) {
        ActionType type = Action::unpackType(action);
        EXPECT_EQ(ActionType::BuildRoad, type);
        resultEdgeId.push_back(static_cast<EdgeId>(Action::unpackArg1(action)));
    }

    EXPECT_THAT(resultEdgeId, ::testing::UnorderedElementsAreArray(expectedEdgeId));
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

class ActionTestBuildSettlementWithRoadsOnBoardParam
    : public ActionTest,
      public ::testing::WithParamInterface<std::tuple<EdgeId, EdgeId, int>> { };


TEST_F(ActionTest, ExpectBuildSettlementActionsWithOneRoad){
    setPlayersResourcesSeven();

    boardState.edges[20] = Edge::packHasRoad(boardState.edges[20], true);
    boardState.edges[20] = Edge::packOwner(boardState.edges[20], PlayerId::Player0);

    std::vector<Action::PackedAction> buildSettlementActions;

    buildSettlementActions = boardState.generateBuildSettlementActions(PlayerId::Player0);
    EXPECT_EQ(buildSettlementActions.size(), 2);
    for (const auto& action : buildSettlementActions) {
        ActionType type = Action::unpackType(action);
        EXPECT_EQ(ActionType::BuildSettlement, type);
    }
}

TEST_P(ActionTestBuildSettlementWithRoadsOnBoardParam, ExpectBuildSettlementActionsWithTwoRoads){
    setPlayersResourcesSeven();

    std::vector<Action::PackedAction> buildSettlementActions;
    std::tuple<EdgeId, EdgeId, int> givenEdgeId = GetParam();
    EdgeId roadEdgeId1 = std::get<0>(givenEdgeId);
    EdgeId roadEdgeId2 = std::get<1>(givenEdgeId);
    int expectedNumActions = std::get<2>(givenEdgeId);

    // Give player two starting roads to build off of
    boardState.edges[roadEdgeId1] = Edge::packHasRoad(boardState.edges[roadEdgeId1], true);
    boardState.edges[roadEdgeId1] = Edge::packOwner(boardState.edges[roadEdgeId1], PlayerId::Player0);
    boardState.edges[roadEdgeId2] = Edge::packHasRoad(boardState.edges[roadEdgeId2], true);
    boardState.edges[roadEdgeId2] = Edge::packOwner(boardState.edges[roadEdgeId2], PlayerId::Player0);

    buildSettlementActions = boardState.generateBuildSettlementActions(PlayerId::Player0);

    EXPECT_EQ(buildSettlementActions.size(), expectedNumActions);
    for (const auto& action : buildSettlementActions) {
        ActionType type = Action::unpackType(action);
        EXPECT_EQ(ActionType::BuildSettlement, type);
    }
}

INSTANTIATE_TEST_SUITE_P(
    BuildSettlementWithTwoRoadsTests,
    ActionTestBuildSettlementWithRoadsOnBoardParam,
    ::testing::Values(
        std::make_tuple(7, 50, 4),
        std::make_tuple(35, 36, 4),
        std::make_tuple(44, 51, 3),
        std::make_tuple(23, 33, 3)
    )
);

TEST_F(ActionTest, ExpectBuildSettlementWithGoodDistance){
    setPlayersResourcesSeven();

    // Place a settlement to block nearby placements
    NodeId blockingNodeId = 11;
    boardState.nodes[blockingNodeId] = Node::packStructure(boardState.nodes[blockingNodeId], StructureType::Settlement);
    boardState.nodes[blockingNodeId] = Node::packOwner(boardState.nodes[blockingNodeId], PlayerId::Player0);

    // Add roads to allow building near the blocking settlement
    for (int i = 0; i < 3; ++i) {
        EdgeId adjacentEdgeId = Node::unpackAdjacentEdge(boardState.nodes[blockingNodeId], i);
        boardState.edges[adjacentEdgeId] = Edge::packHasRoad(boardState.edges[adjacentEdgeId], true);
        boardState.edges[adjacentEdgeId] = Edge::packOwner(boardState.edges[adjacentEdgeId], PlayerId::Player0);
    }
    boardState.edges[27] = Edge::packHasRoad(boardState.edges[27], true);
    boardState.edges[27] = Edge::packOwner(boardState.edges[27], PlayerId::Player0);

    boardState.edges[33] = Edge::packHasRoad(boardState.edges[33], true);
    boardState.edges[33] = Edge::packOwner(boardState.edges[33], PlayerId::Player0);

    std::vector<NodeId> resultNodeIds;
    std::vector<Action::PackedAction> buildSettlementActions;

    buildSettlementActions = boardState.generateBuildSettlementActions(PlayerId::Player0);

    EXPECT_EQ(buildSettlementActions.size(), 3);

    for (const auto& action : buildSettlementActions) {
        ActionType type = Action::unpackType(action);
        EXPECT_EQ(ActionType::BuildSettlement, type);
        resultNodeIds.push_back(static_cast<NodeId>(Action::unpackArg1(action)));
    }

    EXPECT_THAT(resultNodeIds, UnorderedElementsAre(16, 20, 27));
}

TEST_F(ActionTest, ExpectNoBuildSettlementActionsOnEmptyBoard){
    setPlayersResourcesSeven();
    std::vector<Action::PackedAction> buildSettlementActions;

    buildSettlementActions = boardState.generateBuildSettlementActions(PlayerId::Player0);

    EXPECT_EQ(buildSettlementActions.size(), 0);
    for (const auto& action : buildSettlementActions) {
        ActionType type = Action::unpackType(action);
        EXPECT_EQ(ActionType::BuildSettlement, type);
    }
}

TEST_F(ActionTest, ExpectBuildCityOnSettlementActions){
    setPlayersResourcesSeven();

    // Give player a settlement to upgrade
    NodeId settlementNodeId = 5;
    boardState.nodes[settlementNodeId] = Node::packStructure(boardState.nodes[settlementNodeId], StructureType::Settlement);
    boardState.nodes[settlementNodeId] = Node::packOwner(boardState.nodes[settlementNodeId], PlayerId::Player0);

    std::vector<Action::PackedAction> buildCityActions;

    buildCityActions = boardState.generateBuildCityActions(PlayerId::Player0);

    EXPECT_EQ(buildCityActions.size(), 1);
    for (const auto& action : buildCityActions) {
        ActionType type = Action::unpackType(action);
        EXPECT_EQ(ActionType::BuildCity, type);
        EXPECT_EQ(settlementNodeId, Action::unpackArg1(action));
    }
}

class ActionTestStructureTypeParam
    : public ActionTest,
      public ::testing::WithParamInterface<StructureType> { };

TEST_P(ActionTestStructureTypeParam, ExpectNoBuildWithNoAvaliableStructures){
    setPlayersResourcesSeven();

    StructureType structureType = GetParam();

    // Remove all available structures of the given type from Player0
    boardState.packedPlayers[static_cast<uint8_t>(PlayerId::Player0)] = Player::packAvailableStructures(
        boardState.packedPlayers[static_cast<uint8_t>(PlayerId::Player0)],
        structureType,
        0
    );

    std::vector<Action::PackedAction> buildActions;

    if (structureType == StructureType::Road) {
        buildActions = boardState.generateBuildRoadActions(PlayerId::Player0);
    }
    else if (structureType == StructureType::Settlement) {
        buildActions = boardState.generateBuildSettlementActions(PlayerId::Player0);
    }
    else if (structureType == StructureType::City) {
        buildActions = boardState.generateBuildCityActions(PlayerId::Player0);
    }

    EXPECT_EQ(0, buildActions.size());
}

INSTANTIATE_TEST_SUITE_P(
    NoBuildWithNoAvaliableStructuresTests,
    ActionTestStructureTypeParam,
    ::testing::Values(
        StructureType::Road,
        StructureType::Settlement,
        StructureType::City
    )
);

TEST_P(ActionTestStructureTypeParam, ExpectNoBuildWithNoAvaliableResources){
    StructureType structureType = GetParam();
    std::vector<Action::PackedAction> buildActions;

    if (structureType == StructureType::Road) {
        buildActions = boardState.generateBuildRoadActions(PlayerId::Player0);
    }
    else if (structureType == StructureType::Settlement) {
        buildActions = boardState.generateBuildSettlementActions(PlayerId::Player0);
    }
    else if (structureType == StructureType::City) {
        buildActions = boardState.generateBuildCityActions(PlayerId::Player0);
    }

    EXPECT_EQ(0, buildActions.size());
}

INSTANTIATE_TEST_SUITE_P(
    NoBuildWithNoAvaliableResourcesTests,
    ActionTestStructureTypeParam,
    ::testing::Values(
        StructureType::Road,
        StructureType::Settlement,
        StructureType::City
    )
);

TEST_P(ActionTestStructureTypeParam, ExpectNoBuildOnEmptyBoard){
    setPlayersResourcesSeven();

    StructureType structureType = GetParam();
    std::vector<Action::PackedAction> buildActions;

    if (structureType == StructureType::Road) {
        buildActions = boardState.generateBuildRoadActions(PlayerId::Player0);
    }
    else if (structureType == StructureType::Settlement) {
        buildActions = boardState.generateBuildSettlementActions(PlayerId::Player0);
    }
    else if (structureType == StructureType::City) {
        buildActions = boardState.generateBuildCityActions(PlayerId::Player0);
    }

    EXPECT_EQ(0, buildActions.size());
}

INSTANTIATE_TEST_SUITE_P(
    NoBuildOnEmptyBoardTests,
    ActionTestStructureTypeParam,
    ::testing::Values(
        StructureType::Road,
        StructureType::Settlement,
        StructureType::City
    )
);

TEST_F(ActionTest, ExpectGenerateAllActions){
    setBankResourcesTen();
    setPlayersResourcesSeven();

    PlayerId currentPlayer = PlayerId::Player0;

    // Give player a settlement on a wool port
    NodeId woolPortNodeId = 7;
    boardState.nodes[woolPortNodeId] = Node::packStructure(boardState.nodes[woolPortNodeId], StructureType::Settlement);
    boardState.nodes[woolPortNodeId] = Node::packOwner(boardState.nodes[woolPortNodeId], currentPlayer);

    std::vector<Action::PackedAction> allActions;

    allActions = boardState.getLegalActions(currentPlayer);

    EXPECT_GT(allActions.size(), 0);
}

TEST_F(ActionTest, ExpectGeneratePlaceInitialStructuresActions){
    std::vector<Action::PackedAction> placeInitialStructuresActions;

    placeInitialStructuresActions = boardState.generatePlaceInitialStructures(PlayerId::Player0);

    EXPECT_EQ(placeInitialStructuresActions.size(), 144);
    for (const auto& action : placeInitialStructuresActions) {
        ActionType type = Action::unpackType(action);
        EXPECT_EQ(ActionType::PlaceInitialStructures, type);
        EXPECT_LT(Action::unpackArg1(action), NODE_COUNT);
        EXPECT_LT(Action::unpackArg2(action), EDGE_COUNT);
    }
}

TEST_F(ActionTest, ExpectGeneratePlace2InitialStructuresActions){
    std::vector<Action::PackedAction> placeInitialStructuresActions;

    boardState.nodes[31] = Node::packStructure(boardState.nodes[31], StructureType::Settlement);
    boardState.nodes[31] = Node::packOwner(boardState.nodes[31], PlayerId::Player0);
    boardState.edges[42] = Edge::packHasRoad(boardState.edges[42], true);
    boardState.edges[42] = Edge::packOwner(boardState.edges[42], PlayerId::Player0);

    placeInitialStructuresActions = boardState.generatePlace2InitialStructures(PlayerId::Player0);

    EXPECT_EQ(placeInitialStructuresActions.size(), 132);
    for (const auto& action : placeInitialStructuresActions) {
        ActionType type = Action::unpackType(action);
        EXPECT_EQ(ActionType::Place2InitialStructures, type);
        EXPECT_LT(Action::unpackArg1(action), NODE_COUNT);
        EXPECT_LT(Action::unpackArg2(action), EDGE_COUNT);
    }
}