#include "game_simulation/board.hpp"
#include "greedyPlayer.hpp"
#include <algorithm>
#include <iostream>
#include <unordered_map>

constexpr uint8_t pipWeight[13] = {0,0,1,2,3,4,5,0,5,4,3,2,1};

uint8_t pipNodeScore(NodeId nodeId, Board::BoardState* board, Resource fValuableResource, Resource sValuableResource) {
    uint8_t base = 0;
    bool hasWood=false, hasBrick=false, hasWheat=false, hasOre=false, hasSheep=false;

    HexId adjHex[3] = {
        Board::Node::unpackAdjacentHex(board->nodes[nodeId], 0),
        Board::Node::unpackAdjacentHex(board->nodes[nodeId], 1),
        Board::Node::unpackAdjacentHex(board->nodes[nodeId], 2)
    };
    
    for (HexId h : adjHex) {
        if (h == HexIdNone) continue;
        Board::Hex::PackedHex hex = board->hexes[h];
        Resource resource = Board::Hex::unpackResource(hex);
        uint8_t number = Board::Hex::unpackCatanNumber(hex);
        base += pipWeight[number] * 2;

        switch(resource) {
            case Resource::Lumber: hasWood = true; break;
            case Resource::Brick: hasBrick = true; break;
            case Resource::Grain: hasWheat = true; break;
            case Resource::Ore: hasOre = true; break;
            case Resource::Wool: hasSheep = true; break;
            default: break;
        }
    }
    uint8_t score = base;
    uint8_t distinct = (hasWood + hasBrick + hasWheat + hasOre + hasSheep);
    if (distinct == 3) score += 5;
    else if (distinct == 2) score += 1;

    bool hasFValuableResource = false;
    bool hasSValuableResource = false;

    if (fValuableResource == Resource::Lumber && hasWood) hasFValuableResource = true;
    else if (fValuableResource == Resource::Brick && hasBrick) hasFValuableResource = true;
    else if (fValuableResource == Resource::Grain && hasWheat) hasFValuableResource = true;
    else if (fValuableResource == Resource::Ore && hasOre) hasFValuableResource = true;
    else if (fValuableResource == Resource::Wool && hasSheep) hasFValuableResource = true;

    if (sValuableResource == Resource::Lumber && hasWood) hasSValuableResource = true;
    else if (sValuableResource == Resource::Brick && hasBrick) hasSValuableResource = true;
    else if (sValuableResource == Resource::Grain && hasWheat) hasSValuableResource = true;
    else if (sValuableResource == Resource::Ore && hasOre) hasSValuableResource = true;
    else if (sValuableResource == Resource::Wool && hasSheep) hasSValuableResource = true;

    if (hasFValuableResource && hasSValuableResource) score += 2;
    else if (hasFValuableResource || hasSValuableResource) score += 1;

    if (base < 7) score -= 4;
    return score;
}

uint8_t pipEdgeScoreFromNode(NodeId startNodeId, EdgeId edgeId, Board::BoardState* board) {
    uint8_t score1 = 1;
    uint8_t score2 = 1;
    NodeId nodeA = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 0);
    NodeId nodeB = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 1);
    NodeId neighbourNode = (nodeA == startNodeId) ? nodeB : nodeA;

    // simulate buidling road to neighbourNode and all possible  roads from there
    std::vector<EdgeId> adjEdges = Board::Node::getAdjacentEdges(board->nodes[neighbourNode]);
    for (EdgeId adjEdgeId : adjEdges) {
        if (adjEdgeId == edgeId) continue;
        NodeId adjNodeA = Board::Edge::unpackAdjacentNode(board->edges[adjEdgeId], 0);
        NodeId adjNodeB = Board::Edge::unpackAdjacentNode(board->edges[adjEdgeId], 1);
        NodeId adjNeighbourNode = (adjNodeA == neighbourNode) ? adjNodeB : adjNodeA;
        uint8_t edgeScore = pipNodeScore(adjNeighbourNode, board, Resource::Ore, Resource::Grain);
        score1 = std::max(score1, edgeScore);
    }

    return score1;
}

