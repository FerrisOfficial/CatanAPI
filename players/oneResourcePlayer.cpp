#include "oneResourcePlayer.hpp"
#include "playerHelpers.hpp"

#include <array>
#include <algorithm>
#include <limits>
#include <vector>

namespace {

using PlayerHelpers::dice_pips;
using PlayerHelpers::evaluate_position;
using PlayerHelpers::unpack_resources;
using PlayerHelpers::node_distance_rule_ok;

PortType port_for_resource(Resource r) {
	switch (r) {
		case Resource::Brick:  return PortType::BrickPort;
		case Resource::Lumber: return PortType::LumberPort;
		case Resource::Wool:   return PortType::WoolPort;
		case Resource::Grain:  return PortType::GrainPort;
		case Resource::Ore:    return PortType::OrePort;
		default:               return PortType::NoPort;
	}
}

int node_resource_pips(const Board::BoardState* board, NodeId nodeId, Resource r) {
	if (nodeId >= NODE_COUNT) return 0;
	const auto node = board->nodes[nodeId];
	int s = 0;
	for (uint8_t i = 0; i < 3; ++i) {
		const HexId h = Board::Node::unpackAdjacentHex(node, i);
		if (h == HexIdNone || h >= HEX_COUNT) continue;
		const auto hex = board->hexes[h];
		if (Board::Hex::unpackResource(hex) != r) continue;
		s += dice_pips(Board::Hex::unpackCatanNumber(hex));
	}
	return s;
}

std::array<bool, 5> resources_at_node(const Board::BoardState* board, NodeId nodeId) {
	std::array<bool, 5> has {false, false, false, false, false};
	if (nodeId >= NODE_COUNT) return has;
	const auto node = board->nodes[nodeId];
	for (uint8_t i = 0; i < 3; ++i) {
		const HexId hexId = Board::Node::unpackAdjacentHex(node, i);
		if (hexId == HexIdNone || hexId >= HEX_COUNT) continue;
		const Resource r = Board::Hex::unpackResource(board->hexes[hexId]);
		if (r == Resource::NoResource) continue;
		if (static_cast<uint8_t>(r) < 5) {
			has[static_cast<size_t>(r)] = true;
		}
	}
	return has;
}

bool node_has_2to1_port(const Board::BoardState* board, NodeId nodeId, Resource r) {
	if (nodeId >= NODE_COUNT) return false;
	const PortType pt = Board::Node::unpackPortType(board->nodes[nodeId]);
	return pt == port_for_resource(r);
}

Resource choose_priority_resource(const Board::BoardState* board) {
	// For each resource:
	// - totalPips: sum of pips from all hexes of this resource
	// - maxNodePips: best single node (how many pips of this resource one settlement can provide)
	// - bestPortNodePips: best node with 2:1 port for this resource
	// - hexCount: count of hexes with this resource
	std::array<int, 5> totalPips      {0, 0, 0, 0, 0};
	std::array<int, 5> maxNodePips    {0, 0, 0, 0, 0};
	std::array<int, 5> bestPortNodePips {0, 0, 0, 0, 0};
	std::array<int, 5> hexCount       {0, 0, 0, 0, 0};

	// 1) Sum pips across hexes + count hexes of this resource
	for (HexId h = 0; h < HEX_COUNT; ++h) {
		const auto hex = board->hexes[h];
		const Resource r = Board::Hex::unpackResource(hex);
		if (r == Resource::NoResource) continue;

		const int idx = static_cast<int>(r);
		const int p   = dice_pips(Board::Hex::unpackCatanNumber(hex));

		totalPips[idx] += p;
		hexCount[idx]  += 1;
	}

	// 2) Calculate best-scoring nodes and nodes with 2:1 port
	for (NodeId n = 0; n < NODE_COUNT; ++n) {
		for (Resource r : {Resource::Brick, Resource::Lumber, Resource::Wool, Resource::Grain, Resource::Ore}) {
			const int idx = static_cast<int>(r);
			const int p   = node_resource_pips(board, n, r);

			if (p > maxNodePips[idx]) {
				maxNodePips[idx] = p;
			}
			if (node_has_2to1_port(board, n, r)) {
				if (p > bestPortNodePips[idx]) {
					bestPortNodePips[idx] = p;
				}
			}
		}
	}

	// 3) Compose the final score
	int bestScore = std::numeric_limits<int>::min();
	Resource bestR = Resource::Brick;

	for (Resource r : {Resource::Brick, Resource::Lumber, Resource::Wool, Resource::Grain}) {
		const int idx = static_cast<int>(r);

		// Weights chosen so that:
		// - 2:1 port on a good spot is very attractive,
		// - strong single node (e.g., 3 hexes of this resource) is very important,
		// - sum of pips and number of hexes somewhat boost the score.
		int s = 0;
		s += 50 * totalPips[idx];         // general "presence" of the resource on the map
		s += 40 * maxNodePips[idx];       // how strong can the best settlement spot be
		s += 60 * bestPortNodePips[idx];  // how strong can the spot with 2:1 port be
		s += 10 * hexCount[idx];          // prefer resource with many hexes

		// Minimal tie-breaker: slightly reward brick/lumber because they provide roads
		if (r == Resource::Brick || r == Resource::Lumber) {
			s += 20;
		}

		if (s > bestScore) {
			bestScore = s;
			bestR     = r;
		}
	}

	return bestR;
}

struct PlacementScore {
	int score = std::numeric_limits<int>::min();
	Action::PackedAction action = Action::getEmptyAction();
};

PlacementScore pick_best_initial_port(const Board::BoardState* board,
									  const std::vector<Action::PackedAction>& actions,
									  Resource prio) {
	PlacementScore best;
	for (const auto a : actions) {
		const NodeId nodeId = Action::unpackArg1(a);
		const EdgeId edgeId = Action::unpackArg2(a);
		if (nodeId >= NODE_COUNT || edgeId == EdgeIdNone) continue;
		if (!node_has_2to1_port(board, nodeId, prio)) continue; // enforce 2:1 for first

		int s = 0;
		// Strong bonus for matching 2:1 port.
		s += 5000;
		// Prefer nodes with many pips of prioritized resource.
		s += 50 * node_resource_pips(board, nodeId, prio);

		// Slight bonus if the road leads to a node with many adjacent edges (future options).
		NodeId n0 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 0);
		NodeId n1 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 1);
		const NodeId other = (n0 == nodeId) ? n1 : n0;
		if (other < NODE_COUNT) {
			const auto adj = Board::Node::getAdjacentEdges(board->nodes[other]);
			int deg = 0; for (auto e : adj) deg += (e != EdgeIdNone) ? 1 : 0;
			s += deg; // 1..3
		}

