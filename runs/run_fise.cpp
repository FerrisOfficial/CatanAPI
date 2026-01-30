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

#include <chrono>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace {
std::unique_ptr<IPlayer> make_player_from_flag(const std::string& flag) {
    if (flag == "rp") return std::make_unique<RandomPlayer>();
    if (flag == "it1") return std::make_unique<It1Player>();
    if (flag == "it2") return std::make_unique<It2Player>();
    if (flag == "it3") return std::make_unique<It3Player>();
    if (flag == "it4") return std::make_unique<It4Player>();
    if (flag == "it5") return std::make_unique<It5Player>();
    if (flag == "para") return std::make_unique<ParaPlayer>();
    if (flag == "psit5") return std::make_unique<ParaSetIt5Player>();
    if (flag == "ab") return std::make_unique<alphaBetaPlayer>();
    if (flag == "or") return std::make_unique<OneResourcePlayer>();
    if (flag == "dev") return std::make_unique<DevPlayer>();
    if (flag == "road") return std::make_unique<RoadPlayer>();
    if (flag == "cr") return std::make_unique<CityRushPlayer>();
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
    if (flag == "ab") return "AlphaBetaPlayer";
    if (flag == "or") return "OneResourcePlayer";
    if (flag == "dev") return "DevPlayer";
    if (flag == "road") return "RoadPlayer";
    if (flag == "cr") return "CityRushPlayer";
    return flag;
}

std::string timestamp_string() {
    const auto now = std::chrono::system_clock::now();
    const auto t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S");
    return oss.str();
}

struct OrderResult {
    size_t winsFirst = 0;
    size_t winsSecond = 0;
    size_t noWinner = 0;
    unsigned long long totalTurns = 0;
    unsigned maxTurns = 0;
};

OrderResult  run_fixed_seats(const std::string& p0_flag,
                            const std::string& p1_flag,
                            size_t games,
                            const std::string& progressLabel) {
    OrderResult result;
    const size_t progressEvery = (games >= 100) ? std::max<size_t>(1, games / 100) : 1;
    for (size_t i = 0; i < games; ++i) {
        auto player0 = make_player_from_flag(p0_flag);
        auto player1 = make_player_from_flag(p1_flag);

        if (!player0 || !player1) {
            throw std::runtime_error("Unknown player flag in run_fixed_seats");
        }

        Game game(*player0, *player1);
        game.setPlayerDisplayNames(display_name_from_flag(p0_flag), display_name_from_flag(p1_flag));
        game.setDumpEnabled(false);

        const auto winner = game.runGame();
        const unsigned turns = static_cast<unsigned>(game.boardState.currentTurn);
        result.totalTurns += turns;
        result.maxTurns = std::max(result.maxTurns, turns);

        switch (winner) {
            case PlayerId::Player0: ++result.winsFirst; break;
            case PlayerId::Player1: ++result.winsSecond; break;
            case PlayerId::NoPlayer: ++result.noWinner; break;
        }

        const size_t done = i + 1;
        if (done == 1 || done == games || (done % progressEvery) == 0) {
            const double pct = games > 0 ? (100.0 * static_cast<double>(done) / static_cast<double>(games)) : 0.0;
            std::cout << progressLabel << " " << done << "/" << games
                      << " (" << std::fixed << std::setprecision(1) << pct << "%)"
                      << " | W1=" << result.winsFirst
                      << " W2=" << result.winsSecond
                      << " NW=" << result.noWinner
                      << "\n";
        }
    }
    return result;
}

struct SeatTotals {
    size_t winsFirst = 0;
    size_t winsSecond = 0;
    size_t gamesFirst = 0;
    size_t gamesSecond = 0;
    size_t noWinnerFirst = 0;
    size_t noWinnerSecond = 0;
};

void update_totals_for_order(SeatTotals& firstBot, SeatTotals& secondBot, const OrderResult& r, size_t games) {
    firstBot.gamesFirst += games;
    secondBot.gamesSecond += games;
    firstBot.winsFirst += r.winsFirst;
    secondBot.winsSecond += r.winsSecond;
    firstBot.noWinnerFirst += r.noWinner;
    secondBot.noWinnerSecond += r.noWinner;
}

