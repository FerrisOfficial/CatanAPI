#include <gtest/gtest.h>
#include "board.hpp"
#include "consts.hpp"
#include "player.hpp"
#include "actions.hpp"

using namespace Board;

class HexPackTest : public ::testing::TestWithParam<std::tuple<uint8_t, Resource, uint8_t, uint8_t>> {};

TEST_P(HexPackTest, HexPacking) {
    auto [catanNumber, resource, player0Value, player1Value] = GetParam();
    
    Hex::PackedHex hex = Hex::makeHex(catanNumber, resource, player0Value, player1Value);
    
    EXPECT_EQ(Hex::unpackCatanNumber(hex), catanNumber);
    EXPECT_EQ(Hex::unpackResource(hex), resource);
    EXPECT_EQ(Hex::unpackPlayerValue(hex, PlayerId::Player0), player0Value);
    EXPECT_EQ(Hex::unpackPlayerValue(hex, PlayerId::Player1), player1Value);
}

INSTANTIATE_TEST_SUITE_P(
    HexPackTests,
    HexPackTest,
    ::testing::Values(
        std::make_tuple(6, Resource::Brick, 3, 5),
        std::make_tuple(8, Resource::Wool, 2, 4),
        std::make_tuple(12, Resource::Grain, 7, 0),
        std::make_tuple(2, Resource::Lumber, 0, 7),
        std::make_tuple(15, Resource::Ore, 7, 7),  // Max values
        std::make_tuple(0, Resource::NoResource, 0, 0)  // Min values
    )
);

class NodePackTest : public ::testing::TestWithParam<std::tuple<HexId, HexId, HexId, StructureType, PlayerId, PortType>> {};

TEST_P(NodePackTest, NodePacking) {
    auto [hex1, hex2, hex3, structure, owner, portType] = GetParam();
    
    Node::PackedNode node = Node::makeNode(hex1, hex2, hex3, structure, owner, portType);
    
    EXPECT_EQ(Node::unpackStructure(node), structure);
    EXPECT_EQ(Node::unpackOwner(node), owner);
    EXPECT_EQ(Node::unpackAdjacentHex(node, 0), hex1);
    EXPECT_EQ(Node::unpackAdjacentHex(node, 1), hex2);
    EXPECT_EQ(Node::unpackAdjacentHex(node, 2), hex3);
    EXPECT_EQ(Node::unpackPortType(node), portType);
}

INSTANTIATE_TEST_SUITE_P(
    NodePackTests,
    NodePackTest,
    ::testing::Values(
        std::make_tuple(5, 10, 15, StructureType::Settlement, PlayerId::Player0, PortType::WoolPort),
        std::make_tuple(18, 7, 2, StructureType::City, PlayerId::Player1, PortType::GrainPort),
        std::make_tuple(0, 0, 0, StructureType::NoStructure, PlayerId::Player0, PortType::NoPort),
        std::make_tuple(19, 19, 19, StructureType::Road, PlayerId::Player1, PortType::OrePort),  // Max hex values
        std::make_tuple(1, 8, 12, StructureType::City, PlayerId::Player0, PortType::LumberPort)
    )
);

class EdgePackTest : public ::testing::TestWithParam<std::tuple<bool, PlayerId, NodeId, NodeId>> {};

TEST_P(EdgePackTest, EdgePacking) {
    auto [hasRoad, owner, node1, node2] = GetParam();
    
    Edge::PackedEdge edge = Edge::makeEdge(node1, node2, hasRoad, owner);
    
    EXPECT_EQ(Edge::unpackHasRoad(edge), hasRoad);
    EXPECT_EQ(Edge::unpackOwner(edge), owner);
    EXPECT_EQ(Edge::unpackAdjacentNode(edge, 0), node1);
    EXPECT_EQ(Edge::unpackAdjacentNode(edge, 1), node2);
}

INSTANTIATE_TEST_SUITE_P(
    EdgePackTests,
    EdgePackTest,
    ::testing::Values(
        std::make_tuple(true, PlayerId::Player0, 25, 30),
        std::make_tuple(false, PlayerId::Player1, 5, 10),
        std::make_tuple(true, PlayerId::Player1, 53, 40),  // Max node values within range
        std::make_tuple(false, PlayerId::Player0, 0, 0),   // Min values
        std::make_tuple(true, PlayerId::Player0, 127, 127) // Max 7-bit values
    )
);

