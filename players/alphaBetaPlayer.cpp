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

constexpr int kMaxDepth = 3; // search depth (ply-based)

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

// Alpha-beta minimax search
int alphaBeta(Board::BoardState* board, PlayerId selfId, int depth, int alpha, int beta, bool maximizing) {
    // Terminal conditions
    if (depth == 0) {
        return evaluate_position(board, selfId);
    }

    const PlayerId currentPlayer = board->currentPlayer;
    const auto actions = board->getLegalActions(currentPlayer);

    // If no actions or only EndTurn available, evaluate position
    bool hasNonEndTurn = false;
    for (const auto a : actions) {
        if (Action::unpackType(a) != ActionType::EndTurn && is_deterministic_action(a)) {
            hasNonEndTurn = true;
            break;
        }
    }
    if (!hasNonEndTurn) {
        return evaluate_position(board, selfId);
    }

    // Order actions by heuristic for better pruning
    std::vector<std::pair<int, Action::PackedAction>> scoredActions;
    for (const auto a : actions) {
        if (!is_deterministic_action(a)) continue;
        scoredActions.push_back({quick_action_score(a, board, currentPlayer), a});
    }
    
    if (maximizing) {
        // Sort descending for maximizing player (try best moves first)
        std::sort(scoredActions.begin(), scoredActions.end(),
                  [](const auto& a, const auto& b) { return a.first > b.first; });
        
        int maxEval = std::numeric_limits<int>::min();
        for (const auto& [score, a] : scoredActions) {
            board->applyAction(a);
            const int eval = alphaBeta(board, selfId, depth - 1, alpha, beta, false);
            board->undoLastAction();

            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) break; // Beta cutoff
        }
        return maxEval;
    } else {
        // Sort ascending for minimizing player (try worst moves for opponent first)
        std::sort(scoredActions.begin(), scoredActions.end(),
                  [](const auto& a, const auto& b) { return a.first < b.first; });
        
        int minEval = std::numeric_limits<int>::max();
        for (const auto& [score, a] : scoredActions) {
            board->applyAction(a);
            const int eval = alphaBeta(board, selfId, depth - 1, alpha, beta, true);
            board->undoLastAction();

            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) break; // Alpha cutoff
        }
        return minEval;
    }
}

} // namespace

Action::PackedAction alphaBetaPlayer::getTurnAction() {
    const PlayerId selfId = boardState->currentPlayer;

    auto actions = boardState->getLegalActions(selfId);
    if (actions.empty()) return Action::getEmptyAction();

    // Separate deterministic from non-deterministic actions
    std::vector<Action::PackedAction> buildActions;    // Cities, settlements, roads
    std::vector<Action::PackedAction> tradeActions;    // Bank trades
    std::vector<Action::PackedAction> otherActions;    // Everything else
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
    
    // Combine actions with filtering
    std::vector<Action::PackedAction> deterministicActions;
    
    // Always include all building actions (highest value)
    deterministicActions.insert(deterministicActions.end(), buildActions.begin(), buildActions.end());
    deterministicActions.insert(deterministicActions.end(), otherActions.begin(), otherActions.end());
    
    // Filter trades based on game phase and count
    const GamePhase phase = determine_game_phase(boardState);
    size_t maxTrades = 10; // Default limit
    
    if (phase == GamePhase::Late) {
        maxTrades = 6;  // Fewer trades in endgame, focus on building
    } else if (phase == GamePhase::Early) {
        maxTrades = 15; // More trades early for flexibility
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

    int searchDepth = kMaxDepth - 1;
    if (phase == GamePhase::Late) {
        searchDepth = kMaxDepth; // Deeper search in endgame when decisions are critical
    } else if (phase == GamePhase::Early && deterministicActions.size() > 15) {
        searchDepth = std::max(2, kMaxDepth - 2); // Shallower search early game with many options
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