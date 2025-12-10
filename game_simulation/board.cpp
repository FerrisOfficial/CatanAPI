#include "board.hpp"
#include "actions.hpp"
#include "randomDevice.hpp"
#include "consts.hpp"
#include "packedBank.hpp"
#include "display/display.hpp"

namespace Board {

void BoardState::handlePlaceInitialSettlement(Action::PackedAction action, PlayerId playerId) {
    auto nodeId = Action::unpackArg1(action);
    nodes[nodeId] = Node::packStructure(nodes[nodeId], StructureType::Settlement);
    nodes[nodeId] = Node::packOwner(nodes[nodeId], playerId);
    // add victory point to player
    packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packVictoryPoints(
            packedPlayers[static_cast<uint8_t>(playerId)],
            Player::unpackVictoryPoints(
                packedPlayers[static_cast<uint8_t>(playerId)]) + 1
        );
    // remove one available settlement from player
    packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packAvailableStructures(
            packedPlayers[static_cast<uint8_t>(playerId)],
            StructureType::Settlement,
            Player::unpackAvailableStructures(
                packedPlayers[static_cast<uint8_t>(playerId)],
                StructureType::Settlement) - 1
        );
}

void BoardState::handleUndoPlaceInitialSettlement(Action::PackedAction action, PlayerId playerId) {
    auto nodeId = Action::unpackArg1(action);
    nodes[nodeId] = Node::packStructure(nodes[nodeId], StructureType::NoStructure);
    nodes[nodeId] = Node::packOwner(nodes[nodeId], PlayerId::NoPlayer);
    // remove victory point from player
    packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packVictoryPoints(
            packedPlayers[static_cast<uint8_t>(playerId)],
            Player::unpackVictoryPoints(
                packedPlayers[static_cast<uint8_t>(playerId)]) - 1
        );
    // add one available settlement to player
    packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packAvailableStructures(
            packedPlayers[static_cast<uint8_t>(playerId)],
            StructureType::Settlement,
            Player::unpackAvailableStructures(
                packedPlayers[static_cast<uint8_t>(playerId)],
                StructureType::Settlement) + 1
        );
}


void BoardState::handlePlace2InitialSettlement(Action::PackedAction action, PlayerId playerId) {
    auto nodeId = Action::unpackArg1(action);
    nodes[nodeId] = Node::packStructure(nodes[nodeId], StructureType::Settlement);
    nodes[nodeId] = Node::packOwner(nodes[nodeId], playerId);
    // add resources to player for 2nd settlement
    for (int i = 0; i < 3; ++i) {
        auto hexId = Node::unpackAdjacentHex(nodes[nodeId], i);
        auto res = Hex::unpackResource(hexes[hexId]);
        if (res != Resource::NoResource) {
            Player::changeResourceQuantity(packedPlayers[static_cast<uint8_t>(playerId)], res, 1);
            Bank::changeResourceQuantity(BoardState::packedBank, res, -1);

            hexes[hexId] = Hex::packPlayerValue(
                hexes[hexId],
                playerId,
                Hex::unpackPlayerValue(hexes[hexId], playerId) + 1
            );
        }
    }
    packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packVictoryPoints(
            packedPlayers[static_cast<uint8_t>(playerId)],
            Player::unpackVictoryPoints(
                packedPlayers[static_cast<uint8_t>(playerId)]) + 1
        );
    packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packAvailableStructures(
            packedPlayers[static_cast<uint8_t>(playerId)],
            StructureType::Settlement,
            Player::unpackAvailableStructures(
                packedPlayers[static_cast<uint8_t>(playerId)],
                StructureType::Settlement) - 1
        );
}

void BoardState::handleUndoPlace2InitialSettlement(Action::PackedAction action, PlayerId playerId) {
    auto nodeId = Action::unpackArg1(action);
    nodes[nodeId] = Node::packStructure(nodes[nodeId], StructureType::NoStructure);
    nodes[nodeId] = Node::packOwner(nodes[nodeId], PlayerId::NoPlayer);
    // remove resources from player for 2nd settlement
    for (int i = 0; i < 3; ++i) {
        auto hexId = Node::unpackAdjacentHex(nodes[nodeId], i);
        auto res = Hex::unpackResource(hexes[hexId]);
        if (res != Resource::NoResource) {
            Player::changeResourceQuantity(packedPlayers[static_cast<uint8_t>(playerId)], res, -1);
            Bank::changeResourceQuantity(BoardState::packedBank, res, 1);

            hexes[hexId] = Hex::packPlayerValue(
                hexes[hexId],
                playerId,
                Hex::unpackPlayerValue(hexes[hexId], playerId) - 1
            );
        }
    }
    packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packVictoryPoints(
            packedPlayers[static_cast<uint8_t>(playerId)],
            Player::unpackVictoryPoints(
                packedPlayers[static_cast<uint8_t>(playerId)]) - 1
        );
    packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packAvailableStructures(
            packedPlayers[static_cast<uint8_t>(playerId)],
            StructureType::Settlement,
            Player::unpackAvailableStructures(
                packedPlayers[static_cast<uint8_t>(playerId)],
                StructureType::Settlement) + 1
        );
}

void BoardState::handlePlaceInitialRoad(Action::PackedAction action, PlayerId playerId) {
    auto edgeId = Action::unpackArg1(action);
    edges[edgeId] = Edge::packHasRoad(edges[edgeId], true);
    edges[edgeId] = Edge::packOwner(edges[edgeId], playerId);

    // remove one available road from player
    packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packAvailableStructures(
            packedPlayers[static_cast<uint8_t>(playerId)],
            StructureType::Road,
            Player::unpackAvailableStructures(
                packedPlayers[static_cast<uint8_t>(playerId)],
                StructureType::Road) - 1
        );
}

void BoardState::handleUndoPlaceInitialRoad(Action::PackedAction action, PlayerId playerId) {
    auto edgeId = Action::unpackArg1(action);
    edges[edgeId] = Edge::packHasRoad(edges[edgeId], false);
    edges[edgeId] = Edge::packOwner(edges[edgeId], PlayerId::NoPlayer);

    // add one available road to player
    packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packAvailableStructures(
            packedPlayers[static_cast<uint8_t>(playerId)],
            StructureType::Road,
            Player::unpackAvailableStructures(
                packedPlayers[static_cast<uint8_t>(playerId)],
                StructureType::Road) + 1
        );
}

Action::PackedAction BoardState::handleEndTurn() {
    // Advance to next player's turn
    currentPlayer = (currentPlayer == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;
    currentTurn += 1;

    return Action::packType(0, ActionType::EndTurn);
}

void BoardState::handleUndoEndTurn() {
    // Revert to previous player's turn
    currentPlayer = (currentPlayer == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;
    currentTurn -= 1;
}

Action::PackedAction BoardState::handleRollDice(Action::PackedAction action) {
    uint8_t dice = Action::unpackArg1(action);

    // For each hex with matching catan number, produce resources
    for (HexId h = 0; h < HEX_COUNT; ++h) {
        if (Hex::unpackCatanNumber(hexes[h]) != dice) continue;
        Resource res = Hex::unpackResource(hexes[h]);

        auto &p0 = packedPlayers[static_cast<uint8_t>(PlayerId::Player0)];
        auto &p1 = packedPlayers[static_cast<uint8_t>(PlayerId::Player1)];

        Player::changeResourceQuantity(p0, res,
            Hex::unpackPlayerValue(hexes[h], PlayerId::Player0));
        Player::changeResourceQuantity(p1, res, 
            Hex::unpackPlayerValue(hexes[h], PlayerId::Player1));
        Bank::changeResourceQuantity(BoardState::packedBank, res,
            -(Hex::unpackPlayerValue(hexes[h], PlayerId::Player0) +
            Hex::unpackPlayerValue(hexes[h], PlayerId::Player1)));
    }

    return Action::packArg1(action, dice);
}

void BoardState::handleUndoRollDice(Action::PackedAction action){
    uint8_t dice = Action::unpackArg1(action);

    // For each hex with matching catan number, produce resources
    for (HexId h = 0; h < HEX_COUNT; ++h) {
        if (Hex::unpackCatanNumber(hexes[h]) != dice) continue;
        Resource res = Hex::unpackResource(hexes[h]);

        auto &p0 = packedPlayers[static_cast<uint8_t>(PlayerId::Player0)];
        auto &p1 = packedPlayers[static_cast<uint8_t>(PlayerId::Player1)];

        Player::changeResourceQuantity(p0, res,
            -Hex::unpackPlayerValue(hexes[h], PlayerId::Player0));
        Player::changeResourceQuantity(p1, res, 
            -Hex::unpackPlayerValue(hexes[h], PlayerId::Player1));
        Bank::changeResourceQuantity(BoardState::packedBank, res,
            (Hex::unpackPlayerValue(hexes[h], PlayerId::Player0) +
            Hex::unpackPlayerValue(hexes[h], PlayerId::Player1)));
    }
}

Action::PackedAction BoardState::handleMoveRobber(Action::PackedAction action, PlayerId playerId) {
    auto hexId = Action::unpackArg1(action);
    action = Action::packArg1(action, robberPosition);
    robberPosition = hexId;
    
    return action;
}

void BoardState::handleUndoMoveRobber(Action::PackedAction action, PlayerId playerId) {
    auto previousHexId = Action::unpackArg1(action);
    robberPosition = previousHexId;

    auto resourceType = static_cast<Resource>(Action::unpackArg2(action));
    Action::PackedAction undoStealAction = 0;
    undoStealAction = Action::packType(undoStealAction, ActionType::StealResource);
    undoStealAction = Action::packPlayerID(undoStealAction, playerId);
    undoStealAction = Action::packArg1(undoStealAction, static_cast<uint8_t>(resourceType));
    handleUndoStealResource(undoStealAction, playerId);
}

void BoardState::handleBuildRoad(Action::PackedAction action, PlayerId playerId) {
    auto edgeId = Action::unpackArg1(action);
    edges[edgeId] = Edge::packHasRoad(edges[edgeId], true);
    edges[edgeId] = Edge::packOwner(edges[edgeId], playerId);

    // Deduct resources from player
    Player::buy(packedPlayers[static_cast<uint8_t>(playerId)], BuyableType::Road);
    packedBank = Bank::buyableTransaction(packedBank, BuyableType::Road);
}

void BoardState::handleUndoBuildRoad(Action::PackedAction action, PlayerId playerId) {
    auto edgeId = Action::unpackArg1(action);
    edges[edgeId] = Edge::packHasRoad(edges[edgeId], false);
    edges[edgeId] = Edge::packOwner(edges[edgeId], PlayerId::NoPlayer);

    // Refund resources to player
    packedBank = Bank::buyableTransaction(packedBank, BuyableType::Road, DevType::NoDev, false);
    Player::refund(packedPlayers[static_cast<uint8_t>(playerId)], BuyableType::Road);
}

void BoardState::handleBuildSettlement(Action::PackedAction action, PlayerId playerId) {
    auto nodeId = Action::unpackArg1(action);
    nodes[nodeId] = Node::packStructure(nodes[nodeId], StructureType::Settlement);
    nodes[nodeId] = Node::packOwner(nodes[nodeId], playerId);

    HexId adjHex[3] = {
        Node::unpackAdjacentHex(nodes[nodeId], 0),
        Node::unpackAdjacentHex(nodes[nodeId], 1),
        Node::unpackAdjacentHex(nodes[nodeId], 2)
    };

    for (HexId h : adjHex) {
        if (h != HexIdNone) {
            hexes[h] = Hex::packPlayerValue(
                hexes[h],
                playerId,
                Hex::unpackPlayerValue(hexes[h], playerId) + 1
            );
        }
    }

    // Deduct resources from player
    Player::buy(packedPlayers[static_cast<uint8_t>(playerId)], BuyableType::Settlement);
    packedBank = Bank::buyableTransaction(packedBank, BuyableType::Settlement);
}

void BoardState::handleUndoBuildSettlement(Action::PackedAction action, PlayerId playerId) {
    auto nodeId = Action::unpackArg1(action);
    nodes[nodeId] = Node::packStructure(nodes[nodeId], StructureType::NoStructure);
    nodes[nodeId] = Node::packOwner(nodes[nodeId], PlayerId::NoPlayer);

    HexId adjHex[3] = {
        Node::unpackAdjacentHex(nodes[nodeId], 0),
        Node::unpackAdjacentHex(nodes[nodeId], 1),
        Node::unpackAdjacentHex(nodes[nodeId], 2)
    };

    for (HexId h : adjHex) {
        if (h != HexIdNone) {
            hexes[h] = Hex::packPlayerValue(
                hexes[h],
                playerId,
                Hex::unpackPlayerValue(hexes[h], playerId) - 1
            );
        }
    }

    // Refund resources to player
    packedBank = Bank::buyableTransaction(packedBank, BuyableType::Settlement, DevType::NoDev, false);
    Player::refund(packedPlayers[static_cast<uint8_t>(playerId)], BuyableType::Settlement);
}

void BoardState::handleBuildCity(Action::PackedAction action, PlayerId playerId) {
    auto nodeId = Action::unpackArg1(action);
    nodes[nodeId] = Node::packStructure(nodes[nodeId], StructureType::City);

    HexId adjHex[3] = {
        Node::unpackAdjacentHex(nodes[nodeId], 0),
        Node::unpackAdjacentHex(nodes[nodeId], 1),
        Node::unpackAdjacentHex(nodes[nodeId], 2)
    };

    for (HexId h : adjHex) {
        if (h != HexIdNone) {
            hexes[h] = Hex::packPlayerValue(
                hexes[h],
                playerId,
                Hex::unpackPlayerValue(hexes[h], playerId) + 1
            );
        }
    }

    // Deduct resources from player
    Player::buy(packedPlayers[static_cast<uint8_t>(playerId)], BuyableType::City);
    packedBank = Bank::buyableTransaction(packedBank, BuyableType::City);
}

void BoardState::handleUndoBuildCity(Action::PackedAction action, PlayerId playerId) {
    auto nodeId = Action::unpackArg1(action);
    nodes[nodeId] = Node::packStructure(nodes[nodeId], StructureType::Settlement);

    HexId adjHex[3] = {
        Node::unpackAdjacentHex(nodes[nodeId], 0),
        Node::unpackAdjacentHex(nodes[nodeId], 1),
        Node::unpackAdjacentHex(nodes[nodeId], 2)
    };

    for (HexId h : adjHex) {
        if (h != HexIdNone) {
            hexes[h] = Hex::packPlayerValue(
                hexes[h],
                playerId,
                Hex::unpackPlayerValue(hexes[h], playerId) - 1
            );
        }
    }

    // Refund resources to player
    packedBank = Bank::buyableTransaction(packedBank, BuyableType::City, DevType::NoDev, false);
    Player::refund(packedPlayers[static_cast<uint8_t>(playerId)], BuyableType::City);
}

Action::PackedAction BoardState::handleBuyDevCard(PlayerId playerId) {
    const uint32_t pick = RandomDevice::uniform_u32(Bank::unpackTotalDevCount(packedBank));
    uint32_t acc = 0;
    DevType DevTypeOfpick = DevType::NoDev;

    for (DevType d : {
        DevType::Knight,
        DevType::RoadBuilding,
        DevType::YearOfPlenty,
        DevType::Monopoly,
        DevType::VictoryPoint
    }) {
        uint32_t count = Bank::unpackDevCard(BoardState::packedBank, d);
        if (pick < acc + count){
            DevTypeOfpick = d;
            break;
        }
        acc += count;
    }

    Player::buy(packedPlayers[static_cast<uint8_t>(playerId)], BuyableType::DevCard, DevTypeOfpick);
    packedBank = Bank::buyableTransaction(packedBank, BuyableType::DevCard, DevTypeOfpick);

    Action::PackedAction action = 0;
    action = Action::packType(action, ActionType::BuyDevCard);
    action = Action::packPlayerID(action, playerId);
    action = Action::packArg1(action, static_cast<uint8_t>(DevTypeOfpick));
    return action;
}

void BoardState::handleUndoBuyDevCard(Action::PackedAction action, PlayerId playerId) {
    auto d = Action::unpackArg1(action);
    Player::refund(packedPlayers[static_cast<uint8_t>(playerId)], BuyableType::DevCard, static_cast<DevType>(d));
    packedBank = Bank::buyableTransaction(packedBank, BuyableType::DevCard, static_cast<DevType>(d), false);
}

Action::PackedAction BoardState::handlePlayDevCardKnight(Action::PackedAction action, PlayerId playerId) {
    // Decrement knight dev card count
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];
    auto previousRobberPosition = robberPosition;
    p = Player::packDevCard(
        p,
        DevType::Knight,
        Player::unpackDevCard(p, DevType::Knight) - 1
    );

    handleMoveRobber(action, playerId);
    action = Action::packArg1(action, previousRobberPosition);

    return action;
}

void BoardState::handleUndoPlayDevCardKnight(Action::PackedAction action, PlayerId playerId) {
    // Increment knight dev card count
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];
    auto robberPosition = Action::unpackArg1(action);
    p = Player::packDevCard(
        p,
        DevType::Knight,
        Player::unpackDevCard(p, DevType::Knight) + 1
    );

    action = Action::packArg1(action, robberPosition);
    handleUndoMoveRobber(action, playerId);
}

void BoardState::handlePlayDevCardRoadBuilding(Action::PackedAction action, PlayerId playerId) {
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];
    
