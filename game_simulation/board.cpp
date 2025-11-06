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

void BoardState::handlePlace2InitialSettlement(Action::PackedAction action, PlayerId playerId) {
    auto nodeId = Action::unpackArg1(action);
    nodes[nodeId] = Node::packStructure(nodes[nodeId], StructureType::Settlement);
    nodes[nodeId] = Node::packOwner(nodes[nodeId], playerId);
    // add resources to player for 2nd settlement
    for (int i = 0; i < 3; ++i) {
        auto hexId = Node::unpackAdjacentHex(nodes[nodeId], i);
        auto res = Hex::unpackResource(hexes[hexId]);
        if (res != Resource::NoResource) {
            packedPlayers[static_cast<uint8_t>(playerId)] =
                Player::packResource(
                    packedPlayers[static_cast<uint8_t>(playerId)],
                    res,
                    Player::unpackResource(
                        packedPlayers[static_cast<uint8_t>(playerId)], res) + 1
                );
            BoardState::packedBank =
                Bank::packResource(
                    BoardState::packedBank,
                    res,
                    Bank::unpackResource(
                        BoardState::packedBank, res) - 1
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

void BoardState::handleEndTurn(Action::PackedAction /*action*/, PlayerId /*playerId*/) {
    // Advance to next player's turn
    currentPlayer = (currentPlayer == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;
    currentTurn += 1;
}

void BoardState::handleRollDice(Action::PackedAction action, PlayerId /*playerId*/) {
    uint8_t dice = Action::unpackArg1(action);

    // For each hex with matching catan number, produce resources
    for (HexId h = 0; h < HEX_COUNT; ++h) {
        if (Hex::unpackCatanNumber(hexes[h]) != dice) continue;
        Resource res = Hex::unpackResource(hexes[h]);

        auto &p0 = packedPlayers[static_cast<uint8_t>(PlayerId::Player0)];
        auto &p1 = packedPlayers[static_cast<uint8_t>(PlayerId::Player1)];

        p0 =
            Player::packResource(
                p0,
                res,
                Player::unpackResource(p0, res) + Hex::unpackPlayerValue(hexes[h], PlayerId::Player0)
            );

        p1 =
            Player::packResource(
                p1,
                res,
                Player::unpackResource(p1, res) + Hex::unpackPlayerValue(hexes[h], PlayerId::Player1)
            );
        BoardState::packedBank =
            Bank::packResource(
                BoardState::packedBank,
                res,
                Bank::unpackResource(BoardState::packedBank, res) -
                (Hex::unpackPlayerValue(hexes[h], PlayerId::Player0) +
                 Hex::unpackPlayerValue(hexes[h], PlayerId::Player1))
            );
    }
}

void BoardState::handleMoveRobber(Action::PackedAction action, PlayerId playerId) {
    auto hexId = Action::unpackArg1(action);
    auto enemyPlayerId = playerId == PlayerId::Player0 ? PlayerId::Player1 : PlayerId::Player0;
    robberPosition = hexId;
    
    // Count enemy's resources
    auto enemyWool = Player::unpackResource(packedPlayers[static_cast<uint8_t>(enemyPlayerId)], Resource::Wool);
    auto enemyBrick = Player::unpackResource(packedPlayers[static_cast<uint8_t>(enemyPlayerId)], Resource::Brick);
    auto enemyLumber = Player::unpackResource(packedPlayers[static_cast<uint8_t>(enemyPlayerId)], Resource::Lumber);
    auto enemyGrain = Player::unpackResource(packedPlayers[static_cast<uint8_t>(enemyPlayerId)], Resource::Grain);
    auto enemyOre = Player::unpackResource(packedPlayers[static_cast<uint8_t>(enemyPlayerId)], Resource::Ore);

    // Choose a random resource to steal (each card equally likely)
    uint8_t counts[5];
    counts[0] = enemyBrick;
    counts[1] = enemyLumber;
    counts[2] = enemyWool;
    counts[3] = enemyGrain;
    counts[4] = enemyOre;

    uint32_t total = uint32_t(counts[0]) + uint32_t(counts[1]) + uint32_t(counts[2]) + uint32_t(counts[3]) + uint32_t(counts[4]);
    uint32_t pick = RandomDevice::uniform_u32(total);
    int chosenRes = -1;
    uint32_t acc = 0;
    for (int r = 0; r < 5; ++r) {
        acc += counts[r];
        if (pick < acc) { chosenRes = r; break; }
    }
    if (chosenRes < 0) return; // safety

    // Create a StealResource action for the chosen resource
    Action::PackedAction stealAction = 0;
    stealAction = Action::packType(stealAction, ActionType::StealResource);
    stealAction = Action::packPlayerID(stealAction, playerId);
    stealAction = Action::packArg1(stealAction, static_cast<uint8_t>(chosenRes));
    
    handleStealResource(stealAction, playerId);
}

void BoardState::handleBuildRoad(Action::PackedAction action, PlayerId playerId) {
    auto edgeId = Action::unpackArg1(action);
    edges[edgeId] = Edge::packHasRoad(edges[edgeId], true);
    edges[edgeId] = Edge::packOwner(edges[edgeId], playerId);

    // Deduct resources from player
    Player::buy(packedPlayers[static_cast<uint8_t>(playerId)], BuyableType::Road);
    BoardState::packedBank = Bank::sell(BoardState::packedBank, BuyableType::Road);
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
    BoardState::packedBank = Bank::sell(BoardState::packedBank, BuyableType::Settlement);
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
    BoardState::packedBank = Bank::sell(BoardState::packedBank, BuyableType::City);
}

void BoardState::handleBuyDevCard(PlayerId playerId) {
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
    BoardState::packedBank = Bank::sell(BoardState::packedBank, BuyableType::DevCard);

}


void BoardState::handlePlayDevCardKnight(Action::PackedAction action, PlayerId playerId) {
    auto hexId = Action::unpackArg1(action);
    robberPosition = hexId;

    // Decrement knight dev card count
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];
    auto curr = Player::unpackDevCard(p, DevType::Knight);
    if (curr > 0) {
        p = Player::packDevCard(p, DevType::Knight, curr - 1);
    }

    handleMoveRobber(action, playerId);
}

void BoardState::handlePlayDevCardRoadBuilding(Action::PackedAction action, PlayerId playerId) {
    // First, decrement the road building card count
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];
    auto curr = Player::unpackDevCard(p, DevType::RoadBuilding);
    
    // Decrement the card count
    p = Player::packDevCard(p, DevType::RoadBuilding, curr - 1);

    // Build the first road (edge ID in arg1)
    auto firstEdgeId = Action::unpackArg1(action);
    edges[firstEdgeId] = Edge::packHasRoad(edges[firstEdgeId], true);
    edges[firstEdgeId] = Edge::packOwner(edges[firstEdgeId], playerId);
    
    // The second edge ID is packed in the resource fields (since we need two arguments)
    // By convention, let's use the Brick field to store it
    auto secondEdgeId = Action::unpackResource(action, Resource::Brick);
    edges[secondEdgeId] = Edge::packHasRoad(edges[secondEdgeId], true);
    edges[secondEdgeId] = Edge::packOwner(edges[secondEdgeId], playerId);

    // Update the player's available road count for both roads
    packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packAvailableStructures(
            packedPlayers[static_cast<uint8_t>(playerId)],
            StructureType::Road,
            Player::unpackAvailableStructures(
                packedPlayers[static_cast<uint8_t>(playerId)],
                StructureType::Road) - 2
        );
}

void BoardState::handlePlayDevCardYearOfPlenty(Action::PackedAction action, PlayerId playerId) {
    // First, decrement the year of plenty card count
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];
    auto curr = Player::unpackDevCard(p, DevType::YearOfPlenty);

    // Decrement the card count
    p = Player::packDevCard(p, DevType::YearOfPlenty, curr - 1);

    // Get the two resources from the action (using Brick and Lumber fields by convention)
    Resource firstResource = static_cast<Resource>(Action::unpackResource(action, Resource::Brick));
    Resource secondResource = static_cast<Resource>(Action::unpackResource(action, Resource::Lumber));

    // Add first resource to player
    auto firstHave = Player::unpackResource(p, firstResource);
    uint16_t newFirst = uint16_t(firstHave) + 1;
    p = Player::packResource(p, firstResource, uint8_t(newFirst));

    // Add second resource to player
    auto secondHave = Player::unpackResource(p, secondResource);
    uint16_t newSecond = uint16_t(secondHave) + 1;
    p = Player::packResource(p, secondResource, uint8_t(newSecond));
}

void BoardState::handlePlayDevCardMonopoly(Action::PackedAction action, PlayerId playerId) {
    // First, decrement the monopoly card count
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];
    auto curr = Player::unpackDevCard(p, DevType::Monopoly);
    
    // Decrement the card count
    p = Player::packDevCard(p, DevType::Monopoly, curr - 1);

    // Get the resource type from the action (using Brick field by convention)
    Resource targetResource = static_cast<Resource>(Action::unpackResource(action, Resource::Brick));
    auto enemyPlayerId = playerId == PlayerId::Player0 ? PlayerId::Player1 : PlayerId::Player0;
    
    // Count how many of the resource the enemy has
    auto enemyHave = Player::unpackResource(packedPlayers[static_cast<uint8_t>(enemyPlayerId)], targetResource);
    
    // Remove all of this resource from enemy
    if (enemyHave > 0) {
        packedPlayers[static_cast<uint8_t>(enemyPlayerId)] =
            Player::packResource(packedPlayers[static_cast<uint8_t>(enemyPlayerId)], targetResource, 0);

        // Give all to current player
        auto curHave = Player::unpackResource(p, targetResource);
        uint16_t newVal = uint16_t(curHave) + uint16_t(enemyHave);
        p = Player::packResource(p, targetResource, uint8_t(newVal));
    }
}

void BoardState::handleTradeBank(Action::PackedAction action, PlayerId playerId) {
    Resource giveResource = static_cast<Resource>(Action::unpackArg1(action));
    Resource receiveResource = static_cast<Resource>(Action::unpackArg2(action));
    uint8_t ratio = Action::unpackArg3(action);

    auto idx = static_cast<uint8_t>(playerId);
    auto &p = packedPlayers[idx];

    // Player resource count
    uint8_t playerHaveGive = Player::unpackResource(p, giveResource);

    // Bank resource count
    uint8_t bankHaveReceive = Bank::unpackResource(BoardState::packedBank, receiveResource);

    // ---- PERFORM TRADE ----
    // Player gives 'ratio'
    p = Player::packResource(p, giveResource, playerHaveGive - ratio);

    uint8_t bankGiveVal = Bank::unpackResource(BoardState::packedBank, giveResource);
    BoardState::packedBank = Bank::packResource(BoardState::packedBank, giveResource, bankGiveVal + ratio);

    // Bank gives 1
    BoardState::packedBank = Bank::packResource(BoardState::packedBank, receiveResource, bankHaveReceive - 1);

    uint8_t playerHaveReceive = Player::unpackResource(p, receiveResource);
    p = Player::packResource(p, receiveResource, playerHaveReceive + 1);
}

void BoardState::handleReceiveResources(Action::PackedAction action, PlayerId playerId) {
    Resource res = static_cast<Resource>(Action::unpackArg1(action));
    uint8_t amount = Action::unpackArg2(action);
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];
    auto curr = Player::unpackResource(p, res);
    uint16_t newVal = uint16_t(curr) + uint16_t(amount);
    p = Player::packResource(p, res, uint8_t(newVal));
}

