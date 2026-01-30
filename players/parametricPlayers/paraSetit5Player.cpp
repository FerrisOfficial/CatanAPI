#include "paraSetit5Player.hpp"

#include "game_simulation/board.hpp"
#include "utils/randomDevice.hpp"
#include "playerHelpers.hpp"

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

using PlayerHelpers::dice_pips;

constexpr const char* PARA_SETIT5_CFG_ENV = "CATAN_PARA_SETIT5_CFG";

struct ParaSettleParams {
    // policy
    double policy_temperature = 0.0; // 0 = greedy, >0 = softmax
    double policy_epsilon = 0.02;    // random pick
    int policy_top_k = 1;

    // production scoring
    double res_w_brick = 12.0;
    double res_w_lumber = 12.0;
    double res_w_wool = 11.0;
    double res_w_grain = 13.0;
    double res_w_ore = 14.0;
    double port_three_for_one = 35.0;
    double port_two_for_one = 70.0;

    // initial placement
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
        // policy.top_k handled manually (int).

        bind("prod.res_brick", res_w_brick);
        bind("prod.res_lumber", res_w_lumber);
        bind("prod.res_wool", res_w_wool);
        bind("prod.res_grain", res_w_grain);
        bind("prod.res_ore", res_w_ore);
        bind("prod.port_three_for_one", port_three_for_one);
        bind("prod.port_two_for_one", port_two_for_one);

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

        std::string line;
        while (std::getline(in, line)) {
            line = trim(line);
            if (line.empty()) continue;
            if (line[0] == '#') continue;
            if (line.size() >= 2 && line[0] == '/' && line[1] == '/') continue;
            const auto eq = line.find('=');
            if (eq == std::string::npos) continue;
            const std::string key = trim(line.substr(0, eq));
            const std::string val = trim(line.substr(eq + 1));

            if (key == "policy.top_k") {
                try {
                    const int iv = std::stoi(val);
                    policy_top_k = std::max(1, iv);
                } catch (...) {
                    // ignore
                }
                continue;
            }

            auto it = m.find(key);
            if (it == m.end()) continue;
            try {
                *(it->second) = std::stod(val);
            } catch (...) {
                // ignore
            }
        }

        // sanitize
        if (policy_top_k < 1) policy_top_k = 1;
        if (policy_epsilon < 0.0) policy_epsilon = 0.0;
        if (policy_epsilon > 1.0) policy_epsilon = 1.0;
        if (policy_temperature < 0.0) policy_temperature = 0.0;

        return true;
    }
};

std::string find_cfg_upwards(const std::filesystem::path& start, const std::filesystem::path& rel) {
    namespace fs = std::filesystem;

    fs::path p = start;
    for (int i = 0; i < 8; ++i) {
        const fs::path cand = p / rel;
        std::error_code ec;
        if (fs::exists(cand, ec) && fs::is_regular_file(cand, ec)) {
            return cand.string();
        }
        if (!p.has_parent_path()) break;
        const fs::path parent = p.parent_path();
        if (parent == p) break;
        p = parent;
    }
    return std::string();
}

std::string resolve_config_path() {
    namespace fs = std::filesystem;

    if (const char* env = std::getenv(PARA_SETIT5_CFG_ENV)) {
        const std::string v(env);
        if (!v.empty()) return v;
    }

    const fs::path rel = fs::path("players") / "paraSetit5Player.cfg";
    const fs::path cwd = fs::current_path();
    std::string found = find_cfg_upwards(cwd, rel);
    if (!found.empty()) return found;

    // Fallback: relative.
    return rel.string();
}

