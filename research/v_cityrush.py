#!/usr/bin/env python3
"""
Skrypt do wizualizacji wyników CityRushPlayer: Skuteczność w zależności od jakości przeciwnika.

Użycie:
    python research/v_cityrush.py --output-dir docs/
"""

import argparse
from pathlib import Path
import matplotlib.pyplot as plt
import numpy as np
import seaborn as sns

# Konfiguracja stylu
sns.set_style("whitegrid")
plt.rcParams['figure.figsize'] = (12, 8)
plt.rcParams['font.size'] = 10

# Dane z eksperymentów - CityRushPlayer vs różni przeciwnicy (z tabeli w thesis.md)
CITYRUSH_DATA = {
    'It1': {'win_rate': 99.9, 'avg_turns': 116.2, 'loss_vp_cr': 5.00, 'loss_vp_opp': 3.61,
            'lr_pct_cr': 72.4, 'lr_pct_opp': 15.9, 'la_pct_cr': 97.0, 'la_pct_opp': 2.1,
            'dev_cards_cr': 3.6, 'dev_cards_opp': 0.6, 'prod_score_cr': 1079.3, 'prod_score_opp': 257.9},
    'It2': {'win_rate': 99.7, 'avg_turns': 118.2, 'loss_vp_cr': 11.67, 'loss_vp_opp': 4.21,
            'lr_pct_cr': 58.3, 'lr_pct_opp': 33.7, 'la_pct_cr': 97.7, 'la_pct_opp': 1.8,
            'dev_cards_cr': 3.7, 'dev_cards_opp': 0.5, 'prod_score_cr': 1093.1, 'prod_score_opp': 300.1},
    'It3': {'win_rate': 85.3, 'avg_turns': 131.7, 'loss_vp_cr': 10.95, 'loss_vp_opp': 8.35,
            'lr_pct_cr': 39.8, 'lr_pct_opp': 59.3, 'la_pct_cr': 85.9, 'la_pct_opp': 14.0,
            'dev_cards_cr': 3.2, 'dev_cards_opp': 1.7, 'prod_score_cr': 1063.5, 'prod_score_opp': 706.7},
    'It4': {'win_rate': 70.5, 'avg_turns': 137.7, 'loss_vp_cr': 11.00, 'loss_vp_opp': 9.30,
            'lr_pct_cr': 18.2, 'lr_pct_opp': 81.7, 'la_pct_cr': 83.1, 'la_pct_opp': 16.8,
            'dev_cards_cr': 3.1, 'dev_cards_opp': 1.8, 'prod_score_cr': 1035.7, 'prod_score_opp': 946.4},
    'It5': {'win_rate': 45.7, 'avg_turns': 114.8, 'loss_vp_cr': 9.85, 'loss_vp_opp': 9.83,
            'lr_pct_cr': 48.5, 'lr_pct_opp': 49.0, 'la_pct_cr': 38.7, 'la_pct_opp': 61.1,
            'dev_cards_cr': 2.4, 'dev_cards_opp': 2.6, 'prod_score_cr': 978.0, 'prod_score_opp': 990.6},
    'OneResource': {'win_rate': 74.4, 'avg_turns': 118.6, 'loss_vp_cr': 10.82, 'loss_vp_opp': 7.51,
                    'lr_pct_cr': 50.7, 'lr_pct_opp': 44.6, 'la_pct_cr': 83.8, 'la_pct_opp': 16.0,
                    'dev_cards_cr': 3.2, 'dev_cards_opp': 1.7, 'prod_score_cr': 1025.8, 'prod_score_opp': 806.3},
    'Dev': {'win_rate': 69.8, 'avg_turns': 132.8, 'loss_vp_cr': 10.71, 'loss_vp_opp': 8.55,
            'lr_pct_cr': 72.5, 'lr_pct_opp': 24.7, 'la_pct_cr': 36.7, 'la_pct_opp': 63.3,
            'dev_cards_cr': 2.3, 'dev_cards_opp': 2.7, 'prod_score_cr': 1083.3, 'prod_score_opp': 816.6},
    'Road': {'win_rate': 49.6, 'avg_turns': 119.1, 'loss_vp_cr': 10.62, 'loss_vp_opp': 8.94,
             'lr_pct_cr': 21.8, 'lr_pct_opp': 77.4, 'la_pct_cr': 62.3, 'la_pct_opp': 37.7,
             'dev_cards_cr': 2.8, 'dev_cards_opp': 2.2, 'prod_score_cr': 1025.8, 'prod_score_opp': 949.6},
    'Para': {'win_rate': 94.1, 'avg_turns': 122.5, 'loss_vp_cr': 9.42, 'loss_vp_opp': 4.62,
             'lr_pct_cr': 67.9, 'lr_pct_opp': 25.5, 'la_pct_cr': 96.0, 'la_pct_opp': 3.4,
             'dev_cards_cr': 2.8, 'dev_cards_opp': 5.4, 'prod_score_cr': 1062.0, 'prod_score_opp': 412.5},
    'ParaSettleIt5': {'win_rate': 70.0, 'avg_turns': 122.8, 'loss_vp_cr': 10.62, 'loss_vp_opp': 8.17,
                     'lr_pct_cr': 63.6, 'lr_pct_opp': 29.0, 'la_pct_cr': 56.3, 'la_pct_opp': 43.7,
                     'dev_cards_cr': 2.7, 'dev_cards_opp': 2.3, 'prod_score_cr': 1050.2, 'prod_score_opp': 913.1},
    'AlphaBeta': {'win_rate': 37.4, 'avg_turns': 112.1, 'loss_vp_cr': 10.08, 'loss_vp_opp': 9.77,
                  'lr_pct_cr': 23.9, 'lr_pct_opp': 75.3, 'la_pct_cr': 58.4, 'la_pct_opp': 41.3,
                  'dev_cards_cr': 2.6, 'dev_cards_opp': 2.1, 'prod_score_cr': 941.2, 'prod_score_opp': 1063.1},
}