void print_order_summary(std::ostream& os,
                         const std::string& firstFlag,
                         const std::string& secondFlag,
                         size_t games,
                         const OrderResult& r) {
    const auto pct = [games](size_t v) {
        return games > 0 ? (100.0 * static_cast<double>(v) / static_cast<double>(games)) : 0.0;
    };
    const double avgTurns = games > 0 ? (static_cast<double>(r.totalTurns) / static_cast<double>(games)) : 0.0;

    os << "  " << display_name_from_flag(firstFlag) << " (pierwszy) vs "
       << display_name_from_flag(secondFlag) << " (drugi)\n";
    os << "    Wygrane pierwszego: " << r.winsFirst << " (" << std::fixed << std::setprecision(1)
       << pct(r.winsFirst) << "%)\n";
    os << "    Wygrane drugiego:   " << r.winsSecond << " (" << std::fixed << std::setprecision(1)
       << pct(r.winsSecond) << "%)\n";
    os << "    Brak zwycięzcy:      " << r.noWinner << " (" << std::fixed << std::setprecision(1)
       << pct(r.noWinner) << "%)\n";
    os << "    Średnia liczba tur:  " << std::fixed << std::setprecision(1) << avgTurns
       << ", max tur: " << r.maxTurns << "\n";
}
} // namespace

int main() {
    const size_t gamesPerOrder = 100;

    const std::vector<std::string> flags = {
        "rp", "it1", "it2", "it3", "it4", "it5",
        "para", "psit5", "ab", "or", "dev", "road", "cr"
    };

    for (const auto& f : flags) {
        if (!make_player_from_flag(f)) {
            std::cerr << "Unknown player flag in list: " << f << "\n";
            return 2;
        }
    }

    const std::string outputPath = "logs/fise_results_" + timestamp_string() + ".txt";
    std::ofstream out(outputPath);
    if (!out) {
        std::cerr << "Failed to open output file: " << outputPath << "\n";
        return 2;
    }

    out << "Eksperyment FISE: wpływ kolejności (pierwszy/drugi) na liczbę zwycięstw\n";
    out << "Liczba gier na porządek: " << gamesPerOrder << "\n";
    out << "Łącznie botów: " << flags.size() << "\n\n";

    std::vector<SeatTotals> totals(flags.size());

    const auto start = std::chrono::steady_clock::now();

    for (size_t i = 0; i < flags.size(); ++i) {
        for (size_t j = i; j < flags.size(); ++j) {
            const auto& a = flags[i];
            const auto& b = flags[j];

            std::cout << "Matchup: " << display_name_from_flag(a)
                      << " vs " << display_name_from_flag(b)
                      << " (" << (i + 1) << "/" << flags.size()
                      << " vs " << (j + 1) << "/" << flags.size() << ")\n";

            out << "============================================================\n";
            out << "Para: " << display_name_from_flag(a) << " vs " << display_name_from_flag(b) << "\n";

            OrderResult r1 = run_fixed_seats(a, b, gamesPerOrder,
                                             "  [" + display_name_from_flag(a) + "(1) vs " + display_name_from_flag(b) + "(2)]");
            print_order_summary(out, a, b, gamesPerOrder, r1);
            update_totals_for_order(totals[i], totals[j], r1, gamesPerOrder);

            OrderResult r2 = run_fixed_seats(b, a, gamesPerOrder,
                                             "  [" + display_name_from_flag(b) + "(1) vs " + display_name_from_flag(a) + "(2)]");
            print_order_summary(out, b, a, gamesPerOrder, r2);
            update_totals_for_order(totals[j], totals[i], r2, gamesPerOrder);

            out << "\n";
            out.flush();
        }
    }

    const auto end = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);

    out << "============================================================\n";
    out << "Podsumowanie per bot (wygrane zależne od kolejności)\n";

    for (size_t i = 0; i < flags.size(); ++i) {
        const auto& name = display_name_from_flag(flags[i]);
        const auto& t = totals[i];
        const auto pct = [](size_t wins, size_t games) {
            return games > 0 ? (100.0 * static_cast<double>(wins) / static_cast<double>(games)) : 0.0;
        };

        out << "- " << name << "\n";
        out << "    jako pierwszy: " << t.winsFirst << "/" << t.gamesFirst
            << " (" << std::fixed << std::setprecision(1) << pct(t.winsFirst, t.gamesFirst) << "%), "
            << "NW=" << t.noWinnerFirst << "\n";
        out << "    jako drugi:    " << t.winsSecond << "/" << t.gamesSecond
            << " (" << std::fixed << std::setprecision(1) << pct(t.winsSecond, t.gamesSecond) << "%), "
            << "NW=" << t.noWinnerSecond << "\n";
    }

    out << "\nCzas całkowity: " << std::fixed << std::setprecision(2) << elapsed.count() << " s\n";

    std::cout << "Zapisano wyniki do: " << outputPath << "\n";
    return 0;
}
