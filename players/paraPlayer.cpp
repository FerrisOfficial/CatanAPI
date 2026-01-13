#include "paraPlayer.hpp"

#include "game_simulation/board.hpp"
#include "utils/randomDevice.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

struct ParaParams {
    // --- policy ---
    double policy_temperature = 0.0; // 0 = greedy, >0 = softmax
    double policy_epsilon = 0.02;    // exploration (pick random legal action)
    int policy_top_k = 1;           // if >1, sample from top-k (greedy path)

    // Simple within-turn lookahead: evaluate action by value(after action) + gamma * (best_followup - value(after action)).
    // 1 = old behavior, 2 = consider one follow-up action.
    int policy_self_lookahead = 1;
    double policy_lookahead_gamma = 1.0;

    // Optional speed knob: apply expensive lookahead only to top-M actions by a cheap 1-ply score.
    // 0 = old behavior (lookahead on all actions that use it)
    int policy_lookahead_top_m = 0;

    // --- evaluation weights (roughly aligned with It5) ---
    double w_vp = 50000.0;
    double w_prod = 35.0;
    double w_potential = 12.0;

    double w_city_deficit = -2200.0;
    double w_settle_deficit = -1600.0;
    double w_dev_deficit = -650.0;

    double w_overlimit = -180.0; // (hand - 9)
    double w_hand_diff = 60.0;

    double w_dev_count = 250.0;
    double w_road_len = 200.0;

    // --- hand value (small shaping; helps trades / planning) ---
    double w_hand_brick = 20.0;
    double w_hand_lumber = 20.0;
    double w_hand_wool = 16.0;
    double w_hand_grain = 22.0;
    double w_hand_ore = 24.0;

    // Affordability indicators (encourages setting up purchases).
    double w_can_build_city = 900.0;
    double w_can_build_settlement = 650.0;
    double w_can_build_road = 220.0;
    double w_can_buy_dev = 380.0;

    // Game phase shaping (0..1): 1 early, 0 late.
    double phase_early_vp_threshold = 6.0; // vp <= this => early-ish

    // --- production scoring ---
    double res_w_brick = 12.0;
    double res_w_lumber = 12.0;
    double res_w_wool = 11.0;
    double res_w_grain = 13.0;
    double res_w_ore = 14.0;
    double port_three_for_one = 35.0;
    double port_two_for_one = 70.0;

    // --- action shaping bonuses ---
    double bonus_build_city = 0.0;
    double bonus_build_settlement = 0.0;
    double bonus_build_road = 0.0;
    double bonus_trade_bank = 0.0;
    double bonus_buy_dev = 1500.0;
    double penalty_end_turn = 0.0;

    // Dev-buy shaping (used as adjustments on top of bonus_buy_dev).
    double dev_buy_behind_bonus = 2500.0;
    double dev_buy_late_penalty = 500.0;
    double dev_buy_has_trade_penalty = 500.0;
    double dev_buy_has_road_penalty = 200.0;
    double dev_buy_blocks_city_penalty = 8000.0;
    double dev_buy_blocks_settlement_penalty = 5000.0;

    // Deterministic road heuristic (adds to simulated score of BuildRoad).
    double road_heuristic_w = 1.0;

    // --- robber ---
    double robber_enemy_value_w = 120.0; // legacy (kept for backwards compat)
    double robber_self_value_w = -160.0; // legacy
    double robber_pips_w = 8.0;          // legacy

    // Improved robber model (pips-weighted impact).
    double robber_enemy_pips_value_w = 30.0;
    double robber_self_pips_value_w = 45.0;
    double robber_enemy_only_bonus = 500.0;
    double robber_no_enemy_penalty = 200.0;
    double robber_desert_penalty = 800.0;

    // --- initial placement ---
    double init_prod_scale = 1.0;
    double init_diversity_bonus = 25.0;
    double init_duplicate_penalty = 20.0;
    double init_port_first = 15.0;
    double init_port_second = 25.0;
    double init_port_three_for_one_bonus = 10.0;
    double init_port_resource_bonus = 5.0;
    double init_complement_bonus = 18.0;
    double init_complement_brick_lumber_bonus = 10.0;
    double init_road_deg_bonus = 1.0;

    // --- discard keep weights ---
    double keep_brick = 10.0;
    double keep_lumber = 10.0;
    double keep_wool = 8.0;
    double keep_grain = 12.0;
    double keep_ore = 13.0;

    // per-target deltas
    double keep_city_ore = 10.0;
    double keep_city_grain = 8.0;

    double keep_settle_brick = 7.0;
    double keep_settle_lumber = 7.0;
    double keep_settle_wool = 6.0;
    double keep_settle_grain = 6.0;

    double keep_road_brick = 8.0;
    double keep_road_lumber = 8.0;

    double keep_dev_ore = 7.0;
    double keep_dev_grain = 7.0;
    double keep_dev_wool = 7.0;