    p = Player::packDevCard(
        p,
        DevType::RoadBuilding,
        Player::unpackDevCard(p, DevType::RoadBuilding) - 1
    );

    // Build two roads
    auto firstEdgeId = Action::unpackArg1(action);
    edges[firstEdgeId] = Edge::packHasRoad(edges[firstEdgeId], true);
    edges[firstEdgeId] = Edge::packOwner(edges[firstEdgeId], playerId);
    
    auto secondEdgeId = Action::unpackArg2(action);
    edges[secondEdgeId] = Edge::packHasRoad(edges[secondEdgeId], true);
    edges[secondEdgeId] = Edge::packOwner(edges[secondEdgeId], playerId);

    packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packAvailableStructures(
            p,
            StructureType::Road,
            Player::unpackAvailableStructures(
                p,
                StructureType::Road) - 2
        );
}

void BoardState::handleUndoPlayDevCardRoadBuilding(Action::PackedAction action, PlayerId playerId) {
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];
    
    p = Player::packDevCard(
        p,
        DevType::RoadBuilding,
        Player::unpackDevCard(p, DevType::RoadBuilding) + 1
    );

    // Remove two roads
    auto firstEdgeId = Action::unpackArg1(action);
    edges[firstEdgeId] = Edge::packHasRoad(edges[firstEdgeId], false);
    edges[firstEdgeId] = Edge::packOwner(edges[firstEdgeId], PlayerId::NoPlayer);
    
    auto secondEdgeId = Action::unpackArg2(action);
    edges[secondEdgeId] = Edge::packHasRoad(edges[secondEdgeId], false);
    edges[secondEdgeId] = Edge::packOwner(edges[secondEdgeId], PlayerId::NoPlayer);

    packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packAvailableStructures(
            p,
            StructureType::Road,
            Player::unpackAvailableStructures(
                p,
                StructureType::Road) + 2
        );
}