		if (s > best.score) { best.score = s; best.action = a; }
	}
	return best;
}

PlacementScore pick_best_second_placement(const Board::BoardState* board,
										  const std::vector<Action::PackedAction>& actions,
										  Resource prio,
										  const std::array<bool,5>& firstRes) {
	PlacementScore best;
	for (const auto a : actions) {
		const NodeId nodeId = Action::unpackArg1(a);
		const EdgeId edgeId = Action::unpackArg2(a);
		if (nodeId >= NODE_COUNT || edgeId == EdgeIdNone) continue;

		int s = 0;
		// Prefer lots of prioritized resource pips.
		s += 60 * node_resource_pips(board, nodeId, prio);
		// Small complementary bonus for covering missing non-prior resources.
		auto has = resources_at_node(board, nodeId);
		for (size_t i = 0; i < 5; ++i) {
			if (has[i] && !firstRes[i] && i != static_cast<size_t>(prio)) s += 10;
		}

		// Connectivity bonus via the road target degree.
		NodeId n0 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 0);
		NodeId n1 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 1);
		const NodeId other = (n0 == nodeId) ? n1 : n0;
		if (other < NODE_COUNT) {
			const auto adj = Board::Node::getAdjacentEdges(board->nodes[other]);
			int deg = 0; for (auto e : adj) deg += (e != EdgeIdNone) ? 1 : 0;
			s += deg;
		}

		if (s > best.score) { best.score = s; best.action = a; }
	}

	if (best.score == std::numeric_limits<int>::min()) {
		best.action = actions.empty() ? Action::getEmptyAction() : actions.front();
		best.score = 0;
	}
	return best;
}