    bool loadFromFile(const std::string& path) {
        std::ifstream in(path);
        if (!in) return false;

        auto trim = [](std::string s) {
            size_t b = 0;
            while (b < s.size() && std::isspace(static_cast<unsigned char>(s[b]))) ++b;
            size_t e = s.size();
            while (e > b && std::isspace(static_cast<unsigned char>(s[e - 1]))) --e;
            return s.substr(b, e - b);
        };

        std::unordered_map<std::string, double*> m;
        auto bind = [&](const char* k, double& v) { m.emplace(std::string(k), &v); };

        bind("policy.temperature", policy_temperature);
        bind("policy.epsilon", policy_epsilon);
        // policy.top_k is handled manually below (int).
        // policy.self_lookahead is handled manually below (int).
        // policy.lookahead_top_m is handled manually below (int).
        bind("policy.lookahead_gamma", policy_lookahead_gamma);

        bind("eval.w_vp", w_vp);
        bind("eval.w_prod", w_prod);
        bind("eval.w_potential", w_potential);
        bind("eval.w_city_deficit", w_city_deficit);
        bind("eval.w_settle_deficit", w_settle_deficit);
        bind("eval.w_dev_deficit", w_dev_deficit);
        bind("eval.w_overlimit", w_overlimit);
        bind("eval.w_hand_diff", w_hand_diff);
        bind("eval.w_dev_count", w_dev_count);
        bind("eval.w_road_len", w_road_len);

        bind("eval.w_hand_brick", w_hand_brick);
        bind("eval.w_hand_lumber", w_hand_lumber);
        bind("eval.w_hand_wool", w_hand_wool);
        bind("eval.w_hand_grain", w_hand_grain);
        bind("eval.w_hand_ore", w_hand_ore);

        bind("eval.w_can_build_city", w_can_build_city);
        bind("eval.w_can_build_settlement", w_can_build_settlement);
        bind("eval.w_can_build_road", w_can_build_road);
        bind("eval.w_can_buy_dev", w_can_buy_dev);

        bind("phase.early_vp_threshold", phase_early_vp_threshold);

        bind("prod.res_brick", res_w_brick);
        bind("prod.res_lumber", res_w_lumber);
        bind("prod.res_wool", res_w_wool);
        bind("prod.res_grain", res_w_grain);
        bind("prod.res_ore", res_w_ore);
        bind("prod.port_three_for_one", port_three_for_one);
        bind("prod.port_two_for_one", port_two_for_one);

        bind("action.bonus_build_city", bonus_build_city);
        bind("action.bonus_build_settlement", bonus_build_settlement);
        bind("action.bonus_build_road", bonus_build_road);
        bind("action.bonus_trade_bank", bonus_trade_bank);
        bind("action.bonus_buy_dev", bonus_buy_dev);
        bind("action.penalty_end_turn", penalty_end_turn);

        bind("action.dev_buy_behind_bonus", dev_buy_behind_bonus);
        bind("action.dev_buy_late_penalty", dev_buy_late_penalty);
        bind("action.dev_buy_has_trade_penalty", dev_buy_has_trade_penalty);
        bind("action.dev_buy_has_road_penalty", dev_buy_has_road_penalty);
        bind("action.dev_buy_blocks_city_penalty", dev_buy_blocks_city_penalty);
        bind("action.dev_buy_blocks_settlement_penalty", dev_buy_blocks_settlement_penalty);

        bind("action.road_heuristic_w", road_heuristic_w);

        bind("robber.enemy_value_w", robber_enemy_value_w);
        bind("robber.self_value_w", robber_self_value_w);
        bind("robber.pips_w", robber_pips_w);

        bind("robber.enemy_pips_value_w", robber_enemy_pips_value_w);
        bind("robber.self_pips_value_w", robber_self_pips_value_w);
        bind("robber.enemy_only_bonus", robber_enemy_only_bonus);
        bind("robber.no_enemy_penalty", robber_no_enemy_penalty);
        bind("robber.desert_penalty", robber_desert_penalty);

        bind("init.prod_scale", init_prod_scale);
        bind("init.diversity_bonus", init_diversity_bonus);
        bind("init.duplicate_penalty", init_duplicate_penalty);
        bind("init.port_first", init_port_first);
        bind("init.port_second", init_port_second);
        bind("init.port_three_for_one_bonus", init_port_three_for_one_bonus);
        bind("init.port_resource_bonus", init_port_resource_bonus);
        bind("init.complement_bonus", init_complement_bonus);
        bind("init.complement_brick_lumber_bonus", init_complement_brick_lumber_bonus);
        bind("init.road_deg_bonus", init_road_deg_bonus);

        bind("discard.keep_brick", keep_brick);
        bind("discard.keep_lumber", keep_lumber);
        bind("discard.keep_wool", keep_wool);
        bind("discard.keep_grain", keep_grain);
        bind("discard.keep_ore", keep_ore);

        bind("discard.keep_city_ore", keep_city_ore);
        bind("discard.keep_city_grain", keep_city_grain);

        bind("discard.keep_settle_brick", keep_settle_brick);
        bind("discard.keep_settle_lumber", keep_settle_lumber);
        bind("discard.keep_settle_wool", keep_settle_wool);
        bind("discard.keep_settle_grain", keep_settle_grain);

        bind("discard.keep_road_brick", keep_road_brick);
        bind("discard.keep_road_lumber", keep_road_lumber);

        bind("discard.keep_dev_ore", keep_dev_ore);
        bind("discard.keep_dev_grain", keep_dev_grain);
        bind("discard.keep_dev_wool", keep_dev_wool);

        std::string line;
        while (std::getline(in, line)) {
            line = trim(line);
            if (line.empty()) continue;
            if (line[0] == '#') continue;
            if (line.size() >= 2 && line[0] == '/' && line[1] == '/') continue;

            const auto pos = line.find('=');
            if (pos == std::string::npos) continue;

            const std::string key = trim(line.substr(0, pos));
            const std::string valStr = trim(line.substr(pos + 1));
            if (key.empty() || valStr.empty()) continue;

            char* end = nullptr;
            const double v = std::strtod(valStr.c_str(), &end);
            if (end == valStr.c_str()) continue;

            if (key == "policy.top_k") {
                const int iv = static_cast<int>(std::llround(v));
                policy_top_k = std::max(1, iv);
                continue;
            }

            if (key == "policy.self_lookahead") {
                const int iv = static_cast<int>(std::llround(v));
                policy_self_lookahead = std::max(1, std::min(3, iv));
                continue;
            }

            if (key == "policy.lookahead_top_m") {
                const int iv = static_cast<int>(std::llround(v));
                policy_lookahead_top_m = std::max(0, std::min(50, iv));
                continue;
            }

            auto it = m.find(key);
            if (it != m.end() && it->second) {
                *(it->second) = v;
            }
        }

        return true;
    }
};

std::string default_config_path() {
    namespace fs = std::filesystem;

    // Single canonical config file location:
    //   <repo_root>/players/paraPlayer.cfg
    // The runner may be started from various working directories (e.g. build/),
    // so we search upwards a few levels.
    fs::path dir = fs::current_path();
    for (int depth = 0; depth <= 6; ++depth) {
        const fs::path candidate = dir / "players" / "paraPlayer.cfg";
        std::ifstream t(candidate.string());
        if (t) return candidate.string();

        if (!dir.has_parent_path()) break;
        const fs::path parent = dir.parent_path();
        if (parent == dir) break;
        dir = parent;
    }

    // Final fallback: relative to current working directory.
    return (fs::path("players") / "paraPlayer.cfg").string();
}

std::string resolve_config_path(const std::string& current) {
    // Training / benchmarking helper: allow overriding the config path per-process.
    // This is intentionally opt-in so normal gameplay keeps using the canonical file.
    // Example:
    //   set CATAN_PARA_CFG=C:\path\to\cand.cfg
    //   build\runs\run.exe --games 200 para it5
    if (const char* env = std::getenv("CATAN_PARA_CFG")) {
        if (env[0] != '\0') return std::string(env);
    }

    // If we already resolved it once, keep it to avoid repeated upward-search on the hot path.
    if (!current.empty()) return current;
    return default_config_path();
}

