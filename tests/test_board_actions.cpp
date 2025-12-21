#include <gtest/gtest.h>
#include "board.hpp"
#include "consts.hpp"
#include "player.hpp"
#include "actions.hpp"

using namespace Board;

class ApplyActionTest : public ::testing::Test {
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

TEST_F(ApplyActionTest, ExpextPlaceInitialSettlement) {
    PlayerId playerId = PlayerId::Player0;

    Action::PackedAction placeSettlementAction{};
    placeSettlementAction = Action::packType(placeSettlementAction, ActionType::PlaceInitialSettlement);
    placeSettlementAction = Action::packPlayerID(placeSettlementAction, playerId);
    placeSettlementAction = Action::packArg1(placeSettlementAction, 0); // NodeId 0
    
    boardState.applyAction(placeSettlementAction);

    EXPECT_EQ(Node::unpackStructure(boardState.nodes[0]), StructureType::Settlement);
    EXPECT_EQ(Node::unpackOwner(boardState.nodes[0]), playerId);
    EXPECT_EQ(Player::unpackVictoryPoints(boardState.packedPlayers[static_cast<size_t>(playerId)]), 1);
    EXPECT_EQ(Player::unpackAvailableStructures(
        boardState.packedPlayers[static_cast<size_t>(playerId)],
        StructureType::Settlement
    ), 4); // Starting with 5 settlements
}

TEST_F(ApplyActionTest, ExpextPlace2InitialSettlement) {
    setBankResourcesTen();
    setPlayersResourcesSeven();
    NodeId settlementNodeId = 2;
    Node::PackedNode settlementNode = boardState.nodes[settlementNodeId];

    PlayerId playerId = PlayerId::Player1;

    HexId adjHex[3] = {
        Node::unpackAdjacentHex(settlementNode, 0),
        Node::unpackAdjacentHex(settlementNode, 1),
        Node::unpackAdjacentHex(settlementNode, 2)
    };

    for (HexId h : adjHex) {
        if (h != HexIdNone) {
            boardState.hexes[h] = Hex::packResource(boardState.hexes[h], Resource::Brick);
        }
    }

    boardState.packedPlayers[static_cast<size_t>(playerId)] = 
        Player::packVictoryPoints(
            boardState.packedPlayers[static_cast<size_t>(playerId)],
            0
        );
    boardState.packedPlayers[static_cast<size_t>(playerId)] = 
        Player::packAvailableStructures(
            boardState.packedPlayers[static_cast<size_t>(playerId)],
            StructureType::Settlement,
            5
        );

    
    
    Action::PackedAction place2SettlementAction{};
    place2SettlementAction = Action::packType(place2SettlementAction, ActionType::Place2InitialSettlement);
    place2SettlementAction = Action::packPlayerID(place2SettlementAction, playerId);
    place2SettlementAction = Action::packArg1(place2SettlementAction, settlementNodeId);
    
    boardState.applyAction(place2SettlementAction);

    EXPECT_EQ(Node::unpackStructure(boardState.nodes[settlementNodeId]), StructureType::Settlement);
    EXPECT_EQ(Node::unpackOwner(boardState.nodes[settlementNodeId]), playerId);
    EXPECT_EQ(Player::unpackVictoryPoints(boardState.packedPlayers[static_cast<size_t>(playerId)]), 1);
    EXPECT_EQ(Player::unpackAvailableStructures(
        boardState.packedPlayers[static_cast<size_t>(playerId)],
        StructureType::Settlement
    ), 4); // Starting with 5 settlements
    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Brick), 8);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(playerId)], Resource::Brick), 9);
    
    for (HexId h : adjHex) {
        if (h != HexIdNone) {
            EXPECT_EQ(
                Hex::unpackPlayerValue(boardState.hexes[h], playerId),
                1
            );
        }
    }
}