const ParaSettleParams& get_cached_params() {
    namespace fs = std::filesystem;

    static ParaSettleParams params;
    static std::string cfgPath;
    static bool loadedOk = false;
    static bool haveMtime = false;
    static fs::file_time_type lastMtime{};

    const std::string resolved = resolve_config_path();
    if (resolved != cfgPath) {
        cfgPath = resolved;
        loadedOk = params.loadFromFile(cfgPath);
        haveMtime = false;
    }

    if (!loadedOk && !cfgPath.empty()) {
        loadedOk = params.loadFromFile(cfgPath);
        haveMtime = false;
    }

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

int node_production_score(const Board::BoardState* board, NodeId nodeId, const ParaSettleParams& p) {
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
        const uint8_t pip = PlayerHelpers::dice_pips(Board::Hex::unpackCatanNumber(hex));
        s += static_cast<double>(pip) * res_weight(r);
    }

    const auto pt = Board::Node::unpackPortType(node);
    if (pt != PortType::NoPort) {
        if (pt == PortType::ThreeForOne) s += p.port_three_for_one;
        else s += p.port_two_for_one;
    }

    return static_cast<int>(std::llround(s));
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
                                 const std::array<bool, 5>& alreadyHave, const ParaSettleParams& p) {
    if (nodeId >= NODE_COUNT) return std::numeric_limits<int>::min();
    const auto node = board->nodes[nodeId];

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

    int unique = 0;
    int duplicatePenalty = 0;
    for (size_t i = 0; i < 5; ++i) {
        unique += (resourceCounts[i] > 0) ? 1 : 0;
        if (resourceCounts[i] > 1) duplicatePenalty += static_cast<int>(p.init_duplicate_penalty) * static_cast<int>(resourceCounts[i] - 1);
    }
    const int diversityBonus = static_cast<int>(p.init_diversity_bonus) * unique;

    int portBonus = 0;
    const auto port = Board::Node::unpackPortType(node);
    if (port != PortType::NoPort) {
        portBonus += static_cast<int>(secondPlacement ? p.init_port_second : p.init_port_first);
        if (port == PortType::ThreeForOne) portBonus += static_cast<int>(p.init_port_three_for_one_bonus);
        else portBonus += static_cast<int>(p.init_port_resource_bonus);
    }

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

    const int prodScore = node_production_score(board, nodeId, p);
    if (prodScore == std::numeric_limits<int>::min()) return prodScore;

    const double scaledProd = static_cast<double>(prodScore) * p.init_prod_scale;
    const double total = scaledProd + static_cast<double>(diversityBonus + portBonus + complementBonus - duplicatePenalty);
    return static_cast<int>(std::llround(total));
}

int road_degree_bonus(const Board::BoardState* board, NodeId settlementNodeId, EdgeId edgeId, const ParaSettleParams& p) {
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

Action::PackedAction greedy_pick_from_scores(const std::vector<std::pair<double, Action::PackedAction>>& scored, const ParaSettleParams& p) {
    if (scored.empty()) return Action::getEmptyAction();

    const double r01 = RandomDevice::uniform_u32_range(0, 1000000) / 1000000.0;
    if (r01 < p.policy_epsilon) {
        const uint32_t idx = RandomDevice::uniform_u32_range(0, static_cast<uint32_t>(scored.size()) - 1);
        return scored[idx].second;
    }

    if (p.policy_temperature > 1e-9) {
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

} // namespace

std::pair<Action::PackedAction, Action::PackedAction> ParaSettleIt5Player::getInitialPlacement() {
    const ParaSettleParams& params = get_cached_params();

    const PlayerId selfId = boardState->currentPlayer;
    auto actions = boardState->generatePlaceInitialStructures(selfId);
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
        int s = score_initial_settlement_node(boardState, nodeId, false, already, params);
        s += road_degree_bonus(boardState, nodeId, edgeId, params);
        scored.push_back({static_cast<double>(s), a});
    }

    const auto pick = greedy_pick_from_scores(scored, params);

    // Keep state for second placement scoring.
    firstPlacementResources = resources_at_node(boardState, Action::unpackArg1(pick));

    return {pick, pick};
}

std::pair<Action::PackedAction, Action::PackedAction> ParaSettleIt5Player::get2InitialPlacement() {
    const ParaSettleParams& params = get_cached_params();

    const PlayerId selfId = boardState->currentPlayer;
    auto actions = boardState->generatePlace2InitialStructures(selfId);
    if (actions.empty()) {
        auto noAction = Action::getEmptyAction();
        return {noAction, noAction};
    }

    // Use already-covered resources from current board state; fall back to local remembered first placement.
    // (This keeps it safe if the simulation sequence changes.)
    auto already = resources_covered_by_player(boardState, selfId);
    for (size_t i = 0; i < 5; ++i) {
        already[i] = already[i] || firstPlacementResources[i];
    }

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