std::array<uint8_t, 5> unpack_resources(Player::PackedPlayer p) {
    return {
        Player::unpackResource(p, Resource::Brick),
        Player::unpackResource(p, Resource::Lumber),
        Player::unpackResource(p, Resource::Wool),
        Player::unpackResource(p, Resource::Grain),
        Player::unpackResource(p, Resource::Ore),
    };
}

uint8_t hand_count(const std::array<uint8_t, 5>& have) {
    uint8_t s = 0;
    for (auto v : have) s = static_cast<uint8_t>(s + v);
    return s;
}

std::array<uint8_t, 5> cost_for(BuyableType b) {
    const auto& c = StructureCost[static_cast<size_t>(b)];
    return {c[0], c[1], c[2], c[3], c[4]};
}

bool has_own_settlement_to_upgrade(const Board::BoardState* board, PlayerId pid) {
    for (NodeId n = 0; n < NODE_COUNT; ++n) {
        const auto node = board->nodes[n];
        if (Board::Node::unpackOwner(node) != pid) continue;
        if (Board::Node::unpackStructure(node) == StructureType::Settlement) return true;
    }
    return false;
}

bool can_afford(const std::array<uint8_t, 5>& have, BuyableType b) {
    const auto need = cost_for(b);
    for (size_t i = 0; i < 5; ++i) {
        if (have[i] < need[i]) return false;
    }
    return true;
}

uint16_t deficit(const std::array<uint8_t, 5>& have, const std::array<uint8_t, 5>& need) {
    uint16_t d = 0;
    for (size_t i = 0; i < 5; ++i) {
        if (have[i] < need[i]) d = static_cast<uint16_t>(d + (need[i] - have[i]));
    }
    return d;
}

int effective_vp(const Board::BoardState* board, PlayerId pid) {
    const auto p = board->packedPlayers[static_cast<uint8_t>(pid)];
    int vp = static_cast<int>(Player::unpackVictoryPoints(p));
    if (Player::unpackLargestArmyFlag(p)) vp += 2;
    if (Player::unpackLongestRoadFlag(p)) vp += 2;
    return vp;
}

uint8_t dice_pips(uint8_t diceNumber) {
    switch (diceNumber) {
        case 2:  return 1;
        case 3:  return 2;
        case 4:  return 3;
        case 5:  return 4;
        case 6:  return 5;
        case 8:  return 5;
        case 9:  return 4;
        case 10: return 3;
        case 11: return 2;
        case 12: return 1;
        default: return 0;
    }
}

int node_production_score(const Board::BoardState* board, NodeId nodeId, const ParaParams& p) {
    if (nodeId >= NODE_COUNT) return std::numeric_limits<int>::min();
    const auto node = board->nodes[nodeId];

    auto res_weight = [&](Resource r) -> double {
        switch (r) {
            case Resource::Ore: return p.res_w_ore;
            case Resource::Grain: return p.res_w_grain;
            case Resource::Brick: return p.res_w_brick;
            case Resource::Lumber: return p.res_w_lumber;
            case Resource::Wool: return p.res_w_wool;
            default: return 0.0;
        }
    };

    double s = 0.0;
    for (int i = 0; i < 3; ++i) {
        const HexId h = Board::Node::unpackAdjacentHex(node, i);
        if (h == HexIdNone || h >= HEX_COUNT) continue;
        const auto hex = board->hexes[h];
        const Resource r = Board::Hex::unpackResource(hex);
        if (r == Resource::NoResource) continue;
        const uint8_t pip = dice_pips(Board::Hex::unpackCatanNumber(hex));
        s += static_cast<double>(pip) * res_weight(r);
    }

    const auto pt = Board::Node::unpackPortType(node);
    if (pt != PortType::NoPort) {
        if (pt == PortType::ThreeForOne) s += p.port_three_for_one;
        else s += p.port_two_for_one;
    }

    return static_cast<int>(std::llround(s));
}

int production_score_for_player(const Board::BoardState* board, PlayerId pid, const ParaParams& p) {
    int s = 0;
    for (NodeId n = 0; n < NODE_COUNT; ++n) {
        const auto node = board->nodes[n];
        if (Board::Node::unpackOwner(node) != pid) continue;
        const auto st = Board::Node::unpackStructure(node);
        if (st == StructureType::Settlement) {
            s += node_production_score(board, n, p);
        } else if (st == StructureType::City) {
            s += 2 * node_production_score(board, n, p);
        }
    }
    return s;
}

bool node_distance_rule_ok(const Board::BoardState* board, NodeId nodeId) {
    if (nodeId >= NODE_COUNT) return false;
    const auto node = board->nodes[nodeId];
    if (Board::Node::unpackStructure(node) != StructureType::NoStructure) return false;

    for (int i = 0; i < 3; ++i) {
        const EdgeId e = Board::Node::unpackAdjacentEdge(node, i);
        if (e == EdgeIdNone || e >= EDGE_COUNT) continue;
        const auto edge = board->edges[e];
        const NodeId a = Board::Edge::unpackAdjacentNode(edge, 0);
        const NodeId b = Board::Edge::unpackAdjacentNode(edge, 1);
        for (NodeId adj : {a, b}) {
            if (adj == nodeId || adj >= NODE_COUNT) continue;
            const auto st = Board::Node::unpackStructure(board->nodes[adj]);
            if (st == StructureType::Settlement || st == StructureType::City) return false;
        }
    }
    return true;
}

bool node_is_adjacent_to_own_road(const Board::BoardState* board, PlayerId pid, NodeId nodeId) {
    if (nodeId >= NODE_COUNT) return false;
    const auto node = board->nodes[nodeId];
    for (uint8_t i = 0; i < 3; ++i) {
        const EdgeId e = Board::Node::unpackAdjacentEdge(node, i);
        if (e == EdgeIdNone || e >= EDGE_COUNT) continue;
        if (!Board::Edge::unpackHasRoad(board->edges[e])) continue;
        if (Board::Edge::unpackOwner(board->edges[e]) != pid) continue;
        return true;
    }
    return false;
}