TEST_F(ApplyActionTest, ExpectPlaceInitialRoad){
    PlayerId playerId = PlayerId::Player0;
    EdgeId roadEdgeId = 10;

    Action::PackedAction buildRoadAction{};
    buildRoadAction = Action::packType(buildRoadAction, ActionType::BuildRoad);
    buildRoadAction = Action::packArg1(buildRoadAction, roadEdgeId);
    buildRoadAction = Action::packPlayerID(buildRoadAction, playerId);
    boardState.applyAction(buildRoadAction);
    
    Edge::PackedEdge roadEdge = boardState.edges[roadEdgeId];
    EXPECT_TRUE(Edge::unpackHasRoad(roadEdge));
    EXPECT_EQ(Edge::unpackOwner(roadEdge), playerId);
    EXPECT_EQ(Player::unpackAvailableStructures(boardState.packedPlayers[static_cast<size_t>(playerId)], StructureType::Road), 14);

}

TEST_F(ApplyActionTest, ExpectResourceDistributionOnEightRoll) {
    setBankResourcesTen();

    std::vector<HexId> expectedHexes;
    for (HexId h = 0; h < HEX_COUNT; ++h) {
        if (Hex::unpackCatanNumber(boardState.hexes[h]) == 8) {
            expectedHexes.push_back(h);
        }
    }

    ASSERT_EQ(expectedHexes.size(), 2);
    
    HexId hex1 = expectedHexes[0];
    HexId hex2 = expectedHexes[1];

    boardState.hexes[hex1] = Hex::packPlayerValue(boardState.hexes[hex1], PlayerId::Player0, 3);
    boardState.hexes[hex1] = Hex::packPlayerValue(boardState.hexes[hex1], PlayerId::Player1, 2);
    boardState.hexes[hex2] = Hex::packPlayerValue(boardState.hexes[hex2], PlayerId::Player0, 4);
    boardState.hexes[hex2] = Hex::packPlayerValue(boardState.hexes[hex2], PlayerId::Player1, 0);

    Resource res1 = Hex::unpackResource(boardState.hexes[hex1]);
    Resource res2 = Hex::unpackResource(boardState.hexes[hex2]);

    Action::PackedAction rollDiceAction{};
    rollDiceAction = Action::packType(rollDiceAction, ActionType::RollDice);
    rollDiceAction = Action::packArg1(rollDiceAction, 8);
    boardState.applyAction(rollDiceAction);

    auto player0Resources1 = Player::unpackResource(boardState.packedPlayers[0], res1);
    auto player0Resources2 = Player::unpackResource(boardState.packedPlayers[0], res2);
    auto player1Resources1 = Player::unpackResource(boardState.packedPlayers[1], res1);
    auto player1Resources2 = Player::unpackResource(boardState.packedPlayers[1], res2);

    if (res1 == res2)
    {
        EXPECT_EQ(player0Resources1, 7); // 3 + 4
        EXPECT_EQ(player1Resources1, 2); // 2 + 0
    }
    else
    {
        EXPECT_EQ(player0Resources1, 3);
        EXPECT_EQ(player0Resources2, 4);
        EXPECT_EQ(player1Resources1, 2);
        EXPECT_EQ(player1Resources2, 0);
    }

    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, res1), 10 - (3 + 2));
    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, res2), 10 - (4 + 0));
}

