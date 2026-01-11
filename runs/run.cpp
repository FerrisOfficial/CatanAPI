#include "game_simulation/game.hpp"
#include "players/randomPlayer.hpp"
#include "players/it1Player.hpp"
#include "players/it2Player.hpp"
#include "players/it3Player.hpp"
#include "players/it4Player.hpp"
#include "players/it5Player.hpp"
#include "players/alphaBetaPlayer.hpp"

#include <iostream>
#include <iomanip>
#include <memory>
#include <string>
#include <exception>
#include <chrono>
#include <sstream>
#include <vector>

namespace {
std::unique_ptr<IPlayer> make_player_from_flag(const std::string& flag) {
    // Player flags (extensible):
    // - rp : RandomPlayer
    if (flag == "rp") {
        return std::make_unique<RandomPlayer>();
    }
    if (flag == "it1") {
        return std::make_unique<It1Player>();
    }
    if (flag == "it2") {
        return std::make_unique<It2Player>();
    }
    if (flag == "it3") {
        return std::make_unique<It3Player>();
    }
    if (flag == "it4") {
        return std::make_unique<It4Player>();
    }
    if (flag == "it5") {
        return std::make_unique<It5Player>();
    }
    if (flag == "ab") {
        return std::make_unique<alphaBetaPlayer>();
    }

    return nullptr;
}

std::string display_name_from_flag(const std::string& flag) {
    if (flag == "rp") return "RandomPlayer";
    if (flag == "it1") return "It1Player";
    if (flag == "it2") return "It2Player";
    if (flag == "it3") return "It3Player";
    if (flag == "it4") return "It4Player";
    if (flag == "it5") return "It5Player";
    if (flag == "it6") return "It6Player";
    return flag;
}

void print_usage(const char* exe) {
    std::cerr
        << "Usage: " << exe << " [options] <player0_flag> <player1_flag>\n"
        << "\nOptions:\n"
        << "  -n, --games <N>     Number of games to run (default: 1)\n"
    << "      --switch        Alternate seats each game (swap players every 2nd game)\n"
    << "      --swap          Alias for --switch\n"
        << "      --no-dump       Disable JSONL dumper logs\n"
        << "  -h, --help          Show this help\n"
        << "\nPlayers:\n"
        << "  rp  RandomPlayer\n"
        << "  it1 It1Player\n"
        << "  it2 It2Player\n"
        << "  it3 It3Player\n"
        << "  it4 It4Player\n"
        << "  it5 It5Player\n"
        << "  it6 It6Player\n";
}

struct Options {
    size_t games = 1;
    bool dump = true;
    bool switchSeats = false;
};

bool starts_with(const std::string& s, const char* prefix) {
    const size_t n = std::char_traits<char>::length(prefix);
    return s.size() >= n && s.compare(0, n, prefix) == 0;
}

bool parse_size_t(const std::string& s, size_t& out) {
    try {
        size_t idx = 0;
        unsigned long long v = std::stoull(s, &idx, 10);
        if (idx != s.size()) return false;
        if (v == 0) return false;
        out = static_cast<size_t>(v);
        return true;
    } catch (...) {
        return false;
    }
}

std::string format_hhmmss(std::chrono::seconds secs) {
    const auto total = secs.count();
    const auto h = total / 3600;
    const auto m = (total % 3600) / 60;
    const auto s = total % 60;
    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(2) << h << ':'
        << std::setw(2) << m << ':'
        << std::setw(2) << s;
    return oss.str();
}

void print_progress(size_t done, size_t total,
                    size_t winsP0, size_t winsP1, size_t winsNP,
                    size_t winsBotA, size_t winsBotB,
                    bool switchSeats,
                    const std::string& botAName, const std::string& botBName,
                    unsigned long long totalTurns, unsigned maxTurns,
                    std::chrono::steady_clock::time_point start) {
    using namespace std::chrono;
    const auto now = steady_clock::now();
    const auto elapsed = duration_cast<duration<double>>(now - start);
    const double eps = 1e-9;
    const double elapsedSeconds = std::max(elapsed.count(), eps);
    const double gps = done > 0 ? (static_cast<double>(done) / elapsedSeconds) : 0.0;
    const double remaining = (total > done && gps > 0.0) ? (static_cast<double>(total - done) / gps) : 0.0;
    const double avgTurns = done > 0 ? (static_cast<double>(totalTurns) / static_cast<double>(done)) : 0.0;

    const auto pct = [&](size_t v) -> double {
        return done > 0 ? (100.0 * static_cast<double>(v) / static_cast<double>(done)) : 0.0;
    };

    // Keep this line compact to reduce wrapping in narrow terminals.
    std::ostringstream oss;
    oss << "[" << std::setw(5) << done << "/" << total << "] ";

    if (switchSeats) {
        // When seats alternate, report wins by bot flag order (positional args).
        oss << botAName << "=" << winsBotA << "(" << std::fixed << std::setprecision(1) << pct(winsBotA) << "%) "
            << botBName << "=" << winsBotB << "(" << std::fixed << std::setprecision(1) << pct(winsBotB) << "%) ";
    } else {
        // Without switching seats, seat wins correspond to bot wins.
        oss << "P0=" << winsP0 << "(" << std::fixed << std::setprecision(1) << pct(winsP0) << "%) "
            << "P1=" << winsP1 << "(" << std::fixed << std::setprecision(1) << pct(winsP1) << "%) ";
    }

    oss << "NP=" << winsNP << "(" << std::fixed << std::setprecision(1) << pct(winsNP) << "%) "
        << "avgT=" << std::fixed << std::setprecision(1) << avgTurns << " "
        << "maxT=" << maxTurns << " "
        << std::fixed << std::setprecision(1) << gps << "g/s "
        << "ETA=" << format_hhmmss(std::chrono::seconds(static_cast<long long>(remaining)));

    std::cout << oss.str() << "\n";
}
} // namespace