// Heuristic road potential for single-resource strategy.
// Looks at nodes within 1-2 edge range and ignores places
// where a settlement can no longer be placed (distance rule).
int road_priority_potential(const Board::BoardState* board, EdgeId edgeId, Resource prio) {
	if (edgeId == EdgeIdNone || edgeId >= EDGE_COUNT) return 0;
	const auto edge = board->edges[edgeId];

	auto eval_node = [&](NodeId nodeId, int dist) -> int {
		if (nodeId >= NODE_COUNT) return 0;

		// If we can't build a settlement here, then the road in this direction
		// gives little (we skip this node).
		if (!node_distance_rule_ok(board, nodeId)) return 0;

		int s = 0;

		// Pips of the prioritized resource at this node.
		const int pips = node_resource_pips(board, nodeId, prio);
		s += 50 * pips;

		// Large bonus for potential 2:1 port to the prioritized resource.
		// if (node_has_2to1_port(board, nodeId, prio)) {
		// 	s += 600;
		// }

		// Minimal bonus for node "degree" (more road/settlement options).
		const auto adj = Board::Node::getAdjacentEdges(board->nodes[nodeId]);
		int deg = 0; for (auto e : adj) deg += (e != EdgeIdNone) ? 1 : 0;
		s += 2 * deg;

		// Light discount for distance (distance 2 slightly worse than 1).
		s -= dist * 20;

		return s;
	};

	int best = 0;

	// Check both edge endpoints (this is distance 1 from current position).
	for (int i = 0; i < 2; ++i) {
		const NodeId node1 = Board::Edge::unpackAdjacentNode(edge, i);
		if (node1 >= NODE_COUNT) continue;

		int localBest = 0;

		// Node directly at the road.
		localBest = std::max(localBest, eval_node(node1, 1));

		// Nodes at distance 2: traverse one edge further from this node.
		const auto adjEdges = Board::Node::getAdjacentEdges(board->nodes[node1]);
		for (auto e2 : adjEdges) {
			if (e2 == EdgeIdNone || e2 == edgeId || e2 >= EDGE_COUNT) continue;
			const auto edge2 = board->edges[e2];

			NodeId node2a = Board::Edge::unpackAdjacentNode(edge2, 0);
			NodeId node2b = Board::Edge::unpackAdjacentNode(edge2, 1);
			const NodeId node2 = (node2a == node1) ? node2b : node2a;

			localBest = std::max(localBest, eval_node(node2, 2));
		}

		if (localBest > best) best = localBest;
	}

	return best;
}

} // namespace

std::pair<Action::PackedAction, Action::PackedAction> OneResourcePlayer::getInitialPlacement() {
	auto actions = boardState->generatePlaceInitialStructures(boardState->currentPlayer);
	if (actions.empty()) {
		auto noAction = Action::getEmptyAction();
		return {noAction, noAction};
	}

	// Choose prioritized resource based on current board.
	prioritizedResource = choose_priority_resource(boardState);
	priorityChosen = true;

	// Try to pick an action that places on a 2:1 port for the prioritized resource.
	auto best = pick_best_initial_port(boardState, actions, prioritizedResource);

	// If none found due to legal constraints, try any resource that has a 2:1 among actions.
	if (Action::unpackType(best.action) == ActionType::NoAction) {
		for (Resource r : {Resource::Brick, Resource::Lumber, Resource::Wool, Resource::Grain, Resource::Ore}) {
			auto cand = pick_best_initial_port(boardState, actions, r);
			if (Action::unpackType(cand.action) != ActionType::NoAction) {
				prioritizedResource = r;
				best = cand;
				break;
			}
		}
	}

	// If still no 2:1, fall back to the first legal action.
	if (Action::unpackType(best.action) == ActionType::NoAction) {
		best.action = actions.front();
	}

	firstSettlementNode = Action::unpackArg1(best.action);
	firstPlacementResources = resources_at_node(boardState, firstSettlementNode);
	return {best.action, best.action};
}

std::pair<Action::PackedAction, Action::PackedAction> OneResourcePlayer::get2InitialPlacement() {
	auto actions = boardState->generatePlace2InitialStructures(boardState->currentPlayer);
	if (actions.empty()) {
		auto noAction = Action::getEmptyAction();
		return {noAction, noAction};
	}

	if (!priorityChosen) {
		prioritizedResource = choose_priority_resource(boardState);
		priorityChosen = true;
	}

	auto best = pick_best_second_placement(boardState, actions, prioritizedResource, firstPlacementResources);
	return {best.action, best.action};
}