void BoardState::handlePlayDevCardYearOfPlenty(Action::PackedAction action, PlayerId playerId) {
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];

    p = Player::packDevCard(
        p,
        DevType::YearOfPlenty,
        Player::unpackDevCard(p, DevType::YearOfPlenty) - 1
    );

    Resource firstResource = static_cast<Resource>(Action::unpackArg1(action));
    Resource secondResource = static_cast<Resource>(Action::unpackArg2(action));

    // Add first resource to player
    Player::changeResourceQuantity(p, firstResource, 1);
    Bank::changeResourceQuantity(packedBank, firstResource, -1);


    // Add second resource to player
    Player::changeResourceQuantity(p, secondResource, 1);
    Bank::changeResourceQuantity(packedBank, secondResource, -1);
}

void BoardState::handleUndoPlayDevCardYearOfPlenty(Action::PackedAction action, PlayerId playerId) {
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];

    p = Player::packDevCard(
        p,
        DevType::YearOfPlenty,
        Player::unpackDevCard(p, DevType::YearOfPlenty) + 1
    );

    Resource firstResource = static_cast<Resource>(Action::unpackArg1(action));
    Resource secondResource = static_cast<Resource>(Action::unpackArg2(action));

    // Remove first resource from player
    Player::changeResourceQuantity(p, firstResource, -1);
    Bank::changeResourceQuantity(packedBank, firstResource, 1);

    // Remove second resource from player
    Player::changeResourceQuantity(p, secondResource, -1);
    Bank::changeResourceQuantity(packedBank, secondResource, 1);
}