int main(int argc, char** argv) {
    Options opt;
    std::vector<std::string> positional;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        }
        if (arg == "--no-dump") {
            opt.dump = false;
            continue;
        }
        if (arg == "--switch" || arg == "--swap") {
            opt.switchSeats = true;
            continue;
        }
        if (arg == "--dump") {
            opt.dump = true;
            continue;
        }
        if (arg == "-n" || arg == "--games") {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for " << arg << "\n";
                print_usage(argv[0]);
                return 2;
            }
            size_t n = 0;
            if (!parse_size_t(argv[++i], n)) {
                std::cerr << "Invalid games count: '" << argv[i] << "'\n";
                return 2;
            }
            opt.games = n;
            continue;
        }
        if (starts_with(arg, "--games=")) {
            size_t n = 0;
            if (!parse_size_t(arg.substr(std::string("--games=").size()), n)) {
                std::cerr << "Invalid games count: '" << arg << "'\n";
                return 2;
            }
            opt.games = n;
            continue;
        }
        if (!arg.empty() && arg[0] == '-') {
            std::cerr << "Unknown option: '" << arg << "'\n";
            print_usage(argv[0]);
            return 2;
        }

        positional.push_back(arg);
    }

    if (positional.size() != 2) {
        print_usage(argv[0]);
        return 2;
    }

    const std::string p0_flag = positional[0];
    const std::string p1_flag = positional[1];

    // Validate flags early.
    if (!make_player_from_flag(p0_flag) || !make_player_from_flag(p1_flag)) {
        std::cerr << "Unknown player flag(s): '" << p0_flag << "', '" << p1_flag << "'\n";
        print_usage(argv[0]);
        return 2;
    }

    const auto p0_name = display_name_from_flag(p0_flag);
    const auto p1_name = display_name_from_flag(p1_flag);

    if (opt.games > 1 && opt.dump) {
        std::cout << "Note: dump is ON and will create " << opt.games << " log files in ./logs. "
                  << "Use --no-dump for batch runs.\n";
    }
    std::cout << "Running " << opt.games << " game(s): " << p0_name << " vs " << p1_name
              << " | dump=" << (opt.dump ? "on" : "off")
              << " | switchSeats=" << (opt.switchSeats ? "on" : "off") << "\n";

    size_t winsP0 = 0;
    size_t winsP1 = 0;
    size_t winsNP = 0;

    // Wins tracked by bot flag order (positional args), independent of seat.
    size_t winsBotA = 0; // p0_flag bot
    size_t winsBotB = 0; // p1_flag bot
    unsigned long long totalTurns = 0;
    unsigned maxTurns = 0;

    const size_t progressEvery = (opt.games >= 100) ? std::max<size_t>(1, opt.games / 100) : 1;
    const auto start = std::chrono::steady_clock::now();

    PlayerId lastWinner = PlayerId::NoPlayer;
    unsigned lastTurns = 0;

    for (size_t gameIndex = 1; gameIndex <= opt.games; ++gameIndex) {
        try {
            const bool swapped = opt.switchSeats && ((gameIndex % 2) == 0);

            const std::string seat0_flag = swapped ? p1_flag : p0_flag;
            const std::string seat1_flag = swapped ? p0_flag : p1_flag;
            const std::string seat0_name = swapped ? p1_name : p0_name;
            const std::string seat1_name = swapped ? p0_name : p1_name;

            auto player0 = make_player_from_flag(seat0_flag);
            auto player1 = make_player_from_flag(seat1_flag);

            Game game(*player0, *player1);
            game.setPlayerDisplayNames(seat0_name, seat1_name);
            game.setDumpEnabled(opt.dump);

            const auto winner = game.runGame();
            lastWinner = winner;
            lastTurns = static_cast<unsigned>(game.boardState.currentTurn);

            switch (winner) {
                case PlayerId::Player0: ++winsP0; break;
                case PlayerId::Player1: ++winsP1; break;
                case PlayerId::NoPlayer: ++winsNP; break;
            }

            // Map seat winner back to the original bot ordering.
            if (winner == PlayerId::NoPlayer) {
                // nothing
            } else {
                const bool seat0Won = (winner == PlayerId::Player0);
                const std::string& winningFlag = seat0Won ? seat0_flag : seat1_flag;
                if (winningFlag == p0_flag) ++winsBotA;
                else if (winningFlag == p1_flag) ++winsBotB;
            }

        } catch (const std::exception& e) {
            std::cerr << "Game " << gameIndex << " failed: " << e.what() << "\n";
            return 1;
        } catch (...) {
            std::cerr << "Game " << gameIndex << " failed: unknown exception\n";
            return 1;
        }

        totalTurns += lastTurns;
        maxTurns = std::max(maxTurns, lastTurns);

        if (gameIndex == 1 || gameIndex == opt.games || (gameIndex % progressEvery) == 0) {
            print_progress(
                gameIndex,
                opt.games,
                winsP0,
                winsP1,
                winsNP,
                winsBotA,
                winsBotB,
                opt.switchSeats,
                p0_flag,
                p1_flag,
                totalTurns,
                maxTurns,
                start
            );
        }
    }

    const auto end = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    const double gps = opt.games > 0 ? (static_cast<double>(opt.games) / std::max(elapsed.count(), 1e-9)) : 0.0;
    const double avgTurns = opt.games > 0 ? (static_cast<double>(totalTurns) / static_cast<double>(opt.games)) : 0.0;

    const char* winner_name = "?";
    switch (lastWinner) {
        case PlayerId::Player0: winner_name = "Player0"; break;
        case PlayerId::Player1: winner_name = "Player1"; break;
        case PlayerId::NoPlayer: winner_name = "NoPlayer"; break;
    }

    std::cout << "Summary: "
              << "P0=" << winsP0 << ", P1=" << winsP1 << ", NP=" << winsNP
              << ", avgTurns=" << std::fixed << std::setprecision(1) << avgTurns
              << ", maxTurns=" << maxTurns
              << ", elapsed=" << std::fixed << std::setprecision(2) << elapsed.count() << "s"
              << ", speed=" << std::fixed << std::setprecision(1) << gps << " g/s\n";

    if (opt.switchSeats) {
        std::cout << "ByBot: "
                  << p0_flag << "=" << winsBotA << ", "
                  << p1_flag << "=" << winsBotB << ", "
                  << "NP=" << winsNP << "\n";
    }

    // Preserve the original single-value output for scripts.
    std::cout << "winner=" << winner_name << "\n";
    std::cout << "turns=" << lastTurns << "\n";
    return 0;
}