int settlement_potential_score(const Board::BoardState* board, PlayerId pid, const ParaParams& p) {
    std::array<int, 3> best = {0, 0, 0};

    const auto packed = board->packedPlayers[static_cast<uint8_t>(pid)];
    if (Player::unpackAvailableStructures(packed, StructureType::Settlement) == 0) return 0;

    for (NodeId n = 0; n < NODE_COUNT; ++n) {
        if (!node_distance_rule_ok(board, n)) continue;
        if (!node_is_adjacent_to_own_road(board, pid, n)) continue;

        const int sc = node_production_score(board, n, p);
        if (sc > best[0]) {
            best[2] = best[1];
            best[1] = best[0];
            best[0] = sc;
        } else if (sc > best[1]) {
            best[2] = best[1];
            best[1] = sc;
        } else if (sc > best[2]) {
            best[2] = sc;
        }
    }

    return best[0] + best[1] + best[2];
}

double early_phase01(const Board::BoardState* board, PlayerId pid, const ParaParams& p) {
    const int vp = effective_vp(board, pid);
    const double thr = std::max(1.0, p.phase_early_vp_threshold);
    // vp <= thr => ~1, vp >= thr+4 => ~0
    const double x = (static_cast<double>(vp) - thr) / 4.0;
    return std::max(0.0, std::min(1.0, 1.0 - x));
}

double evaluate_position(const Board::BoardState* board, PlayerId selfId, const ParaParams& p) {
    const PlayerId enemyId = (selfId == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;

    const auto selfPacked = board->packedPlayers[static_cast<uint8_t>(selfId)];
    const auto enemyPacked = board->packedPlayers[static_cast<uint8_t>(enemyId)];

    const int selfVP = effective_vp(board, selfId);
    const int enemyVP = effective_vp(board, enemyId);

    const int prodSelf = production_score_for_player(board, selfId, p);
    const int prodEnemy = production_score_for_player(board, enemyId, p);

    const int potSelf = settlement_potential_score(board, selfId, p);
    const int potEnemy = settlement_potential_score(board, enemyId, p);

    const auto selfHave = unpack_resources(selfPacked);
    const auto enemyHave = unpack_resources(enemyPacked);
    const int selfHand = static_cast<int>(hand_count(selfHave));
    const int enemyHand = static_cast<int>(hand_count(enemyHave));

    const int cityDef = static_cast<int>(deficit(selfHave, cost_for(BuyableType::City)));
    const int settleDef = static_cast<int>(deficit(selfHave, cost_for(BuyableType::Settlement)));
    const int devDef = static_cast<int>(deficit(selfHave, cost_for(BuyableType::DevCard)));

    const int overLimit = std::max(0, selfHand - 9);

    const int devCntDiff = static_cast<int>(Player::totalDevCards(selfPacked)) - static_cast<int>(Player::totalDevCards(enemyPacked));
    const int roadLenDiff = static_cast<int>(Player::unpackLongestRoadLength(selfPacked)) - static_cast<int>(Player::unpackLongestRoadLength(enemyPacked));

    const double early01 = early_phase01(board, selfId, p);

    const bool canCity = can_afford(selfHave, BuyableType::City)
        && Player::unpackAvailableStructures(selfPacked, StructureType::City) > 0
        && has_own_settlement_to_upgrade(board, selfId);
    const bool canSettle = can_afford(selfHave, BuyableType::Settlement)
        && Player::unpackAvailableStructures(selfPacked, StructureType::Settlement) > 0;
    const bool canRoad = can_afford(selfHave, BuyableType::Road)
        && Player::unpackAvailableStructures(selfPacked, StructureType::Road) > 0;
    const bool canDev = can_afford(selfHave, BuyableType::DevCard);

    double s = 0.0;
    s += p.w_vp * static_cast<double>(selfVP - enemyVP);
    s += p.w_prod * static_cast<double>(prodSelf - prodEnemy);
    s += p.w_potential * static_cast<double>(potSelf - potEnemy);

    s += p.w_city_deficit * static_cast<double>(cityDef);
    s += p.w_settle_deficit * static_cast<double>(settleDef);
    s += p.w_dev_deficit * static_cast<double>(devDef);

    s += p.w_overlimit * static_cast<double>(overLimit);
    s += p.w_hand_diff * static_cast<double>(selfHand - enemyHand);

    s += p.w_dev_count * static_cast<double>(devCntDiff);
    s += p.w_road_len * static_cast<double>(roadLenDiff);

    // Hand shaping: important early for tempo (roads/settles), later less so.
    s += early01 * (
        p.w_hand_brick * static_cast<double>(selfHave[static_cast<size_t>(Resource::Brick)]) +
        p.w_hand_lumber * static_cast<double>(selfHave[static_cast<size_t>(Resource::Lumber)]) +
        p.w_hand_wool * static_cast<double>(selfHave[static_cast<size_t>(Resource::Wool)]) +
        p.w_hand_grain * static_cast<double>(selfHave[static_cast<size_t>(Resource::Grain)]) +
        p.w_hand_ore * static_cast<double>(selfHave[static_cast<size_t>(Resource::Ore)])
    );

    // Encourage setting up immediate buys (helps trades and multi-step planning).
    s += p.w_can_build_city * (canCity ? 1.0 : 0.0);
    s += p.w_can_build_settlement * (canSettle ? 1.0 : 0.0);
    s += p.w_can_build_road * (canRoad ? 1.0 : 0.0);
    s += p.w_can_buy_dev * (canDev ? 1.0 : 0.0);

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

std::array<bool, 5> resources_covered_by_player(const Board::BoardState* board, PlayerId pid) {
    std::array<bool, 5> has {false, false, false, false, false};
    for (NodeId n = 0; n < NODE_COUNT; ++n) {
        const auto node = board->nodes[n];
        if (Board::Node::unpackOwner(node) != pid) continue;
        const auto st = Board::Node::unpackStructure(node);
        if (st != StructureType::Settlement && st != StructureType::City) continue;
        const auto at = resources_at_node(board, n);
        for (size_t i = 0; i < 5; ++i) has[i] = has[i] || at[i];
    }
    return has;
}

int score_initial_settlement_node(const Board::BoardState* board, NodeId nodeId, bool secondPlacement,
                                 const std::array<bool, 5>& alreadyHave, const ParaParams& p) {
    if (nodeId >= NODE_COUNT) return std::numeric_limits<int>::min();
    const auto node = board->nodes[nodeId];

    // Collect resource diversity info for the node.
    std::array<uint8_t, 5> resourceCounts {0, 0, 0, 0, 0};
    for (uint8_t i = 0; i < 3; ++i) {
        const HexId hexId = Board::Node::unpackAdjacentHex(node, i);
        if (hexId == HexIdNone || hexId >= HEX_COUNT) continue;
        const auto hex = board->hexes[hexId];
        const Resource r = Board::Hex::unpackResource(hex);
        if (r == Resource::NoResource) continue;
        if (static_cast<uint8_t>(r) < 5) {
            resourceCounts[static_cast<size_t>(r)]++;
        }
    }

    // Diversity / duplicates.
    int unique = 0;
    int duplicatePenalty = 0;
    for (size_t i = 0; i < 5; ++i) {
        unique += (resourceCounts[i] > 0) ? 1 : 0;
        if (resourceCounts[i] > 1) duplicatePenalty += static_cast<int>(p.init_duplicate_penalty) * static_cast<int>(resourceCounts[i] - 1);
    }
    int diversityBonus = static_cast<int>(p.init_diversity_bonus) * unique;

    // Port.
    int portBonus = 0;
    const auto port = Board::Node::unpackPortType(node);
    if (port != PortType::NoPort) {
        portBonus += static_cast<int>(secondPlacement ? p.init_port_second : p.init_port_first);
        if (port == PortType::ThreeForOne) portBonus += static_cast<int>(p.init_port_three_for_one_bonus);
        else portBonus += static_cast<int>(p.init_port_resource_bonus);
    }

    // Complement (2nd settlement covers missing resources).
    int complementBonus = 0;
    if (secondPlacement) {
        for (size_t i = 0; i < 5; ++i) {
            if (resourceCounts[i] > 0 && !alreadyHave[i]) complementBonus += static_cast<int>(p.init_complement_bonus);
        }
        if (!alreadyHave[static_cast<size_t>(Resource::Brick)] && resourceCounts[static_cast<size_t>(Resource::Brick)] > 0) {
            complementBonus += static_cast<int>(p.init_complement_brick_lumber_bonus);
        }
        if (!alreadyHave[static_cast<size_t>(Resource::Lumber)] && resourceCounts[static_cast<size_t>(Resource::Lumber)] > 0) {
            complementBonus += static_cast<int>(p.init_complement_brick_lumber_bonus);
        }
    }

    // Base production score from existing helper (already includes ports). Use it as primary signal.
    const int prodScore = node_production_score(board, nodeId, p);
    if (prodScore == std::numeric_limits<int>::min()) return prodScore;

    const double scaledProd = static_cast<double>(prodScore) * p.init_prod_scale;
    const double total = scaledProd + static_cast<double>(diversityBonus + portBonus + complementBonus - duplicatePenalty);
    return static_cast<int>(std::llround(total));
}

int road_degree_bonus(const Board::BoardState* board, NodeId settlementNodeId, EdgeId edgeId, const ParaParams& p) {
    if (settlementNodeId >= NODE_COUNT || edgeId == EdgeIdNone || edgeId >= EDGE_COUNT) return 0;
    const auto edge = board->edges[edgeId];
    const NodeId a = Board::Edge::unpackAdjacentNode(edge, 0);
    const NodeId b = Board::Edge::unpackAdjacentNode(edge, 1);
    const NodeId other = (a == settlementNodeId) ? b : a;
    if (other >= NODE_COUNT) return 0;
    const auto adj = Board::Node::getAdjacentEdges(board->nodes[other]);
    int deg = 0;
    for (auto e : adj) deg += (e != EdgeIdNone) ? 1 : 0;
    return static_cast<int>(std::llround(p.init_road_deg_bonus * static_cast<double>(deg)));
}

int edge_network_score(const Board::BoardState* board, PlayerId selfId, EdgeId edgeId) {
    if (edgeId == EdgeIdNone || edgeId >= EDGE_COUNT) return -10000;

    int s = 0;

    const NodeId n0 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 0);
    const NodeId n1 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 1);

    const auto score_node = [&](NodeId n) {
        if (n >= NODE_COUNT) return;
        const auto node = board->nodes[n];
        const auto owner = Board::Node::unpackOwner(node);
        const auto st = Board::Node::unpackStructure(node);

        if (owner == selfId && (st == StructureType::Settlement || st == StructureType::City)) {
            s += 200;
        }

        for (uint8_t i = 0; i < 3; ++i) {
            const EdgeId e = Board::Node::unpackAdjacentEdge(node, i);
            if (e == EdgeIdNone || e >= EDGE_COUNT) continue;
            if (Board::Edge::unpackHasRoad(board->edges[e]) && Board::Edge::unpackOwner(board->edges[e]) == selfId) {
                s += 120;
                break;
            }
        }
    };

    score_node(n0);
    score_node(n1);

    if (n0 < NODE_COUNT) {
        if (Board::Node::unpackPortType(board->nodes[n0]) != PortType::NoPort) s += 10;
    }
    if (n1 < NODE_COUNT) {
        if (Board::Node::unpackPortType(board->nodes[n1]) != PortType::NoPort) s += 10;
    }

    return s;
}

