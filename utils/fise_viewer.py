import matplotlib.pyplot as plt
import numpy as np

# List of bot flags and display names (order must match run_fise.cpp)
bot_flags = [
    "rp", "it1", "it2", "it3", "it4", "it5",
    "para", "psit5", "ab", "or", "dev", "road", "cr"
]
bot_names = [
    "RandomPlayer", "It1Player", "It2Player", "It3Player", "It4Player", "It5Player",
    "ParaPlayer", "ParaSettleIt5Player", "AlphaBetaPlayer", "OneResourcePlayer", "DevPlayer", "RoadPlayer", "CityRushPlayer"
]

 # Layout: two rows, top and bottom, with circles for each bot
n = len(bot_names)
x = np.linspace(-n/2, n/2, n)
row_y = [1, -1]

fig, ax = plt.subplots(figsize=(18, 6))


# Draw circles for each bot in both rows
def draw_bot_circles(ax, x, y, names):
    for i, name in enumerate(names):
        circle = plt.Circle((x[i], y), 0.5, color='skyblue', alpha=0.7, zorder=2)
        ax.add_patch(circle)
        ax.text(x[i], y, name, color='black', ha='center', va='center', fontsize=10, weight='bold', zorder=3)

# Draw top row (first player)
draw_bot_circles(ax, x, row_y[0], bot_flags)
# Draw bottom row (second player)
draw_bot_circles(ax, x, row_y[1], bot_flags)



# --- Parse results from file ---
import re
import os

RESULTS_FILE = os.path.join(os.path.dirname(__file__), 'fise_results_20260130_131345.txt')

# Map display names to flag for lookup
name_to_flag = dict(zip([
    "RandomPlayer", "It1Player", "It2Player", "It3Player", "It4Player", "It5Player",
    "ParaPlayer", "ParaSettleIt5Player", "AlphaBetaPlayer", "OneResourcePlayer", "DevPlayer", "RoadPlayer", "CityRushPlayer"
], bot_flags))

# Build a dict: (first, second) -> (first_wins, second_wins)
matchup_results = {}
with open(RESULTS_FILE, encoding='utf-8') as f:
    lines = f.readlines()

    i = 0
    while i < len(lines):
        if lines[i].startswith('Para: '):
            # Get bot names
            m = re.match(r'Para: (.+) vs (.+)', lines[i])
            if not m:
                i += 1
                continue
            botA, botB = m.group(1).strip(), m.group(2).strip()
            # First order: A (first) vs B (second)
            i += 1
            while i < len(lines) and not lines[i].strip().startswith('Wygrane pierwszego:'):
                i += 1
            if i+1 < len(lines):
                w1 = int(re.search(r'(\d+)', lines[i]).group(1))
                w2 = int(re.search(r'(\d+)', lines[i+1]).group(1))
                matchup_results[(botA, botB)] = (w1, w2)
            # Second order: B (first) vs A (second)
            i += 4
            if i+1 < len(lines) and lines[i].strip().startswith('Wygrane pierwszego:'):
                w1 = int(re.search(r'(\d+)', lines[i]).group(1))
                w2 = int(re.search(r'(\d+)', lines[i+1]).group(1))
                matchup_results[(botB, botA)] = (w1, w2)
        i += 1

# Draw arrows and annotate using parsed results
for i in range(n):
    for j in range(n):
        if i == j:
            continue
        botA = bot_names[i]
        botB = bot_names[j]
        key = (botA, botB)
        if key in matchup_results:
            w1, w2 = matchup_results[key]
            if w1 > w2:
                color = 'green'
                label = f"{bot_flags[i]}>{bot_flags[j]}: {w1}-{w2}"
            elif w2 > w1:
                color = 'red'
                label = f"{bot_flags[i]}<{bot_flags[j]}: {w1}-{w2}"
            else:
                color = 'gray'
                label = f"{bot_flags[i]}={bot_flags[j]}: {w1}-{w2}"
            ax.annotate('', xy=(x[j], row_y[1]), xytext=(x[i], row_y[0]),
                        arrowprops=dict(arrowstyle='->', color=color, lw=1, alpha=0.5), zorder=1)
            mx = (x[i] + x[j]) / 2
            my = (row_y[0] + row_y[1]) / 2
            ax.text(mx, my, label, color=color, fontsize=8, ha='center', va='center', zorder=4)

# Set limits and view
ax.set_xlim(x[0]-1, x[-1]+1)
ax.set_ylim(-2, 2)
ax.axis('off')
plt.tight_layout()
plt.show()
