#include <gtest/gtest.h>
#include "board.hpp"

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