TEST_F(ApplyActionTest, ExpectTurnRotationOnEndTurn) {
    Action::PackedAction endTurnAction{};
    endTurnAction = Action::packType(endTurnAction, ActionType::EndTurn);

    PlayerId startingPlayer = boardState.currentPlayer;
    uint8_t startingTurn = boardState.currentTurn;

    boardState.applyAction(endTurnAction);

    PlayerId expectedNextPlayer = (startingPlayer == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;
    EXPECT_EQ(boardState.currentPlayer, expectedNextPlayer);
    EXPECT_EQ(boardState.currentTurn, startingTurn + 1);

    boardState.applyAction(endTurnAction);
    EXPECT_EQ(boardState.currentPlayer, startingPlayer);
    EXPECT_EQ(boardState.currentTurn, startingTurn + 2);

    boardState.applyAction(endTurnAction);
    EXPECT_EQ(boardState.currentPlayer, expectedNextPlayer);
    EXPECT_EQ(boardState.currentTurn, startingTurn + 3);
}

TEST_F(ApplyActionTest, ExpectRobberMovementAndStealCardTrigger) {
    EXPECT_EQ(Hex::unpackResource(boardState.hexes[boardState.robberPosition]), Resource::NoResource);

    HexId newRobberPosition;
    for (HexId h = 0; h < HEX_COUNT; ++h) {
        if (Hex::unpackCatanNumber(boardState.hexes[h]) == 8) {
            newRobberPosition = h;
            break;
        }
    }

    PlayerId stealingPlayer = PlayerId::Player0;
    PlayerId victimPlayer = PlayerId::Player1;
    Resource robbedResource = Resource::Lumber;

    boardState.packedPlayers[static_cast<size_t>(stealingPlayer)] = Player::packResource(
        boardState.packedPlayers[static_cast<size_t>(stealingPlayer)],
        robbedResource,
        2
    );

    boardState.packedPlayers[static_cast<size_t>(victimPlayer)] = Player::packResource(
        boardState.packedPlayers[static_cast<size_t>(victimPlayer)],
        robbedResource,
        3
    );

    boardState.hexes[newRobberPosition] = Hex::packPlayerValue(
        boardState.hexes[newRobberPosition],
        victimPlayer,
        2
    );

    Action::PackedAction moveRobberAction{};
    moveRobberAction = Action::packType(moveRobberAction, ActionType::MoveRobber);
    moveRobberAction = Action::packArg1(moveRobberAction, newRobberPosition);
    moveRobberAction = Action::packPlayerID(moveRobberAction, stealingPlayer);
    boardState.applyAction(moveRobberAction);
    boardState.applyAction(
        Action::packArg1(
            Action::packPlayerID(
                Action::packType(0, ActionType::StealResource),
                stealingPlayer),
            static_cast<uint8_t>(robbedResource))
    );
    EXPECT_EQ(boardState.robberPosition, newRobberPosition);
    EXPECT_EQ(
        Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(stealingPlayer)], robbedResource),
        3
    );
    EXPECT_EQ(
        Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(victimPlayer)], robbedResource),
        2
    );
}

TEST_F(ApplyActionTest, ExpectStealResource) {
    PlayerId stealingPlayer = PlayerId::Player0;
    PlayerId victimPlayer = PlayerId::Player1;
    Resource robbedResource = Resource::Grain;

    boardState.packedPlayers[static_cast<size_t>(stealingPlayer)] = Player::packResource(
        boardState.packedPlayers[static_cast<size_t>(stealingPlayer)],
        robbedResource,
        1
    );

    boardState.packedPlayers[static_cast<size_t>(victimPlayer)] = Player::packResource(
        boardState.packedPlayers[static_cast<size_t>(victimPlayer)],
        robbedResource,
        4
    );

    Action::PackedAction stealResourceAction{};
    stealResourceAction = Action::packType(stealResourceAction, ActionType::StealResource);
    stealResourceAction = Action::packArg1(stealResourceAction, static_cast<uint8_t>(robbedResource));
    stealResourceAction = Action::packPlayerID(stealResourceAction, stealingPlayer);
    boardState.applyAction(stealResourceAction);

    EXPECT_EQ(
        Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(stealingPlayer)], robbedResource),
        2
    );
    EXPECT_EQ(
        Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(victimPlayer)], robbedResource),
        3
    );
}