// Helper copied conceptually from It5: pick target build minimizing deficit with tie-breaks
static int priority_index(BuyableType b) {
	switch (b) {
		case BuyableType::City: return 0;
		case BuyableType::Settlement: return 1;
		case BuyableType::Road: return 2;
		case BuyableType::DevCard: return 3;
		default: return 99;
	}
}

static BuyableType best_target_for_discard(const std::array<uint8_t, 5>& have) {
	using PlayerHelpers::cost_for;
	using PlayerHelpers::deficit;
	BuyableType best = BuyableType::Road;
	uint16_t bestDef = std::numeric_limits<uint16_t>::max();
	for (BuyableType t : {BuyableType::City, BuyableType::Settlement, BuyableType::Road, BuyableType::DevCard}) {
		const auto need = cost_for(t);
		const auto d = deficit(have, need);
		if (d < bestDef || (d == bestDef && priority_index(t) < priority_index(best))) {
			bestDef = d; best = t;
		}
	}
	return best;
}

Action::PackedAction OneResourcePlayer::getDiscardAction() {
	using PlayerHelpers::unpack_resources;
	using PlayerHelpers::hand_count;
	using PlayerHelpers::cost_for;

	const PlayerId selfId = boardState->currentPlayer;
	const auto packed = boardState->packedPlayers[static_cast<uint8_t>(selfId)];

	auto have = unpack_resources(packed);
	const uint8_t total = hand_count(have);
	if (total <= 9) return Action::getEmptyAction();

	const uint8_t toDiscard = static_cast<uint8_t>(total / 2);

	const BuyableType target = best_target_for_discard(have);
	const auto need = cost_for(target);

	// Base keep weights with strong bias to keep the prioritized resource.
	std::array<int, 5> w = {10, 10, 8, 12, 13};
	w[static_cast<size_t>(prioritizedResource)] += 20; // strong keep bias

	Action::PackedAction action = Action::getEmptyAction();

	uint8_t remaining = toDiscard;
	while (remaining > 0) {
		int best = std::numeric_limits<int>::max();
		int bestIdx = -1;
		for (int i = 0; i < 5; ++i) {
			if (have[static_cast<size_t>(i)] == 0) continue;
			const bool neededForTarget = have[static_cast<size_t>(i)] <= need[static_cast<size_t>(i)];
			int discardCost = w[static_cast<size_t>(i)];
			if (neededForTarget) discardCost += 500;
			if (discardCost < best) { best = discardCost; bestIdx = i; }
		}
		if (bestIdx < 0) break;
		const auto r = static_cast<Resource>(bestIdx);
		const uint8_t current = Action::unpackResource(action, r);
		action = Action::packResource(action, r, static_cast<uint8_t>(current + 1));
		have[static_cast<size_t>(bestIdx)]--; remaining--;
	}
	return action;
}

