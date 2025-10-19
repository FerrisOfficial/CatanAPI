#include "board.hpp"

namespace Board {

// Global board state
struct BoardState {
    HexId robberPosition = 0;
    
    // Board arrays - using the constants from consts.hpp
    Hex::PackedHex hexes[HEX_COUNT] = {0};
    Node::PackedNode nodes[NODE_COUNT] = {0};  
    Edge::PackedEdge edges[EDGE_COUNT] = {0};
};

} // namespace Board