TEST(HexTest, IndividualFieldPacking) {
    Hex::PackedHex hex = 0;
    
    // Test individual field packing
    hex = Hex::packCatanNumber(hex, 8);
    EXPECT_EQ(Hex::unpackCatanNumber(hex), 8);
    
    hex = Hex::packResource(hex, Resource::Ore);
    EXPECT_EQ(Hex::unpackResource(hex), Resource::Ore);
    EXPECT_EQ(Hex::unpackCatanNumber(hex), 8);  // Should not affect other fields
    
    hex = Hex::packPlayerValue(hex, PlayerId::Player0, 5);
    EXPECT_EQ(Hex::unpackPlayerValue(hex, PlayerId::Player0), 5);
    EXPECT_EQ(Hex::unpackPlayerValue(hex, PlayerId::Player1), 0);
    
    // All previous values should still be intact
    EXPECT_EQ(Hex::unpackCatanNumber(hex), 8);
    EXPECT_EQ(Hex::unpackResource(hex), Resource::Ore);
    EXPECT_EQ(Hex::unpackPlayerValue(hex, PlayerId::Player0), 5);
}

TEST(NodeTest, IndividualFieldPacking) {
    Node::PackedNode node = 0;
    
    node = Node::packStructure(node, StructureType::City);
    EXPECT_EQ(Node::unpackStructure(node), StructureType::City);
    
    node = Node::packOwner(node, PlayerId::Player1);
    EXPECT_EQ(Node::unpackOwner(node), PlayerId::Player1);
    
    // Test each hex slot independently
    node = Node::packAdjacentHex(node, 0, 18);
    node = Node::packAdjacentHex(node, 1, 12);  
    node = Node::packAdjacentHex(node, 2, 6);
    EXPECT_EQ(Node::unpackAdjacentHex(node, 0), 18);
    EXPECT_EQ(Node::unpackAdjacentHex(node, 1), 12);
    EXPECT_EQ(Node::unpackAdjacentHex(node, 2), 6);
    
    node = Node::packPortType(node, PortType::WoolPort);
    EXPECT_EQ(Node::unpackPortType(node), PortType::WoolPort);
    
    // Previous values should remain
    EXPECT_EQ(Node::unpackStructure(node), StructureType::City);
    EXPECT_EQ(Node::unpackOwner(node), PlayerId::Player1);
}

TEST(EdgeTest, IndividualFieldPacking) {
    Edge::PackedEdge edge = 0;
    
    edge = Edge::packHasRoad(edge, true);
    EXPECT_EQ(Edge::unpackHasRoad(edge), true);
    
    edge = Edge::packOwner(edge, PlayerId::Player1);
    EXPECT_EQ(Edge::unpackOwner(edge), PlayerId::Player1);
    
    edge = Edge::packAdjacentNode(edge, 0, 45);
    edge = Edge::packAdjacentNode(edge, 1, 30);
    EXPECT_EQ(Edge::unpackAdjacentNode(edge, 0), 45);
    EXPECT_EQ(Edge::unpackAdjacentNode(edge, 1), 30);
    
    // Previous values should remain
    EXPECT_EQ(Edge::unpackHasRoad(edge), true);
    EXPECT_EQ(Edge::unpackOwner(edge), PlayerId::Player1);
}

TEST(BoardTest, ConstantValidation) {
    // Test that the constants are reasonable values for Catan
    EXPECT_EQ(HEX_COUNT, 19);    // Standard Catan board has 19 hexes
    EXPECT_EQ(NODE_COUNT, 54);   // Standard Catan board has 54 intersection points
    EXPECT_EQ(EDGE_COUNT, 72);   // Standard Catan board has 72 edges
}

TEST(BoardTest, BitBoundaries) {
    // Test maximum values for each type
    
    // Hex boundaries
    Hex::PackedHex hex = 0;
    hex = Hex::packCatanNumber(hex, 15);  // 4 bits max
    EXPECT_EQ(Hex::unpackCatanNumber(hex), 15);
    
    hex = 0;
    hex = Hex::packPlayerValue(hex, PlayerId::Player0, 7);  // 3 bits max
    EXPECT_EQ(Hex::unpackPlayerValue(hex, PlayerId::Player0), 7);
    
    // Node boundaries  
    Node::PackedNode node = 0;
    node = Node::packAdjacentHex(node, 0, 31);  // 5 bits max (19 is actual max for hex count)
    EXPECT_EQ(Node::unpackAdjacentHex(node, 0), 31);
    
    // Edge boundaries
    Edge::PackedEdge edge = 0;
    edge = Edge::packAdjacentNode(edge, 0, 127); // 7 bits max (54 is actual max for node count)
    EXPECT_EQ(Edge::unpackAdjacentNode(edge, 0), 127);
}

