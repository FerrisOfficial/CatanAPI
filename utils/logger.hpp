
#pragma once
#include <string>
#include <fstream>
#include <iostream>

class Logger {
public:
#ifdef LOGGER
    Logger() : log_file("logs/log.txt", std::ios::app) {
        if (!log_file.is_open()) {
            std::cerr << "Failed to open log file!" << std::endl;
        }
    }
    ~Logger() {
        if (log_file.is_open()) {
            log_file.close();
        }
    }

    void clean_log() {
        log_file.close();
        log_file.open("logs/log.txt", std::ios::trunc);
    }

    void log(const std::string& message, const std::string& arg1 = "", const std::string& arg2 = "") {
        if (log_file.is_open()) {
            log_file << message << arg1 << arg2 << std::endl;
        }
    }

    // Log info about both players from BoardState
    void log_players(const Board::BoardState& board) {
        if (!log_file.is_open()) return;
        for (int i = 0; i < 2; ++i) {
            const auto& p = board.packedPlayers[i];
            log_file << "\tPlayer " << i << ": ";
            log_file << "\tVP=" << (int)Player::unpackVictoryPoints(p) << ", ";
            log_file << "\tResources=" << (int)Player::totalResources(p) << ", ";
            log_file << "\tKnights=" << (int)Player::unpackUsedKnights(p) << ", ";
            log_file << "\tLongestRoad=" << (int)Player::unpackLongestRoadLength(p) << ", ";
            log_file << "\tLongestRoadFlag=" << Player::unpackLongestRoadFlag(p) << ", ";
            log_file << "\tLargestArmyFlag=" << Player::unpackLargestArmyFlag(p) << std::endl;
        }
    }

    void error(const std::string& message) {
        if (log_file.is_open()) {
            log_file << "[ERROR] " << message << std::endl;
            std::cerr << "[ERROR] " << message << std::endl;
        }
    }

private:
    std::ofstream log_file;
#else
    void log(const std::string& message, const std::string& arg1 = "", const std::string& arg2 = "") {}
    void clean_log() {}
    void log_players(const Board::BoardState&) {}
    void error(const std::string& message) {}
#endif
};