TEST_F(ApplyActionTest, ExpectDiscardResources) {
    PlayerId discardingPlayer = PlayerId::Player1;
    setPlayersResourcesSeven();

    Action::PackedAction discardResourcesAction{};
    discardResourcesAction = Action::packType(discardResourcesAction, ActionType::DiscardResources);
    discardResourcesAction = Action::packPlayerID(discardResourcesAction, discardingPlayer);
    discardResourcesAction = Action::packResource(discardResourcesAction, Resource::Ore, 1);
    discardResourcesAction = Action::packResource(discardResourcesAction, Resource::Wool, 2);
    discardResourcesAction = Action::packResource(discardResourcesAction, Resource::Grain, 3);
    discardResourcesAction = Action::packResource(discardResourcesAction, Resource::Brick, 0);
    discardResourcesAction = Action::packResource(discardResourcesAction, Resource::Lumber, 1);
    boardState.applyAction(discardResourcesAction);

    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(discardingPlayer)], Resource::Ore), 6);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(discardingPlayer)], Resource::Wool), 5);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(discardingPlayer)], Resource::Grain), 4);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(discardingPlayer)], Resource::Brick), 7);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(discardingPlayer)], Resource::Lumber), 6);
}

TEST_F(ApplyActionTest, ExpectBuildRoad) {
    setBankResourcesTen();
    setPlayersResourcesSeven();
    PlayerId buildingPlayer = PlayerId::Player0;
    EdgeId roadEdgeId = 10;

    Action::PackedAction buildRoadAction{};
    buildRoadAction = Action::packType(buildRoadAction, ActionType::BuildRoad);
    buildRoadAction = Action::packArg1(buildRoadAction, roadEdgeId);
    buildRoadAction = Action::packPlayerID(buildRoadAction, buildingPlayer);
    boardState.applyAction(buildRoadAction);

    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Grain), 10);
    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Wool), 10);
    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Ore), 10);
    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Brick), 11);
    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Lumber), 11);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(buildingPlayer)], Resource::Brick), 6);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(buildingPlayer)], Resource::Lumber), 6);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(buildingPlayer)], Resource::Grain), 7);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(buildingPlayer)], Resource::Ore), 7);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(buildingPlayer)], Resource::Wool), 7);
    
    Edge::PackedEdge roadEdge = boardState.edges[roadEdgeId];
    EXPECT_TRUE(Edge::unpackHasRoad(roadEdge));
    EXPECT_EQ(Edge::unpackOwner(roadEdge), buildingPlayer);
    EXPECT_EQ(Player::unpackAvailableStructures(boardState.packedPlayers[static_cast<size_t>(buildingPlayer)], StructureType::Road), 14);
}

TEST_F(ApplyActionTest, ExpectBuildSettlement) {
    setBankResourcesTen();
    setPlayersResourcesSeven();
    PlayerId buildingPlayer = PlayerId::Player1;
    NodeId settlementNodeId = 20;

    Action::PackedAction buildSettlementAction{};
    buildSettlementAction = Action::packType(buildSettlementAction, ActionType::BuildSettlement);
    buildSettlementAction = Action::packArg1(buildSettlementAction, settlementNodeId);
    buildSettlementAction = Action::packPlayerID(buildSettlementAction, buildingPlayer);
    boardState.applyAction(buildSettlementAction);

    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Grain), 11);
    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Wool), 11);
    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Ore), 10);
    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Brick), 11);
    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Lumber), 11);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(buildingPlayer)], Resource::Brick), 6);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(buildingPlayer)], Resource::Lumber), 6);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(buildingPlayer)], Resource::Grain), 6);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(buildingPlayer)], Resource::Ore), 7);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(buildingPlayer)], Resource::Wool), 6);

    Node::PackedNode settlementNode = boardState.nodes[settlementNodeId];
    EXPECT_EQ(Node::unpackStructure(settlementNode), StructureType::Settlement);
    EXPECT_EQ(Node::unpackOwner(settlementNode), buildingPlayer);
    EXPECT_EQ(Player::unpackAvailableStructures(boardState.packedPlayers[static_cast<size_t>(buildingPlayer)], StructureType::Settlement), 4);
    EXPECT_EQ(Player::unpackVictoryPoints(boardState.packedPlayers[static_cast<size_t>(buildingPlayer)]), 1);
    
    HexId adjHex[3] = {
        Node::unpackAdjacentHex(settlementNode, 0),
        Node::unpackAdjacentHex(settlementNode, 1),
        Node::unpackAdjacentHex(settlementNode, 2)
    };

    for (HexId h : adjHex) {
        if (h != HexIdNone) {
            EXPECT_EQ(
                Hex::unpackPlayerValue(boardState.hexes[h], buildingPlayer),
                1
            );
        }
    }
}