int road_action_score(const Board::BoardState* board, PlayerId selfId, EdgeId edgeId, const ParaParams& p) {
    if (edgeId == EdgeIdNone || edgeId >= EDGE_COUNT) return std::numeric_limits<int>::min();

    int s = edge_network_score(board, selfId, edgeId);

    const NodeId n0 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 0);
    const NodeId n1 = Board::Edge::unpackAdjacentNode(board->edges[edgeId], 1);
    for (NodeId n : {n0, n1}) {
        if (!node_distance_rule_ok(board, n)) continue;
        s += 3 * node_production_score(board, n, p);
    }

    return s;
}

double score_one_ply_action(Board::BoardState* board, PlayerId selfId, const ParaParams& p, Action::PackedAction a, double currentEval) {
    if (a == Action::getEmptyAction()) return currentEval;
    const auto t = Action::unpackType(a);
    double extra = 0.0;

    switch (t) {
        case ActionType::BuildCity: extra += p.bonus_build_city; break;
        case ActionType::BuildSettlement: extra += p.bonus_build_settlement; break;
        case ActionType::BuildRoad: extra += p.bonus_build_road; break;
        case ActionType::TradeBank: extra += p.bonus_trade_bank; break;
        case ActionType::EndTurn: extra += p.penalty_end_turn; break;
        default: break;
    }

    if (t == ActionType::BuyDevCard) {
        // BuyDevCard has RNG side-effects; score heuristically.
        return currentEval + p.bonus_buy_dev + extra;
    }

    board->applyAction(a);
    const double v = evaluate_position(board, selfId, p) + extra;
    board->undoLastAction();
    return v;
}