TEST(BoardTest, InitializedNodesAndEdges) {
    // The 'None' HexId was stored using HexId(-1) masked into 5 bits -> 31
    constexpr HexId NoneMasked = 0x1F;
    
    Board::BoardState boardState;
    // nodes[0]  = Node::makeNode(None, 0, None);
    Node::PackedNode n0 = boardState.nodes[0];
    EXPECT_EQ(Node::unpackAdjacentHex(n0, 0), NoneMasked);
    EXPECT_EQ(Node::unpackAdjacentHex(n0, 1), (HexId)0);
    EXPECT_EQ(Node::unpackAdjacentHex(n0, 2), NoneMasked);

    // nodes[9]  = Node::makeNode(3, 4, 0);
    Node::PackedNode n9 = boardState.nodes[9];
    EXPECT_EQ(Node::unpackAdjacentHex(n9, 0), (HexId)3);
    EXPECT_EQ(Node::unpackAdjacentHex(n9, 1), (HexId)4);
    EXPECT_EQ(Node::unpackAdjacentHex(n9, 2), (HexId)0);

    // nodes[53] = Node::makeNode(18,  None, None);
    Node::PackedNode n53 = boardState.nodes[53];
    EXPECT_EQ(Node::unpackAdjacentHex(n53, 0), (HexId)18);
    EXPECT_EQ(Node::unpackAdjacentHex(n53, 1), NoneMasked);
    EXPECT_EQ(Node::unpackAdjacentHex(n53, 2), NoneMasked);

    // edges[0]  = Edge::makeEdge(0, 1);
    Edge::PackedEdge e0 = boardState.edges[0];
    EXPECT_EQ(Edge::unpackAdjacentNode(e0, 0), (NodeId)0);
    EXPECT_EQ(Edge::unpackAdjacentNode(e0, 1), (NodeId)1);

    // edges[11] = Edge::makeEdge(8, 9);
    Edge::PackedEdge e11 = boardState.edges[11];
    EXPECT_EQ(Edge::unpackAdjacentNode(e11, 0), (NodeId)8);
    EXPECT_EQ(Edge::unpackAdjacentNode(e11, 1), (NodeId)9);

    // edges[71] = Edge::makeEdge(52, 53);
    Edge::PackedEdge e71 = boardState.edges[71];
    EXPECT_EQ(Edge::unpackAdjacentNode(e71, 0), (NodeId)52);
    EXPECT_EQ(Edge::unpackAdjacentNode(e71, 1), (NodeId)53);
}

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

    Action::PackedAction rollDiceAction;
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
    Action::PackedAction endTurnAction;
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

    Action::PackedAction moveRobberAction;
    moveRobberAction = Action::packType(moveRobberAction, ActionType::MoveRobber);
    moveRobberAction = Action::packArg1(moveRobberAction, newRobberPosition);
    moveRobberAction = Action::packPlayerID(moveRobberAction, stealingPlayer);
    boardState.applyAction(moveRobberAction);
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

    Action::PackedAction stealResourceAction;
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

    boardState.packedPlayers[static_cast<size_t>(discardingPlayer)] = Player::packResource(
        boardState.packedPlayers[static_cast<size_t>(discardingPlayer)],
        Resource::Wool,
        6
    );

    boardState.packedPlayers[static_cast<size_t>(discardingPlayer)] = Player::packResource(
        boardState.packedPlayers[static_cast<size_t>(discardingPlayer)],
        Resource::Grain,
        3
    );

    boardState.packedPlayers[static_cast<size_t>(discardingPlayer)] = Player::packResource(
        boardState.packedPlayers[static_cast<size_t>(discardingPlayer)],
        Resource::Ore,
        19
    );

    Action::PackedAction discardResourcesAction;
    discardResourcesAction = Action::packType(discardResourcesAction, ActionType::DiscardResources);
    discardResourcesAction = Action::packPlayerID(discardResourcesAction, discardingPlayer);

    boardState.applyAction(discardResourcesAction);

    auto sumResources = Player::totalResources(boardState.packedPlayers[static_cast<size_t>(discardingPlayer)]);
    EXPECT_EQ(sumResources, 14); // 50% of 28 rounded down is 14

    boardState.packedPlayers[static_cast<size_t>(discardingPlayer)] = Player::packResource(
        boardState.packedPlayers[static_cast<size_t>(discardingPlayer)],
        Resource::Lumber,
        19
    );

    boardState.applyAction(discardResourcesAction);
    sumResources = Player::totalResources(boardState.packedPlayers[static_cast<size_t>(discardingPlayer)]);
    EXPECT_EQ(sumResources, 17); // 50% of 33 rounded up is 17
}