TEST_F(ApplyActionTest, ExpectBuildCity) {
    PlayerId buildingPlayer = PlayerId::Player0;
    NodeId cityNodeId = 15;
    setBankResourcesTen();
    setPlayersResourcesSeven();

    boardState.packedPlayers[static_cast<size_t>(buildingPlayer)] = Player::packAvailableStructures(
        boardState.packedPlayers[static_cast<size_t>(buildingPlayer)],
        StructureType::Settlement,
        3
    );
    boardState.packedPlayers[static_cast<size_t>(buildingPlayer)] = Player::packVictoryPoints(
        boardState.packedPlayers[static_cast<size_t>(buildingPlayer)],
        1
    );

    Node::PackedNode& cityNode = boardState.nodes[cityNodeId];
    Node::PackedNode& targetNode = boardState.nodes[cityNodeId];
    targetNode = Node::packStructure(targetNode, StructureType::Settlement);
    targetNode = Node::packOwner(targetNode, buildingPlayer);

    HexId adjHex[3] = {
        Node::unpackAdjacentHex(cityNode, 0),
        Node::unpackAdjacentHex(cityNode, 1),
        Node::unpackAdjacentHex(cityNode, 2)
    };

    for (HexId h : adjHex) {
        if (h != HexIdNone) {
            boardState.hexes[h] = Hex::packPlayerValue(
                boardState.hexes[h],
                buildingPlayer,
                1
            );
        }
    }

    Action::PackedAction buildCityAction{};
    buildCityAction = Action::packType(buildCityAction, ActionType::BuildCity);
    buildCityAction = Action::packArg1(buildCityAction, cityNodeId);
    buildCityAction = Action::packPlayerID(buildCityAction, buildingPlayer);
    boardState.applyAction(buildCityAction);

    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Grain), 12);
    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Wool), 10);
    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Ore), 13);
    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Brick), 10);
    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Lumber), 10);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(buildingPlayer)], Resource::Brick), 7);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(buildingPlayer)], Resource::Lumber), 7);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(buildingPlayer)], Resource::Grain), 5);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(buildingPlayer)], Resource::Ore), 4);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(buildingPlayer)], Resource::Wool), 7);

    EXPECT_EQ(Node::unpackStructure(cityNode), StructureType::City);
    EXPECT_EQ(Player::unpackAvailableStructures(boardState.packedPlayers[static_cast<size_t>(buildingPlayer)], StructureType::City), 3);
    EXPECT_EQ(Player::unpackAvailableStructures(boardState.packedPlayers[static_cast<size_t>(buildingPlayer)], StructureType::Settlement), 4);
    EXPECT_EQ(Player::unpackVictoryPoints(boardState.packedPlayers[static_cast<size_t>(buildingPlayer)]), 2);

    for (HexId h : adjHex) {
        if (h != HexIdNone) {
            EXPECT_EQ(
                Hex::unpackPlayerValue(boardState.hexes[h], buildingPlayer),
                2
            );
        }
    }
}

class ActionTestDevCardTypeParam
    : public ApplyActionTest,
      public ::testing::WithParamInterface<DevType> { };