Action::PackedAction BoardState::handlePlayDevCardMonopoly(Action::PackedAction action, PlayerId playerId) {
    // First, decrement the monopoly card count
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];
   
    p = Player::packDevCard(
        p,
        DevType::Monopoly,
        Player::unpackDevCard(p, DevType::Monopoly) - 1
    );


    Resource targetResource = static_cast<Resource>(Action::unpackArg1(action));
    auto enemyPlayerId = playerId == PlayerId::Player0 ? PlayerId::Player1 : PlayerId::Player0;
    
    auto enemyHave = Player::unpackResource(packedPlayers[static_cast<uint8_t>(enemyPlayerId)], targetResource);
    
    // Remove all of this resource from enemy
    if (enemyHave > 0) {
        packedPlayers[static_cast<uint8_t>(enemyPlayerId)] =
            Player::packResource(packedPlayers[static_cast<uint8_t>(enemyPlayerId)], targetResource, 0);

        Player::changeResourceQuantity(p, targetResource, enemyHave);
    }

    action = Action::packArg2(action, enemyHave);
    return action;
}

void BoardState::handleUndoPlayDevCardMonopoly(Action::PackedAction action, PlayerId playerId) {
    // First, increment the monopoly card count
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];
   
    p = Player::packDevCard(
        p,
        DevType::Monopoly,
        Player::unpackDevCard(p, DevType::Monopoly) + 1
    );

    Resource targetResource = static_cast<Resource>(Action::unpackArg1(action));
    auto enemyPlayerId = playerId == PlayerId::Player0 ? PlayerId::Player1 : PlayerId::Player0;

    uint8_t stolenAmount = Action::unpackArg2(action);

    // Return resources to enemy
    if (stolenAmount > 0) {
        Player::changeResourceQuantity(
            packedPlayers[static_cast<uint8_t>(enemyPlayerId)],
            targetResource,
            stolenAmount
        );

        Player::changeResourceQuantity(p, targetResource, -stolenAmount);
    }
}