uint8_t pipEdgeScore(EdgeId edgeId, Board::BoardState* board) {
    uint8_t score1 = 1;
    uint8_t score2 = 1;
    NodeId nodeA = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 0);
    NodeId nodeB = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 1);

    // simulate buidling road to nodeA and all possible  roads from there
    std::vector<EdgeId> adjEdgesA = Board::Node::getAdjacentEdges(board->nodes[nodeA]);
    for (EdgeId adjEdgeId : adjEdgesA) {
        if (adjEdgeId == edgeId) continue;
        NodeId adjNodeA = Board::Edge::unpackAdjacentNode(board->edges[adjEdgeId], 0);
        NodeId adjNodeB = Board::Edge::unpackAdjacentNode(board->edges[adjEdgeId], 1);
        NodeId adjNeighbourNode = (adjNodeA == nodeA) ? adjNodeB : adjNodeA;
        if (Board::Node::unpackStructure(board->nodes[adjNeighbourNode]) != StructureType::NoStructure) continue;
        uint8_t edgeScore = pipNodeScore(adjNeighbourNode, board, Resource::Ore, Resource::Grain);
        score1 = std::max(score1, edgeScore);
    }

    // simulate buidling road to nodeB and all possible  roads from there
    std::vector<EdgeId> adjEdgesB = Board::Node::getAdjacentEdges(board->nodes[nodeB]);
    for (EdgeId adjEdgeId : adjEdgesB) {
        if (adjEdgeId == edgeId) continue;
        NodeId adjNodeA = Board::Edge::unpackAdjacentNode(board->edges[adjEdgeId], 0);
        NodeId adjNodeB = Board::Edge::unpackAdjacentNode(board->edges[adjEdgeId], 1);
        NodeId adjNeighbourNode = (adjNodeA == nodeB) ? adjNodeB : adjNodeA;
        uint8_t edgeScore = pipNodeScore(adjNeighbourNode, board, Resource::Ore, Resource::Grain);
        score2 = std::max(score2, edgeScore);
    }

    return std::max(score1, score2);
}

uint8_t pipHexScore(HexId hexId, Board::BoardState* board) {
    Board::Hex::PackedHex hex = board->hexes[hexId];
    Resource resource = Board::Hex::unpackResource(hex);
    uint8_t number = Board::Hex::unpackCatanNumber(hex);

    uint8_t personalValue = Player::unpackResource(
        board->packedPlayers[static_cast<uint8_t>(board->currentPlayer)],
        resource
    );
    if (personalValue > 0) return -1;

    uint8_t enemyValue = Player::unpackResource(
        board->packedPlayers[static_cast<uint8_t>(board->currentPlayer == PlayerId::Player0 ? PlayerId::Player1 : PlayerId::Player0)],
        resource
    );

    return pipWeight[number] * 2 + enemyValue;
}

std::pair<Action::PackedAction, Action::PackedAction> GreedyPlayer::getInitialPlacement() {
    auto actions = boardState->generatePlaceInitialStructures(boardState->currentPlayer);

    Action::PackedAction bestAction = actions[0];
    NodeId bestNodeId = 0;
    uint8_t bestNodeScore = 0;
    uint8_t bestEdgeScore = 0;

    for (auto action : actions) {
        NodeId nodeId = Action::unpackArg1(action);
        uint8_t nodeScore = pipNodeScore(nodeId, boardState, Resource::Ore, Resource::Grain);
        std::cout << "Node " << static_cast<int>(nodeId) << " has score " << static_cast<int>(nodeScore) << "\n";
        if (nodeScore > bestNodeScore) {
            bestNodeScore = nodeScore;
            bestAction = action;
            bestNodeId = nodeId;
        }
    }

    std::vector<EdgeId> adjEdges = Board::Node::getAdjacentEdges(boardState->nodes[bestNodeId]);

    for (EdgeId edgeId : adjEdges) {
        uint8_t edgeScore = pipEdgeScoreFromNode(bestNodeId, edgeId, boardState);
        std::cout << "  Node : " << static_cast<int>(bestNodeId) << "\n";
        std::cout << "  Edge " << static_cast<int>(edgeId) << " has score " << static_cast<int>(edgeScore) << "\n";
        if (edgeScore > bestEdgeScore) {
            bestEdgeScore = edgeScore;
            bestAction = Action::packArg2(bestAction, edgeId);
        }
    }

    return {bestAction, bestAction};
}