Action::PackedAction OneResourcePlayer::getTurnAction() {
	using PlayerHelpers::is_deterministic_action;

	const PlayerId selfId = boardState->currentPlayer;
	auto actions = boardState->getLegalActions(selfId);
	if (actions.empty()) return Action::getEmptyAction();

	if (!priorityChosen) {
		prioritizedResource = choose_priority_resource(boardState);
		priorityChosen = true;
	}

	auto simulate_score = [&](Action::PackedAction a, int extraBonus) -> int {
		if (!is_deterministic_action(a)) return std::numeric_limits<int>::min();
		boardState->applyAction(a);
		int s = evaluate_position(boardState, selfId) + extraBonus;
		boardState->undoLastAction();
		return s;
	};

	std::vector<Action::PackedAction> cityActions;
	std::vector<Action::PackedAction> settlementActions;
	std::vector<Action::PackedAction> roadActions;
	std::vector<Action::PackedAction> tradeActions;

	for (const auto a : actions) {
		switch (Action::unpackType(a)) {
			case ActionType::BuildCity:       cityActions.push_back(a);       break;
			case ActionType::BuildSettlement: settlementActions.push_back(a); break;
			case ActionType::BuildRoad:       roadActions.push_back(a);       break;
			case ActionType::TradeBank:       tradeActions.push_back(a);      break;
			default: break;
		}
	}

	Action::PackedAction best = Action::getEmptyAction();
	int bestScore = std::numeric_limits<int>::min();

	if (!cityActions.empty()) {
		for (const auto a : cityActions) {
			const NodeId nodeId = Action::unpackArg1(a);
			const int prioPips = node_resource_pips(boardState, nodeId, prioritizedResource);

			int bonus = 0;
			bonus += 400 * prioPips;
			if (node_has_2to1_port(boardState, nodeId, prioritizedResource)) {
				bonus += 1200;
			}

			int s = simulate_score(a, bonus);
			if (s > bestScore) { bestScore = s; best = a; }
		}
		if (Action::unpackType(best) == ActionType::BuildCity) {
			return best;
		}
	}

	if (!settlementActions.empty()) {
		for (const auto a : settlementActions) {
			const NodeId nodeId = Action::unpackArg1(a);
			const int prioPips = node_resource_pips(boardState, nodeId, prioritizedResource);

			int bonus = 0;
			bonus += 250 * prioPips;
			if (node_has_2to1_port(boardState, nodeId, prioritizedResource)) {
				bonus += 1000;
			}

			int s = simulate_score(a, bonus);
			if (s > bestScore) { bestScore = s; best = a; }
		}
		if (Action::unpackType(best) == ActionType::BuildSettlement) {
			return best;
		}
	}

	// 3) TRADE – use accumulated prioritized resource to get other resources via 2:1 port.
	// Prefer trades that SPEND prioritized resource (we should have a lot of it) to get resources needed for building.
	if (!tradeActions.empty()) {
		using PlayerHelpers::cost_for;
		using PlayerHelpers::deficit;
		
		const auto preHave = unpack_resources(boardState->packedPlayers[static_cast<uint8_t>(selfId)]);
		
		for (const auto a : tradeActions) {
			boardState->applyAction(a);
			const auto postHave = unpack_resources(boardState->packedPlayers[static_cast<uint8_t>(selfId)]);
			boardState->undoLastAction();

			const int delta = static_cast<int>(postHave[static_cast<size_t>(prioritizedResource)]) -
							  static_cast<int>(preHave[static_cast<size_t>(prioritizedResource)]);

			// We want to SPEND the prioritized resource (delta < 0) to get other resources
			if (delta >= 0) {
				continue;
			}

			// Check if trade helps us get closer to building something valuable
			const auto cityDefPre = deficit(preHave, cost_for(BuyableType::City));
			const auto cityDefPost = deficit(postHave, cost_for(BuyableType::City));
			const auto settleDefPre = deficit(preHave, cost_for(BuyableType::Settlement));
			const auto settleDefPost = deficit(postHave, cost_for(BuyableType::Settlement));
			
			int bonus = 0;
			
			// Strong bonus for enabling city/settlement build
			if (cityDefPost == 0 && cityDefPre > 0) bonus += 3000;
			if (settleDefPost == 0 && settleDefPre > 0) bonus += 2500;
			
			// Bonus for reducing deficit to city/settlement
			bonus += 150 * (cityDefPre - cityDefPost);
			bonus += 120 * (settleDefPre - settleDefPost);
			
			// Extra bonus for spending prioritized resource efficiently (via 2:1 port)
			bonus += 200 * (-delta);
			
			int s = simulate_score(a, bonus);
			if (s > bestScore) { bestScore = s; best = a; }
		}
		if (Action::unpackType(best) == ActionType::TradeBank) {
			return best;
		}
	}

	// 4) ROADS – those that potentially lead to fields with prioritized resource / 2:1 port.
	if (!roadActions.empty()) {
		for (const auto a : roadActions) {
			const EdgeId e = Action::unpackArg1(a);
			const int bonus = road_priority_potential(boardState, e, prioritizedResource);
			int s = simulate_score(a, bonus);
			if (s > bestScore) { bestScore = s; best = a; }
		}
		if (Action::unpackType(best) == ActionType::BuildRoad) {
			return best;
		}
	}

	// 5) If nothing stands out strongly – return to general, balanced bot.
	return It5Player::getTurnAction();
}
