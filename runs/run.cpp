#include "game_simulation/game.hpp"
#include "players/randomPlayer.hpp"
#include "players/greedyPlayer.hpp"
#include "players/settlePrioGreedyPlayer.hpp"

#include <iostream>
#include <memory>
#include <string>

namespace {
std::unique_ptr<IPlayer> make_player_from_flag(const std::string& flag) {
    // Player flags (extensible):
    // - rp : RandomPlayer
    if (flag == "rp") {
        return std::make_unique<RandomPlayer>();
    }
    if (flag == "gp") {
        return std::make_unique<GreedyPlayer>();
    }
    if (flag == "sp") {
        return std::make_unique<SettlePrioGreedyPlayer>();
    }

    return nullptr;
}

std::string display_name_from_flag(const std::string& flag) {
    if (flag == "rp") return "RandomPlayer";
    if (flag == "gp") return "GreedyPlayer";
    if (flag == "sp") return "SettlePrioGreedyPlayer";
    return flag;
}

void print_usage(const char* exe) {
    std::cerr << "Usage: " << exe << " <player0_flag> <player1_flag>\n"
              << "  Currently supported flags:\n"
              << "    rp  RandomPlayer\n"
              << "    gp  GreedyPlayer\n"
              << "    sp  SettlePrioGreedyPlayer\n";
}
} // namespace

int main(int argc, char** argv) {
    if (argc != 3) {
        print_usage(argv[0]);
        return 2;
    }

    const std::string p0_flag = argv[1];
    const std::string p1_flag = argv[2];

    auto player0 = make_player_from_flag(p0_flag);
    auto player1 = make_player_from_flag(p1_flag);

    if (!player0 || !player1) {
        std::cerr << "Unknown player flag(s): '" << p0_flag << "', '" << p1_flag << "'\n";
        print_usage(argv[0]);
        return 2;
    }

    Game game(*player0, *player1);
    game.setPlayerDisplayNames(display_name_from_flag(p0_flag), display_name_from_flag(p1_flag));
    const auto winner = game.runGame();

    const char* winner_name = "?";
    switch (winner) {
    case PlayerId::Player0: winner_name = "Player0"; break;
    case PlayerId::Player1: winner_name = "Player1"; break;
    case PlayerId::NoPlayer: winner_name = "NoPlayer"; break;
    }

    std::cout << "winner=" << winner_name << "\n";
    return 0;
}