TEST_P(ActionTestDevCardTypeParam, ExpectBuyOneDevelopmentCard) {
    setBankResourcesTen();
    setPlayersResourcesSeven();

    DevType devCardInBank = GetParam();

    // Ensure the bank contains only one type of development cards for this test.
    for (DevType devType : {
        DevType::Knight,
        DevType::RoadBuilding,
        DevType::YearOfPlenty,
        DevType::Monopoly,
        DevType::VictoryPoint
    }) {
        if (devType == devCardInBank) {
            boardState.packedBank = Bank::packDevCard(boardState.packedBank, devType, 3);
        } else {
            boardState.packedBank = Bank::packDevCard(boardState.packedBank, devType, 0);
        }
    }
    boardState.packedBank = Bank::packTotalDevCount(boardState.packedBank, 3);

    PlayerId buyingPlayer = PlayerId::Player1;

    Action::PackedAction buyDevCardAction{};
    buyDevCardAction = Action::packType(buyDevCardAction, ActionType::BuyDevCard);
    buyDevCardAction = Action::packPlayerID(buyDevCardAction, buyingPlayer);
    boardState.applyAction(buyDevCardAction);

    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Grain), 11);
    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Wool), 11);
    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Ore), 11);
    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Brick), 10);
    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Lumber), 10);
    EXPECT_EQ(Bank::unpackDevCard(boardState.packedBank, devCardInBank), 2);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(buyingPlayer)], Resource::Brick), 7);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(buyingPlayer)], Resource::Lumber), 7);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(buyingPlayer)], Resource::Grain), 6);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(buyingPlayer)], Resource::Ore), 6);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(buyingPlayer)], Resource::Wool), 6);

    int totalDevCards = Player::totalDevCards(boardState.packedPlayers[static_cast<size_t>(buyingPlayer)]);

    EXPECT_EQ(totalDevCards, 1);

    if (Player::unpackDevCard(boardState.packedPlayers[static_cast<size_t>(buyingPlayer)], DevType::VictoryPoint) == 1) {
        EXPECT_EQ(Player::unpackVictoryPoints(boardState.packedPlayers[static_cast<size_t>(buyingPlayer)]), 1);
    }
}

INSTANTIATE_TEST_SUITE_P(
    DevCardTypes,
    ActionTestDevCardTypeParam,
    ::testing::Values(
        DevType::Knight,
        DevType::RoadBuilding,
        DevType::YearOfPlenty,
        DevType::Monopoly,
        DevType::VictoryPoint
    )
);

TEST_F(ApplyActionTest, ExpectBuyAllDevCards){
    setBankResourcesTen();
    setPlayersResourcesSeven();

    // Ensure the bank contains only one type of development cards for this test.
    for (DevType devType : {
        DevType::Knight,
        DevType::RoadBuilding,
        DevType::YearOfPlenty,
        DevType::Monopoly,
        DevType::VictoryPoint
    }) {
        boardState.packedBank = Bank::packDevCard(boardState.packedBank, devType, 1);
    }
    boardState.packedBank = Bank::packTotalDevCount(boardState.packedBank, 5);

    PlayerId buyingPlayer = PlayerId::Player0;

    for (int i = 0; i < 5; ++i) {
        Action::PackedAction buyDevCardAction{};
        buyDevCardAction = Action::packType(buyDevCardAction, ActionType::BuyDevCard);
        buyDevCardAction = Action::packPlayerID(buyDevCardAction, buyingPlayer);
        boardState.applyAction(buyDevCardAction);
    }

    for (DevType devType : {
        DevType::Knight,
        DevType::RoadBuilding,
        DevType::YearOfPlenty,
        DevType::Monopoly,
        DevType::VictoryPoint
    }) {
        EXPECT_EQ(Bank::unpackDevCard(boardState.packedBank, devType), 0);
        EXPECT_EQ(
            Player::unpackDevCard(boardState.packedPlayers[static_cast<size_t>(buyingPlayer)], devType),
            1
        );
    }

    EXPECT_EQ(Bank::unpackTotalDevCount(boardState.packedBank), 0);
    EXPECT_EQ(Player::unpackVictoryPoints(boardState.packedPlayers[static_cast<size_t>(buyingPlayer)]), 1);
    EXPECT_EQ(
        Player::totalDevCards(boardState.packedPlayers[static_cast<size_t>(buyingPlayer)]),
        5
    );
}

