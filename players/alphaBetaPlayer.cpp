#include "alphaBetaPlayer.hpp"
#include "playerHelpers.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <utility>
#include <vector>

namespace {

using PlayerHelpers::effective_vp;
using PlayerHelpers::unpack_resources;
using PlayerHelpers::hand_count;
using PlayerHelpers::cost_for;
using PlayerHelpers::deficit;
using PlayerHelpers::production_score_for_player;
using PlayerHelpers::settlement_potential_score;
using PlayerHelpers::is_deterministic_action;

enum class GameStage {
    Early,
    Mid,
    Late
};

GameStage detect_stage(const Board::BoardState& board) {
    const int vp0 = effective_vp(board, PlayerId::Player0);
    const int vp1 = effective_vp(board, PlayerId::Player1);
    const int maxVp = std::max(vp0, vp1);

    if (maxVp < 5 && board.currentTurn < 18) return GameStage::Early;
    if (maxVp < 9) return GameStage::Mid;
    return GameStage::Late;
}

struct ExpectedRollGain {
    int selfGain = 0;  // weighted sum over dice outcomes
    int enemyGain = 0;
};

int sum_resources(const std::array<uint8_t, 5>& res) {
    int s = 0;
    for (auto v : res) s += static_cast<int>(v);
    return s;
}

ExpectedRollGain expected_roll_gain(const Board::BoardState& board, PlayerId selfId) {
    static const int diceWeights[13] = {
        0, 0, 1, 2, 3, 4, 5, 6, 5, 4, 3, 2, 1
    };

    const PlayerId enemyId = (selfId == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;

    const auto beforeSelf = unpack_resources(board.packedPlayers[static_cast<uint8_t>(selfId)]);
    const auto beforeEnemy = unpack_resources(board.packedPlayers[static_cast<uint8_t>(enemyId)]);

    ExpectedRollGain out;

    for (uint8_t dice = 2; dice <= 12; ++dice) {
        if (dice == 7) continue;
        const int w = diceWeights[dice];
        if (w == 0) continue;

        Board::BoardState sim = board; // copy for safe simulation
        Action::PackedAction a = Action::packType(0, ActionType::RollDice);
        a = Action::packArg1(a, dice);
        sim.handleRollDice(a);

        const auto afterSelf = unpack_resources(sim.packedPlayers[static_cast<uint8_t>(selfId)]);
        const auto afterEnemy = unpack_resources(sim.packedPlayers[static_cast<uint8_t>(enemyId)]);

        const int selfDelta = sum_resources(afterSelf) - sum_resources(beforeSelf);
        const int enemyDelta = sum_resources(afterEnemy) - sum_resources(beforeEnemy);

        out.selfGain += w * selfDelta;
        out.enemyGain += w * enemyDelta;
    }

    return out;
}

int evaluate_position_stage(const Board::BoardState& board, PlayerId selfId) {
    const PlayerId enemyId = (selfId == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;
    const auto selfPacked = board.packedPlayers[static_cast<uint8_t>(selfId)];
    const auto enemyPacked = board.packedPlayers[static_cast<uint8_t>(enemyId)];

    const int selfVP = effective_vp(board, selfId);
    const int enemyVP = effective_vp(board, enemyId);

    const auto selfHave = unpack_resources(selfPacked);
    const auto enemyHave = unpack_resources(enemyPacked);
    const int selfHand = static_cast<int>(hand_count(selfHave));
    const int enemyHand = static_cast<int>(hand_count(enemyHave));

    const GameStage stage = detect_stage(board);

    int vpWeight = 50000;
    int prodWeight = 35;
    int potWeight = 12;
    int cityDefW = -2200;
    int settleDefW = -1600;
    int devDefW = -650;
    int resTermW = 60;
    int devTermW = 250;
    int roadTermW = 200;
    int expectedRollW = 320;

    if (stage == GameStage::Early) {
        prodWeight = 45;
        potWeight = 16;
        cityDefW = -1500;
        settleDefW = -1850;
        devDefW = -350;
        resTermW = 70;
        devTermW = 180;
        roadTermW = 240;
        expectedRollW = 420;
    } else if (stage == GameStage::Late) {
        vpWeight = 60000;
        prodWeight = 25;
        potWeight = 6;
        cityDefW = -3000;
        settleDefW = -1100;
        devDefW = -900;
        resTermW = 50;
        devTermW = 320;
        roadTermW = 160;
        expectedRollW = 260;
    }

    const int vpTerm = vpWeight * (selfVP - enemyVP);
    const int prodTerm = prodWeight *
        (production_score_for_player(&board, selfId) - production_score_for_player(&board, enemyId));
    const int potTerm = potWeight *
        (settlement_potential_score(&board, selfId) - settlement_potential_score(&board, enemyId));

    const int cityDef = static_cast<int>(deficit(selfHave, cost_for(BuyableType::City)));
    const int settleDef = static_cast<int>(deficit(selfHave, cost_for(BuyableType::Settlement)));
    const int devDef = static_cast<int>(deficit(selfHave, cost_for(BuyableType::DevCard)));
    const int deficitTerm = cityDefW * cityDef + settleDefW * settleDef + devDefW * devDef;

    const int overLimit = std::max(0, selfHand - 9);
    const int riskTerm = -180 * overLimit;

    const int resTerm = resTermW * (selfHand - enemyHand);
    const int devTerm = devTermW *
        (static_cast<int>(Player::totalDevCards(selfPacked)) - static_cast<int>(Player::totalDevCards(enemyPacked)));
    const int roadTerm = roadTermW *
        (static_cast<int>(Player::unpackLongestRoadLength(selfPacked)) -
         static_cast<int>(Player::unpackLongestRoadLength(enemyPacked)));

    const auto expected = expected_roll_gain(board, selfId);
    const int expectedTerm = expectedRollW * (expected.selfGain - expected.enemyGain);

    int immediateBuildBonus = 0;
    if (Player::hasEnoughResources(selfPacked, BuyableType::City)) immediateBuildBonus += (stage == GameStage::Late) ? 9000 : 6000;
    if (Player::hasEnoughResources(selfPacked, BuyableType::Settlement)) immediateBuildBonus += (stage == GameStage::Early) ? 6000 : 4500;

    return vpTerm + prodTerm + potTerm + deficitTerm + riskTerm + resTerm + devTerm + roadTerm + expectedTerm + immediateBuildBonus;
}

int action_order_score(const Board::BoardState& board, PlayerId selfId, Action::PackedAction a) {
    Board::BoardState sim = board;
    sim.applyAction(a);
    int s = evaluate_position_stage(sim, selfId);

    const auto type = Action::unpackType(a);
    if (type == ActionType::TradeBank || type == ActionType::BuildRoad) {
        auto next = sim.getLegalActions(selfId);
        bool canCity = false;
        bool canSettle = false;
        for (const auto na : next) {
            if (Action::unpackType(na) == ActionType::BuildCity) canCity = true;
            if (Action::unpackType(na) == ActionType::BuildSettlement) canSettle = true;
        }
        if (canCity) s += 8000;
        if (canSettle) s += 5000;
    }

    return s;
}

std::vector<Action::PackedAction> filtered_actions(
    const Board::BoardState& board,
    PlayerId playerId,
    PlayerId selfId
) {
    auto actions = const_cast<Board::BoardState&>(board).getLegalActions(playerId);
    std::vector<Action::PackedAction> out;
    out.reserve(actions.size());

    for (const auto a : actions) {
        if (is_deterministic_action(a)) out.push_back(a);
    }

    if (out.empty()) return actions;

    std::vector<std::pair<int, Action::PackedAction>> scored;
    scored.reserve(out.size());
    for (const auto a : out) {
        scored.emplace_back(action_order_score(board, selfId, a), a);
    }

    const bool maximizing = (playerId == selfId);
    std::sort(scored.begin(), scored.end(), [&](const auto& lhs, const auto& rhs) {
        return maximizing ? (lhs.first > rhs.first) : (lhs.first < rhs.first);
    });

    const size_t maxActions = 14;
    out.clear();
    out.reserve(std::min(maxActions, scored.size()));

    bool hasEndTurn = false;
    for (size_t i = 0; i < scored.size() && out.size() < maxActions; ++i) {
        out.push_back(scored[i].second);
        if (Action::unpackType(scored[i].second) == ActionType::EndTurn) hasEndTurn = true;
    }

    if (!hasEndTurn) {
        for (const auto& s : scored) {
            if (Action::unpackType(s.second) == ActionType::EndTurn) {
                out.push_back(s.second);
                break;
            }
        }
    }

    return out;
}

int alphabeta(
    const Board::BoardState& board,
    PlayerId selfId,
    int depth,
    int alpha,
    int beta
) {
    if (depth <= 0) {
        return evaluate_position_stage(board, selfId);
    }

    const PlayerId current = board.currentPlayer;
    auto actions = filtered_actions(board, current, selfId);
    if (actions.empty()) {
        return evaluate_position_stage(board, selfId);
    }

    const bool maximizing = (current == selfId);

    if (maximizing) {
        int best = std::numeric_limits<int>::min();
        for (const auto a : actions) {
            Board::BoardState sim = board;
            sim.applyAction(a);
            const int score = alphabeta(sim, selfId, depth - 1, alpha, beta);
            best = std::max(best, score);
            alpha = std::max(alpha, score);
            if (alpha >= beta) break;
        }
        return best;
    }

    int best = std::numeric_limits<int>::max();
    for (const auto a : actions) {
        Board::BoardState sim = board;
        sim.applyAction(a);
        const int score = alphabeta(sim, selfId, depth - 1, alpha, beta);
        best = std::min(best, score);
        beta = std::min(beta, score);
        if (alpha >= beta) break;
    }
    return best;
}

} // namespace

Action::PackedAction alphaBetaPlayer::getTurnAction() {
    const PlayerId selfId = boardState->currentPlayer;

    auto actions = boardState->getLegalActions(selfId);
    if (actions.empty()) return Action::getEmptyAction();

    const int baseScore = evaluate_position_stage(*boardState, selfId);

    Action::PackedAction best = Action::getEmptyAction();
    int bestScore = std::numeric_limits<int>::min();

    const int depth = 3;
    auto candidates = filtered_actions(*boardState, selfId, selfId);

    for (const auto a : candidates) {
        Board::BoardState sim = *boardState; // simulate on copy
        sim.applyAction(a);
        const int score = alphabeta(sim, selfId, depth - 1,
                                    std::numeric_limits<int>::min(),
                                    std::numeric_limits<int>::max());
        if (score > bestScore) {
            bestScore = score;
            best = a;
        }
    }

    // It5-style dev-buy heuristic as a backstop (RNG, so not in alpha-beta).
    Action::PackedAction devBuy = Action::getEmptyAction();
    bool hasTrade = false;
    bool hasRoad = false;
    for (const auto a : actions) {
        if (Action::unpackType(a) == ActionType::BuyDevCard) devBuy = a;
        if (Action::unpackType(a) == ActionType::TradeBank) hasTrade = true;
        if (Action::unpackType(a) == ActionType::BuildRoad) hasRoad = true;
    }

    if (Action::unpackType(devBuy) == ActionType::BuyDevCard) {
        const PlayerId enemyId = (selfId == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;
        const int selfVP = effective_vp(*boardState, selfId);
        const int enemyVP = effective_vp(*boardState, enemyId);

        int devScore = baseScore + 1500;
        if (enemyVP > selfVP) devScore += 2500;
        if (selfVP >= 10) devScore -= 700;
        if (hasTrade) devScore -= 600;
        if (hasRoad) devScore -= 250;

        if (devScore > bestScore) {
            bestScore = devScore;
            best = devBuy;
        }
    }

    if (Action::unpackType(best) == ActionType::NoAction) {
        return It5Player::getTurnAction();
    }

    if (Action::unpackType(best) == ActionType::EndTurn && bestScore < baseScore) {
        return It5Player::getTurnAction();
    }

    return best;
}
