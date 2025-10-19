#include "board.hpp"

namespace Board {

// Global board state
struct BoardState {
    HexId robberPosition = 0;
    
    // Board arrays - using the constants from consts.hpp
    Hex::PackedHex hexes[HEX_COUNT] = {0};
    Node::PackedNode nodes[NODE_COUNT] = {0};  
    Edge::PackedEdge edges[EDGE_COUNT] = {0};

    BoardState() {
        nodes[0] = Node::makeNode(5, 10, 15);
        nodes[1] = Node::makeNode(0, 1, 2);
        //itd

        edges[0] = Edge::makeEdge(0, 1);
        edges[1] = Edge::makeEdge(1, 2);
        // itd
    }
};

} // namespace Board