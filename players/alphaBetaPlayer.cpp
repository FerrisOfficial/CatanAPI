#include "alphaBetaPlayer.hpp"
#include "playerHelpers.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <vector>

namespace {

using PlayerHelpers::dice_pips;
using PlayerHelpers::effective_vp;
using PlayerHelpers::unpack_resources;
using PlayerHelpers::hand_count;
using PlayerHelpers::cost_for;
using PlayerHelpers::deficit;
using PlayerHelpers::production_score_for_player;
using PlayerHelpers::settlement_potential_score;
using PlayerHelpers::is_deterministic_action;

constexpr int kMaxDepth = 3; // search depth (turn-based, not action-based)

enum class GamePhase {
    Early,   // Max VP < 5
    Mid,     // Max VP 5-7
    Late     // Max VP >= 8
};

GamePhase determine_game_phase(const Board::BoardState* board) {
    int maxVP = 0;
    for (int i = 0; i < 2; ++i) {
        const int vp = effective_vp(board, static_cast<PlayerId>(i));
        if (vp > maxVP) maxVP = vp;
    }
    
    if (maxVP >= 8) return GamePhase::Late;
    if (maxVP >= 5) return GamePhase::Mid;
    return GamePhase::Early;
}

int evaluate_position(const Board::BoardState* board, PlayerId selfId) {
    const PlayerId enemyId = (selfId == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;

    const auto selfPacked = board->packedPlayers[static_cast<uint8_t>(selfId)];
    const auto enemyPacked = board->packedPlayers[static_cast<uint8_t>(enemyId)];

    const int selfVP = effective_vp(board, selfId);
    const int enemyVP = effective_vp(board, enemyId);
    
    const GamePhase phase = determine_game_phase(board);
    
    // Phase-dependent VP weight: heavier in late game
    int vpWeight = 50000;
    if (phase == GamePhase::Late) {
        vpWeight = 100000; // Drastically increase VP importance in endgame
        // Extra bonus if we're close to winning
        if (selfVP >= 9) vpWeight = 200000;
    } else if (phase == GamePhase::Mid) {
        vpWeight = 65000;
    }
    const int vpTerm = vpWeight * (selfVP - enemyVP);

    // Phase-dependent production weight: less important in late game
    int prodWeight = 35;
    int potWeight = 12;
    if (phase == GamePhase::Late) {
        prodWeight = 15;  // Production matters less when racing to 10 VP
        potWeight = 5;    // Settlement potential matters less
    } else if (phase == GamePhase::Early) {
        prodWeight = 45;  // Production very important early
        potWeight = 18;   // Expansion potential critical
    }
    
    const int prodTerm = prodWeight * (production_score_for_player(board, selfId) - production_score_for_player(board, enemyId));
    const int potTerm = potWeight * (settlement_potential_score(board, selfId) - settlement_potential_score(board, enemyId));

    const auto selfHave = unpack_resources(selfPacked);
    const auto enemyHave = unpack_resources(enemyPacked);
    const int selfHand = static_cast<int>(hand_count(selfHave));
    const int enemyHand = static_cast<int>(hand_count(enemyHave));

    // Phase-dependent deficit penalties
    int cityDefWeight = 2200;
    int settleDefWeight = 1600;
    int devDefWeight = 650;
    
    if (phase == GamePhase::Late) {
        // In late game, prioritize immediate VP gains
        cityDefWeight = 4500;      // Cities are 2 VP, heavily prioritize
        settleDefWeight = 3500;    // Settlements are 1 VP, also important
        devDefWeight = 1200;       // Dev cards can be VPs too
    } else if (phase == GamePhase::Early) {
        settleDefWeight = 2000;    // Settlements more important than cities early
        cityDefWeight = 1800;
    }
    
    const int cityDef = static_cast<int>(deficit(selfHave, cost_for(BuyableType::City)));
    const int settleDef = static_cast<int>(deficit(selfHave, cost_for(BuyableType::Settlement)));
    const int devDef = static_cast<int>(deficit(selfHave, cost_for(BuyableType::DevCard)));
    const int deficitTerm = -cityDefWeight * cityDef - settleDefWeight * settleDef - devDefWeight * devDef;

    // Hand-size risk management
    const int overLimit = std::max(0, selfHand - 9);
    const int riskTerm = -180 * overLimit;

    const int resTerm = 60 * (selfHand - enemyHand);

    // Dev cards and longest road bonuses
    int devWeight = 250;
    int roadWeight = 200;
    if (phase == GamePhase::Late) {
        devWeight = 400;   // Dev cards can have VPs
        roadWeight = 500;  // Longest road is 2 VP, critical in endgame
    }
    
    const int devTerm = devWeight * (static_cast<int>(Player::totalDevCards(selfPacked)) - static_cast<int>(Player::totalDevCards(enemyPacked)));
    const int roadLenTerm = roadWeight * (static_cast<int>(Player::unpackLongestRoadLength(selfPacked)) - static_cast<int>(Player::unpackLongestRoadLength(enemyPacked)));

    return vpTerm + prodTerm + potTerm + deficitTerm + riskTerm + resTerm + devTerm + roadLenTerm;
}

// Quick heuristic to prioritize actions (higher = better)
int quick_action_score(Action::PackedAction action, const Board::BoardState* board, PlayerId selfId) {
    const ActionType type = Action::unpackType(action);
    
    switch (type) {
        case ActionType::BuildCity:
            return 10000; // Highest priority: +2 VP
        case ActionType::BuildSettlement:
            return 9000;  // High priority: +1 VP and production
        case ActionType::BuildRoad:
            return 3000;  // Medium priority: enables settlements
        case ActionType::TradeBank: {
            // Evaluate if trade helps us afford high-value purchases
            const auto selfPacked = board->packedPlayers[static_cast<uint8_t>(selfId)];
            const auto have = unpack_resources(selfPacked);
            
            // Check if trade gets us closer to city or settlement
            const int cityDefBefore = static_cast<int>(deficit(have, cost_for(BuyableType::City)));
            const int settleDefBefore = static_cast<int>(deficit(have, cost_for(BuyableType::Settlement)));
            
            // Simulate trade to see improvement (simplified check)
            // Lower deficit after trade = higher score
            return 2000 - (cityDefBefore + settleDefBefore) * 100;
        }
        case ActionType::EndTurn:
            return 0;  // Lowest priority
        default:
            return 1000;
    }
}

// Add expected resource production based on pips and probabilities
// Returns the exact amount of resources added for each type
std::array<int, 5> addExpectedResources(Board::BoardState* board, PlayerId playerId) {
    auto packed = board->packedPlayers[static_cast<uint8_t>(playerId)];
    auto resources = unpack_resources(packed);
    
    // Calculate expected production for each resource type
    std::array<int, 5> expectedProduction = {0, 0, 0, 0, 0};
    
    for (NodeId nodeId = 0; nodeId < NODE_COUNT; ++nodeId) {
        const auto node = board->nodes[nodeId];
        const auto owner = Board::Node::unpackOwner(node);
        if (owner != playerId) continue;
        
        const auto structure = Board::Node::unpackStructure(node);
        const int multiplier = (structure == StructureType::City) ? 2 : 1;
        
        // Check adjacent hexes
        for (uint8_t i = 0; i < 3; ++i) {
            const HexId hexId = Board::Node::unpackAdjacentHex(node, i);
            if (hexId == HexIdNone || hexId >= HEX_COUNT) continue;
            
            const auto hex = board->hexes[hexId];
            const auto resource = Board::Hex::unpackResource(hex);
            if (resource == Resource::NoResource) continue;
            
            const uint8_t pips = dice_pips(Board::Hex::unpackCatanNumber(hex));
            const int productionAmount = (pips * multiplier) / 12;
            
            expectedProduction[static_cast<size_t>(resource)] += productionAmount;
        }
    }
    
    // Add expected resources to player's hand
    for (size_t i = 0; i < 5; ++i) {
        if (expectedProduction[i] > 0) {
            resources[i] = std::min(255, resources[i] + expectedProduction[i]);
        }
    }
    
    // Pack resources back
    packed = Player::packResource(packed, Resource::Brick, resources[0]);
    packed = Player::packResource(packed, Resource::Lumber, resources[1]);
    packed = Player::packResource(packed, Resource::Wool, resources[2]);
    packed = Player::packResource(packed, Resource::Grain, resources[3]);
    packed = Player::packResource(packed, Resource::Ore, resources[4]);
    board->packedPlayers[static_cast<uint8_t>(playerId)] = packed;
    
    return expectedProduction;
}

// Remove exact amount of resources that were added
void removeExpectedResources(Board::BoardState* board, PlayerId playerId, const std::array<int, 5>& amountAdded) {
    auto packed = board->packedPlayers[static_cast<uint8_t>(playerId)];
    auto resources = unpack_resources(packed);
    
    // Remove exactly what was added
    for (size_t i = 0; i < 5; ++i) {
        if (amountAdded[i] > 0) {
            resources[i] = static_cast<uint8_t>(std::max(0, static_cast<int>(resources[i]) - amountAdded[i]));
        }
    }
    
    // Pack resources back
    packed = Player::packResource(packed, Resource::Brick, resources[0]);
    packed = Player::packResource(packed, Resource::Lumber, resources[1]);
    packed = Player::packResource(packed, Resource::Wool, resources[2]);
    packed = Player::packResource(packed, Resource::Grain, resources[3]);
    packed = Player::packResource(packed, Resource::Ore, resources[4]);
    board->packedPlayers[static_cast<uint8_t>(playerId)] = packed;
}

// Simulate a full turn for a player (all actions until EndTurn)
// Returns the number of actions taken (for undo purposes)
int simulateFullTurn(Board::BoardState* board, PlayerId playerId, bool maximizing) {
    int actionCount = 0;
    const int maxActionsPerTurn = 20; // Safety limit to prevent infinite loops
    
    while (actionCount < maxActionsPerTurn) {
        auto actions = board->getLegalActions(playerId);
        if (actions.empty()) break;
        
        // Filter to deterministic actions
        std::vector<std::pair<int, Action::PackedAction>> scoredActions;
        Action::PackedAction endTurn = Action::getEmptyAction();
        
        for (const auto a : actions) {
            if (!is_deterministic_action(a)) continue;
            
            if (Action::unpackType(a) == ActionType::EndTurn) {
                endTurn = a;
            } else {
                scoredActions.push_back({quick_action_score(a, board, playerId), a});
            }
        }
        
        // If only EndTurn available, take it
        if (scoredActions.empty()) {
            if (Action::unpackType(endTurn) == ActionType::EndTurn) {
                board->applyAction(endTurn);
                actionCount++;
            }
            break;
        }
        
        // Sort actions by heuristic score
        if (maximizing) {
            std::sort(scoredActions.begin(), scoredActions.end(),
                     [](const auto& a, const auto& b) { return a.first > b.first; });
        } else {
            std::sort(scoredActions.begin(), scoredActions.end(),
                     [](const auto& a, const auto& b) { return a.first < b.first; });
        }
        
        // Take the best action (greedy simulation for opponent)
        board->applyAction(scoredActions[0].second);
        actionCount++;
    }
    
    return actionCount;
}

// Alpha-beta minimax search operating on full turns
int alphaBeta(Board::BoardState* board, PlayerId selfId, int depth, int alpha, int beta, bool maximizing) {
    if (depth == 0) {
        return evaluate_position(board, selfId);
    }

    const PlayerId currentPlayer = board->currentPlayer;
    
    auto actions = board->getLegalActions(currentPlayer);
    
    // Filter to deterministic actions only
    std::vector<std::pair<int, Action::PackedAction>> scoredActions;
    for (const auto a : actions) {
        if (!is_deterministic_action(a)) continue;
        if (Action::unpackType(a) == ActionType::EndTurn) continue;
        scoredActions.push_back({quick_action_score(a, board, currentPlayer), a});
    }
    
    // If no meaningful actions, just evaluate position
    if (scoredActions.empty()) {
        return evaluate_position(board, selfId);
    }
    
    // Sort actions by heuristic for better pruning
    if (maximizing) {
        std::sort(scoredActions.begin(), scoredActions.end(),
                  [](const auto& a, const auto& b) { return a.first > b.first; });
    } else {
        std::sort(scoredActions.begin(), scoredActions.end(),
                  [](const auto& a, const auto& b) { return a.first < b.first; });
    }
    
    if (maximizing) {
        int maxEval = std::numeric_limits<int>::min();
        
        // Try each first action, then simulate rest of turn greedily
        for (const auto& [score, firstAction] : scoredActions) {
            board->applyAction(firstAction);
            int actionsApplied = 1;
            
            // Simulate rest of turn greedily
            actionsApplied += simulateFullTurn(board, currentPlayer, true);
            const auto resourcesAdded = addExpectedResources(board, currentPlayer);
            
            // Switch to opponent's turn
            board->currentPlayer = (currentPlayer == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;
            
            const int eval = alphaBeta(board, selfId, depth - 1, alpha, beta, false);

            board->currentPlayer = currentPlayer;
            removeExpectedResources(board, currentPlayer, resourcesAdded);
            
            // Undo all actions from this turn (in reverse)
            for (int i = 0; i < actionsApplied; ++i) {
                board->undoLastAction();
            }

            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) break; // Beta cutoff
        }
        return maxEval;
        
    } else {
        int minEval = std::numeric_limits<int>::max();
        
        // Try each first action, then simulate rest of turn greedily
        for (const auto& [score, firstAction] : scoredActions) {
            board->applyAction(firstAction);
            int actionsApplied = 1;
            
            // Simulate rest of turn greedily
            actionsApplied += simulateFullTurn(board, currentPlayer, false);
            const auto resourcesAdded = addExpectedResources(board, currentPlayer);
            
            // Switch to opponent's turn
            board->currentPlayer = (currentPlayer == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;
            
            // Recursively evaluate opponent's turn
            const int eval = alphaBeta(board, selfId, depth - 1, alpha, beta, true);
            
            // Undo opponent player switch
            board->currentPlayer = currentPlayer;
            removeExpectedResources(board, currentPlayer, resourcesAdded);
            
            // Undo all actions from this turn (in reverse)
            for (int i = 0; i < actionsApplied; ++i) {
                board->undoLastAction();
            }

            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) break;
        }
        return minEval;
    }
}

} // namespace

Action::PackedAction alphaBetaPlayer::getTurnAction() {
    const PlayerId selfId = boardState->currentPlayer;

    auto actions = boardState->getLegalActions(selfId);
    if (actions.empty()) return Action::getEmptyAction();

    std::vector<Action::PackedAction> buildActions;
    std::vector<Action::PackedAction> tradeActions;
    std::vector<Action::PackedAction> otherActions; 
    Action::PackedAction devBuyAction = Action::getEmptyAction();
    Action::PackedAction endTurnAction = Action::getEmptyAction();

    for (const auto a : actions) {
        if (!is_deterministic_action(a)) {
            if (Action::unpackType(a) == ActionType::BuyDevCard) {
                devBuyAction = a;
            }
            continue;
        }
        
        const ActionType type = Action::unpackType(a);
        if (type == ActionType::BuildCity || type == ActionType::BuildSettlement || type == ActionType::BuildRoad) {
            buildActions.push_back(a);
        } else if (type == ActionType::TradeBank) {
            tradeActions.push_back(a);
        } else if (type == ActionType::EndTurn) {
            endTurnAction = a;
        } else {
            otherActions.push_back(a);
        }
    }
    
    std::vector<Action::PackedAction> deterministicActions;
    
    // Always include all building actions (highest value)
    deterministicActions.insert(deterministicActions.end(), buildActions.begin(), buildActions.end());
    deterministicActions.insert(deterministicActions.end(), otherActions.begin(), otherActions.end());
    
    // Filter trades based on game phase and count
    const GamePhase phase = determine_game_phase(boardState);
    size_t maxTrades = 10;
    
    if (phase == GamePhase::Late) {
        maxTrades = 6;
    } else if (phase == GamePhase::Early) {
        maxTrades = 15;
    }
    
    if (tradeActions.size() > maxTrades) {
        // Score and sort trades, keep only the best ones
        std::vector<std::pair<int, Action::PackedAction>> scoredTrades;
        for (const auto a : tradeActions) {
            scoredTrades.push_back({quick_action_score(a, boardState, selfId), a});
        }
        std::sort(scoredTrades.begin(), scoredTrades.end(), 
                  [](const auto& a, const auto& b) { return a.first > b.first; });
        
        for (size_t i = 0; i < maxTrades && i < scoredTrades.size(); ++i) {
            deterministicActions.push_back(scoredTrades[i].second);
        }
    } else {
        deterministicActions.insert(deterministicActions.end(), tradeActions.begin(), tradeActions.end());
    }
    
    // Always include EndTurn as fallback
    if (Action::unpackType(endTurnAction) == ActionType::EndTurn) {
        deterministicActions.push_back(endTurnAction);
    }

    if (deterministicActions.empty()) {
        // If only non-deterministic actions available (like dev card buy), fall back to it5 logic
        return It5Player::getTurnAction();
    }

    int searchDepth = kMaxDepth;
    if (phase == GamePhase::Late) {
        searchDepth = kMaxDepth;
    } else if (phase == GamePhase::Early && deterministicActions.size() > 15) {
        searchDepth = std::max(1, kMaxDepth - 1);
    }

    // Run alpha-beta search on deterministic actions
    Action::PackedAction bestAction = Action::getEmptyAction();
    int bestScore = std::numeric_limits<int>::min();

    // In late game, prioritize city/settlement builds
    if (phase == GamePhase::Late) {
        for (const auto a : deterministicActions) {
            if (Action::unpackType(a) == ActionType::BuildCity || Action::unpackType(a) == ActionType::BuildSettlement) {
                boardState->applyAction(a);
                const int score = alphaBeta(boardState, selfId, searchDepth, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), false);
                boardState->undoLastAction();

                if (score > bestScore) {
                    bestScore = score;
                    bestAction = a;
                }
            }
        }
        
        // If we found a good build in late game, take it immediately
        if (Action::unpackType(bestAction) == ActionType::BuildCity || Action::unpackType(bestAction) == ActionType::BuildSettlement) {
            return bestAction;
        }
    }