void BoardState::handleTradeBank(Action::PackedAction action, PlayerId playerId) {
    Resource giveResource = static_cast<Resource>(Action::unpackArg1(action));
    Resource receiveResource = static_cast<Resource>(Action::unpackArg2(action));
    uint8_t ratio = Action::unpackArg3(action);

    auto idx = static_cast<uint8_t>(playerId);
    auto &p = packedPlayers[idx];

    // Player gives 'ratio'
    Player::changeResourceQuantity(p, giveResource, -ratio);
    Player::changeResourceQuantity(p, receiveResource, 1);    

    uint8_t bankGive = Bank::unpackResource(packedBank, giveResource);
    Bank::changeResourceQuantity(packedBank, giveResource, ratio);
    Bank::changeResourceQuantity(packedBank, receiveResource, -1);
}

void BoardState::handleUndoTradeBank(Action::PackedAction action, PlayerId playerId) {
    Resource giveResource = static_cast<Resource>(Action::unpackArg1(action));
    Resource receiveResource = static_cast<Resource>(Action::unpackArg2(action));
    uint8_t ratio = Action::unpackArg3(action);

    auto idx = static_cast<uint8_t>(playerId);
    auto &p = packedPlayers[idx];

    // Player gets back 'ratio'
    Player::changeResourceQuantity(p, giveResource, ratio);
    Player::changeResourceQuantity(p, receiveResource, -1);    

    Bank::changeResourceQuantity(packedBank, giveResource, -ratio);
    Bank::changeResourceQuantity(packedBank, receiveResource, 1);
}

void BoardState::handleReceiveResources(Action::PackedAction action, PlayerId playerId) {
    Resource res = static_cast<Resource>(Action::unpackArg1(action));
    uint8_t amount = Action::unpackArg2(action);
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];

    Player::changeResourceQuantity(p, res, amount);
    Bank::changeResourceQuantity(packedBank, res, -amount);
}

void BoardState::handleUndoReceiveResources(Action::PackedAction action, PlayerId playerId) {
    Resource res = static_cast<Resource>(Action::unpackArg1(action));
    uint8_t amount = Action::unpackArg2(action);
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];

    Player::changeResourceQuantity(p, res, -amount);
    Bank::changeResourceQuantity(packedBank, res, amount);
}

void BoardState::handleDiscardResources(Action::PackedAction action, PlayerId playerId) {
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];
    
    Player::changeResourceQuantity(p, Resource::Brick, -Action::unpackResource(action, Resource::Brick));
    Player::changeResourceQuantity(p, Resource::Lumber, -Action::unpackResource(action, Resource::Lumber));
    Player::changeResourceQuantity(p, Resource::Wool, -Action::unpackResource(action, Resource::Wool));
    Player::changeResourceQuantity(p, Resource::Grain, -Action::unpackResource(action, Resource::Grain));
    Player::changeResourceQuantity(p, Resource::Ore, -Action::unpackResource(action, Resource::Ore));
}

void BoardState::handleUndoDiscardResources(Action::PackedAction action, PlayerId playerId) {
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];
    
    Player::changeResourceQuantity(p, Resource::Brick, Action::unpackResource(action, Resource::Brick));
    Player::changeResourceQuantity(p, Resource::Lumber, Action::unpackResource(action, Resource::Lumber));
    Player::changeResourceQuantity(p, Resource::Wool, Action::unpackResource(action, Resource::Wool));
    Player::changeResourceQuantity(p, Resource::Grain, Action::unpackResource(action, Resource::Grain));
    Player::changeResourceQuantity(p, Resource::Ore, Action::unpackResource(action, Resource::Ore));
}

