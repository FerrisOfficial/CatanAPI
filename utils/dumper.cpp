#include "utils/dumper.hpp"

#include <array>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "game_simulation/player.hpp"
#include "game_simulation/packedBank.hpp"

namespace {

const char* playerIdName(PlayerId p) {
    switch (p) {
        case PlayerId::Player0: return "Player0";
        case PlayerId::Player1: return "Player1";
        case PlayerId::NoPlayer: return "NoPlayer";
    }
    return "UnknownPlayer";
}

const char* resourceName(Resource r) {
    switch (r) {
        case Resource::Brick: return "Brick";
        case Resource::Lumber: return "Lumber";
        case Resource::Wool: return "Wool";
        case Resource::Grain: return "Grain";
        case Resource::Ore: return "Ore";
        case Resource::NoResource: return "NoResource";
    }
    return "UnknownResource";
}

const char* devTypeName(DevType d) {
    switch (d) {
        case DevType::Knight: return "Knight";
        case DevType::RoadBuilding: return "RoadBuilding";
        case DevType::YearOfPlenty: return "YearOfPlenty";
        case DevType::Monopoly: return "Monopoly";
        case DevType::VictoryPoint: return "VictoryPoint";
        case DevType::NoDev: return "NoDev";
    }
    return "UnknownDev";
}

const char* structureTypeName(StructureType s) {
    switch (s) {
        case StructureType::Road: return "Road";
        case StructureType::Settlement: return "Settlement";
        case StructureType::City: return "City";
        case StructureType::NoStructure: return "NoStructure";
    }
    return "UnknownStructure";
}

const char* portTypeName(PortType p) {
    switch (p) {
        case PortType::ThreeForOne: return "ThreeForOne";
        case PortType::BrickPort: return "BrickPort";
        case PortType::LumberPort: return "LumberPort";
        case PortType::WoolPort: return "WoolPort";
        case PortType::GrainPort: return "GrainPort";
        case PortType::OrePort: return "OrePort";
        case PortType::NoPort: return "NoPort";
    }
    return "UnknownPort";
}

template <typename T>
std::string jsonArrayFromSpan(const T* data, size_t count) {
    std::ostringstream oss;
    oss << '[';
    for (size_t i = 0; i < count; ++i) {
        if (i) oss << ',';
        oss << static_cast<unsigned long long>(data[i]);
    }
    oss << ']';
    return oss.str();
}

} // namespace

Dumper::Dumper(std::filesystem::path logsDir) {
    std::error_code ec;
    std::filesystem::create_directories(logsDir, ec);

    auto filename = std::string("game_") + timestampForFilenameLocal() + ".jsonl";
    auto outPath = logsDir / filename;

    outPath_ = outPath.string();
    out_.open(outPath_, std::ios::out | std::ios::binary);
    if (!out_.is_open()) {
        throw std::runtime_error("Dumper: failed to open log file: " + outPath_);
    }

    recordStart();
}

Dumper::~Dumper() {
    if (out_.is_open()) {
        out_.flush();
        out_.close();
    }
}

const std::string& Dumper::filePath() const {
    return outPath_;
}

void Dumper::recordStart() {
    std::ostringstream oss;
    oss << "{\"seq\":" << nextSeq()
        << ",\"event\":\"start\",\"format_version\":1,\"timestamp\":\"" << escapeJson(nowIso8601Local())
        << "\",\"file\":\"" << escapeJson(outPath_) << "\"}";
    writeJsonLine(oss.str());
}

void Dumper::recordTurnStart(const Board::BoardState& state) {
    std::ostringstream oss;
    oss << "{\"seq\":" << nextSeq()
        << ",\"event\":\"turn_start\",\"timestamp\":\"" << escapeJson(nowIso8601Local())
        << "\",\"turn\":" << static_cast<unsigned>(state.currentTurn)
        << ",\"current_player\":" << static_cast<unsigned>(state.currentPlayer)
        << ",\"state\":" << stateToJson(state)
        << '}';
    writeJsonLine(oss.str());
}

void Dumper::recordTurnEnd(const Board::BoardState& state) {
    std::ostringstream oss;
    oss << "{\"seq\":" << nextSeq()
        << ",\"event\":\"turn_end\",\"timestamp\":\"" << escapeJson(nowIso8601Local())
        << "\",\"turn\":" << static_cast<unsigned>(state.currentTurn)
        << ",\"current_player\":" << static_cast<unsigned>(state.currentPlayer)
        << ",\"state\":" << stateToJson(state)
        << '}';
    writeJsonLine(oss.str());
}

