#pragma once

#include "board.hpp"
#include "actions.hpp"
#include "consts.hpp"
#include "player.hpp"
#include <vector>

namespace Board {

namespace GenerateActions {

struct PlayerPortInfo {
    bool hasThreeForOnePort = false;
    bool hasBrickPort = false;
    bool hasLumberPort = false;
    bool hasWoolPort = false;
    bool hasGrainPort = false;
    bool hasOrePort = false;
};

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

PlayerPortInfo detectPlayerPorts(const BoardState& board, PlayerId playerId) {
    PlayerPortInfo portInfo;
    
    for (NodeId nodeId = 0; nodeId < NODE_COUNT; ++nodeId) {
        StructureType structure = Node::unpackStructure(board.nodes[nodeId]);
        PlayerId owner = Node::unpackOwner(board.nodes[nodeId]);
        
        if (owner != playerId || 
            (structure != StructureType::Settlement && structure != StructureType::City)) {
            continue;
        }
        
        PortType portType = Node::unpackPortType(board.nodes[nodeId]);
        
        switch (portType) {
            case PortType::ThreeForOne:
                portInfo.hasThreeForOnePort = true;
                break;
            case PortType::BrickPort:
                portInfo.hasBrickPort = true;
                break;
            case PortType::LumberPort:
                portInfo.hasLumberPort = true;
                break;
            case PortType::WoolPort:
                portInfo.hasWoolPort = true;
                break;
            case PortType::GrainPort:
                portInfo.hasGrainPort = true;
                break;
            case PortType::OrePort:
                portInfo.hasOrePort = true;
                break;
            default:
                break;
        }
    }
    
    return portInfo;
}

std::vector<Action::PackedAction> generateBuildActions(
    const BoardState& board, 
    PlayerId playerId
) {
    std::vector<Action::PackedAction> buildActions;

    // Build road actions
    for (EdgeId edgeId = 0; edgeId < EDGE_COUNT; ++edgeId) {
        if (!Edge::unpackHasRoad(board.edges[edgeId])) {
            buildActions.push_back(buildAction(ActionType::BuildRoad, playerId, edgeId));
        }
    }

    // Build settlement actions
    for (NodeId nodeId = 0; nodeId < NODE_COUNT; ++nodeId) {
        if (Node::unpackStructure(board.nodes[nodeId]) == StructureType::NoStructure) {
            buildActions.push_back(buildAction(ActionType::BuildSettlement, playerId, nodeId));
        }
    }

    // Build city actions
    for (NodeId nodeId = 0; nodeId < NODE_COUNT; ++nodeId) {
        if (Node::unpackStructure(board.nodes[nodeId]) == StructureType::Settlement && 
            Node::unpackOwner(board.nodes[nodeId]) == playerId) {
            buildActions.push_back(buildAction(ActionType::BuildCity, playerId, nodeId));
        }
    }

    return buildActions;
}

std::vector<Action::PackedAction> generateTwoToOnePortTradeActions(
    const BoardState& board, 
    PlayerId playerId
) {
    std::vector<Action::PackedAction> tradeActions;
    
    auto &player = board.packedPlayers[static_cast<uint8_t>(playerId)];
    PlayerPortInfo portInfo = detectPlayerPorts(board, playerId);

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
                has2to1Port = portInfo.hasBrickPort;
                break;
            case Resource::Lumber:
                has2to1Port = portInfo.hasLumberPort;
                break;
            case Resource::Wool:
                has2to1Port = portInfo.hasWoolPort;
                break;
            case Resource::Grain:
                has2to1Port = portInfo.hasGrainPort;
                break;
            case Resource::Ore:
                has2to1Port = portInfo.hasOrePort;
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

std::vector<Action::PackedAction> generateThreeToOnePortTradeActions(
    const BoardState& board, 
    PlayerId playerId
) {
    std::vector<Action::PackedAction> tradeActions;
    
    auto &player = board.packedPlayers[static_cast<uint8_t>(playerId)];
    PlayerPortInfo portInfo = detectPlayerPorts(board, playerId);

    if (portInfo.hasThreeForOnePort) {
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

std::vector<Action::PackedAction> generateBankTradeActions(
    const BoardState& board, 
    PlayerId playerId
) {
    std::vector<Action::PackedAction> tradeActions;
    
    auto &player = board.packedPlayers[static_cast<uint8_t>(playerId)];
    PlayerPortInfo portInfo = detectPlayerPorts(board, playerId);

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

std::vector<Action::PackedAction> generateDevCardActions(
    const BoardState& board, 
    PlayerId playerId
) {
    std::vector<Action::PackedAction> devCardActions;
    
    auto &player = board.packedPlayers[static_cast<uint8_t>(playerId)];
    
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
            if (hexId != board.robberPosition) {
                devCardActions.push_back(buildAction(
                    ActionType::PlayDevCardKnight, playerId, hexId
                ));
            }
        }
    }
    
    // RoadBuilding cards
    if (Player::unpackDevCard(player, DevType::RoadBuilding) > 0) {
        for (EdgeId firstEdge = 0; firstEdge < EDGE_COUNT; ++firstEdge) {
            if (!Edge::unpackHasRoad(board.edges[firstEdge])) {
                for (EdgeId secondEdge = firstEdge + 1; secondEdge < EDGE_COUNT; ++secondEdge) {
                    if (!Edge::unpackHasRoad(board.edges[secondEdge])) {
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

} // namespace GenerateActions

} // namespace Board