# Kolejność przeciwników (od najsłabszego do najsilniejszego)
OPPONENTS_ORDER = [
    'It1', 'It2', 'It3', 'It4', 'It5', 'OneResource', 'Dev', 'Road',
    'Para', 'ParaSettleIt5', 'AlphaBeta',
]


def plot_cityrush_effectiveness(output_dir: Path):
    """Wykres: Skuteczność CityRushPlayer w zależności od jakości przeciwnika"""
    fig, ax = plt.subplots(figsize=(12, 7))

    opponents = OPPONENTS_ORDER
    win_rates = [CITYRUSH_DATA[opp]['win_rate'] for opp in opponents]

    x = np.arange(len(opponents))
    ax.plot(x, win_rates, 'o-', linewidth=3, markersize=10,
            color='#3498db', label='CityRushPlayer Win Rate',
            markerfacecolor='#3498db', markeredgecolor='white', markeredgewidth=2)

    ax.axhline(y=50.0, color='grey', linestyle='--', linewidth=2,
               alpha=0.7, label='Linia remisu (50%)')

    for i, (opp, wr) in enumerate(zip(opponents, win_rates)):
        ax.text(i, wr + 2, f'{wr:.1f}%', ha='center', va='bottom',
                fontweight='bold', fontsize=9)

    ax.set_xlabel('Przeciwnik', fontsize=13, fontweight='bold')
    ax.set_ylabel('Współczynnik zwycięstw CityRushPlayer (%)', fontsize=13, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(opponents, rotation=45, ha='right')
    ax.set_ylim([0, 105])
    ax.grid(True, alpha=0.3, linestyle='-', linewidth=0.5)
    ax.legend(loc='upper right', fontsize=11, framealpha=0.9)

    plt.tight_layout()
    output_dir.mkdir(parents=True, exist_ok=True)
    plt.savefig(output_dir / 'cityrush_effectiveness.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'cityrush_effectiveness.png'}")


def main():
    parser = argparse.ArgumentParser(description='Wizualizacja wyników CityRushPlayer')
    parser.add_argument('--output-dir', type=Path, default=Path('docs'),
                        help='Katalog wyjściowy dla wykresów (domyślnie: docs)')
    args = parser.parse_args()

    print("Generowanie wykresu CityRushPlayer...")
    plot_cityrush_effectiveness(args.output_dir)
    print(f"\nWykres zapisany w: {args.output_dir}")
    print("  1. cityrush_effectiveness.png - Skuteczność w zależności od jakości przeciwnika")


if __name__ == '__main__':
    main()
