#include "board.hpp"
#include "actions.hpp"
#include "consts.hpp"
#include "player.hpp"
#include <vector>
#include <iostream>
#include <algorithm>

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
    NodeId adjacentNode1 = Edge::unpackAdjacentNode(edge, 0);
    NodeId adjacentNode2 = Edge::unpackAdjacentNode(edge, 1);

    EdgeId adjacentEdge1 = Node::unpackAdjacentEdge(board.nodes[adjacentNode1], 0);
    EdgeId adjacentEdge2 = Node::unpackAdjacentEdge(board.nodes[adjacentNode1], 1);
    EdgeId adjacentEdge3 = Node::unpackAdjacentEdge(board.nodes[adjacentNode1], 2);
    EdgeId adjacentEdge4 = Node::unpackAdjacentEdge(board.nodes[adjacentNode2], 0);
    EdgeId adjacentEdge5 = Node::unpackAdjacentEdge(board.nodes[adjacentNode2], 1);
    EdgeId adjacentEdge6 = Node::unpackAdjacentEdge(board.nodes[adjacentNode2], 2);

    for (EdgeId adjacentEdgeId : {adjacentEdge1, adjacentEdge2, adjacentEdge3, adjacentEdge4, adjacentEdge5, adjacentEdge6}) {
        if (adjacentEdgeId != EdgeIdNone && adjacentEdgeId != edgeId) {
            auto adjacentEdge = board.edges[adjacentEdgeId];
            if (Edge::unpackHasRoad(adjacentEdge) && 
                Edge::unpackOwner(adjacentEdge) == playerId) {
                return true;
            }
        }
    }

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

static bool playerHasAvailableStructure(
    const BoardState& board, 
    PlayerId playerId,
    StructureType structureType
) {
    const auto& player = board.packedPlayers[static_cast<uint8_t>(playerId)];
    return Player::unpackAvailableStructures(player, structureType) > 0;
}

std::vector<Action::PackedAction> BoardState::generateBuildRoadActions(
    PlayerId playerId
) {
    std::vector<Action::PackedAction> buildRoadActions;

    for (EdgeId edgeId = 0; edgeId < EDGE_COUNT; ++edgeId) {
        if (!Edge::unpackHasRoad(edges[edgeId]) &&
            Player::hasEnoughResources(packedPlayers[static_cast<uint8_t>(playerId)], BuyableType::Road) &&
            playerHasAvailableStructure(*this, playerId, StructureType::Road) &&
            (playerHasAdjacentRoad(*this, playerId, edgeId) || playerHasAdjacentSettlementOrCity(*this, playerId, edgeId))) {
            buildRoadActions.push_back(buildAction(ActionType::BuildRoad, playerId, edgeId));
        }
    }

    return buildRoadActions;
}

std::vector<Action::PackedAction> BoardState::generateBuildSettlementActions(
    PlayerId playerId
) {
    std::vector<Action::PackedAction> buildSettlementActions;

    for (NodeId nodeId = 0; nodeId < NODE_COUNT; ++nodeId) {
        if (Node::unpackStructure(nodes[nodeId]) == StructureType::NoStructure &&
            Player::hasEnoughResources(packedPlayers[static_cast<uint8_t>(playerId)], BuyableType::Settlement) &&
            playerHasAvailableStructure(*this, playerId, StructureType::Settlement)) {
            
            // Check distance rule: no adjacent settlements/cities and at least one adjacent road
            bool hasAdjacentSettlementOrCity = false;
            bool hasAtLeastOneAdjacentRoad = false;

            for (int i = 0; i < 3; ++i) {
                EdgeId adjacentEdgeId = Node::unpackAdjacentEdge(nodes[nodeId], i);
                
                if (Edge::unpackHasRoad(edges[adjacentEdgeId]) &&
                    Edge::unpackOwner(edges[adjacentEdgeId]) == playerId) {
                    hasAtLeastOneAdjacentRoad = true;
                }

                if (adjacentEdgeId != EdgeIdNone) {
                    Edge::PackedEdge adjacentEdge = edges[adjacentEdgeId];
                    NodeId adjacentNodeA = Edge::unpackAdjacentNode(adjacentEdge, 0);
                    NodeId adjacentNodeB = Edge::unpackAdjacentNode(adjacentEdge, 1);
                    
                    for (NodeId adjacentNodeId : {adjacentNodeA, adjacentNodeB}) {
                        if (adjacentNodeId != nodeId) {
                            StructureType structure = Node::unpackStructure(nodes[adjacentNodeId]);
                            if (structure == StructureType::Settlement || structure == StructureType::City) {
                                hasAdjacentSettlementOrCity = true;
                                break;
                            }
                        }
                    }
                }
                if (hasAdjacentSettlementOrCity) break;
            }
            if (hasAdjacentSettlementOrCity || !hasAtLeastOneAdjacentRoad) continue;
            
            buildSettlementActions.push_back(buildAction(ActionType::BuildSettlement, playerId, nodeId));
        }
    }

    return buildSettlementActions;
}