void BoardState::handleStealResource(Action::PackedAction action, PlayerId playerId) {
    auto enemyPlayerId = playerId == PlayerId::Player0 ? PlayerId::Player1 : PlayerId::Player0;

    uint32_t total = Player::totalResources(packedPlayers[static_cast<uint8_t>(enemyPlayerId)]);
    uint32_t pick = RandomDevice::uniform_u32(total);
    int chosenRes = 0;
    uint32_t acc = 0;

    for (Resource r : {
        Resource::Brick,
        Resource::Lumber,
        Resource::Wool,
        Resource::Grain,
        Resource::Ore
    }) {
        uint8_t resCount = Player::unpackResource(packedPlayers[static_cast<uint8_t>(enemyPlayerId)], r);
        if (pick < acc + resCount) {
            chosenRes = static_cast<int>(r);
            break;
        }
        acc += resCount;
    }
    
    // handleDiscardResources();
    Player::changeResourceQuantity(packedPlayers[static_cast<uint8_t>(enemyPlayerId)], static_cast<Resource>(chosenRes), -1);
    Player::changeResourceQuantity(packedPlayers[static_cast<uint8_t>(playerId)], static_cast<Resource>(chosenRes), 1);
}

void BoardState::handleUndoStealResource(Action::PackedAction action, PlayerId playerId) {
    auto enemyPlayerId = playerId == PlayerId::Player0 ? PlayerId::Player1 : PlayerId::Player0;
    Resource res = static_cast<Resource>(Action::unpackArg1(action));
    
    Player::changeResourceQuantity(packedPlayers[static_cast<uint8_t>(enemyPlayerId)], res, 1);
    Player::changeResourceQuantity(packedPlayers[static_cast<uint8_t>(playerId)], res, -1);
}

void BoardState::applyAction(Action::PackedAction action) {
    auto type = Action::unpackType(action);
    auto playerId = Action::unpackPlayerID(action);

    switch (type) {
    case ActionType::PlaceInitialSettlement:
        handlePlaceInitialSettlement(action, playerId);
        break;
    case ActionType::Place2InitialSettlement:
        handlePlace2InitialSettlement(action, playerId);
        break;
    case ActionType::PlaceInitialRoad:
        handlePlaceInitialRoad(action, playerId);
        break;
    case ActionType::EndTurn:
        action = handleEndTurn();
        break;
    case ActionType::RollDice:
        action |= handleRollDice(action);
        break;
    case ActionType::MoveRobber:
        action = handleMoveRobber(action, playerId);
        break;
    case ActionType::DiscardResources:
        handleDiscardResources(action, playerId);
        break;
    case ActionType::BuildRoad:
        handleBuildRoad(action, playerId);
        break;
    case ActionType::BuildSettlement:
        handleBuildSettlement(action, playerId);
        break;
    case ActionType::BuildCity:
        handleBuildCity(action, playerId);
        break;
    case ActionType::BuyDevCard:
        action |= handleBuyDevCard(playerId);
        break;
    case ActionType::PlayDevCardKnight:
        action = handlePlayDevCardKnight(action, playerId);
        break;
    case ActionType::PlayDevCardRoadBuilding:
        handlePlayDevCardRoadBuilding(action, playerId);
        break;
    case ActionType::PlayDevCardYearOfPlenty:
        handlePlayDevCardYearOfPlenty(action, playerId);
        break;
    case ActionType::PlayDevCardMonopoly:
        handlePlayDevCardMonopoly(action, playerId);
        break;
    case ActionType::TradeBank:
        handleTradeBank(action, playerId);
        break;
    case ActionType::ReceiveResources:
        handleReceiveResources(action, playerId);
        break;
    case ActionType::StealResource:
        handleStealResource(action, playerId);
        break;

    default:
        break;
    }

    actionQueue[actionQueueSize++] = action;
}

void BoardState::undoLastAction() {
    Action::PackedAction action = actionQueue[--actionQueueSize];
    auto type = Action::unpackType(action);
    auto playerId = Action::unpackPlayerID(action);

    switch (type) {
    case ActionType::PlaceInitialSettlement:
        handleUndoPlaceInitialSettlement(action, playerId);
        break;
    case ActionType::Place2InitialSettlement:
        handleUndoPlace2InitialSettlement(action, playerId);
        break;
    case ActionType::PlaceInitialRoad:
        handleUndoPlaceInitialRoad(action, playerId);
        break;
    case ActionType::RollDice:
        handleUndoRollDice(action);
        break;
    case ActionType::MoveRobber:
        handleUndoMoveRobber(action, playerId);
        break;
    case ActionType::DiscardResources:
        handleUndoDiscardResources(action, playerId);
        break;
    case ActionType::BuildRoad:
        handleUndoBuildRoad(action, playerId);
        break;
    case ActionType::BuildSettlement:
        handleUndoBuildSettlement(action, playerId);
        break;
    case ActionType::BuildCity:
        handleUndoBuildCity(action, playerId);
        break;
    case ActionType::BuyDevCard:
        handleUndoBuyDevCard(action, playerId);
        break;
    case ActionType::PlayDevCardKnight:
        handleUndoPlayDevCardKnight(action, playerId);
        break;
    case ActionType::PlayDevCardRoadBuilding:
        handleUndoPlayDevCardRoadBuilding(action, playerId);
        break;
    case ActionType::PlayDevCardYearOfPlenty:
        handleUndoPlayDevCardYearOfPlenty(action, playerId);
        break;
    case ActionType::PlayDevCardMonopoly:
        handleUndoPlayDevCardMonopoly(action, playerId);
        break;
    case ActionType::TradeBank:
        handleUndoTradeBank(action, playerId);
        break;
    case ActionType::ReceiveResources:
        handleUndoReceiveResources(action, playerId);
        break;
    case ActionType::StealResource:
        handleUndoStealResource(action, playerId);
        break;
    default:
        break;
    }
}

