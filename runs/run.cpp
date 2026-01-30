#include "game_simulation/game.hpp"
#include "players/randomPlayer.hpp"
#include "players/it1Player.hpp"
#include "players/it2Player.hpp"
#include "players/it3Player.hpp"
#include "players/it4Player.hpp"
#include "players/it5Player.hpp"
#include "players/paraPlayer.hpp"
#include "players/paraSetit5Player.hpp"
#include "players/alphaBetaPlayer.hpp"
#include "players/oneResourcePlayer.hpp"
#include "players/devPlayer.hpp"
#include "players/roadPlayer.hpp"
#include "players/cityRushPlayer.hpp"
#include "players/playerHelpers.hpp"
#include "utils/randomDevice.hpp"

#include <iostream>
#include <iomanip>
#include <memory>
#include <string>
#include <exception>
#include <chrono>
#include <sstream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <optional>
#include <cstdint>

namespace {
using PlayerHelpers::effective_vp;
using PlayerHelpers::production_score_for_player;
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
    if (flag == "para") {
        return std::make_unique<ParaPlayer>();
    }
    if (flag == "psit5") {
        return std::make_unique<ParaSetIt5Player>();
    }
    if (flag == "ab") {
        return std::make_unique<alphaBetaPlayer>();
    }
    if (flag == "or") {
        return std::make_unique<OneResourcePlayer>();
    }
    if (flag == "dev") {
        return std::make_unique<DevPlayer>();
    }
    if (flag == "road") {
        return std::make_unique<RoadPlayer>();
    }
    if (flag == "cr") {
        return std::make_unique<CityRushPlayer>();
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
    if (flag == "para") return "ParaPlayer";
    if (flag == "psit5") return "ParaSettleIt5Player";
    if (flag == "dev") return "DevPlayer";
    if (flag == "road") return "RoadPlayer";
    if (flag == "cr") return "CityRushPlayer";
    return flag;
}

std::string bybot_label(const std::string& flag, char which, bool disambiguate) {
    if (!disambiguate) return flag;
    std::ostringstream oss;
    oss << which << "(" << flag << ")";
    return oss.str();
}

void print_usage(const char* exe) {
    std::cerr
        << "Usage: " << exe << " [options] <player0_flag> <player1_flag>\n"
        << "\nOptions:\n"
        << "  -n, --games <N>     Number of games to run (default: 1)\n"
        << "      --seed <S>      Deterministic RNG seed (re-seeded per game as seed+gameIndex)\n"
        << "      --switch        Alternate seats each game (swap players every 2nd game)\n"
        << "      --swap          Alias for --switch\n"
        << "      --no-dump       Disable JSONL dumper logs\n"
        << "      --dump-game <N> Enable JSONL dumper logs only for game N\n"
        << "      --dump-range <A> <B> Enable JSONL dumper logs only for games in [A,B]\n"
        << "  -h, --help          Show this help\n"
        << "\nPlayers:\n"
        << "  rp  RandomPlayer\n"
        << "  it1 It1Player\n"
        << "  it2 It2Player\n"
        << "  it3 It3Player\n"
        << "  it4 It4Player\n"
        << "  it5 It5Player\n"
        << "  para ParaPlayer (params from ./players/paraPlayer.cfg; override via CATAN_PARA_CFG)\n"
        << "  psit5 ParaSettleIt5Player (It5 + param init placement from ./players/paraSetit5Player.cfg; override via CATAN_PARA_SETIT5_CFG)\n"
        << "  ab  AlphaBetaPlayer\n"
        << "  or  OneResourcePlayer\n"
        << "  dev DevPlayer (heavily prioritizes development cards)\n"
        << "  road RoadPlayer (road-focused, aims for long continuous road)\n"
        << "  cr  CityRushPlayer (prioritizes grain/ore and city upgrades)\n";
}

struct Options {
    size_t games = 1;
    bool dump = true;
    bool switchSeats = false;
    std::optional<uint32_t> seed;
    std::optional<size_t> dumpFrom;
    std::optional<size_t> dumpTo;
};

// Average VP of the *losing* bot, split by the two compared bots (positional args),
// independent of which seat they occupied in a given game.
struct LossVPByBotStats {
    long long sumBotALoserVp = 0;
    long long sumBotBLoserVp = 0;
    size_t botALosses = 0;
    size_t botBLosses = 0;
    bool botACorrupted = false;
    bool botBCorrupted = false;