double score_action_with_lookahead(Board::BoardState* board, PlayerId selfId, const ParaParams& p, Action::PackedAction a, double baseEval) {
    if (a == Action::getEmptyAction()) return baseEval;
    const auto t = Action::unpackType(a);
    // No lookahead for EndTurn / BuyDevCard.
    if (t == ActionType::EndTurn || t == ActionType::BuyDevCard || p.policy_self_lookahead <= 1) {
        return score_one_ply_action(board, selfId, p, a, baseEval);
    }

    // First step.
    double extra1 = 0.0;
    switch (t) {
        case ActionType::BuildCity: extra1 += p.bonus_build_city; break;
        case ActionType::BuildSettlement: extra1 += p.bonus_build_settlement; break;
        case ActionType::BuildRoad: extra1 += p.bonus_build_road; break;
        case ActionType::TradeBank: extra1 += p.bonus_trade_bank; break;
        default: break;
    }

    board->applyAction(a);
    const double evalAfter = evaluate_position(board, selfId, p);
    const double firstScore = evalAfter + extra1;

    // Second step: best follow-up from the state after 'a'.
    double bestFollow = evalAfter;
    const auto next = board->getLegalActions(selfId);
    for (auto a2 : next) {
        bestFollow = std::max(bestFollow, score_one_ply_action(board, selfId, p, a2, evalAfter));
    }

    board->undoLastAction();

    return firstScore + p.policy_lookahead_gamma * (bestFollow - evalAfter);
}

BuyableType best_target_for_discard(const std::array<uint8_t, 5>& have) {
    auto priority_index = [](BuyableType b) {
        switch (b) {
            case BuyableType::City: return 0;
            case BuyableType::Settlement: return 1;
            case BuyableType::Road: return 2;
            case BuyableType::DevCard: return 3;
            default: return 99;
        }
    };

    BuyableType best = BuyableType::Road;
    uint16_t bestDef = std::numeric_limits<uint16_t>::max();

    for (BuyableType t : {BuyableType::City, BuyableType::Settlement, BuyableType::Road, BuyableType::DevCard}) {
        const auto need = cost_for(t);
        const auto d = deficit(have, need);
        if (d < bestDef || (d == bestDef && priority_index(t) < priority_index(best))) {
            bestDef = d;
            best = t;
        }
    }

    return best;
}

std::array<double, 5> keep_weights_for(BuyableType target, const ParaParams& p) {
    std::array<double, 5> w = {p.keep_brick, p.keep_lumber, p.keep_wool, p.keep_grain, p.keep_ore};

    auto add = [&](Resource r, double v) { w[static_cast<size_t>(r)] += v; };

    switch (target) {
        case BuyableType::City:
            add(Resource::Ore, p.keep_city_ore);
            add(Resource::Grain, p.keep_city_grain);
            break;
        case BuyableType::Settlement:
            add(Resource::Brick, p.keep_settle_brick);
            add(Resource::Lumber, p.keep_settle_lumber);
            add(Resource::Wool, p.keep_settle_wool);
            add(Resource::Grain, p.keep_settle_grain);
            break;
        case BuyableType::Road:
            add(Resource::Brick, p.keep_road_brick);
            add(Resource::Lumber, p.keep_road_lumber);
            break;
        case BuyableType::DevCard:
            add(Resource::Ore, p.keep_dev_ore);
            add(Resource::Grain, p.keep_dev_grain);
            add(Resource::Wool, p.keep_dev_wool);
            break;
        default:
            break;
    }

    return w;
}

Action::PackedAction greedy_pick_from_scores(const std::vector<std::pair<double, Action::PackedAction>>& scored, const ParaParams& p) {
    if (scored.empty()) return Action::getEmptyAction();

    // Epsilon random.
    const double r01 = RandomDevice::uniform_u32_range(0, 1000000) / 1000000.0;
    if (r01 < p.policy_epsilon) {
        const uint32_t idx = RandomDevice::uniform_u32_range(0, static_cast<uint32_t>(scored.size()) - 1);
        return scored[idx].second;
    }

    if (p.policy_temperature > 1e-9) {
        // Softmax sampling.
        double maxS = -1e300;
        for (const auto& it : scored) maxS = std::max(maxS, it.first);

        std::vector<double> w;
        w.reserve(scored.size());
        double sum = 0.0;
        for (const auto& it : scored) {
            const double z = (it.first - maxS) / p.policy_temperature;
            const double e = std::exp(std::max(-60.0, std::min(60.0, z)));
            w.push_back(e);
            sum += e;
        }
        if (sum <= 0.0) return scored[0].second;

        const double pick = RandomDevice::uniform_u32_range(0, 1000000) / 1000000.0 * sum;
        double acc = 0.0;
        for (size_t i = 0; i < scored.size(); ++i) {
            acc += w[i];
            if (pick <= acc) return scored[i].second;
        }
        return scored.back().second;
    }

    // Greedy / top-k random among best. (Does not require 'scored' to be sorted.)
    const int k = std::max(1, p.policy_top_k);

    if (k <= 1 || scored.size() == 1) {
        size_t bestIdx = 0;
        double best = scored[0].first;
        for (size_t i = 1; i < scored.size(); ++i) {
            if (scored[i].first > best) {
                best = scored[i].first;
                bestIdx = i;
            }
        }
        return scored[bestIdx].second;
    }

    const size_t kk = std::min(scored.size(), static_cast<size_t>(k));
    std::vector<size_t> idxs;
    idxs.reserve(scored.size());
    for (size_t i = 0; i < scored.size(); ++i) idxs.push_back(i);

    const auto better = [&](size_t a, size_t b) { return scored[a].first > scored[b].first; };
    if (kk < idxs.size()) {
        std::nth_element(idxs.begin(), idxs.begin() + kk, idxs.end(), better);
    }

    const uint32_t pick = RandomDevice::uniform_u32_range(0, static_cast<uint32_t>(kk) - 1);
    return scored[idxs[pick]].second;
}