std::pair<Action::PackedAction, Action::PackedAction> GreedyPlayer::get2InitialPlacement() {
    auto actions = boardState->generatePlaceInitialStructures(boardState->currentPlayer);

    Action::PackedAction bestAction = actions[0];
    NodeId bestNodeId = 0;
    uint8_t bestNodeScore = 0;
    uint8_t bestEdgeScore = 0;

    for (auto action : actions) {
        NodeId nodeId = Action::unpackArg1(action);
        uint8_t nodeScore = pipNodeScore(nodeId, boardState, Resource::Brick, Resource::Lumber);
        if (nodeScore > bestNodeScore) {
            bestNodeScore = nodeScore;
            bestAction = action;
            bestNodeId = nodeId;
        }
    }

    std::vector<EdgeId> adjEdges = Board::Node::getAdjacentEdges(boardState->nodes[bestNodeId]);

    for (EdgeId edgeId : adjEdges) {
        uint8_t edgeScore = pipEdgeScoreFromNode(bestNodeId, edgeId, boardState);
        if (edgeScore > bestEdgeScore) {
            bestEdgeScore = edgeScore;
            bestAction = Action::packArg2(bestAction, edgeId);
        }
    }

    return {bestAction, bestAction};
}

Action::PackedAction GreedyPlayer::getDevAction() {
    return Action::getEmptyAction();
}

Action::PackedAction GreedyPlayer::getDiscardAction() {
    Action::PackedAction action = Action::getEmptyAction();
    uint8_t totalResources = Player::totalResources(boardState->packedPlayers[static_cast<uint8_t>(boardState->currentPlayer)]);
    uint8_t toDiscard = totalResources / 2;
    uint8_t discarded = 0;
    std::vector<Resource> resourcePool;

    // 0 - Wool, 1 - Brick, 2 - Lumber, 3 - Grain, 4 - Ore
    std::vector<uint8_t> resourcesCount;
    std::vector<Resource> priority = {Resource::Wool, Resource::Brick, Resource::Lumber, Resource::Grain, Resource::Ore};

    for (Resource r : {
            Resource::Wool,
            Resource::Brick,
            Resource::Lumber,
            Resource::Grain,
            Resource::Ore
        }) {

        uint8_t count = Player::unpackResource(boardState->packedPlayers[static_cast<uint8_t>(boardState->currentPlayer)], r);
        resourcesCount.push_back(count);
        for (uint8_t i = 0; i < count; ++i) {
            resourcePool.push_back(r);
        }
    }

    // Now half of the remaining lowest priority resources
    for (size_t i = 0; i < priority.size(); ++i) {
        Resource r = priority[i];
        uint8_t toGive = std::min(resourcesCount[i] / 2 + 1, toDiscard - discarded);
        action = Action::packResource(action, r, toGive);
        discarded += toGive;
        if (discarded >= toDiscard) break;
    }

    // If still need to discard, discard from the lowest priority resources
    for (size_t i = 0; i < priority.size(); ++i) {
        Resource r = priority[i];
        uint8_t currentAmount = Action::unpackResource(action, r);
        uint8_t available = Player::unpackResource(boardState->packedPlayers[static_cast<uint8_t>(boardState->currentPlayer)], r) - currentAmount;
        uint8_t toGive = std::min<uint8_t>(
            available,
            static_cast<uint8_t>(toDiscard - discarded)
        );
        action = Action::packResource(action, r, currentAmount + toGive);
        discarded += toGive;
        if (discarded >= toDiscard) break;
    }

    return action;
}

Action::PackedAction GreedyPlayer::getMoveRobber() {
    auto actions = boardState->generateMoveRobberActions(boardState->currentPlayer);
    Action::PackedAction bestAction = actions[0];
    uint8_t bestScore = 0;

    for (auto action : actions) {
        HexId hexId = Action::unpackArg1(action);
        uint8_t score = pipHexScore(hexId, boardState);

        if (score > bestScore) {
            bestScore = score;
            bestAction = action;
        }
    }

    return bestAction;
}

Action::PackedAction GreedyPlayer::getTurnAction() {
    auto actions = boardState->getLegalActions(boardState->currentPlayer);

    Action::PackedAction bestAction = actions[0];
    uint8_t bestScore = 0;

    for (auto action : actions) {
        uint8_t score = 0;
        ActionType type = Action::unpackType(action);

        switch (type) {
            case ActionType::BuildSettlement: {
                NodeId nodeId = Action::unpackArg1(action);
                score = pipNodeScore(nodeId, boardState, Resource::Ore, Resource::Grain);
                break;
            }
            case ActionType::BuildRoad: {
                EdgeId edgeId = Action::unpackArg1(action);
                score = pipEdgeScore(edgeId, boardState);
                break;
            }
            case ActionType::BuildCity: {
                NodeId nodeId = Action::unpackArg1(action);
                score = pipNodeScore(nodeId, boardState, Resource::Ore, Resource::Grain) + 5;
                break;
            }
            default:
                break;
        }

        if (score > bestScore) {
            bestScore = score;
            bestAction = action;
        }
    }

    return bestAction;
}