import matplotlib.pyplot as plt

bots = [
    "Random", "It1", "It2", "It3", "It4", "It5",
    "Para", "ParaSettle", "AlphaBeta",
    "OneResource", "Dev", "Road", "CityRush"
]

# winrate jako pierwszy i drugi (procenty)
win_first = [11.7, 13.9, 17.9, 41.2, 54.4, 70.7, 51.6, 74.1, 76.5, 53.1, 54.2, 69.3, 67.8]
win_second = [10.4, 15.9, 16.1, 41.5, 53.9, 69.7, 48.1, 70.8, 76.9, 43.3, 52.8, 67.8, 69.0]

# różnica first - second
diff = [f - s for f, s in zip(win_first, win_second)]

x = range(len(bots))

plt.figure(figsize=(12,6))
plt.bar(x, diff, color=['green' if d>0 else 'red' for d in diff])
plt.axhline(0, color='black', linewidth=1)
plt.xticks(x, bots, rotation=45, ha='right')
plt.ylabel("Win rate difference [%] (first - second)")
plt.title("Wpływ kolejności ruchu na wyniki botów")
plt.tight_layout()
plt.show()