void Dumper::recordInitialState(const Board::BoardState& state) {
    std::ostringstream oss;
    oss << "{\"seq\":" << nextSeq()
        << ",\"event\":\"initial_state\",\"timestamp\":\"" << escapeJson(nowIso8601Local())
        << "\",\"state\":" << stateToJson(state) << '}';
    writeJsonLine(oss.str());
}

void Dumper::recordDiceRoll(uint8_t diceNumber, const Board::BoardState& state) {
    std::ostringstream oss;
    oss << "{\"seq\":" << nextSeq()
        << ",\"event\":\"dice\",\"timestamp\":\"" << escapeJson(nowIso8601Local())
        << "\",\"dice\":" << static_cast<unsigned>(diceNumber)
        << ",\"turn\":" << static_cast<unsigned>(state.currentTurn)
        << ",\"current_player\":" << static_cast<unsigned>(state.currentPlayer)
        << ",\"state\":" << stateToJson(state)
        << '}';
    writeJsonLine(oss.str());
}

void Dumper::recordActionApplied(Action::PackedAction action, const Board::BoardState& state, const std::string& phase) {
    std::ostringstream oss;
    oss << "{\"seq\":" << nextSeq()
        << ",\"event\":\"action_applied\",\"timestamp\":\"" << escapeJson(nowIso8601Local())
        << "\",\"phase\":\"" << escapeJson(phase) << "\",\"action\":" << actionToJson(action)
        << ",\"state\":" << stateToJson(state) << '}';
    writeJsonLine(oss.str());
}

void Dumper::recordGameEnd(PlayerId winner, const Board::BoardState& state) {
    std::ostringstream oss;
    oss << "{\"seq\":" << nextSeq()
        << ",\"event\":\"game_end\",\"timestamp\":\"" << escapeJson(nowIso8601Local())
        << "\",\"winner\":" << static_cast<unsigned>(winner)
        << ",\"state\":" << stateToJson(state) << '}';
    writeJsonLine(oss.str());
}

uint64_t Dumper::nextSeq() {
    return seq_++;
}

std::string Dumper::escapeJson(std::string_view s) {
    std::string out;
    out.reserve(s.size());

    for (char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    std::ostringstream oss;
                    oss << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                        << static_cast<int>(static_cast<unsigned char>(c));
                    out += oss.str();
                } else {
                    out += c;
                }
        }
    }

    return out;
}

std::string Dumper::nowIso8601Local() {
    using clock = std::chrono::system_clock;
    auto now = clock::now();
    auto tt = clock::to_time_t(now);

    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S") << '.' << std::setw(3) << std::setfill('0') << ms.count();
    return oss.str();
}

std::string Dumper::timestampForFilenameLocal() {
    using clock = std::chrono::system_clock;
    auto now = clock::now();
    auto tt = clock::to_time_t(now);

    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &tt);
#else
    localtime_r(&tt, &tm);
#endif

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S") << '_' << std::setw(3) << std::setfill('0') << ms.count();
    return oss.str();
}

std::string Dumper::actionToJson(Action::PackedAction action) {
    std::ostringstream oss;

    const auto type = Action::unpackType(action);
    const auto player = Action::unpackPlayerID(action);

    oss << '{'
        << "\"raw\":" << static_cast<unsigned long long>(action)
        << ",\"type\":" << static_cast<unsigned>(type)
        << ",\"type_name\":\"" << escapeJson(actionTypeName(type)) << "\""
        << ",\"player\":" << static_cast<unsigned>(player)
        << ",\"arg1\":" << static_cast<unsigned>(Action::unpackArg1(action))
        << ",\"arg2\":" << static_cast<unsigned>(Action::unpackArg2(action))
        << ",\"arg3\":" << static_cast<unsigned>(Action::unpackArg3(action))
        << ",\"resources\":{"
        << "\"brick\":" << static_cast<unsigned>(Action::unpackResource(action, Resource::Brick))
        << ",\"lumber\":" << static_cast<unsigned>(Action::unpackResource(action, Resource::Lumber))
        << ",\"wool\":" << static_cast<unsigned>(Action::unpackResource(action, Resource::Wool))
        << ",\"grain\":" << static_cast<unsigned>(Action::unpackResource(action, Resource::Grain))
        << ",\"ore\":" << static_cast<unsigned>(Action::unpackResource(action, Resource::Ore))
        << "}}";

    return oss.str();
}

