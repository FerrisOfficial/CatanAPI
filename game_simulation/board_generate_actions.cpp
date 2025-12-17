#include "board.hpp"
#include "actions.hpp"
#include "consts.hpp"
#include "player.hpp"
#include <vector>

namespace Board {

Action::PackedAction buildAction(
    ActionType type, 
    PlayerId playerId, 
    uint8_t arg1 = 0, 
    uint8_t arg2 = 0, 
    uint8_t arg3 = 0
) {
    Action::PackedAction action = 0;
    action = Action::packType(action, type);
    action = Action::packPlayerID(action, playerId);
    action = Action::packArg1(action, arg1);
    action = Action::packArg2(action, arg2);
    action = Action::packArg3(action, arg3);
    return action;
}

static bool playerHasAdjacentRoad(
    const BoardState& board, 
    PlayerId playerId, 
    EdgeId edgeId
) {
    auto edge = board.edges[edgeId];
    NodeId nodeA = Edge::unpackAdjacentNode(edge, 0);
    NodeId nodeB = Edge::unpackAdjacentNode(edge, 1);

    std::vector<EdgeId> adjacentEdges;
    // Dodać strukturę do pobierania krawędzi sąsiednich do node'ów albo strukturę do pobierania sąsiednich krawędzi dla edge'ów
    return false;
}

static bool playerHasAdjacentSettlementOrCity(
    const BoardState& board, 
    PlayerId playerId, 
    EdgeId edgeId
) {
    auto edge = board.edges[edgeId];
    NodeId nodeA = Edge::unpackAdjacentNode(edge, 0);
    NodeId nodeB = Edge::unpackAdjacentNode(edge, 1);

    StructureType structureA = Node::unpackStructure(board.nodes[nodeA]);
    PlayerId ownerA = Node::unpackOwner(board.nodes[nodeA]);
    if (ownerA == playerId && (structureA == StructureType::Settlement || structureA == StructureType::City)) {
        return true;
    }

    StructureType structureB = Node::unpackStructure(board.nodes[nodeB]);
    PlayerId ownerB = Node::unpackOwner(board.nodes[nodeB]);
    if (ownerB == playerId && (structureB == StructureType::Settlement || structureB == StructureType::City)) {
        return true;
    }

    return false;
}

static bool playerCanAffordBuildRoad(
    const BoardState& board, 
    PlayerId playerId
) {
    const auto& player = board.packedPlayers[static_cast<uint8_t>(playerId)];
    return Player::unpackResource(player, Resource::Lumber) >= 1 &&
           Player::unpackResource(player, Resource::Brick) >= 1;
}

static bool playerCanAffordBuildSettlement(
    const BoardState& board, 
    PlayerId playerId
) {
    const auto& player = board.packedPlayers[static_cast<uint8_t>(playerId)];
    return Player::unpackResource(player, Resource::Lumber) >= 1 &&
           Player::unpackResource(player, Resource::Brick) >= 1 &&
           Player::unpackResource(player, Resource::Wool) >= 1 &&
           Player::unpackResource(player, Resource::Grain) >= 1;
}

static bool playerCanAffordBuildCity(
    const BoardState& board, 
    PlayerId playerId
) {
    const auto& player = board.packedPlayers[static_cast<uint8_t>(playerId)];
    return Player::unpackResource(player, Resource::Grain) >= 2 &&
           Player::unpackResource(player, Resource::Ore) >= 3;
}

std::vector<Action::PackedAction> generateBuildRoadActions(
    const BoardState& board, 
    PlayerId playerId
) {
    std::vector<Action::PackedAction> buildRoadActions;

    for (EdgeId edgeId = 0; edgeId < EDGE_COUNT; ++edgeId) {
        if (!Edge::unpackHasRoad(board.edges[edgeId]) &&
            playerCanAffordBuildRoad(board, playerId) &&
            (playerHasAdjacentRoad(board, playerId, edgeId) || playerHasAdjacentSettlementOrCity(board, playerId, edgeId))) {
            buildRoadActions.push_back(buildAction(ActionType::BuildRoad, playerId, edgeId));
        }
    }

    return buildRoadActions;
}

std::vector<Action::PackedAction> BoardState::generateBuildActions(
    PlayerId playerId
) {
    std::vector<Action::PackedAction> buildActions;

    // Build settlement actions
    for (NodeId nodeId = 0; nodeId < NODE_COUNT; ++nodeId) {
        if (Node::unpackStructure(nodes[nodeId]) == StructureType::NoStructure) {
            buildActions.push_back(buildAction(ActionType::BuildSettlement, playerId, nodeId));
        }
    }

    // Build city actions
    for (NodeId nodeId = 0; nodeId < NODE_COUNT; ++nodeId) {
        if (Node::unpackStructure(nodes[nodeId]) == StructureType::Settlement && 
            Node::unpackOwner(nodes[nodeId]) == playerId) {
            buildActions.push_back(buildAction(ActionType::BuildCity, playerId, nodeId));
        }
    }

    return buildActions;
}

std::vector<Action::PackedAction> BoardState::generateTwoToOnePortTradeActions(
    PlayerId playerId
) {
    std::vector<Action::PackedAction> tradeActions;
    
    auto &player = packedPlayers[static_cast<uint8_t>(playerId)];
    
    // Detect which 2:1 ports the player owns by checking specific port nodes
    bool hasBrickPort = false;
    bool hasLumberPort = false;
    bool hasWoolPort = false;
    bool hasGrainPort = false;
    bool hasOrePort = false;
    
    // Check Brick Ports (nodes 15, 25)
    for (NodeId nodeId : {15, 25}) {
        StructureType structure = Node::unpackStructure(nodes[nodeId]);
        PlayerId owner = Node::unpackOwner(nodes[nodeId]);
        if (owner == playerId && (structure == StructureType::Settlement || structure == StructureType::City)) {
            hasBrickPort = true;
        }
    }
    
    // Check Lumber Ports (nodes 36, 46)
    for (NodeId nodeId : {36, 46}) {
        StructureType structure = Node::unpackStructure(nodes[nodeId]);
        PlayerId owner = Node::unpackOwner(nodes[nodeId]);
        if (owner == playerId && (structure == StructureType::Settlement || structure == StructureType::City)) {
            hasLumberPort = true;
        }
    }
    
    // Check Wool Ports (nodes 7, 8)
    for (NodeId nodeId : {7, 8}) {
        StructureType structure = Node::unpackStructure(nodes[nodeId]);
        PlayerId owner = Node::unpackOwner(nodes[nodeId]);
        if (owner == playerId && (structure == StructureType::Settlement || structure == StructureType::City)) {
            hasWoolPort = true;
        }
    }
    
    // Check Grain Ports (nodes 49, 50)
    for (NodeId nodeId : {49, 50}) {
        StructureType structure = Node::unpackStructure(nodes[nodeId]);
        PlayerId owner = Node::unpackOwner(nodes[nodeId]);
        if (owner == playerId && (structure == StructureType::Settlement || structure == StructureType::City)) {
            hasGrainPort = true;
        }
    }
    
    // Check Ore Ports (nodes 38, 39)
    for (NodeId nodeId : {38, 39}) {
        StructureType structure = Node::unpackStructure(nodes[nodeId]);
        PlayerId owner = Node::unpackOwner(nodes[nodeId]);
        if (owner == playerId && (structure == StructureType::Settlement || structure == StructureType::City)) {
            hasOrePort = true;
        }
    }

    for (Resource giveResource : {
        Resource::Brick,
        Resource::Lumber,
        Resource::Wool,
        Resource::Grain,
        Resource::Ore
    }) {
        uint8_t playerHas = Player::unpackResource(player, giveResource);
        
        bool has2to1Port = false;
        switch (giveResource) {
            case Resource::Brick:
                has2to1Port = hasBrickPort;
                break;
            case Resource::Lumber:
                has2to1Port = hasLumberPort;
                break;
            case Resource::Wool:
                has2to1Port = hasWoolPort;
                break;
            case Resource::Grain:
                has2to1Port = hasGrainPort;
                break;
            case Resource::Ore:
                has2to1Port = hasOrePort;
                break;
            default:
                break;
        }
        
        if (has2to1Port) {
            for (Resource receiveResource : {
                Resource::Brick,
                Resource::Lumber,
                Resource::Wool,
                Resource::Grain,
                Resource::Ore
            }) {
                if (giveResource == receiveResource) continue;
                
                for (uint8_t tradeCount = 1; tradeCount * 2 <= playerHas; ++tradeCount) {
                    tradeActions.push_back(buildAction(
                        ActionType::TradeBank, playerId, 
                        static_cast<uint8_t>(giveResource), 
                        static_cast<uint8_t>(receiveResource), 
                        2
                    ));
                }
            }
        }
    }

    return tradeActions;
}

std::vector<Action::PackedAction> BoardState::generateThreeToOnePortTradeActions(
    PlayerId playerId
) {
    std::vector<Action::PackedAction> tradeActions;
    
    auto &player = packedPlayers[static_cast<uint8_t>(playerId)];
    
    // Check ThreeForOne Ports (nodes 2, 3, 5, 6, 16, 27, 52, 53)
    bool hasThreeForOnePort = false;
    for (NodeId nodeId : {2, 3, 5, 6, 16, 27, 52, 53}) {
        StructureType structure = Node::unpackStructure(nodes[nodeId]);
        PlayerId owner = Node::unpackOwner(nodes[nodeId]);
        if (owner == playerId && (structure == StructureType::Settlement || structure == StructureType::City)) {
            hasThreeForOnePort = true;
            break;
        }
    }

    if (hasThreeForOnePort) {
        for (Resource giveResource : {
            Resource::Brick,
            Resource::Lumber,
            Resource::Wool,
            Resource::Grain,
            Resource::Ore
        }) {
            uint8_t playerHas = Player::unpackResource(player, giveResource);
            
            for (Resource receiveResource : {
                Resource::Brick,
                Resource::Lumber,
                Resource::Wool,
                Resource::Grain,
                Resource::Ore
            }) {
                if (giveResource == receiveResource) continue;
                
                for (uint8_t tradeCount = 1; tradeCount * 3 <= playerHas; ++tradeCount) {
                    tradeActions.push_back(buildAction(
                        ActionType::TradeBank, playerId, 
                        static_cast<uint8_t>(giveResource), 
                        static_cast<uint8_t>(receiveResource), 
                        3
                    ));
                }
            }
        }
    }

    return tradeActions;
}

std::vector<Action::PackedAction> BoardState::generateBankTradeActions(
    PlayerId playerId
) {
    std::vector<Action::PackedAction> tradeActions;
    
    auto &player = packedPlayers[static_cast<uint8_t>(playerId)];
    for (Resource giveResource : {
        Resource::Brick,
        Resource::Lumber,
        Resource::Wool,
        Resource::Grain,
        Resource::Ore
    }) {
        uint8_t playerHas = Player::unpackResource(player, giveResource);
        
        for (Resource receiveResource : {
            Resource::Brick,
            Resource::Lumber,
            Resource::Wool,
            Resource::Grain,
            Resource::Ore
        }) {
            if (giveResource == receiveResource) continue;
            
            // 4:1 trades
            for (uint8_t tradeCount = 1; tradeCount * 4 <= playerHas; ++tradeCount) {
                tradeActions.push_back(buildAction(
                    ActionType::TradeBank, playerId, 
                    static_cast<uint8_t>(giveResource), 
                    static_cast<uint8_t>(receiveResource), 
                    4
                ));
            }
        }
    }

    return tradeActions;
}

std::vector<Action::PackedAction> BoardState::generateDevCardActions(
    PlayerId playerId
) {
    std::vector<Action::PackedAction> devCardActions;
    
    auto &player = packedPlayers[static_cast<uint8_t>(playerId)];
    
    // BuyDevCard actions
    uint8_t playerGrain = Player::unpackResource(player, Resource::Grain);
    uint8_t playerOre = Player::unpackResource(player, Resource::Ore);
    uint8_t playerWool = Player::unpackResource(player, Resource::Wool);
    uint8_t maxAffordable = std::min({playerGrain, playerOre, playerWool});
    
    for (uint8_t cardCount = 1; cardCount <= maxAffordable; ++cardCount) {
        devCardActions.push_back(buildAction(ActionType::BuyDevCard, playerId));
    }
        
    // Knight cards
    if (Player::unpackDevCard(player, DevType::Knight) > 0) {
        for (HexId hexId = 0; hexId < HEX_COUNT; ++hexId) {
            if (hexId != robberPosition) {
                devCardActions.push_back(buildAction(
                    ActionType::PlayDevCardKnight, playerId, hexId
                ));
            }
        }
    }
    
    // RoadBuilding cards
    if (Player::unpackDevCard(player, DevType::RoadBuilding) > 0) {
        for (EdgeId firstEdge = 0; firstEdge < EDGE_COUNT; ++firstEdge) {
            if (!Edge::unpackHasRoad(edges[firstEdge])) {
                for (EdgeId secondEdge = firstEdge + 1; secondEdge < EDGE_COUNT; ++secondEdge) {
                    if (!Edge::unpackHasRoad(edges[secondEdge])) {
                        devCardActions.push_back(buildAction(
                            ActionType::PlayDevCardRoadBuilding, playerId, 
                            firstEdge, secondEdge
                        ));
                    }
                }
            }
        }
    }
    
    // YearOfPlenty cards
    if (Player::unpackDevCard(player, DevType::YearOfPlenty) > 0) {
        for (Resource firstResource : {
            Resource::Brick, Resource::Lumber, Resource::Wool, 
            Resource::Grain, Resource::Ore
        }) {
            for (Resource secondResource : {
                Resource::Brick, Resource::Lumber, Resource::Wool, 
                Resource::Grain, Resource::Ore
            }) {
                devCardActions.push_back(buildAction(
                    ActionType::PlayDevCardYearOfPlenty, playerId, 
                    static_cast<uint8_t>(firstResource), 
                    static_cast<uint8_t>(secondResource)
                ));
            }
        }
    }
    
    // Monopoly cards
    if (Player::unpackDevCard(player, DevType::Monopoly) > 0) {
        for (Resource targetResource : {
            Resource::Brick, Resource::Lumber, Resource::Wool, 
            Resource::Grain, Resource::Ore
        }) {
            devCardActions.push_back(buildAction(
                ActionType::PlayDevCardMonopoly, playerId, 
                static_cast<uint8_t>(targetResource)
            ));
        }
    }
        
    return devCardActions;
}

// Nie można zagrać tą kartą którą przed chwilą się kupiło (??)
// można kupić dowolną liczbę kart, ale zac jedną
std::vector<Action::PackedAction> BoardState::getLegalActions(PlayerId playerId){
    std::vector<Action::PackedAction> legalActions;
    
    auto buildActions = generateBuildActions(playerId);
    auto bankTradeActions = generateBankTradeActions(playerId);
    auto twoToOnePortTradeActions = generateTwoToOnePortTradeActions(playerId);
    auto threeToOnePortTradeActions = generateThreeToOnePortTradeActions(playerId);
    auto devCardActions = generateDevCardActions(playerId);
    
    legalActions.insert(legalActions.end(), buildActions.begin(), buildActions.end());
    legalActions.insert(legalActions.end(), bankTradeActions.begin(), bankTradeActions.end());
    legalActions.insert(legalActions.end(), twoToOnePortTradeActions.begin(), twoToOnePortTradeActions.end());
    legalActions.insert(legalActions.end(), threeToOnePortTradeActions.begin(), threeToOnePortTradeActions.end());
    legalActions.insert(legalActions.end(), devCardActions.begin(), devCardActions.end());
    
    return legalActions;
}



} // namespace Board
