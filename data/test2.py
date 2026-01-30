import matplotlib.pyplot as plt
import numpy as np

players = [
    "Random", "It1", "It2", "It3", "It4", "It5",
    "Para", "ParaSettle", "AlphaBeta",
    "OneResource", "Dev", "Road", "CityRush"
]

# winrate[playerA][playerB] = % wygranych A jako pierwszy
winrate_first = {
    # Random
    ("Random", "Random"): 48,
    ("Random", "It1"): 32,
    ("Random", "It2"): 29,
    ("Random", "It3"): 1,
    ("Random", "It4"): 1,
    ("Random", "It5"): 0,
    ("Random", "Para"): 0,
    ("Random", "ParaSettle"): 0,
    ("Random", "AlphaBeta"): 0,
    ("Random", "OneResource"): 0,
    ("Random", "Dev"): 0,
    ("Random", "Road"): 0,
    ("Random", "CityRush"): 0,

    # It1
    ("It1", "Random"): 55,
    ("It1", "It1"): 43,
    ("It1", "It2"): 38,
    ("It1", "It3"): 4,
    ("It1", "It4"): 3,
    ("It1", "It5"): 0,
    ("It1", "Para"): 3,
    ("It1", "ParaSettle"): 0,
    ("It1", "AlphaBeta"): 0,
    ("It1", "OneResource"): 1,
    ("It1", "Dev"): 1,
    ("It1", "Road"): 1,
    ("It1", "CityRush"): 0,

    # It2
    ("It2", "Random"): 74,
    ("It2", "It1"): 50,
    ("It2", "It2"): 57,
    ("It2", "It3"): 2,
    ("It2", "It4"): 4,
    ("It2", "It5"): 0,
    ("It2", "Para"): 2,
    ("It2", "ParaSettle"): 0,
    ("It2", "AlphaBeta"): 1,
    ("It2", "OneResource"): 3,
    ("It2", "Dev"): 3,
    ("It2", "Road"): 0,
    ("It2", "CityRush"): 0,

    # It3
    ("It3", "Random"): 99,
    ("It3", "It1"): 95,
    ("It3", "It2"): 96,
    ("It3", "It3"): 45,
    ("It3", "It4"): 36,
    ("It3", "It5"): 12,
    ("It3", "Para"): 36,
    ("It3", "ParaSettle"): 6,
    ("It3", "AlphaBeta"): 6,
    ("It3", "OneResource"): 34,
    ("It3", "Dev"): 33,
    ("It3", "Road"): 15,
    ("It3", "CityRush"): 11,

    # It4
    ("It4", "Random"): 98,
    ("It4", "It1"): 99,
    ("It4", "It2"): 98,
    ("It4", "It3"): 66,
    ("It4", "It4"): 47,
    ("It4", "It5"): 32,
    ("It4", "Para"): 64,
    ("It4", "ParaSettle"): 26,
    ("It4", "AlphaBeta"): 18,
    ("It4", "OneResource"): 61,
    ("It4", "Dev"): 56,
    ("It4", "Road"): 25,
    ("It4", "CityRush"): 22,

    # It5
    ("It5", "Random"): 100,
    ("It5", "It1"): 100,
    ("It5", "It2"): 100,
    ("It5", "It3"): 79,
    ("It5", "It4"): 74,
    ("It5", "It5"): 47,
    ("It5", "Para"): 77,
    ("It5", "ParaSettle"): 52,
    ("It5", "AlphaBeta"): 44,
    ("It5", "OneResource"): 83,
    ("It5", "Dev"): 75,
    ("It5", "Road"): 58,
    ("It5", "CityRush"): 51,

    # Para
    ("Para", "Random"): 100,
    ("Para", "It1"): 97,
    ("Para", "It2"): 96,
    ("Para", "It3"): 58,
    ("Para", "It4"): 38,
    ("Para", "It5"): 23,
    ("Para", "Para"): 56,
    ("Para", "ParaSettle"): 18,
    ("Para", "AlphaBeta"): 19,
    ("Para", "OneResource"): 65,
    ("Para", "Dev"): 48,
    ("Para", "Road"): 31,
    ("Para", "CityRush"): 21,

    # ParaSettle
    ("ParaSettle", "Random"): 100,
    ("ParaSettle", "It1"): 100,
    ("ParaSettle", "It2"): 98,
    ("ParaSettle", "It3"): 91,
    ("ParaSettle", "It4"): 76,
    ("ParaSettle", "It5"): 53,
    ("ParaSettle", "Para"): 87,
    ("ParaSettle", "ParaSettle"): 59,
    ("ParaSettle", "AlphaBeta"): 48,
    ("ParaSettle", "OneResource"): 82,
    ("ParaSettle", "Dev"): 77,
    ("ParaSettle", "Road"): 54,
    ("ParaSettle", "CityRush"): 50,

    # AlphaBeta
    ("AlphaBeta", "Random"): 100,
    ("AlphaBeta", "It1"): 100,
    ("AlphaBeta", "It2"): 100,
    ("AlphaBeta", "It3"): 97,
    ("AlphaBeta", "It4"): 79,
    ("AlphaBeta", "It5"): 66,
    ("AlphaBeta", "Para"): 88,
    ("AlphaBeta", "ParaSettle"): 55,
    ("AlphaBeta", "AlphaBeta"): 43,
    ("AlphaBeta", "OneResource"): 91,
    ("AlphaBeta", "Dev"): 79,
    ("AlphaBeta", "Road"): 66,
    ("AlphaBeta", "CityRush"): 57,

    # OneResource
    ("OneResource", "Random"): 99,
    ("OneResource", "It1"): 98,
    ("OneResource", "It2"): 99,
    ("OneResource", "It3"): 63,
    ("OneResource", "It4"): 32,
    ("OneResource", "It5"): 23,
    ("OneResource", "Para"): 44,
    ("OneResource", "ParaSettle"): 16,
    ("OneResource", "AlphaBeta"): 8,
    ("OneResource", "OneResource"): 87,
    ("OneResource", "Dev"): 45,
    ("OneResource", "Road"): 20,
    ("OneResource", "CityRush"): 25,

    # Dev
    ("Dev", "Random"): 100,
    ("Dev", "It1"): 97,
    ("Dev", "It2"): 97,
    ("Dev", "It3"): 72,
    ("Dev", "It4"): 46,
    ("Dev", "It5"): 21,
    ("Dev", "Para"): 61,
    ("Dev", "ParaSettle"): 27,
    ("Dev", "AlphaBeta"): 19,
    ("Dev", "OneResource"): 55,
    ("Dev", "Dev"): 55,
    ("Dev", "Road"): 25,
    ("Dev", "CityRush"): 37,

    # Road
    ("Road", "Random"): 100,
    ("Road", "It1"): 100,
    ("Road", "It2"): 100,
    ("Road", "It3"): 88,
    ("Road", "It4"): 73,
    ("Road", "It5"): 51,
    ("Road", "Para"): 75,
    ("Road", "ParaSettle"): 51,
    ("Road", "AlphaBeta"): 36,
    ("Road", "OneResource"): 74,
    ("Road", "Dev"): 64,
    ("Road", "Road"): 58,
    ("Road", "CityRush"): 51,

    # CityRush
    ("CityRush", "Random"): 100,
    ("CityRush", "It1"): 100,
    ("CityRush", "It2"): 100,
    ("CityRush", "It3"): 91,
    ("CityRush", "It4"): 71,
    ("CityRush", "It5"): 45,
    ("CityRush", "Para"): 71,
    ("CityRush", "ParaSettle"): 37,
    ("CityRush", "AlphaBeta"): 31,
    ("CityRush", "OneResource"): 74,
    ("CityRush", "Dev"): 74,
    ("CityRush", "Road"): 47,
    ("CityRush", "CityRush"): 54,
}

diff = {
    (a, b): winrate_first[(a, b)] - (100 - winrate_first[(b, a)])
    for a in players for b in players
}

# Prepare data for heatmap
matrix = np.zeros((len(players), len(players)))
for i, player_a in enumerate(players):
    for j, player_b in enumerate(players):
        matrix[i, j] = diff[(player_a, player_b)]
# Plot heatmap
plt.figure(figsize=(10, 8))
plt.imshow(matrix, cmap='bwr', vmin=-20, vmax=20)
plt.colorbar(label='Win Rate Difference (%)')
plt.xticks(ticks=np.arange(len(players)), labels=players, rotation=45, ha='right')
plt.yticks(ticks=np.arange(len(players)), labels=players)
# plt.xlabel('Gracz Drugi')
plt.ylabel('Gracz Rozpoczynający')
plt.title('Różnica współczynnika wygranych\n(Wygrane pierwszego gracza - Wygrane drugiego gracza)')
plt.tight_layout()
plt.show()