Action::PackedAction buildAction(ActionType type, PlayerId playerId, uint8_t arg1 = 0, uint8_t arg2 = 0, uint8_t arg3 = 0) {
    Action::PackedAction action = 0;
    action = Action::packType(action, type);
    action = Action::packPlayerID(action, playerId);
    action = Action::packArg1(action, arg1);
    action = Action::packArg2(action, arg2);
    action = Action::packArg3(action, arg3);
    return action;
};

std::vector<Action::PackedAction> BoardState::generateBuildActions(PlayerId playerId){
    std::vector<Action::PackedAction> buildActions;

    // Generate build road actions
    for (EdgeId edgeId = 0; edgeId < EDGE_COUNT; ++edgeId) {;
        if (!(Edge::unpackHasRoad(edges[edgeId])))
            buildActions.push_back(buildAction(ActionType::BuildRoad, playerId, edgeId));
    }

    // Generate build settlement actions
    for (NodeId nodeId = 0; nodeId < NODE_COUNT; ++nodeId) {
        if (Node::unpackStructure(nodes[nodeId]) == StructureType::NoStructure)
            buildActions.push_back(buildAction(ActionType::BuildSettlement, playerId, nodeId));
    }

    // Generate build city actions
    for (NodeId nodeId = 0; nodeId < NODE_COUNT; ++nodeId) {
        if (Node::unpackStructure(nodes[nodeId]) == StructureType::Settlement && Node::unpackOwner(nodes[nodeId]) == playerId)
            buildActions.push_back(buildAction(ActionType::BuildCity, playerId, nodeId));
    }

    return buildActions;
};

std::vector<Action::PackedAction> BoardState::generateTradeActions(PlayerId playerId){
    std::vector<Action::PackedAction> tradeActions;
    auto &player = packedPlayers[static_cast<uint8_t>(playerId)];

    bool hasThreeForOnePort = false;
    bool hasBrickPort = false;
    bool hasLumberPort = false;
    bool hasWoolPort = false;
    bool hasGrainPort = false;
    bool hasOrePort = false;

    // Check all nodes for player's settlements/cities
    for (NodeId nodeId = 0; nodeId < NODE_COUNT; ++nodeId) {
        StructureType structure = Node::unpackStructure(nodes[nodeId]);
        PlayerId owner = Node::unpackOwner(nodes[nodeId]);
        
        // Only consider nodes where this player has a settlement or city
        if (owner != playerId || (structure != StructureType::Settlement && structure != StructureType::City)) {
            continue;
        }
        
        PortType portType = Node::unpackPortType(nodes[nodeId]);
        
        if (portType == PortType::ThreeForOne) {
            hasThreeForOnePort = true;
        } else if (portType == PortType::BrickPort) {
            hasBrickPort = true;
        } else if (portType == PortType::LumberPort) {
            hasLumberPort = true;
        } else if (portType == PortType::WoolPort) {
            hasWoolPort = true;
        } else if (portType == PortType::GrainPort) {
            hasGrainPort = true;
        } else if (portType == PortType::OrePort) {
            hasOrePort = true;
        }
    }

    // Generate bank trade actions
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
            // można zostawić żeby mąre agenty były lepsze od randmowego :)
            if (giveResource == receiveResource) continue;
            
            // 4:1 trades
            for (uint8_t tradeCount = 1; tradeCount * 4 <= playerHas; ++tradeCount) {
                tradeActions.push_back(buildAction(ActionType::TradeBank, playerId, 
                    static_cast<uint8_t>(giveResource), 
                    static_cast<uint8_t>(receiveResource), 
                    4));
            }
            
            // 3:1 trades if player has ThreeForOne port
            if (hasThreeForOnePort) {
                for (uint8_t tradeCount = 1; tradeCount * 3 <= playerHas; ++tradeCount) {
                    tradeActions.push_back(buildAction(ActionType::TradeBank, playerId, 
                        static_cast<uint8_t>(giveResource), 
                        static_cast<uint8_t>(receiveResource), 
                        3));
                }
            }
            
            // Generate 2:1 trades for specific resource ports
            if (giveResource == Resource::Brick && hasBrickPort) {
                for (uint8_t tradeCount = 1; tradeCount * 2 <= playerHas; ++tradeCount) {
                    tradeActions.push_back(buildAction(ActionType::TradeBank, playerId, 
                        static_cast<uint8_t>(giveResource), 
                        static_cast<uint8_t>(receiveResource), 
                        2));
                }
            } else if (giveResource == Resource::Lumber && hasLumberPort) {
                for (uint8_t tradeCount = 1; tradeCount * 2 <= playerHas; ++tradeCount) {
                    tradeActions.push_back(buildAction(ActionType::TradeBank, playerId, 
                        static_cast<uint8_t>(giveResource), 
                        static_cast<uint8_t>(receiveResource), 
                        2));
                }
            } else if (giveResource == Resource::Wool && hasWoolPort) {
                for (uint8_t tradeCount = 1; tradeCount * 2 <= playerHas; ++tradeCount) {
                    tradeActions.push_back(buildAction(ActionType::TradeBank, playerId, 
                        static_cast<uint8_t>(giveResource), 
                        static_cast<uint8_t>(receiveResource), 
                        2));
                }
            } else if (giveResource == Resource::Grain && hasGrainPort) {
                for (uint8_t tradeCount = 1; tradeCount * 2 <= playerHas; ++tradeCount) {
                    tradeActions.push_back(buildAction(ActionType::TradeBank, playerId, 
                        static_cast<uint8_t>(giveResource), 
                        static_cast<uint8_t>(receiveResource), 
                        2));
                }
            } else if (giveResource == Resource::Ore && hasOrePort) {
                for (uint8_t tradeCount = 1; tradeCount * 2 <= playerHas; ++tradeCount) {
                    tradeActions.push_back(buildAction(ActionType::TradeBank, playerId, 
                        static_cast<uint8_t>(giveResource), 
                        static_cast<uint8_t>(receiveResource), 
                        2));
                }
            }
        }
    }

    return tradeActions;
};