    // Keep some context for debugging corruption.
    int lastRawLoserVpA = 0;
    int lastRawLoserVpB = 0;
    size_t lastGameIndexA = 0;
    size_t lastGameIndexB = 0;

    // seat0Flag/seat1Flag: which bot played in Player0/Player1 in this particular game.
    void addGame(const std::string& botAFlag,
                 const std::string& botBFlag,
                 const std::string& seat0Flag,
                 const std::string& seat1Flag,
                 PlayerId winner,
                 int vpSeat0,
                 int vpSeat1,
                 size_t gameIndex) {
        if (winner == PlayerId::NoPlayer) return;

        const bool seat0Won = (winner == PlayerId::Player0);
        const std::string& loserFlag = seat0Won ? seat1Flag : seat0Flag;
        const int rawLoserVp = seat0Won ? vpSeat1 : vpSeat0;
        const int loserVp = std::max(0, rawLoserVp);

        // Defensive sanity check: loser VP should be in a small range.
        // If this trips, it likely indicates memory corruption in the game state.
        if (rawLoserVp < 0 || rawLoserVp > 25) {
            std::cerr << "WARNING: suspicious loser VP at game " << gameIndex
                      << ": seat0='" << seat0Flag << "' seat1='" << seat1Flag << "'"
                      << ", winner=" << (seat0Won ? "Player0" : "Player1")
                      << ", vpSeat0=" << vpSeat0 << ", vpSeat1=" << vpSeat1
                      << "\n";
        }

        if (loserFlag == botAFlag) {
            if (!botACorrupted) {
                sumBotALoserVp += static_cast<long long>(loserVp);
                ++botALosses;
                lastRawLoserVpA = rawLoserVp;
                lastGameIndexA = gameIndex;
            }
        } else if (loserFlag == botBFlag) {
            if (!botBCorrupted) {
                sumBotBLoserVp += static_cast<long long>(loserVp);
                ++botBLosses;
                lastRawLoserVpB = rawLoserVp;
                lastGameIndexB = gameIndex;
            }
        }

        // Detect accumulator corruption early but do not terminate the batch run.
        if (!botACorrupted && botALosses && (sumBotALoserVp < 0 || sumBotALoserVp > static_cast<long long>(botALosses) * 100LL)) {
            botACorrupted = true;
            const uint64_t u = static_cast<uint64_t>(sumBotALoserVp);
            std::cerr << "WARNING: LossVP accumulator corrupted for botA; disabling LossVP for botA after game " << gameIndex
                      << " (sumBotALoserVp=" << sumBotALoserVp << ", botALosses=" << botALosses
                      << ", lastRawLoserVp=" << lastRawLoserVpA << ", lastGameIndex=" << lastGameIndexA
                      << ", sum_hi32=0x" << std::hex << static_cast<unsigned>((u >> 32) & 0xffffffffULL)
                      << ", sum_lo32=0x" << static_cast<unsigned>(u & 0xffffffffULL) << std::dec
                      << ")\n";
        }
        if (!botBCorrupted && botBLosses && (sumBotBLoserVp < 0 || sumBotBLoserVp > static_cast<long long>(botBLosses) * 100LL)) {
            botBCorrupted = true;
            const uint64_t u = static_cast<uint64_t>(sumBotBLoserVp);
            std::cerr << "WARNING: LossVP accumulator corrupted for botB; disabling LossVP for botB after game " << gameIndex
                      << " (sumBotBLoserVp=" << sumBotBLoserVp << ", botBLosses=" << botBLosses
                      << ", lastRawLoserVp=" << lastRawLoserVpB << ", lastGameIndex=" << lastGameIndexB
                      << ", sum_hi32=0x" << std::hex << static_cast<unsigned>((u >> 32) & 0xffffffffULL)
                      << ", sum_lo32=0x" << static_cast<unsigned>(u & 0xffffffffULL) << std::dec
                      << ")\n";
        }
    }