    // Sort actions for root search (best first for better alpha-beta pruning)
    std::vector<std::pair<int, Action::PackedAction>> rootScoredActions;
    for (const auto a : deterministicActions) {
        rootScoredActions.push_back({quick_action_score(a, boardState, selfId), a});
    }
    std::sort(rootScoredActions.begin(), rootScoredActions.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });
    
    // Search all deterministic actions (now in priority order)
    for (const auto& [heuristic, a] : rootScoredActions) {
        boardState->applyAction(a);
        const int score = alphaBeta(boardState, selfId, searchDepth, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), false);
        boardState->undoLastAction();

        if (score > bestScore) {
            bestScore = score;
            bestAction = a;
        }
    }

    // Consider dev card buy heuristically (same as it5Player)
    if (Action::unpackType(devBuyAction) == ActionType::BuyDevCard) {
        const PlayerId enemyId = (selfId == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;
        const int selfVP = effective_vp(boardState, selfId);
        const int enemyVP = effective_vp(boardState, enemyId);
        const int baseScore = evaluate_position(boardState, selfId);

        int devScore = baseScore + 1500;
        if (enemyVP > selfVP) devScore += 2500;
        if (selfVP >= 8) devScore -= 500;

        // Check if we have trades/roads as alternatives
        bool hasTrade = false;
        bool hasRoad = false;
        for (const auto a : deterministicActions) {
            if (Action::unpackType(a) == ActionType::TradeBank) hasTrade = true;
            if (Action::unpackType(a) == ActionType::BuildRoad) hasRoad = true;
        }
        if (hasTrade) devScore -= 500;
        if (hasRoad) devScore -= 200;

        if (devScore > bestScore) {
            bestScore = devScore;
            bestAction = devBuyAction;
        }
    }

    // If the best action is EndTurn and doesn't improve position, fall back to it5
    if (Action::unpackType(bestAction) == ActionType::EndTurn) {
        const int currentScore = evaluate_position(boardState, selfId);
        if (bestScore < currentScore) {
            return It5Player::getTurnAction();
        }
    }

    if (Action::unpackType(bestAction) == ActionType::NoAction) {
        return It5Player::getTurnAction();
    }

    return bestAction;
}