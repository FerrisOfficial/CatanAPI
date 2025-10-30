#include "board.hpp"
#include "actions.hpp"
#include "consts.hpp"

namespace Board {

void BoardState::applyAction(Action::PackedAction action) {
    auto type = Action::unpackType(action);
    auto playerId = Action::unpackPlayerID(action);

    switch (type)
    {
    case ActionType::PlaceInitialSettlement:
        {
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
    case ActionType::Place2InitialSettlement:
        {
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
    case ActionType::PlaceInitialRoad:
        {
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
    case ActionType::EndTurn:
        {
        // Advance to next player's turn
        currentPlayer = (currentPlayer == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;
        currentTurn += 1;
        }
    case ActionType::RollDice:
    {
        auto diceValue = Action::unpackArg1(action);
        // Implement later
    }
    case ActionType::MoveRobber:
        {
            auto hexId = Action::unpackArg1(action);
            auto enemyPlayerId = playerId == PlayerId::Player0 ? PlayerId::Player1 : PlayerId::Player0;
            robberPosition = hexId;
            auto enemyWool = Player::unpackResource(packedPlayers[static_cast<uint8_t>(enemyPlayerId)], Resource::Wool);
            auto enemyBrick = Player::unpackResource(packedPlayers[static_cast<uint8_t>(enemyPlayerId)], Resource::Brick);
            auto enemyLumber = Player::unpackResource(packedPlayers[static_cast<uint8_t>(enemyPlayerId)], Resource::Lumber);
            auto enemyGrain = Player::unpackResource(packedPlayers[static_cast<uint8_t>(enemyPlayerId)], Resource::Grain);
            auto enemyOre = Player::unpackResource(packedPlayers[static_cast<uint8_t>(enemyPlayerId)], Resource::Ore);
            auto enemyTotalResources = enemyWool + enemyBrick + enemyLumber + enemyGrain + enemyOre;
            if (enemyTotalResources == 0)
                break; // No resources to steal

            // Steal a random resource from the enemy player
            // Right now, just steal Wool
            if (enemyWool > 0) {
                packedPlayers[static_cast<uint8_t>(enemyPlayerId)] =
                    Player::packResource(
                        packedPlayers[static_cast<uint8_t>(enemyPlayerId)],
                        Resource::Wool,
                        enemyWool - 1
                    );
                auto currentWool = Player::unpackResource(packedPlayers[static_cast<uint8_t>(playerId)], Resource::Wool);
                packedPlayers[static_cast<uint8_t>(playerId)] =
                    Player::packResource(
                        packedPlayers[static_cast<uint8_t>(playerId)],
                        Resource::Wool,
                        currentWool + 1
                    );
            }
        }
    case ActionType::DiscardResources:
        {
            // Implement later
        }
    case ActionType::BuildRoad:
        {
            auto edgeId = Action::unpackArg1(action);
            edges[edgeId] = Edge::packHasRoad(edges[edgeId], true);
            edges[edgeId] = Edge::packOwner(edges[edgeId], playerId);

            // Deduct resources from player
            Player::buy(packedPlayers[static_cast<uint8_t>(playerId)], BuyableType::Road);
        }
    case ActionType::BuildSettlement:
        {
            auto nodeId = Action::unpackArg1(action);
            nodes[nodeId] = Node::packStructure(nodes[nodeId], StructureType::Settlement);
            nodes[nodeId] = Node::packOwner(nodes[nodeId], playerId);

            // Deduct resources from player
            Player::buy(packedPlayers[static_cast<uint8_t>(playerId)], BuyableType::Settlement);
        }
    case ActionType::BuildCity:
        {
            auto nodeId = Action::unpackArg1(action);
            nodes[nodeId] = Node::packStructure(nodes[nodeId], StructureType::City);
            nodes[nodeId] = Node::packOwner(nodes[nodeId], playerId);

            // Deduct resources from player
            Player::buy(packedPlayers[static_cast<uint8_t>(playerId)], BuyableType::City);
        }
    case ActionType::BuyDevCard:
        {
           // To be implemented: randomly draw a dev card from the deck
        }


    default:
        break;
    }

}
} // namespace Board