TEST_F(ApplyActionTest, ExpectTradeBank) {
    setBankResourcesTen();
    setPlayersResourcesSeven();
    PlayerId tradingPlayer = PlayerId::Player0;
    Resource giveResource = Resource::Lumber;
    Resource receiveResource = Resource::Ore;

    Action::PackedAction tradeBankAction{};
    tradeBankAction = Action::packType(tradeBankAction, ActionType::TradeBank);
    tradeBankAction = Action::packArg1(tradeBankAction, static_cast<uint8_t>(giveResource));
    tradeBankAction = Action::packArg2(tradeBankAction, static_cast<uint8_t>(receiveResource));
    tradeBankAction = Action::packArg3(tradeBankAction, 2); // 2:1 trade
    tradeBankAction = Action::packPlayerID(tradeBankAction, tradingPlayer);
    boardState.applyAction(tradeBankAction);

    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, giveResource), 12);
    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, receiveResource), 9);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(tradingPlayer)], giveResource), 5);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(tradingPlayer)], receiveResource), 8);
}

TEST_F(ApplyActionTest, ExpectReceiveResources) {
    setBankResourcesTen();
    PlayerId receivingPlayer = PlayerId::Player1;
    Resource resToReceive = Resource::Grain;

    Action::PackedAction receiveResourcesAction{};
    receiveResourcesAction = Action::packType(receiveResourcesAction, ActionType::ReceiveResources);
    receiveResourcesAction = Action::packArg1(receiveResourcesAction, static_cast<uint8_t>(resToReceive));
    receiveResourcesAction = Action::packArg2(receiveResourcesAction, 4);
    receiveResourcesAction = Action::packPlayerID(receiveResourcesAction, receivingPlayer);
    boardState.applyAction(receiveResourcesAction);

    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, resToReceive), 6);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(receivingPlayer)], resToReceive), 4);
}

TEST_F(ApplyActionTest, ExpectPlayDevCardKnight){
    EXPECT_EQ(Hex::unpackResource(boardState.hexes[boardState.robberPosition]), Resource::NoResource);

    PlayerId playingPlayer = PlayerId::Player0;
    PlayerId victimPlayer = PlayerId::Player1;
    HexId newRobberPosition = 5;

    boardState.packedPlayers[static_cast<size_t>(playingPlayer)] = Player::packDevCard(
        boardState.packedPlayers[static_cast<size_t>(playingPlayer)],
        DevType::Knight,
        1
    );

    Action::PackedAction playKnightAction{};
    playKnightAction = Action::packType(playKnightAction, ActionType::PlayDevCardKnight);
    playKnightAction = Action::packArg1(playKnightAction, newRobberPosition);
    playKnightAction = Action::packPlayerID(playKnightAction, playingPlayer);
    boardState.applyAction(playKnightAction);

    EXPECT_EQ(boardState.robberPosition, newRobberPosition);
    EXPECT_EQ(
        Player::unpackDevCard(boardState.packedPlayers[static_cast<size_t>(playingPlayer)], DevType::Knight),
        0
    );

}

TEST_F(ApplyActionTest, ExpectPlayDevCardRoadBuilding){
    PlayerId playingPlayer = PlayerId::Player0;
    EdgeId firstRoadEdgeId = 20;
    EdgeId secondRoadEdgeId = 28;

    boardState.packedPlayers[static_cast<size_t>(playingPlayer)] = Player::packDevCard(
        boardState.packedPlayers[static_cast<size_t>(playingPlayer)],
        DevType::RoadBuilding,
        1
    );

    Action::PackedAction playRoadBuildingAction{};
    playRoadBuildingAction = Action::packType(playRoadBuildingAction, ActionType::PlayDevCardRoadBuilding);
    playRoadBuildingAction = Action::packArg1(playRoadBuildingAction, firstRoadEdgeId);
    playRoadBuildingAction = Action::packArg2(playRoadBuildingAction, secondRoadEdgeId);
    playRoadBuildingAction = Action::packPlayerID(playRoadBuildingAction, playingPlayer);
    boardState.applyAction(playRoadBuildingAction);

    EXPECT_EQ(
        Player::unpackDevCard(boardState.packedPlayers[static_cast<size_t>(playingPlayer)], DevType::RoadBuilding),
        0
    );

    Edge::PackedEdge firstRoadEdge = boardState.edges[firstRoadEdgeId];
    EXPECT_TRUE(Edge::unpackHasRoad(firstRoadEdge));
    EXPECT_EQ(Edge::unpackOwner(firstRoadEdge), playingPlayer);

    Edge::PackedEdge secondRoadEdge = boardState.edges[secondRoadEdgeId];
    EXPECT_TRUE(Edge::unpackHasRoad(secondRoadEdge));
    EXPECT_EQ(Edge::unpackOwner(secondRoadEdge), playingPlayer);

    EXPECT_EQ(Player::unpackAvailableStructures(boardState.packedPlayers[static_cast<size_t>(playingPlayer)], StructureType::Road), 13);
}

