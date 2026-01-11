#include "alphaBetaPlayer.hpp"
#include "playerHelpers.hpp"

#include <algorithm>
#include <array>
#include <limits>
#include <vector>

namespace {

using PlayerHelpers::dice_pips;
using PlayerHelpers::effective_vp;

constexpr int kMaxDepth = 5; // simple lookahead (ply-based)

bool node_distance_rule_ok(const Board::BoardState& board, NodeId nodeId) {
	const auto node = board.nodes[nodeId];
	if (Board::Node::unpackStructure(node) != StructureType::NoStructure) return false;

	for (uint8_t i = 0; i < 3; ++i) {
		const EdgeId edgeId = Board::Node::unpackAdjacentEdge(node, i);
		if (edgeId == EdgeIdNone || edgeId >= EDGE_COUNT) continue;

		const auto edge = board.edges[edgeId];
		for (uint8_t j = 0; j < 2; ++j) {
			const NodeId adj = Board::Edge::unpackAdjacentNode(edge, j);
			if (adj == nodeId || adj >= NODE_COUNT) continue;
			const auto adjNode = board.nodes[adj];
			if (Board::Node::unpackStructure(adjNode) != StructureType::NoStructure) return false;
		}
	}
	return true;
}

bool node_adjacent_to_own_road(const Board::BoardState& board, PlayerId pid, NodeId nodeId) {
	const auto node = board.nodes[nodeId];
	for (uint8_t i = 0; i < 3; ++i) {
		const EdgeId edgeId = Board::Node::unpackAdjacentEdge(node, i);
		if (edgeId == EdgeIdNone || edgeId >= EDGE_COUNT) continue;
		const auto edge = board.edges[edgeId];
		if (Board::Edge::unpackHasRoad(edge) && Board::Edge::unpackOwner(edge) == pid) {
			return true;
		}
	}
	return false;
}

struct ProdStats {
	int pipScore = 0;
	std::array<bool, 5> hasResource {false, false, false, false, false};
};

ProdStats production_stats(const Board::BoardState& board, PlayerId pid) {
	ProdStats ps;

	for (NodeId nid = 0; nid < NODE_COUNT; ++nid) {
		const auto node = board.nodes[nid];
		if (Board::Node::unpackOwner(node) != pid) continue;

		const auto structure = Board::Node::unpackStructure(node);
		if (structure == StructureType::NoStructure) continue;

		const int mult = (structure == StructureType::City) ? 2 : 1;

		for (uint8_t i = 0; i < 3; ++i) {
			const HexId hid = Board::Node::unpackAdjacentHex(node, i);
			if (hid == HexIdNone || hid >= HEX_COUNT) continue;
			const auto hex = board.hexes[hid];
			const Resource r = Board::Hex::unpackResource(hex);
			if (r == Resource::NoResource || static_cast<uint8_t>(r) >= 5) continue;

			const uint8_t pips = dice_pips(Board::Hex::unpackCatanNumber(hex));
			ps.pipScore += static_cast<int>(pips) * mult;
			ps.hasResource[static_cast<size_t>(r)] = true;
		}
	}

	return ps;
}

int expansion_potential(const Board::BoardState& board, PlayerId pid) {
	int potential = 0;
	for (NodeId nid = 0; nid < NODE_COUNT; ++nid) {
		if (!node_distance_rule_ok(board, nid)) continue;
		if (!node_adjacent_to_own_road(board, pid, nid)) continue;
		++potential;
	}
	return potential;
}

bool is_endgame(const Board::BoardState& board) {
    int vp0 = effective_vp(board, PlayerId::Player0);
    int vp1 = effective_vp(board, PlayerId::Player1);
    int maxVp = std::max(vp0, vp1);

    return maxVp >= 7;
}

struct EvalWeights {
    int wVp;
    int wPip;
    int wDiv;
    int wExp;
};

constexpr EvalWeights kMidGameWeights  { 3000, 120, 220, 80 };
constexpr EvalWeights kEndGameWeights  { 5000, 120, 200, 70 };

int evaluate_player(const Board::BoardState& board, PlayerId pid, const EvalWeights& w) {
    const int vp = effective_vp(board, pid);
    const auto prod = production_stats(board, pid);

    int diversity = 0;
    for (bool h : prod.hasResource) diversity += h ? 1 : 0;

    const int expansion = expansion_potential(board, pid);

    return vp           * w.wVp
         + prod.pipScore* w.wPip
         + diversity    * w.wDiv
         + expansion    * w.wExp;
}

int evaluate_state(const Board::BoardState& board, PlayerId root) {
    const PlayerId opp = (root == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;

    const EvalWeights& W = is_endgame(board) ? kEndGameWeights : kMidGameWeights;

    return evaluate_player(board, root, W) - evaluate_player(board, opp, W);
}

bool is_deterministic(ActionType t) {
	switch (t) {
		case ActionType::BuildCity:
		case ActionType::BuildSettlement:
		case ActionType::BuildRoad:
		case ActionType::TradeBank:
		case ActionType::EndTurn:
			return true;
		default:
			return false;
	}
}

int alphabeta(Board::BoardState state, int depth, int alpha, int beta, PlayerId root) {
	if (depth == 0) return evaluate_state(state, root);

	auto actions = state.getLegalActions(state.currentPlayer);
	std::vector<Action::PackedAction> filtered;
	filtered.reserve(actions.size());
	for (auto a : actions) {
		if (is_deterministic(Action::unpackType(a))) filtered.push_back(a);
	}
	if (filtered.empty()) filtered = std::move(actions); // fall back if nothing deterministic

	if (filtered.empty()) return evaluate_state(state, root);

	const bool maximizing = (state.currentPlayer == root);
	int best = maximizing ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();

	for (const auto a : filtered) {
		state.applyAction(a);

		const int score = alphabeta(state, depth - 1, alpha, beta, root);

		state.undoLastAction();

		if (maximizing) {
			best = std::max(best, score);
			alpha = std::max(alpha, score);
		} else {
			best = std::min(best, score);
			beta = std::min(beta, score);
		}

		if (beta <= alpha) break; // prune
	}

	return best;
}

} // namespace

namespace {
int scoreCityAction(const Board::BoardState& board, PlayerId pid, Action::PackedAction a) {
    NodeId nodeId = Action::unpackArg1(a);

    // policz, ile pipów daje upgrade tej osady do miasta
    const auto node = board.nodes[nodeId];
    int addPips = 0;
    for (uint8_t i = 0; i < 3; ++i) {
        HexId hid = Board::Node::unpackAdjacentHex(node, i);
        if (hid == HexIdNone || hid >= HEX_COUNT) continue;
        const auto hex = board.hexes[hid];
        uint8_t pips = dice_pips(Board::Hex::unpackCatanNumber(hex));
        addPips += pips; // miasto = +1x produkcji z każdego hexa
    }

    return 10000 + addPips * 100; 
}

int scoreSettlementAction(const Board::BoardState& board, PlayerId pid, Action::PackedAction a) {
    NodeId nodeId = Action::unpackArg1(a);

    const auto node = board.nodes[nodeId];

    int pipScore = 0;
    std::array<bool, 5> hasRes{false, false, false, false, false};

    for (uint8_t i = 0; i < 3; ++i) {
        HexId hid = Board::Node::unpackAdjacentHex(node, i);
        if (hid == HexIdNone || hid >= HEX_COUNT) continue;
        const auto hex = board.hexes[hid];
        Resource r = Board::Hex::unpackResource(hex);
        if (r == Resource::NoResource || (size_t)r >= 5) continue;
        uint8_t p = dice_pips(Board::Hex::unpackCatanNumber(hex));
        pipScore += p;
        hasRes[(size_t)r] = true;
    }

    int diversity = 0;
    for (bool h : hasRes) diversity += h ? 1 : 0;

    // prosty score lokalny
    return 8000 + pipScore * 100 + diversity * 200;
}

int scoreRoadAction(const Board::BoardState& board, PlayerId pid, Action::PackedAction a) {
    EdgeId eid = Action::unpackArg1(a);

    auto edge = board.edges[eid];
    NodeId n0 = Board::Edge::unpackAdjacentNode(edge, 0);
    NodeId n1 = Board::Edge::unpackAdjacentNode(edge, 1);

    int bestFutureVertexScore = 0;

    auto evalNode = [&](NodeId nid){
        if (nid >= NODE_COUNT) return;

        // jeśli to od razu legalny spot na osadę
        if (!node_distance_rule_ok(board, nid)) return;

        // policz lokalne pipy i różnorodność dla tego node'a
        const auto node = board.nodes[nid];
        int pipScore = 0;
        std::array<bool, 5> hasRes{false, false, false, false, false};

        for (uint8_t i = 0; i < 3; ++i) {
            HexId hid = Board::Node::unpackAdjacentHex(node, i);
            if (hid == HexIdNone || hid >= HEX_COUNT) continue;
            const auto hex = board.hexes[hid];
            Resource r = Board::Hex::unpackResource(hex);
            if (r == Resource::NoResource || (size_t)r >= 5) continue;
            uint8_t p = dice_pips(Board::Hex::unpackCatanNumber(hex));
            pipScore += p;
            hasRes[(size_t)r] = true;
        }

        int diversity = 0;
        for (bool h : hasRes) diversity += h ? 1 : 0;

        int localScore = pipScore * 50 + diversity * 80;
        bestFutureVertexScore = std::max(bestFutureVertexScore, localScore);
    };

    evalNode(n0);
    evalNode(n1);

    // jeśli droga nie prowadzi do niczego sensownego
    if (bestFutureVertexScore == 0) {
        return 0; // nisko, będzie wycięta przez TOP-K
    }

    // generalny priorytet dróg niższy niż osad/miast
    return 2000 + bestFutureVertexScore;
}

bool canBuildSettlement(const Board::BoardState& board, PlayerId playerId) {
    const auto packed = board.packedPlayers[static_cast<uint8_t>(playerId)];
    if (Player::unpackResource(packed, Resource::Brick) < 1) return false;
    if (Player::unpackResource(packed, Resource::Lumber) < 1) return false;
    if (Player::unpackResource(packed, Resource::Wool) < 1) return false;
    if (Player::unpackResource(packed, Resource::Grain) < 1) return false;

    if (Player::unpackAvailableStructures(packed, StructureType::Settlement) < 1) return false;

    return true;
}

bool canBuildCity(const Board::BoardState& board, PlayerId playerId) {
    const auto packed = board.packedPlayers[static_cast<uint8_t>(playerId)];
    if (Player::unpackResource(packed, Resource::Grain) < 2) return false;
    if (Player::unpackResource(packed, Resource::Ore) < 3) return false;

    if (Player::unpackAvailableStructures(packed, StructureType::City) < 1) return false;

    return true;
}

bool canBuildRoad(const Board::BoardState& board, PlayerId playerId) {
    const auto packed = board.packedPlayers[static_cast<uint8_t>(playerId)];
    if (Player::unpackResource(packed, Resource::Brick) < 1) return false;
    if (Player::unpackResource(packed, Resource::Lumber) < 1) return false;

    if (Player::unpackAvailableStructures(packed, StructureType::Road) < 1) return false;

    return true;
}

int scoreTradeAction(Board::BoardState& board, PlayerId pid, Action::PackedAction a) {
    board.applyAction(a);

    bool canSet = canBuildSettlement(board, pid);
    bool canCity = canBuildCity(board, pid);
    bool canRoad = canBuildRoad(board, pid);

    board.undoLastAction();

    if (canCity) return 9000;
    if (canSet)  return 8500;
    if (canRoad) return 4000;

    return 100;
}

int heuristicActionScore(Board::BoardState& state, Action::PackedAction a) {
    const auto type = Action::unpackType(a);
    const PlayerId pid = state.currentPlayer;

    switch (type) {
        case ActionType::BuildCity:
            return scoreCityAction(state, pid, a);

        case ActionType::BuildSettlement:
            return scoreSettlementAction(state, pid, a);

        case ActionType::BuildRoad:
            return scoreRoadAction(state, pid, a);

        case ActionType::TradeBank:
            return scoreTradeAction(state, pid, a);

        case ActionType::EndTurn:
            return -100000;

        default:
            return 0;
    }
}


} // namespace

Action::PackedAction alphaBetaPlayer::getTurnAction() {
	const PlayerId selfId = boardState->currentPlayer;
	auto actions = boardState->getLegalActions(selfId);
	if (actions.empty()) return Action::getEmptyAction();

	std::vector<Action::PackedAction> deterministic;
	for (auto a : actions) {
		if (is_deterministic(Action::unpackType(a))) deterministic.push_back(a);
	}
	if (deterministic.empty()) deterministic = std::move(actions);

	std::vector<std::pair<int, Action::PackedAction>> scored;
	scored.reserve(deterministic.size());
	for (auto a : deterministic) {
		int h = heuristicActionScore(*boardState, a);
		scored.push_back({h, a});
	}

	std::sort(scored.begin(), scored.end(),
			[](auto& lhs, auto& rhs){ return lhs.first > rhs.first; });

	const int K = 5;
	std::vector<Action::PackedAction> pruned;
	for (int i = 0; i < (int)scored.size() && i < K; ++i) {
		pruned.push_back(scored[i].second);
	}

	Action::PackedAction best = Action::getEmptyAction();
	int bestScore = std::numeric_limits<int>::min();

	for (const auto a : pruned) {
		boardState->applyAction(a);
		const int score = alphabeta(*boardState, kMaxDepth - 1, std::numeric_limits<int>::min() / 2, std::numeric_limits<int>::max() / 2, selfId);
		boardState->undoLastAction();
		if (score > bestScore) {
			bestScore = score;
			best = a;
		}
	}

	// Fallback to the previous heuristic if nothing was chosen.
	if (Action::unpackType(best) == ActionType::NoAction) {
		return It5Player::getTurnAction();
	}

	return best;
}