    double avgBotALoserVp() const {
        if (botACorrupted) return -1.0;
        return botALosses ? (static_cast<double>(sumBotALoserVp) / static_cast<double>(botALosses)) : 0.0;
    }
    double avgBotBLoserVp() const {
        if (botBCorrupted) return -1.0;
        return botBLosses ? (static_cast<double>(sumBotBLoserVp) / static_cast<double>(botBLosses)) : 0.0;
    }
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

bool should_dump_game(const Options& opt, size_t gameIndex) {
    if (opt.dumpFrom || opt.dumpTo) {
        const size_t from = opt.dumpFrom.value_or(opt.dumpTo.value_or(0));
        const size_t to = opt.dumpTo.value_or(opt.dumpFrom.value_or(0));
        return (from != 0 && to != 0 && gameIndex >= from && gameIndex <= to);
    }
    return opt.dump;
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
        if (arg == "--dump-game") {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for " << arg << "\n";
                print_usage(argv[0]);
                return 2;
            }
            size_t n = 0;
            if (!parse_size_t(argv[++i], n)) {
                std::cerr << "Invalid game index for --dump-game: '" << argv[i] << "'\n";
                return 2;
            }
            opt.dumpFrom = n;
            opt.dumpTo = n;
            continue;
        }
        if (arg == "--dump-range") {
            if (i + 2 >= argc) {
                std::cerr << "Missing value(s) for " << arg << "\n";
                print_usage(argv[0]);
                return 2;
            }
            size_t from = 0;
            size_t to = 0;
            if (!parse_size_t(argv[++i], from) || !parse_size_t(argv[++i], to) || from > to) {
                std::cerr << "Invalid range for --dump-range (expected A B with 1<=A<=B): '" << argv[i - 1] << "' '" << argv[i] << "'\n";
                return 2;
            }
            opt.dumpFrom = from;
            opt.dumpTo = to;
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
        if (arg == "--seed") {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for " << arg << "\n";
                print_usage(argv[0]);
                return 2;
            }
            try {
                const unsigned long long v = std::stoull(argv[++i], nullptr, 10);
                opt.seed = static_cast<uint32_t>(v);
            } catch (...) {
                std::cerr << "Invalid seed: '" << argv[i] << "'\n";
                return 2;
            }
            continue;
        }
        if (starts_with(arg, "--seed=")) {
            try {
                const unsigned long long v = std::stoull(arg.substr(std::string("--seed=").size()), nullptr, 10);
                opt.seed = static_cast<uint32_t>(v);
            } catch (...) {
                std::cerr << "Invalid seed: '" << arg << "'\n";
                return 2;
            }
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

    const bool sameFlags = (p0_flag == p1_flag);

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

    if (opt.games > 1 && (opt.dumpFrom || opt.dumpTo)) {
        const size_t from = opt.dumpFrom.value_or(opt.dumpTo.value_or(0));
        const size_t to = opt.dumpTo.value_or(opt.dumpFrom.value_or(0));
        std::cout << "Note: selective dumping is ON and will create " << (to >= from ? (to - from + 1) : 0)
                  << " log file(s) in ./logs for games " << from << ".." << to << ".\n";
    }

    if (opt.games > 1 && sameFlags && !opt.switchSeats) {
        std::cout << "Note: you are running the same bot ('" << p0_flag << "' vs '" << p1_flag
                  << "') without --switch. Results will reflect seat advantage (P0/P1), not bot strength. "
                  << "Use --switch for a fair ~50/50 comparison.\n";
    }

    std::cout << "Running " << opt.games << " game(s): " << p0_name << " vs " << p1_name;
    if (opt.dumpFrom || opt.dumpTo) {
        const size_t from = opt.dumpFrom.value_or(opt.dumpTo.value_or(0));
        const size_t to = opt.dumpTo.value_or(opt.dumpFrom.value_or(0));
        std::cout << " | dump=range(" << from << ".." << to << ")";
    } else {
        std::cout << " | dump=" << (opt.dump ? "on" : "off");
    }
    std::cout << " | switchSeats=" << (opt.switchSeats ? "on" : "off");
    if (opt.seed) {
        std::cout << " | seed=" << *opt.seed;
    }
    std::cout << "\n";

    size_t winsP0 = 0;
    size_t winsP1 = 0;
    size_t winsNP = 0;

    // Wins tracked by bot flag order (positional args), independent of seat.
    size_t winsBotA = 0; // p0_flag bot
    size_t winsBotB = 0; // p1_flag bot
    unsigned long long totalTurns = 0;
    unsigned maxTurns = 0;

    // Additional metrics
    size_t longestRoadCountBotA = 0;
    size_t longestRoadCountBotB = 0;
    size_t largestArmyCountBotA = 0;
    size_t largestArmyCountBotB = 0;

    unsigned long long sumDevCardsBotA = 0;
    unsigned long long sumDevCardsBotB = 0;

    unsigned long long sumProdScoreBotA = 0; // pips-weighted production score (approx production per turn)
    unsigned long long sumProdScoreBotB = 0;

    unsigned long long sumCitiesBuiltBotA = 0;
    unsigned long long sumCitiesBuiltBotB = 0;
    unsigned long long sumSettlementsBuiltBotA = 0;
    unsigned long long sumSettlementsBuiltBotB = 0;
    unsigned long long sumRoadsBuiltBotA = 0;
    unsigned long long sumRoadsBuiltBotB = 0;

    // Avg VP of the losing bot, split by the two compared bots (positional args), independent of seat.
    LossVPByBotStats lossVpByBot;

    const size_t progressEvery = (opt.games >= 100) ? std::max<size_t>(1, opt.games / 100) : 1;
    const auto start = std::chrono::steady_clock::now();

    PlayerId lastWinner = PlayerId::NoPlayer;
    unsigned lastTurns = 0;

    for (size_t gameIndex = 1; gameIndex <= opt.games; ++gameIndex) {
        try {
            const bool swapped = opt.switchSeats && ((gameIndex % 2) == 0);

            // Deterministic seeding (useful for reproducing intermittent corruption).
            if (opt.seed) {
                // Re-seed per game to make each gameIndex reproducible independently.
                RandomDevice::seed(static_cast<uint32_t>(*opt.seed + static_cast<uint32_t>(gameIndex)));
            }

            const std::string seat0_flag = swapped ? p1_flag : p0_flag;
            const std::string seat1_flag = swapped ? p0_flag : p1_flag;
            const std::string seat0_name = swapped ? p1_name : p0_name;
            const std::string seat1_name = swapped ? p0_name : p1_name;

            auto player0 = make_player_from_flag(seat0_flag);
            auto player1 = make_player_from_flag(seat1_flag);

            Game game(*player0, *player1);
            game.setPlayerDisplayNames(seat0_name, seat1_name);
            game.setDumpEnabled(should_dump_game(opt, gameIndex));

            const auto winner = game.runGame();
            lastWinner = winner;
            lastTurns = static_cast<unsigned>(game.boardState.currentTurn);

            // Final state snapshots for metrics
            const auto packedP0 = game.boardState.packedPlayers[0];
            const auto packedP1 = game.boardState.packedPlayers[1];

            const int vpSeat0 = effective_vp(&game.boardState, PlayerId::Player0);
            const int vpSeat1 = effective_vp(&game.boardState, PlayerId::Player1);
            lossVpByBot.addGame(p0_flag, p1_flag, seat0_flag, seat1_flag, winner, vpSeat0, vpSeat1, gameIndex);

            switch (winner) {
                case PlayerId::Player0: ++winsP0; break;
                case PlayerId::Player1: ++winsP1; break;
                case PlayerId::NoPlayer: ++winsNP; break;
            }

            // Map seat winner back to the original bot ordering.
            // IMPORTANT: When flags are identical (e.g., it5 vs it5), comparing strings cannot
            // distinguish bot A from bot B. Use seating (and swap state) instead.
            if (winner != PlayerId::NoPlayer) {
                const bool seat0Won = (winner == PlayerId::Player0);
                const bool seat0IsBotA = !swapped; // when swapped, seat0 is botB

                if (seat0Won) {
                    if (seat0IsBotA) ++winsBotA;
                    else ++winsBotB;
                } else {
                    // seat1 won
                    if (seat0IsBotA) ++winsBotB;
                    else ++winsBotA;
                }
            }

            // Award frequencies (count per bot across all games)
            {
                const bool seat0IsBotA = !swapped;
                const bool lrP0 = Player::unpackLongestRoadFlag(packedP0);
                const bool lrP1 = Player::unpackLongestRoadFlag(packedP1);
                const bool laP0 = Player::unpackLargestArmyFlag(packedP0);
                const bool laP1 = Player::unpackLargestArmyFlag(packedP1);

                if (seat0IsBotA) {
                    if (lrP0) ++longestRoadCountBotA; else (void)0;
                    if (laP0) ++largestArmyCountBotA; else (void)0;
                    if (lrP1) ++longestRoadCountBotB; else (void)0;
                    if (laP1) ++largestArmyCountBotB; else (void)0;
                } else {
                    if (lrP0) ++longestRoadCountBotB; else (void)0;
                    if (laP0) ++largestArmyCountBotB; else (void)0;
                    if (lrP1) ++longestRoadCountBotA; else (void)0;
                    if (laP1) ++largestArmyCountBotA; else (void)0;
                }
            }

            // Average dev cards purchased (proxy: total dev cards in final state)
            {
                const bool seat0IsBotA = !swapped;
                const int devP0 = static_cast<int>(Player::totalDevCards(packedP0));
                const int devP1 = static_cast<int>(Player::totalDevCards(packedP1));
                if (seat0IsBotA) {
                    sumDevCardsBotA += devP0;
                    sumDevCardsBotB += devP1;
                } else {
                    sumDevCardsBotA += devP1;
                    sumDevCardsBotB += devP0;
                }
            }

            // Average production score (pips-weighted expected production per turn)
            {
                const bool seat0IsBotA = !swapped;
                const int prodP0 = production_score_for_player(&game.boardState, PlayerId::Player0);
                const int prodP1 = production_score_for_player(&game.boardState, PlayerId::Player1);
                if (seat0IsBotA) {
                    sumProdScoreBotA += prodP0;
                    sumProdScoreBotB += prodP1;
                } else {
                    sumProdScoreBotA += prodP1;
                    sumProdScoreBotB += prodP0;
                }
            }

            // Average built structures (derived from remaining pieces in final packed state).
            // NOTE: Settlement count accounts for city upgrades returning a settlement piece.
            {
                constexpr int kInitialRoads = 15;
                constexpr int kInitialSettlements = 5;
                constexpr int kInitialCities = 4;

                const bool seat0IsBotA = !swapped;

                const int roadsLeftP0 = static_cast<int>(Player::unpackAvailableStructures(packedP0, StructureType::Road));
                const int roadsLeftP1 = static_cast<int>(Player::unpackAvailableStructures(packedP1, StructureType::Road));
                const int citiesLeftP0 = static_cast<int>(Player::unpackAvailableStructures(packedP0, StructureType::City));
                const int citiesLeftP1 = static_cast<int>(Player::unpackAvailableStructures(packedP1, StructureType::City));
                const int settlementsLeftP0 = static_cast<int>(Player::unpackAvailableStructures(packedP0, StructureType::Settlement));
                const int settlementsLeftP1 = static_cast<int>(Player::unpackAvailableStructures(packedP1, StructureType::Settlement));

                const int citiesBuiltP0 = kInitialCities - citiesLeftP0;
                const int citiesBuiltP1 = kInitialCities - citiesLeftP1;
                const int roadsBuiltP0 = kInitialRoads - roadsLeftP0;
                const int roadsBuiltP1 = kInitialRoads - roadsLeftP1;

                const int settlementsBuiltP0 = kInitialSettlements + citiesBuiltP0 - settlementsLeftP0;
                const int settlementsBuiltP1 = kInitialSettlements + citiesBuiltP1 - settlementsLeftP1;

                if (seat0IsBotA) {
                    sumCitiesBuiltBotA += static_cast<unsigned long long>(citiesBuiltP0);
                    sumCitiesBuiltBotB += static_cast<unsigned long long>(citiesBuiltP1);
                    sumSettlementsBuiltBotA += static_cast<unsigned long long>(settlementsBuiltP0);
                    sumSettlementsBuiltBotB += static_cast<unsigned long long>(settlementsBuiltP1);
                    sumRoadsBuiltBotA += static_cast<unsigned long long>(roadsBuiltP0);
                    sumRoadsBuiltBotB += static_cast<unsigned long long>(roadsBuiltP1);
                } else {
                    sumCitiesBuiltBotA += static_cast<unsigned long long>(citiesBuiltP1);
                    sumCitiesBuiltBotB += static_cast<unsigned long long>(citiesBuiltP0);
                    sumSettlementsBuiltBotA += static_cast<unsigned long long>(settlementsBuiltP1);
                    sumSettlementsBuiltBotB += static_cast<unsigned long long>(settlementsBuiltP0);
                    sumRoadsBuiltBotA += static_cast<unsigned long long>(roadsBuiltP1);
                    sumRoadsBuiltBotB += static_cast<unsigned long long>(roadsBuiltP0);
                }
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
            const std::string botAProgressLabel = bybot_label(p0_flag, 'A', sameFlags);
            const std::string botBProgressLabel = bybot_label(p1_flag, 'B', sameFlags);
            print_progress(
                gameIndex,
                opt.games,
                winsP0,
                winsP1,
                winsNP,
                winsBotA,
                winsBotB,
                opt.switchSeats,
                botAProgressLabel,
                botBProgressLabel,
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

    // Avg VP of the bot in games it LOST (per compared bot, not per seat).
    {
        const std::string botALabel = bybot_label(p0_flag, 'A', sameFlags);
        const std::string botBLabel = bybot_label(p1_flag, 'B', sameFlags);
        const double lossA = lossVpByBot.avgBotALoserVp();
        const double lossB = lossVpByBot.avgBotBLoserVp();
        std::cout << std::fixed << std::setprecision(2)
                  << "LossVP(avg VP when bot lost): "
                  << botALabel << "=";
        if (lossA < 0.0) std::cout << "N/A";
        else std::cout << lossA;

        std::cout << ", " << botBLabel << "=";
        if (lossB < 0.0) std::cout << "N/A";
        else std::cout << lossB;

        std::cout << "\n";
    }

    if (opt.switchSeats) {
        const std::string botALabel = bybot_label(p0_flag, 'A', sameFlags);
        const std::string botBLabel = bybot_label(p1_flag, 'B', sameFlags);
        std::cout << "ByBot: "
                  << botALabel << "=" << winsBotA << ", "
                  << botBLabel << "=" << winsBotB << ", "
                  << "NP=" << winsNP << "\n";

        // Metrics summary per bot
        const auto lrPctA = 100.0 * static_cast<double>(longestRoadCountBotA) / static_cast<double>(opt.games);
        const auto lrPctB = 100.0 * static_cast<double>(longestRoadCountBotB) / static_cast<double>(opt.games);
        const auto laPctA = 100.0 * static_cast<double>(largestArmyCountBotA) / static_cast<double>(opt.games);
        const auto laPctB = 100.0 * static_cast<double>(largestArmyCountBotB) / static_cast<double>(opt.games);
        const auto avgDevA = static_cast<double>(sumDevCardsBotA) / static_cast<double>(opt.games);
        const auto avgDevB = static_cast<double>(sumDevCardsBotB) / static_cast<double>(opt.games);
        const auto avgProdA = static_cast<double>(sumProdScoreBotA) / static_cast<double>(opt.games);
        const auto avgProdB = static_cast<double>(sumProdScoreBotB) / static_cast<double>(opt.games);

        const auto avgCityA = static_cast<double>(sumCitiesBuiltBotA) / static_cast<double>(opt.games);
        const auto avgCityB = static_cast<double>(sumCitiesBuiltBotB) / static_cast<double>(opt.games);
        const auto avgSettlementA = static_cast<double>(sumSettlementsBuiltBotA) / static_cast<double>(opt.games);
        const auto avgSettlementB = static_cast<double>(sumSettlementsBuiltBotB) / static_cast<double>(opt.games);
        const auto avgRoadA = static_cast<double>(sumRoadsBuiltBotA) / static_cast<double>(opt.games);
        const auto avgRoadB = static_cast<double>(sumRoadsBuiltBotB) / static_cast<double>(opt.games);

        std::cout << std::fixed << std::setprecision(1)
                  << "Metrics: " << botALabel
                  << ", LR%=" << lrPctA
                  << ", LA%=" << laPctA
                  << ", avgDevCards=" << avgDevA
                  << ", avgProdScore=" << avgProdA
                  << ", avgCity=" << avgCityA
                  << ", avgSettlement=" << avgSettlementA
                  << ", avgRoad=" << avgRoadA << "\n";
        std::cout << std::fixed << std::setprecision(1)
                  << "         " << botBLabel
                  << ", LR%=" << lrPctB
                  << ", LA%=" << laPctB
                  << ", avgDevCards=" << avgDevB
                  << ", avgProdScore=" << avgProdB
                  << ", avgCity=" << avgCityB
                  << ", avgSettlement=" << avgSettlementB
                  << ", avgRoad=" << avgRoadB << "\n";
    }

    // Preserve the original single-value output for scripts (single-game runs).
    // For multi-game runs, printing just "winner=..." is ambiguous (it would mean last game only),
    // so use explicit names.
    if (opt.games == 1) {
        std::cout << "winner=" << winner_name << "\n";
        std::cout << "turns=" << lastTurns << "\n";
    } else {
        std::cout << "last_winner=" << winner_name << "\n";
        std::cout << "last_turns=" << lastTurns << "\n";
    }
    return 0;
}