TEST_F(ApplyActionTest, ExpectPlayDevCardYearOfPlenty){
    setBankResourcesTen();
    setPlayersResourcesSeven();

    PlayerId playingPlayer = PlayerId::Player1;
    Resource firstResource = Resource::Ore;
    Resource secondResource = Resource::Grain;

    boardState.packedPlayers[static_cast<size_t>(playingPlayer)] = Player::packDevCard(
        boardState.packedPlayers[static_cast<size_t>(playingPlayer)],
        DevType::YearOfPlenty,
        1
    );

    Action::PackedAction playYearOfPlentyAction{};
    playYearOfPlentyAction = Action::packType(playYearOfPlentyAction, ActionType::PlayDevCardYearOfPlenty);
    playYearOfPlentyAction = Action::packArg1(playYearOfPlentyAction, static_cast<uint8_t>(firstResource));
    playYearOfPlentyAction = Action::packArg2(playYearOfPlentyAction, static_cast<uint8_t>(secondResource));
    playYearOfPlentyAction = Action::packPlayerID(playYearOfPlentyAction, playingPlayer);
    boardState.applyAction(playYearOfPlentyAction);

    EXPECT_EQ(
        Player::unpackDevCard(boardState.packedPlayers[static_cast<size_t>(playingPlayer)], DevType::YearOfPlenty),
        0
    );
    EXPECT_EQ(
        Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(playingPlayer)], firstResource),
        8
    );
    EXPECT_EQ(
        Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(playingPlayer)], secondResource),
        8
    );
    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, firstResource), 9);
    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, secondResource), 9);
}

TEST_F(ApplyActionTest, ExpectPlayDevCardMonopoly){
    setPlayersResourcesSeven();

    PlayerId playingPlayer = PlayerId::Player0;
    PlayerId victimPlayer = PlayerId::Player1;
    Resource monopolyResource = Resource::Brick;

    boardState.packedPlayers[static_cast<size_t>(playingPlayer)] = Player::packDevCard(
        boardState.packedPlayers[static_cast<size_t>(playingPlayer)],
        DevType::Monopoly,
        1
    );

    boardState.packedPlayers[static_cast<size_t>(victimPlayer)] = Player::packResource(
        boardState.packedPlayers[static_cast<size_t>(victimPlayer)],
        monopolyResource,
        5
    );

    Action::PackedAction playMonopolyAction{};
    playMonopolyAction = Action::packType(playMonopolyAction, ActionType::PlayDevCardMonopoly);
    playMonopolyAction = Action::packArg1(playMonopolyAction, static_cast<uint8_t>(monopolyResource));
    playMonopolyAction = Action::packPlayerID(playMonopolyAction, playingPlayer);
    boardState.applyAction(playMonopolyAction);

    EXPECT_EQ(
        Player::unpackDevCard(boardState.packedPlayers[static_cast<size_t>(playingPlayer)], DevType::Monopoly),
        0
    );
    EXPECT_EQ(
        Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(playingPlayer)], monopolyResource),
        12
    );
    EXPECT_EQ(
        Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(victimPlayer)], monopolyResource),
        0
    );
}