std::string Dumper::stateToJson(const Board::BoardState& state) {
    std::ostringstream oss;

    oss << '{'
        << "\"robber\":" << static_cast<unsigned>(state.robberPosition)
        << ",\"current_player\":" << static_cast<unsigned>(state.currentPlayer)
        << ",\"current_player_name\":\"" << escapeJson(playerIdName(state.currentPlayer)) << "\""
        << ",\"turn\":" << static_cast<unsigned>(state.currentTurn)

        << ",\"packed_players\":["
        << static_cast<unsigned long long>(state.packedPlayers[0]) << ','
        << static_cast<unsigned long long>(state.packedPlayers[1]) << ']'
        << ",\"players\":[";

    for (int i = 0; i < 2; ++i) {
        if (i) oss << ',';
        const auto p = state.packedPlayers[i];
        oss << '{'
            << "\"id\":" << i
            << ",\"name\":\"" << escapeJson(i == 0 ? "Player0" : "Player1") << "\""
            << ",\"packed\":" << static_cast<unsigned long long>(p)
            << ",\"victory_points\":" << static_cast<unsigned>(Player::unpackVictoryPoints(p))
            << ",\"used_knights\":" << static_cast<unsigned>(Player::unpackUsedKnights(p))
            << ",\"longest_road_length\":" << static_cast<unsigned>(Player::unpackLongestRoadLength(p))
            << ",\"longest_road_flag\":" << (Player::unpackLongestRoadFlag(p) ? "true" : "false")
            << ",\"largest_army_flag\":" << (Player::unpackLargestArmyFlag(p) ? "true" : "false")
            << ",\"resources\":{"
            << "\"brick\":" << static_cast<unsigned>(Player::unpackResource(p, Resource::Brick))
            << ",\"lumber\":" << static_cast<unsigned>(Player::unpackResource(p, Resource::Lumber))
            << ",\"wool\":" << static_cast<unsigned>(Player::unpackResource(p, Resource::Wool))
            << ",\"grain\":" << static_cast<unsigned>(Player::unpackResource(p, Resource::Grain))
            << ",\"ore\":" << static_cast<unsigned>(Player::unpackResource(p, Resource::Ore))
            << ",\"total\":" << static_cast<unsigned>(Player::totalResources(p))
            << "}"
            << ",\"dev_cards\":{"
            << "\"knight\":" << static_cast<unsigned>(Player::unpackDevCard(p, DevType::Knight))
            << ",\"road_building\":" << static_cast<unsigned>(Player::unpackDevCard(p, DevType::RoadBuilding))
            << ",\"year_of_plenty\":" << static_cast<unsigned>(Player::unpackDevCard(p, DevType::YearOfPlenty))
            << ",\"monopoly\":" << static_cast<unsigned>(Player::unpackDevCard(p, DevType::Monopoly))
            << ",\"victory_point\":" << static_cast<unsigned>(Player::unpackDevCard(p, DevType::VictoryPoint))
            << ",\"total\":" << static_cast<unsigned>(Player::totalDevCards(p))
            << "}"
            << ",\"available_structures\":{"
            << "\"roads\":" << static_cast<unsigned>(Player::unpackAvailableStructures(p, StructureType::Road))
            << ",\"settlements\":" << static_cast<unsigned>(Player::unpackAvailableStructures(p, StructureType::Settlement))
            << ",\"cities\":" << static_cast<unsigned>(Player::unpackAvailableStructures(p, StructureType::City))
            << "}"
            << '}';
    }
    oss << ']';

    const auto pb = state.packedBank;
    oss << ",\"packed_bank\":" << static_cast<unsigned long long>(pb)
        << ",\"bank\":{"
        << "\"resources\":{"
        << "\"brick\":" << static_cast<unsigned>(Bank::unpackResource(pb, Resource::Brick))
        << ",\"lumber\":" << static_cast<unsigned>(Bank::unpackResource(pb, Resource::Lumber))
        << ",\"wool\":" << static_cast<unsigned>(Bank::unpackResource(pb, Resource::Wool))
        << ",\"grain\":" << static_cast<unsigned>(Bank::unpackResource(pb, Resource::Grain))
        << ",\"ore\":" << static_cast<unsigned>(Bank::unpackResource(pb, Resource::Ore))
        << "}"
        << ",\"dev_cards\":{"
        << "\"knight\":" << static_cast<unsigned>(Bank::unpackDevCard(pb, DevType::Knight))
        << ",\"road_building\":" << static_cast<unsigned>(Bank::unpackDevCard(pb, DevType::RoadBuilding))
        << ",\"year_of_plenty\":" << static_cast<unsigned>(Bank::unpackDevCard(pb, DevType::YearOfPlenty))
        << ",\"monopoly\":" << static_cast<unsigned>(Bank::unpackDevCard(pb, DevType::Monopoly))
        << ",\"victory_point\":" << static_cast<unsigned>(Bank::unpackDevCard(pb, DevType::VictoryPoint))
        << ",\"total_cached\":" << static_cast<unsigned>(Bank::unpackTotalDevCount(pb))
        << ",\"total_computed\":" << static_cast<unsigned>(Bank::computeTotalDevCards(pb))
        << "}"
        << '}';

    oss << ",\"hexes_packed\":" << jsonArrayFromSpan(state.hexes, HEX_COUNT)
        << ",\"hexes\":[";
    for (int i = 0; i < HEX_COUNT; ++i) {
        if (i) oss << ',';
        const auto h = state.hexes[i];
        const auto res = Board::Hex::unpackResource(h);
        oss << '{'
            << "\"id\":" << i
            << ",\"packed\":" << static_cast<unsigned>(h)
            << ",\"catan_number\":" << static_cast<unsigned>(Board::Hex::unpackCatanNumber(h))
            << ",\"resource\":" << static_cast<unsigned>(res)
            << ",\"resource_name\":\"" << escapeJson(resourceName(res)) << "\""
            << ",\"player_values\":{"
            << "\"player0\":" << static_cast<unsigned>(Board::Hex::unpackPlayerValue(h, PlayerId::Player0))
            << ",\"player1\":" << static_cast<unsigned>(Board::Hex::unpackPlayerValue(h, PlayerId::Player1))
            << "}"
            << '}';
    }
    oss << ']';

    oss << ",\"nodes_packed\":" << jsonArrayFromSpan(state.nodes, NODE_COUNT)
        << ",\"nodes\":[";
    for (int i = 0; i < NODE_COUNT; ++i) {
        if (i) oss << ',';
        const auto n = state.nodes[i];
        const auto st = Board::Node::unpackStructure(n);
        const auto owner = Board::Node::unpackOwner(n);
        const auto port = Board::Node::unpackPortType(n);
        oss << '{'
            << "\"id\":" << i
            << ",\"packed\":" << static_cast<unsigned long long>(n)
            << ",\"structure\":" << static_cast<unsigned>(st)
            << ",\"structure_name\":\"" << escapeJson(structureTypeName(st)) << "\""
            << ",\"owner\":" << static_cast<unsigned>(owner)
            << ",\"owner_name\":\"" << escapeJson(playerIdName(owner)) << "\""
            << ",\"port\":" << static_cast<unsigned>(port)
            << ",\"port_name\":\"" << escapeJson(portTypeName(port)) << "\""
            << ",\"adjacent_hexes\":[";
        for (int j = 0; j < 3; ++j) {
            if (j) oss << ',';
            const auto hid = Board::Node::unpackAdjacentHex(n, static_cast<uint8_t>(j));
            if (hid == HexIdNone) {
                oss << "null";
            } else {
                oss << static_cast<unsigned>(hid);
            }
        }
        oss << "],\"adjacent_edges\":[";
        for (int j = 0; j < 3; ++j) {
            if (j) oss << ',';
            const auto eid = Board::Node::unpackAdjacentEdge(n, static_cast<uint8_t>(j));
            if (eid == EdgeIdNone) {
                oss << "null";
            } else {
                oss << static_cast<unsigned>(eid);
            }
        }
        oss << "]}";
    }
    oss << ']';

    oss << ",\"edges_packed\":" << jsonArrayFromSpan(state.edges, EDGE_COUNT)
        << ",\"edges\":[";
    for (int i = 0; i < EDGE_COUNT; ++i) {
        if (i) oss << ',';
        const auto e = state.edges[i];
        const auto owner = Board::Edge::unpackOwner(e);
        const auto n0 = Board::Edge::unpackAdjacentNode(e, 0);
        const auto n1 = Board::Edge::unpackAdjacentNode(e, 1);
        oss << '{'
            << "\"id\":" << i
            << ",\"packed\":" << static_cast<unsigned>(e)
            << ",\"has_road\":" << (Board::Edge::unpackHasRoad(e) ? "true" : "false")
            << ",\"owner\":" << static_cast<unsigned>(owner)
            << ",\"owner_name\":\"" << escapeJson(playerIdName(owner)) << "\""
            << ",\"adjacent_nodes\":[";
        oss << static_cast<unsigned>(n0) << ',' << static_cast<unsigned>(n1);
        oss << "]}";
    }
    oss << ']';

    oss << '}';

    return oss.str();
}

void Dumper::writeJsonLine(const std::string& jsonLine) {
    if (!out_.is_open()) return;
    out_ << jsonLine << '\n';
    out_.flush();
}