std::vector<Action::PackedAction> BoardState::generateBuildCityActions(
    PlayerId playerId
) {
    std::vector<Action::PackedAction> buildCityActions;

    for (NodeId nodeId = 0; nodeId < NODE_COUNT; ++nodeId) {
        if (Node::unpackStructure(nodes[nodeId]) == StructureType::Settlement &&
            Node::unpackOwner(nodes[nodeId]) == playerId &&
            Player::hasEnoughResources(packedPlayers[static_cast<uint8_t>(playerId)], BuyableType::City) &&
            playerHasAvailableStructure(*this, playerId, StructureType::City)) {
            buildCityActions.push_back(buildAction(ActionType::BuildCity, playerId, nodeId));
        }
    }

    return buildCityActions;
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

std::vector<Action::PackedAction> BoardState::generateBuyDevCardActions(
    PlayerId playerId
) {
    std::vector<Action::PackedAction> buyDevCardActions;
    
    auto &player = packedPlayers[static_cast<uint8_t>(playerId)];
    
    // BuyDevCard actions
    uint8_t playerGrain = Player::unpackResource(player, Resource::Grain);
    uint8_t playerOre = Player::unpackResource(player, Resource::Ore);
    uint8_t playerWool = Player::unpackResource(player, Resource::Wool);
    uint8_t playerCanBuy = std::min({playerGrain, playerOre, playerWool});
    uint8_t availableDevCards = Bank::unpackTotalDevCount(packedBank);
    uint8_t maxAffordable = std::min(playerCanBuy, availableDevCards);

    for (uint8_t cardCount = 1; cardCount <= maxAffordable; ++cardCount) {
        buyDevCardActions.push_back(buildAction(ActionType::BuyDevCard, playerId));
    }
    
    return buyDevCardActions;
}

std::vector<Action::PackedAction> BoardState::generatePlayDevCardKnightActions(
    PlayerId playerId
) {
    std::vector<Action::PackedAction> knightActions;
    
    auto &player = packedPlayers[static_cast<uint8_t>(playerId)];

    if (Player::unpackDevCard(player, DevType::Knight) > 0) {
        for (HexId hexId = 0; hexId < HEX_COUNT; ++hexId) {
            if (hexId != robberPosition) {
                knightActions.push_back(buildAction(
                    ActionType::PlayDevCardKnight, playerId, hexId
                ));
            }
        }
    }

    return knightActions;
}

static bool edgeIsValidForRoadBuildingDevCard(
    const BoardState& board,
    PlayerId playerId,
    EdgeId edgeId
) {
    return !Edge::unpackHasRoad(board.edges[edgeId]) &&
           playerHasAvailableStructure(board, playerId, StructureType::Road) &&
           (playerHasAdjacentRoad(board, playerId, edgeId) ||
            playerHasAdjacentSettlementOrCity(board, playerId, edgeId));
}

// Zawiera duplikaty (budowanie dwóch dróg na tych samych krawędziach w różnej kolejności)
std::vector<Action::PackedAction> BoardState::generatePlayDevCardRoadBuildingActions(
    PlayerId playerId
) {
    std::vector<Action::PackedAction> roadBuildingActions;
    
    auto &player = packedPlayers[static_cast<uint8_t>(playerId)];
    
    if (Player::unpackDevCard(player, DevType::RoadBuilding) > 0) {
        const uint8_t availableRoads =
            Player::unpackAvailableStructures(player, StructureType::Road);

        if (availableRoads >= 1) {
            std::vector<EdgeId> firstCandidates;
            firstCandidates.reserve(EDGE_COUNT);

            for (EdgeId e = 0; e < EDGE_COUNT; ++e) {
                if (edgeIsValidForRoadBuildingDevCard(*this, playerId, e)) {
                    firstCandidates.push_back(e);
                }
            }

            if (availableRoads == 1) {
                // Only one road can be built
                for (EdgeId firstEdge : firstCandidates) {
                    roadBuildingActions.push_back(buildAction(
                        ActionType::PlayDevCardRoadBuilding,
                        playerId,
                        firstEdge,
                        EdgeIdNone
                    ));
                }
            } else {
                // Two roads can be built
                for (size_t i = 0; i < firstCandidates.size(); ++i) {
                    EdgeId firstEdge = firstCandidates[i];

                    // Temporarily modify board to account for first road being built
                    BoardState tempBoard = *this;
                    tempBoard.handleBuildRoad(buildAction(
                        ActionType::BuildRoad, playerId, firstEdge
                    ), playerId);

                    for (EdgeId secondEdge = 0; secondEdge < EDGE_COUNT; ++secondEdge) {
                        if (secondEdge != firstEdge &&
                            edgeIsValidForRoadBuildingDevCard(tempBoard, playerId, secondEdge)) {
                            roadBuildingActions.push_back(buildAction(
                                ActionType::PlayDevCardRoadBuilding,
                                playerId,
                                firstEdge,
                                secondEdge
                            ));
                        }
                    }
                }
            }
        }
    }
        
    return roadBuildingActions;
}

std::vector<Action::PackedAction> BoardState::generatePlayDevCardYearOfPlentyActions(
    PlayerId playerId
) {
    std::vector<Action::PackedAction> yearOfPlentyActions;
    
    auto &player = packedPlayers[static_cast<uint8_t>(playerId)];
    
    if (Player::unpackDevCard(player, DevType::YearOfPlenty) > 0) {
        for (Resource firstResource : {
            Resource::Brick, Resource::Lumber, Resource::Wool, 
            Resource::Grain, Resource::Ore
        }) {
            for (Resource secondResource : {
                Resource::Brick, Resource::Lumber, Resource::Wool, 
                Resource::Grain, Resource::Ore
            }) {
                yearOfPlentyActions.push_back(buildAction(
                    ActionType::PlayDevCardYearOfPlenty, playerId, 
                    static_cast<uint8_t>(firstResource), 
                    static_cast<uint8_t>(secondResource)
                ));
            }
        }
    }
        
    return yearOfPlentyActions;
}

std::vector<Action::PackedAction> BoardState::generatePlayDevCardMonopolyActions(
    PlayerId playerId
) {
    std::vector<Action::PackedAction> monopolyActions;
    
    auto &player = packedPlayers[static_cast<uint8_t>(playerId)];
    
    if (Player::unpackDevCard(player, DevType::Monopoly) > 0) {
        for (Resource targetResource : {
            Resource::Brick, Resource::Lumber, Resource::Wool, 
            Resource::Grain, Resource::Ore
        }) {
            monopolyActions.push_back(buildAction(
                ActionType::PlayDevCardMonopoly, playerId, 
                static_cast<uint8_t>(targetResource)
            ));
        }
    }
        
    return monopolyActions;
}

std::vector<Action::PackedAction> BoardState::generatePlaceInitialStructures(PlayerId playerId) {
    std::vector<Action::PackedAction> placeSettlementActions;
    
    for (NodeId nodeId = 0; nodeId < NODE_COUNT; ++nodeId) {
        if (Node::unpackStructure(nodes[nodeId]) != StructureType::NoStructure) {
            continue;
        }

        // Enforce distance rule: no adjacent settlements/cities
        bool violatesDistanceRule = false;
        for (int i = 0; i < 3 && !violatesDistanceRule; ++i) {
            EdgeId edgeId = Node::unpackAdjacentEdge(nodes[nodeId], i);
            if (edgeId == EdgeIdNone) {
                continue;
            }
            Edge::PackedEdge e = edges[edgeId];
            NodeId a = Edge::unpackAdjacentNode(e, 0);
            NodeId b = Edge::unpackAdjacentNode(e, 1);
            NodeId neighbor = (a == nodeId) ? b : a;
            if (neighbor < NODE_COUNT) {
                auto s = Node::unpackStructure(nodes[neighbor]);
                if (s == StructureType::Settlement || s == StructureType::City) {
                    violatesDistanceRule = true;
                }
            }
        }
        if (violatesDistanceRule) {
            continue;
        }

        EdgeId adjEdges[3] = {
            Node::unpackAdjacentEdge(nodes[nodeId], 0),
            Node::unpackAdjacentEdge(nodes[nodeId], 1),
            Node::unpackAdjacentEdge(nodes[nodeId], 2)
        };

        // Generate actions for each available adjacent edge to pair with the settlement
        for (EdgeId eId : adjEdges) {
            if (eId != EdgeIdNone && !Edge::unpackHasRoad(edges[eId])) {
                placeSettlementActions.push_back(buildAction(
                    ActionType::PlaceInitialStructures,
                    playerId,
                    nodeId,
                    eId
                ));
            }
        }
    }
    
    return placeSettlementActions;
}

std::vector<Action::PackedAction> BoardState::generatePlace2InitialStructures(PlayerId playerId) {
    std::vector<Action::PackedAction> place2SettlementActions;
    
    for (NodeId nodeId = 0; nodeId < NODE_COUNT; ++nodeId) {
        if (Node::unpackStructure(nodes[nodeId]) != StructureType::NoStructure) {
            continue;
        }

        // Enforce distance rule: no adjacent settlements/cities
        bool violatesDistanceRule = false;
        for (int i = 0; i < 3 && !violatesDistanceRule; ++i) {
            EdgeId edgeId = Node::unpackAdjacentEdge(nodes[nodeId], i);
            if (edgeId == EdgeIdNone) {
                continue;
            }
            Edge::PackedEdge e = edges[edgeId];
            NodeId a = Edge::unpackAdjacentNode(e, 0);
            NodeId b = Edge::unpackAdjacentNode(e, 1);
            NodeId neighbor = (a == nodeId) ? b : a;
            if (neighbor < NODE_COUNT) {
                auto s = Node::unpackStructure(nodes[neighbor]);
                if (s == StructureType::Settlement || s == StructureType::City) {
                    violatesDistanceRule = true;
                }
            }
        }
        if (violatesDistanceRule) {
            continue;
        }

        EdgeId adjEdges[3] = {
            Node::unpackAdjacentEdge(nodes[nodeId], 0),
            Node::unpackAdjacentEdge(nodes[nodeId], 1),
            Node::unpackAdjacentEdge(nodes[nodeId], 2)
        };

        // Generate actions for each available adjacent edge to pair with the settlement
        for (EdgeId eId : adjEdges) {
            if (eId != EdgeIdNone && !Edge::unpackHasRoad(edges[eId])) {
                place2SettlementActions.push_back(buildAction(
                    ActionType::Place2InitialStructures,
                    playerId,
                    nodeId,
                    eId
                ));
            }
        }
    }
    
    return place2SettlementActions;
}

std::vector<Action::PackedAction> BoardState::generatePlayDevCardActions(PlayerId playerId) {
    std::vector<Action::PackedAction> playDevCardActions;

    auto knightActions = generatePlayDevCardKnightActions(playerId);
    auto roadBuildingActions = generatePlayDevCardRoadBuildingActions(playerId);
    auto yearOfPlentyActions = generatePlayDevCardYearOfPlentyActions(playerId);
    auto monopolyActions = generatePlayDevCardMonopolyActions(playerId);

    playDevCardActions.insert(playDevCardActions.end(), knightActions.begin(), knightActions.end());
    playDevCardActions.insert(playDevCardActions.end(), roadBuildingActions.begin(), roadBuildingActions.end());
    playDevCardActions.insert(playDevCardActions.end(), yearOfPlentyActions.begin(), yearOfPlentyActions.end());
    playDevCardActions.insert(playDevCardActions.end(), monopolyActions.begin(), monopolyActions.end());
    playDevCardActions.push_back(buildAction(ActionType::NoAction, playerId));

    return playDevCardActions;
}

std::vector<Action::PackedAction> BoardState::getLegalActions(PlayerId playerId){
    std::vector<Action::PackedAction> legalActions;
    
    auto buildRoadActions = generateBuildRoadActions(playerId);
    auto buildSettlementActions = generateBuildSettlementActions(playerId);
    auto bankTradeActions = generateBankTradeActions(playerId);
    auto twoToOnePortTradeActions = generateTwoToOnePortTradeActions(playerId);
    auto threeToOnePortTradeActions = generateThreeToOnePortTradeActions(playerId);
    auto buyDevCardActions = generateBuyDevCardActions(playerId);
    
    legalActions.insert(legalActions.end(), buildRoadActions.begin(), buildRoadActions.end());
    legalActions.insert(legalActions.end(), buildSettlementActions.begin(), buildSettlementActions.end());
    legalActions.insert(legalActions.end(), bankTradeActions.begin(), bankTradeActions.end());
    legalActions.insert(legalActions.end(), twoToOnePortTradeActions.begin(), twoToOnePortTradeActions.end());
    legalActions.insert(legalActions.end(), threeToOnePortTradeActions.begin(), threeToOnePortTradeActions.end());
    legalActions.insert(legalActions.end(), buyDevCardActions.begin(), buyDevCardActions.end());
    legalActions.push_back(buildAction(ActionType::EndTurn, playerId));

    return legalActions;
}

} // namespace Board