TEST_F(ApplyActionTest, ExpectBuildRoad) {
    setBankResourcesTen();
    setPlayersResourcesSeven();
    PlayerId buildingPlayer = PlayerId::Player0;
    EdgeId roadEdgeId = 10;

    Action::PackedAction buildRoadAction;
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

    Action::PackedAction buildSettlementAction;
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

    boardState.packedPlayers[static_cast<size_t>(buildingPlayer)] = Player::packAvailableStructures(
        boardState.packedPlayers[static_cast<size_t>(buildingPlayer)],
        StructureType::Settlement,
        3
    );
    boardState.packedPlayers[static_cast<size_t>(buildingPlayer)] = Player::packVictoryPoints(
        boardState.packedPlayers[static_cast<size_t>(buildingPlayer)],
        1
    );

    Node::PackedNode& targetNode = boardState.nodes[cityNodeId];
    targetNode = Node::packStructure(targetNode, StructureType::Settlement);
    targetNode = Node::packOwner(targetNode, buildingPlayer);

    Action::PackedAction buildCityAction;
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

    Node::PackedNode cityNode = boardState.nodes[cityNodeId];
    EXPECT_EQ(Node::unpackStructure(cityNode), StructureType::City);
    EXPECT_EQ(Node::unpackOwner(cityNode), buildingPlayer);
    EXPECT_EQ(Player::unpackAvailableStructures(boardState.packedPlayers[static_cast<size_t>(buildingPlayer)], StructureType::City), 3);
    EXPECT_EQ(Player::unpackAvailableStructures(boardState.packedPlayers[static_cast<size_t>(buildingPlayer)], StructureType::Settlement), 4);
    EXPECT_EQ(Player::unpackVictoryPoints(boardState.packedPlayers[static_cast<size_t>(buildingPlayer)]), 2);
    
    HexId adjHex[3] = {
        Node::unpackAdjacentHex(cityNode, 0),
        Node::unpackAdjacentHex(cityNode, 1),
        Node::unpackAdjacentHex(cityNode, 2)
    };

    for (HexId h : adjHex) {
        if (h != HexIdNone) {
            EXPECT_EQ(
                Hex::unpackPlayerValue(boardState.hexes[h], buildingPlayer),
                2
            );
        }
    }
}

TEST_F(ApplyActionTest, ExpectBuyDevelopmentCard) {
    setBankResourcesTen();
    setPlayersResourcesSeven();

    boardState.packedBank = Bank::packDevCard(
        boardState.packedBank,
        DevType::VictoryPoint,
        5
    );

    PlayerId buyingPlayer = PlayerId::Player1;

    Action::PackedAction buyDevCardAction;
    buyDevCardAction = Action::packType(buyDevCardAction, ActionType::BuyDevCard);
    buyDevCardAction = Action::packPlayerID(buyDevCardAction, buyingPlayer);
    boardState.applyAction(buyDevCardAction);

    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Grain), 11);
    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Wool), 11);
    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Ore), 11);
    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Brick), 10);
    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, Resource::Lumber), 10);
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

TEST_F(ApplyActionTest, ExpectTradeBank) {
    setBankResourcesTen();
    setPlayersResourcesSeven();
    PlayerId tradingPlayer = PlayerId::Player0;
    Resource giveResource = Resource::Lumber;
    Resource receiveResource = Resource::Ore;

    Action::PackedAction tradeBankAction;
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

    Action::PackedAction receiveResourcesAction;
    receiveResourcesAction = Action::packType(receiveResourcesAction, ActionType::ReceiveResources);
    receiveResourcesAction = Action::packArg1(receiveResourcesAction, static_cast<uint8_t>(resToReceive));
    receiveResourcesAction = Action::packArg2(receiveResourcesAction, 4);
    receiveResourcesAction = Action::packPlayerID(receiveResourcesAction, receivingPlayer);
    boardState.applyAction(receiveResourcesAction);

    EXPECT_EQ(Bank::unpackResource(boardState.packedBank, resToReceive), 6);
    EXPECT_EQ(Player::unpackResource(boardState.packedPlayers[static_cast<size_t>(receivingPlayer)], resToReceive), 4);
}