const ParaParams& get_cached_params() {
    namespace fs = std::filesystem;

    static ParaParams params;
    static std::string cfgPath;
    static bool loadedOk = false;
    static bool haveMtime = false;
    static fs::file_time_type lastMtime{};

    // Resolve config path once (or when env override changes).
    const std::string resolved = resolve_config_path(cfgPath);
    if (resolved != cfgPath) {
        cfgPath = resolved;
        loadedOk = params.loadFromFile(cfgPath);
        haveMtime = false;
    }

    // If we failed to load previously, retry (respecting env override if set).
    if (!loadedOk) {
        cfgPath = resolve_config_path(std::string());
        loadedOk = params.loadFromFile(cfgPath);
        haveMtime = false;
    }

    // Hot path: reload only when the config file changes.
    // If last_write_time fails (missing file etc), keep the previously loaded params.
    if (!cfgPath.empty()) {
        try {
            const auto mt = fs::last_write_time(cfgPath);
            if (!haveMtime || mt != lastMtime) {
                (void)params.loadFromFile(cfgPath);
                lastMtime = mt;
                haveMtime = true;
                loadedOk = true;
            }
        } catch (...) {
            // ignore
        }
    }

    return params;
}

} // namespace

ParaPlayer::ParaPlayer() : IPlayer() {}

std::pair<Action::PackedAction, Action::PackedAction> ParaPlayer::getInitialPlacement() {
    const ParaParams& params = get_cached_params();

    const PlayerId selfId = boardState->currentPlayer;
    auto actions = boardState->generatePlaceInitialStructures(selfId);
    if (actions.empty()) {
        auto noAction = Action::getEmptyAction();
        return {noAction, noAction};
    }

    // Dedicated placement heuristic: generally stronger than generic eval delta at game start.
    const auto already = resources_covered_by_player(boardState, selfId);

    std::vector<std::pair<double, Action::PackedAction>> scored;
    scored.reserve(actions.size());

    for (auto a : actions) {
        const NodeId nodeId = Action::unpackArg1(a);
        const EdgeId edgeId = Action::unpackArg2(a);
        int s = score_initial_settlement_node(boardState, nodeId, false, already, params);
        s += road_degree_bonus(boardState, nodeId, edgeId, params);
        scored.push_back({static_cast<double>(s), a});
    }

    const auto pick = greedy_pick_from_scores(scored, params);
    return {pick, pick};
}

std::pair<Action::PackedAction, Action::PackedAction> ParaPlayer::get2InitialPlacement() {
    const ParaParams& params = get_cached_params();

    const PlayerId selfId = boardState->currentPlayer;
    auto actions = boardState->generatePlace2InitialStructures(selfId);
    if (actions.empty()) {
        auto noAction = Action::getEmptyAction();
        return {noAction, noAction};
    }

    const auto already = resources_covered_by_player(boardState, selfId);

    std::vector<std::pair<double, Action::PackedAction>> scored;
    scored.reserve(actions.size());

    for (auto a : actions) {
        const NodeId nodeId = Action::unpackArg1(a);
        const EdgeId edgeId = Action::unpackArg2(a);
        int s = score_initial_settlement_node(boardState, nodeId, true, already, params);
        s += road_degree_bonus(boardState, nodeId, edgeId, params);
        scored.push_back({static_cast<double>(s), a});
    }

    const auto pick = greedy_pick_from_scores(scored, params);
    return {pick, pick};
}

Action::PackedAction ParaPlayer::getDevAction() {
    const ParaParams& params = get_cached_params();

    const PlayerId selfId = boardState->currentPlayer;
    auto actions = boardState->generatePlayDevCardActions(selfId);
    actions.push_back(Action::getEmptyAction());

    std::vector<std::pair<double, Action::PackedAction>> scored;
    scored.reserve(actions.size());

    const double base = evaluate_position(boardState, selfId, params);

    for (auto a : actions) {
        if (a == Action::getEmptyAction()) {
            scored.push_back({base, a});
            continue;
        }
        boardState->applyAction(a);
        const double s = evaluate_position(boardState, selfId, params);
        boardState->undoLastAction();
        scored.push_back({s, a});
    }

    return greedy_pick_from_scores(scored, params);
}

Action::PackedAction ParaPlayer::getDiscardAction() {
    const ParaParams& params = get_cached_params();

    auto action = Action::getEmptyAction();

    const auto packed = boardState->packedPlayers[static_cast<uint8_t>(boardState->currentPlayer)];
    const uint8_t totalResources = Player::totalResources(packed);
    if (totalResources <= 9) return action;

    const uint8_t toDiscard = static_cast<uint8_t>(totalResources / 2);
    const auto have = unpack_resources(packed);

    const BuyableType target = best_target_for_discard(have);
    const auto keepW = keep_weights_for(target, params);

    std::vector<Resource> pool;
    pool.reserve(totalResources);
    for (Resource r : {Resource::Brick, Resource::Lumber, Resource::Wool, Resource::Grain, Resource::Ore}) {
        const uint8_t cnt = Player::unpackResource(packed, r);
        for (uint8_t i = 0; i < cnt; ++i) pool.push_back(r);
    }

    std::sort(pool.begin(), pool.end(), [&](Resource a, Resource b) {
        return keepW[static_cast<size_t>(a)] < keepW[static_cast<size_t>(b)];
    });

    for (uint8_t i = 0; i < toDiscard && i < pool.size(); ++i) {
        const auto r = pool[i];
        action = Action::packResource(action, r, Action::unpackResource(action, r) + 1);
    }

    return action;
}

