#!/usr/bin/env python3
"""
Skrypt do wizualizacji wyników Scenariusza 4: Porównanie wszystkich strategii wyspecjalizowanych
(OneResourcePlayer, DevPlayer, RoadPlayer, CityRushPlayer).

Użycie:
    python research/visualize_scenario4_all_specialized.py --output-dir docs/
"""

import argparse
from pathlib import Path
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns

# Konfiguracja stylu
sns.set_style("whitegrid")
plt.rcParams['figure.figsize'] = (14, 8)
plt.rcParams['font.size'] = 10

# Dane z eksperymentów - wszystkie 4 strategie wyspecjalizowane (z tabel w docs/thesis.md)
DATA = {
    'OneResource': {
        'It1': 98.3, 'It2': 95.5, 'It3': 56.8, 'It4': 36.4, 'It5': 19.0,
        'Para': 34.8, 'ParaSettleIt5': 18.2, 'AlphaBeta': 15.8
    },
    'Dev': {
        'It1': 98.8, 'It2': 97.5, 'It3': 68.1, 'It4': 45.8, 'It5': 24.9,
        'Para': 58.0, 'ParaSettleIt5': 23.6, 'AlphaBeta': 18.3
    },
    'Road': {
        'It1': 100.0, 'It2': 99.2, 'It3': 86.7, 'It4': 68.2, 'It5': 47.5,
        'Para': 74.2, 'ParaSettleIt5': 43.9, 'AlphaBeta': 33.7
    },
    'CityRush': {
        'It1': 99.9, 'It2': 99.7, 'It3': 85.3, 'It4': 70.5, 'It5': 45.7,
        'Para': 94.1, 'ParaSettleIt5': 70.0, 'AlphaBeta': 37.4
    }
}

# Kolejność przeciwników
OPPONENTS = ['It1', 'It2', 'It3', 'It4', 'It5', 'Para', 'ParaSettleIt5', 'AlphaBeta']


# Kolory i etykiety dla 4 strategii
STRATEGIES = ['OneResource', 'Dev', 'Road', 'CityRush']
STRATEGY_COLORS = {
    'OneResource': '#e74c3c',
    'Dev': '#f39c12',
    'Road': '#3498db',
    'CityRush': '#27ae60'
}
STRATEGY_LABELS = {
    'OneResource': 'OneResourcePlayer',
    'Dev': 'DevPlayer',
    'Road': 'RoadPlayer',
    'CityRush': 'CityRushPlayer'
}