void BoardState::handleDiscardResources(Action::PackedAction action, PlayerId playerId) {
    auto &p = packedPlayers[static_cast<uint8_t>(playerId)];
    
    p = Player::packResource(
        p,
        Resource::Brick,
        Player::unpackResource(p, Resource::Brick) - Action::unpackResource(action, Resource::Brick)
    );
    p = Player::packResource(
        p,
        Resource::Lumber,
        Player::unpackResource(p, Resource::Lumber) - Action::unpackResource(action, Resource::Lumber)
    );
    p = Player::packResource(
        p,
        Resource::Wool,
        Player::unpackResource(p, Resource::Wool) - Action::unpackResource(action, Resource::Wool)
    );
    p = Player::packResource(
        p,
        Resource::Grain,
        Player::unpackResource(p, Resource::Grain) - Action::unpackResource(action, Resource::Grain)
    );
    p = Player::packResource(
        p,
        Resource::Ore,
        Player::unpackResource(p, Resource::Ore) - Action::unpackResource(action, Resource::Ore)
    );
}

void BoardState::handleStealResource(Action::PackedAction action, PlayerId playerId) {
    auto enemyPlayerId = playerId == PlayerId::Player0 ? PlayerId::Player1 : PlayerId::Player0;
    Resource res = static_cast<Resource>(Action::unpackArg1(action));
    
    // Remove one from enemy
    // handleDiscardResources();
    auto enemyHave = Player::unpackResource(packedPlayers[static_cast<uint8_t>(enemyPlayerId)], res);
    packedPlayers[static_cast<uint8_t>(enemyPlayerId)] =
        Player::packResource(packedPlayers[static_cast<uint8_t>(enemyPlayerId)], res, enemyHave - 1);

    // Give one to current player
    auto curHave = Player::unpackResource(packedPlayers[static_cast<uint8_t>(playerId)], res);
    uint16_t newVal = uint16_t(curHave) + 1;
    packedPlayers[static_cast<uint8_t>(playerId)] =
        Player::packResource(packedPlayers[static_cast<uint8_t>(playerId)], res, uint8_t(newVal));
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
        handleEndTurn(action, playerId);
        break;
    case ActionType::RollDice:
        handleRollDice(action, playerId);
        break;
    case ActionType::MoveRobber:
        handleMoveRobber(action, playerId);
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
        handleBuyDevCard(playerId);
        break;
    case ActionType::PlayDevCardKnight:
        handlePlayDevCardKnight(action, playerId);
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