Action::PackedAction ParaPlayer::getMoveRobber() {
    const ParaParams& params = get_cached_params();

    const PlayerId selfId = boardState->currentPlayer;
    const PlayerId enemyId = (selfId == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;

    auto actions = boardState->generateMoveRobberActions(selfId);
    if (actions.empty()) return Action::getEmptyAction();

    double best = -1e300;
    Action::PackedAction bestA = actions[0];

    for (auto a : actions) {
        const HexId robberPos = Action::unpackArg1(a);
        if (robberPos >= HEX_COUNT) continue;
        const auto hex = boardState->hexes[robberPos];

        const Resource r = Board::Hex::unpackResource(hex);
        const uint8_t pip = dice_pips(Board::Hex::unpackCatanNumber(hex));

        const auto selfVal = Board::Hex::unpackPlayerValue(hex, selfId);
        const auto enemyVal = Board::Hex::unpackPlayerValue(hex, enemyId);

        double s = 0.0;

        // New pips-weighted model.
        s += static_cast<double>(pip) * (params.robber_enemy_pips_value_w * static_cast<double>(enemyVal)
                                         - params.robber_self_pips_value_w * static_cast<double>(selfVal));
        if (enemyVal > 0 && selfVal == 0) s += params.robber_enemy_only_bonus;
        if (enemyVal == 0) s -= params.robber_no_enemy_penalty;
        if (r == Resource::NoResource) s -= params.robber_desert_penalty;

        // Keep legacy knobs as minor shaping so older configs still behave reasonably.
        s += params.robber_enemy_value_w * static_cast<double>(enemyVal);
        s += params.robber_self_value_w * static_cast<double>(selfVal);
        s += params.robber_pips_w * static_cast<double>(pip);

        if (s > best) {
            best = s;
            bestA = a;
        }
    }

    return bestA;
}

Action::PackedAction ParaPlayer::getTurnAction() {
    const ParaParams& params = get_cached_params();

    const PlayerId selfId = boardState->currentPlayer;

    auto actions = boardState->getLegalActions(selfId);
    if (actions.empty()) return Action::getEmptyAction();

    const double base = evaluate_position(boardState, selfId, params);

    // 1) If we can build a city/settlement, prefer the best of those by simulation.
    {
        Action::PackedAction bestBuild = Action::getEmptyAction();
        double bestBuildScore = -1e300;
        for (auto a : actions) {
            const auto t = Action::unpackType(a);
            if (t != ActionType::BuildCity && t != ActionType::BuildSettlement) continue;
            const double s = score_action_with_lookahead(boardState, selfId, params, a, base);
            if (s > bestBuildScore) {
                bestBuildScore = s;
                bestBuild = a;
            }
        }
        if (Action::unpackType(bestBuild) == ActionType::BuildCity || Action::unpackType(bestBuild) == ActionType::BuildSettlement) {
            return bestBuild;
        }
    }

    // 2) Score deterministic actions (roads/trades/end turn) using simulation + short lookahead.
    std::vector<std::pair<double, Action::PackedAction>> scored;
    scored.reserve(actions.size());

    Action::PackedAction devBuy = Action::getEmptyAction();
    bool hasRoad = false;
    bool hasTrade = false;
    bool canCityNow = false;
    bool canSettleNow = false;
    for (auto a : actions) {
        const auto t = Action::unpackType(a);
        if (t == ActionType::BuyDevCard) devBuy = a;
        if (t == ActionType::BuildRoad) hasRoad = true;
        if (t == ActionType::TradeBank) hasTrade = true;
        if (t == ActionType::BuildCity) canCityNow = true;
        if (t == ActionType::BuildSettlement) canSettleNow = true;
    }

    for (auto a : actions) {
        const auto t = Action::unpackType(a);
        if (t != ActionType::BuildRoad && t != ActionType::TradeBank && t != ActionType::EndTurn) continue;

        // Cheap score (1-ply simulation). We'll optionally apply expensive lookahead only to top-M.
        double s = score_one_ply_action(boardState, selfId, params, a, base);
        if (t == ActionType::BuildRoad && std::abs(params.road_heuristic_w) > 1e-9) {
            const EdgeId e = Action::unpackArg1(a);
            s += params.road_heuristic_w * static_cast<double>(road_action_score(boardState, selfId, e, params));
        }
        scored.push_back({s, a});
    }

    // Optional: apply expensive lookahead only to top-M actions by the cheap score.
    if (params.policy_self_lookahead > 1 && params.policy_lookahead_top_m > 0 && scored.size() > 1) {
        std::vector<size_t> idxs;
        idxs.reserve(scored.size());
        for (size_t i = 0; i < scored.size(); ++i) {
            const auto t = Action::unpackType(scored[i].second);
            if (t == ActionType::BuildRoad || t == ActionType::TradeBank || t == ActionType::EndTurn) {
                idxs.push_back(i);
            }
        }

        const size_t M = std::min(idxs.size(), static_cast<size_t>(std::max(0, params.policy_lookahead_top_m)));
        if (M > 0 && M < idxs.size()) {
            std::nth_element(
                idxs.begin(),
                idxs.begin() + M,
                idxs.end(),
                [&](size_t a, size_t b) { return scored[a].first > scored[b].first; }
            );
            idxs.resize(M);
        }

        for (size_t idx : idxs) {
            const auto a = scored[idx].second;
            const auto t = Action::unpackType(a);
            double s = score_action_with_lookahead(boardState, selfId, params, a, base);
            if (t == ActionType::BuildRoad && std::abs(params.road_heuristic_w) > 1e-9) {
                const EdgeId e = Action::unpackArg1(a);
                s += params.road_heuristic_w * static_cast<double>(road_action_score(boardState, selfId, e, params));
            }
            scored[idx].first = s;
        }
    }

    // 3) Dev-buy heuristic (RNG action): prefer when it doesn't block a deterministic build.
    if (Action::unpackType(devBuy) == ActionType::BuyDevCard) {
        const PlayerId enemyId = (selfId == PlayerId::Player0) ? PlayerId::Player1 : PlayerId::Player0;
        const int selfVP = effective_vp(boardState, selfId);
        const int enemyVP = effective_vp(boardState, enemyId);

        double devScore = base + params.bonus_buy_dev;
        if (enemyVP > selfVP) devScore += params.dev_buy_behind_bonus;
        if (selfVP >= 8) devScore -= params.dev_buy_late_penalty;
        if (hasTrade) devScore -= params.dev_buy_has_trade_penalty;
        if (hasRoad) devScore -= params.dev_buy_has_road_penalty;
        if (canCityNow) devScore -= params.dev_buy_blocks_city_penalty;
        if (canSettleNow) devScore -= params.dev_buy_blocks_settlement_penalty;

        scored.push_back({devScore, devBuy});
    }

    if (scored.empty()) return Action::getEmptyAction();

    const auto pick = greedy_pick_from_scores(scored, params);

    // Avoid ending the turn if it makes the position worse.
    if (Action::unpackType(pick) == ActionType::EndTurn) {
        Action::PackedAction bestAlt = Action::getEmptyAction();
        double bestAltScore = -1e300;
        for (const auto& it : scored) {
            if (Action::unpackType(it.second) == ActionType::EndTurn) continue;
            if (it.first > bestAltScore) {
                bestAltScore = it.first;
                bestAlt = it.second;
            }
        }
        if (bestAlt != Action::getEmptyAction() && bestAltScore + 1e-9 >= base) return bestAlt;
    }

    return pick;
}