// Nie można zagrać tą kartą którą przed chwilą się kupiło (??)
// można kupić dowolną liczbę kart, ale zac jedną
std::vector<Action::PackedAction> BoardState::generateDevCardActions(PlayerId playerId) {
    std::vector<Action::PackedAction> devCardActions;
    
    auto &player = packedPlayers[static_cast<uint8_t>(playerId)];
    
    uint8_t playerGrain = Player::unpackResource(player, Resource::Grain);
    uint8_t playerOre = Player::unpackResource(player, Resource::Ore);
    uint8_t playerWool = Player::unpackResource(player, Resource::Wool);
    
    uint8_t maxAffordable = std::min({playerGrain, playerOre, playerWool});
    
    // Buy as many dev cards as player can afford
    for (uint8_t cardCount = 1; cardCount <= maxAffordable; ++cardCount) {
        devCardActions.push_back(buildAction(ActionType::BuyDevCard, playerId));
    }
        
    // Knight cards - need to specify new robber position
    if (Player::unpackDevCard(player, DevType::Knight) > 0) {
        for (HexId hexId = 0; hexId < HEX_COUNT; ++hexId) {
            if (hexId != robberPosition) {
                devCardActions.push_back(buildAction(ActionType::PlayDevCardKnight, playerId, hexId));
            }
        }
    }
    
    // RoadBuilding cards - need two edge positions
    if (Player::unpackDevCard(player, DevType::RoadBuilding) > 0) {
        for (EdgeId firstEdge = 0; firstEdge < EDGE_COUNT; ++firstEdge) {
            if (!Edge::unpackHasRoad(edges[firstEdge])) {
                for (EdgeId secondEdge = firstEdge + 1; secondEdge < EDGE_COUNT; ++secondEdge) {
                    if (!Edge::unpackHasRoad(edges[secondEdge])) {
                        devCardActions.push_back(buildAction(ActionType::PlayDevCardRoadBuilding, playerId, firstEdge, secondEdge));
                    }
                }
            }
        }
    }
    
    // YearOfPlenty cards - need two resources
    if (Player::unpackDevCard(player, DevType::YearOfPlenty) > 0) {
        for (Resource firstResource : {Resource::Brick, Resource::Lumber, Resource::Wool, Resource::Grain, Resource::Ore}) {
            for (Resource secondResource : {Resource::Brick, Resource::Lumber, Resource::Wool, Resource::Grain, Resource::Ore}) {
                devCardActions.push_back(buildAction(ActionType::PlayDevCardYearOfPlenty, playerId, 
                    static_cast<uint8_t>(firstResource), 
                    static_cast<uint8_t>(secondResource)));
            }
        }
    }
    
    // Monopoly cards - need target resource
    if (Player::unpackDevCard(player, DevType::Monopoly) > 0) {
        for (Resource targetResource : {Resource::Brick, Resource::Lumber, Resource::Wool, Resource::Grain, Resource::Ore}) {
            devCardActions.push_back(buildAction(ActionType::PlayDevCardMonopoly, playerId, 
                static_cast<uint8_t>(targetResource)));
        }
    }
    
    return devCardActions;
}

std::vector<Action::PackedAction> BoardState::getLegalActions(PlayerId playerId){
    std::vector<Action::PackedAction> legalActions;
    
    auto buildActions = generateBuildActions(playerId);
    auto tradeActions = generateTradeActions(playerId);
    auto devCardActions = generateDevCardActions(playerId);
    
    legalActions.insert(legalActions.end(), buildActions.begin(), buildActions.end());
    legalActions.insert(legalActions.end(), tradeActions.begin(), tradeActions.end());
    legalActions.insert(legalActions.end(), devCardActions.begin(), devCardActions.end());
    
    return legalActions;
}

void BoardState::generateRandomBoard() {
    auto& rng = RandomDevice::get_rng();

    uint8_t numberDistribution[HEX_COUNT] = {2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12};
    Resource resourceDistribution[HEX_COUNT] =
    {
        Resource::Brick, Resource::Brick, Resource::Brick,
        Resource::Lumber, Resource::Lumber, Resource::Lumber, Resource::Lumber,
        Resource::Wool, Resource::Wool, Resource::Wool, Resource::Wool,
        Resource::Grain, Resource::Grain, Resource::Grain,
        Resource::Ore, Resource::Ore, Resource::Ore,
        Resource::NoResource
    };
    for (size_t i = 0; i < HEX_COUNT; ++i) {
        uint32_t j = rng() % (i + 1);
        std::swap(numberDistribution[i], numberDistribution[j]);
        std::swap(resourceDistribution[i], resourceDistribution[j]);
    }
    for (HexId h = 0; h < HEX_COUNT; ++h) {
        hexes[h] = Hex::packResource(hexes[h], resourceDistribution[h]);
        hexes[h] = Hex::packCatanNumber(hexes[h], numberDistribution[h]);

        if (resourceDistribution[h] == Resource::NoResource) {
            robberPosition = h;
        }
    }

    Display display;
    display.renderBoard(*this);
}

} // namespace Board