def plot_all_specialized_comparison(output_dir: Path):
    """Wykres porównujący wszystkie 4 strategie wyspecjalizowane"""
    fig, ax = plt.subplots(figsize=(14, 8))
    
    x = np.arange(len(OPPONENTS))
    n_strategies = len(STRATEGIES)
    width = 0.8 / n_strategies
    offset = (n_strategies - 1) * width / 2
    
    all_bars = []
    for i, strat in enumerate(STRATEGIES):
        pos = x - offset + i * width
        bars = ax.bar(pos, [DATA[strat][opp] for opp in OPPONENTS],
                     width, label=STRATEGY_LABELS[strat], color=STRATEGY_COLORS[strat], alpha=0.7,
                     edgecolor='black', linewidth=1.5)
        all_bars.append(bars)
    
    for bars in all_bars:
        for bar in bars:
            height = bar.get_height()
            if height > 0:
                ax.text(bar.get_x() + bar.get_width()/2., height + 1,
                       f'{height:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=7)
    
    ax.axhline(y=50, color='gray', linestyle='--', linewidth=2, alpha=0.5, label='Granica równości (50%)')
    
    ax.set_xlabel('Przeciwnik', fontsize=14, fontweight='bold')
    ax.set_ylabel('Współczynnik zwycięstw (%)', fontsize=14, fontweight='bold')

    ax.set_xticks(x)
    ax.set_xticklabels(OPPONENTS)
    ax.set_ylim([0, 105])
    ax.grid(True, alpha=0.3, axis='y')
    ax.legend(loc='upper right', fontsize=10)
    
    plt.tight_layout()
    plt.savefig(output_dir / 'scenario4_all_slupki.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'scenario4_all_slupki.png'}")


def plot_all_specialized_lines(output_dir: Path):
    """Wykres liniowy porównujący wszystkie 4 strategie"""
    fig, ax = plt.subplots(figsize=(14, 8))
    
    x = np.arange(len(OPPONENTS))
    markers = ['o', 's', '^', 'D']
    
    for i, strat in enumerate(STRATEGIES):
        ax.plot(x, [DATA[strat][opp] for opp in OPPONENTS],
                f'{markers[i]}-', linewidth=2.5, markersize=9, color=STRATEGY_COLORS[strat],
                label=STRATEGY_LABELS[strat])
    
    for i, opp in enumerate(OPPONENTS):
        for strat in STRATEGIES:
            val = DATA[strat][opp]
            ax.text(i, val + 1.5, f'{val:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=6,
                   color=STRATEGY_COLORS[strat])
    
    ax.axhline(y=50.0, color='gray', linestyle='--', linewidth=2, alpha=0.5, label='Linia remisu (50%)')
    ax.axvline(x=2, color='red', linestyle='--', linewidth=2, alpha=0.5, label='Przepaść kompetencyjna')
    ax.axvline(x=3, color='orange', linestyle='--', linewidth=2, alpha=0.5, label='Punkt przełomowy')
    
    ax.set_xlabel('Przeciwnik', fontsize=14, fontweight='bold')
    ax.set_ylabel('Współczynnik zwycięstw (%)', fontsize=14, fontweight='bold')
    ax.set_title('Porównanie skuteczności strategii wyspecjalizowanych', fontsize=16, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(OPPONENTS)
    ax.set_ylim([0, 105])
    ax.grid(True, alpha=0.3)
    ax.legend(loc='upper right', fontsize=10)
    
    plt.tight_layout()
    plt.savefig(output_dir / 'scenario4_all_specialized_lines.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'scenario4_all_specialized_lines.png'}")


def plot_effectiveness_ranking(output_dir: Path):
    """Wykres pokazujący ranking skuteczności 4 strategii dla każdego przeciwnika"""
    fig, axes = plt.subplots(2, 4, figsize=(16, 8))
    fig.suptitle('Ranking skuteczności strategii wyspecjalizowanych dla każdego przeciwnika',
                 fontsize=16, fontweight='bold')
    
    short_labels = {s: STRATEGY_LABELS[s].replace('Player', '') for s in STRATEGIES}
    
    for idx, opp in enumerate(OPPONENTS):
        row = idx // 4
        col = idx % 4
        ax = axes[row, col]
        
        win_rates = [(DATA[strat][opp], strat) for strat in STRATEGIES]
        win_rates.sort(reverse=True)
        
        bars = ax.barh(range(len(win_rates)), [wr[0] for wr in win_rates],
                      color=[STRATEGY_COLORS[wr[1]] for wr in win_rates], alpha=0.7,
                      edgecolor='black', linewidth=1.5)
        
        for i, (bar, (wr, strat)) in enumerate(zip(bars, win_rates)):
            w = bar.get_width()
            ax.text(w + 1, bar.get_y() + bar.get_height()/2, f'{wr:.1f}%',
                   ha='left', va='center', fontweight='bold', fontsize=8)
        
        ax.set_yticks(range(len(win_rates)))
        ax.set_yticklabels([short_labels[wr[1]] for wr in win_rates])
        ax.set_xlabel('Win Rate (%)', fontsize=10, fontweight='bold')
        ax.set_title(f'vs {opp}', fontsize=12, fontweight='bold')
        ax.set_xlim([0, 105])
        ax.grid(True, alpha=0.3, axis='x')
        ax.axvline(x=50, color='gray', linestyle=':', linewidth=1, alpha=0.5)
    
    plt.tight_layout()
    plt.savefig(output_dir / 'scenario4_all_specialized_ranking.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'scenario4_all_specialized_ranking.png'}")


def plot_competency_gap_comparison(output_dir: Path):
    """Wykres pokazujący przepaść kompetencyjną dla wszystkich 4 strategii"""
    fig, ax = plt.subplots(figsize=(14, 8))
    
    x = np.arange(len(OPPONENTS))
    markers = ['o', 's', '^', 'D']
    
    for i, strat in enumerate(STRATEGIES):
        ax.plot(x, [DATA[strat][opp] for opp in OPPONENTS],
                f'{markers[i]}-', linewidth=2.5, markersize=10, color=STRATEGY_COLORS[strat],
                label=STRATEGY_LABELS[strat])
    
    ax.axhline(y=50.0, color='gray', linestyle='--', linewidth=2, alpha=0.5, label='Linia remisu (50%)')
    
    ax.set_xlabel('Przeciwnik', fontsize=14, fontweight='bold')
    ax.set_ylabel('Współczynnik zwycięstw (%)', fontsize=14, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(OPPONENTS)
    ax.set_ylim([0, 105])
    ax.grid(True, alpha=0.3)
    ax.legend(loc='upper right', fontsize=10)
    
    plt.tight_layout()
    plt.savefig(output_dir / 'scenario4_all.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'scenario4_all.png'}")


def main():
    parser = argparse.ArgumentParser(
        description='Wizualizacja wyników Scenariusza 4 - 4 strategie wyspecjalizowane (OR, Dev, Road, CityRush)'
    )
    parser.add_argument('--output-dir', type=Path, default=Path('docs'),
                        help='Katalog wyjściowy dla wykresów')
    args = parser.parse_args()
    
    args.output_dir.mkdir(exist_ok=True)
    
    print("Generowanie wykresów Scenariusza 4 - wszystkie 4 strategie wyspecjalizowane...")
    plot_all_specialized_comparison(args.output_dir)
    plot_all_specialized_lines(args.output_dir)
    plot_effectiveness_ranking(args.output_dir)
    plot_competency_gap_comparison(args.output_dir)
    
    print(f"\nWszystkie wykresy zapisane w: {args.output_dir}")
    print("Wygenerowane wykresy:")
    print("  1. scenario4_all_slupki.png - Porównanie słupkowe")
    print("  2. scenario4_all_specialized_lines.png - Porównanie liniowe")
    print("  3. scenario4_all_specialized_ranking.png - Ranking dla każdego przeciwnika")
    print("  4. scenario4_all.png - Przepaść kompetencyjna dla wszystkich")


if __name__ == '__main__':
    main()
