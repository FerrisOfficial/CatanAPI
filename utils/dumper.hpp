#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>

#include "game_simulation/actions.hpp"
#include "game_simulation/board.hpp"
#include "game_simulation/consts.hpp"

class Dumper {
public:
    explicit Dumper(std::filesystem::path logsDir = "logs");
    ~Dumper();

    Dumper(const Dumper&) = delete;
    Dumper& operator=(const Dumper&) = delete;

    const std::string& filePath() const;

    void recordStart();
    void recordTurnStart(const Board::BoardState& state);
    void recordTurnEnd(const Board::BoardState& state);
    void recordInitialState(const Board::BoardState& state);
    void recordDiceRoll(uint8_t diceNumber, const Board::BoardState& state);
    void recordActionApplied(Action::PackedAction action, const Board::BoardState& state, const std::string& phase);
    void recordGameEnd(PlayerId winner, const Board::BoardState& state);

private:
    std::ofstream out_;
    std::string outPath_;
    uint64_t seq_ = 0;

    static std::string escapeJson(std::string_view s);
    static std::string nowIso8601Local();
    static std::string timestampForFilenameLocal();

    static std::string actionToJson(Action::PackedAction action);
    static std::string stateToJson(const Board::BoardState& state);

    uint64_t nextSeq();
    void writeJsonLine(const std::string& jsonLine);
};
