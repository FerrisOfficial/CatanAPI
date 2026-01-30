#!/usr/bin/env python3
"""
Skrypt do wizualizacji wyników RoadPlayer: Skuteczność w zależności od jakości przeciwnika.

Użycie:
    python utils/visualize_roadplayer.py --output-dir docs/
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

# Dane z eksperymentów - RoadPlayer vs różni przeciwnicy
ROADPLAYER_DATA = {
    'It1': {'win_rate': 100.0, 'avg_turns': 131.2, 'loss_vp_road': 0.00, 'loss_vp_opp': 3.58,
            'lr_pct_road': 95.9, 'lr_pct_opp': 3.9, 'la_pct_road': 96.7, 'la_pct_opp': 2.6,
            'dev_cards_road': 3.5, 'dev_cards_opp': 0.7, 'prod_score_road': 1059.2, 'prod_score_opp': 271.2},
    'It2': {'win_rate': 99.2, 'avg_turns': 134.2, 'loss_vp_road': 11.57, 'loss_vp_opp': 3.96,
            'lr_pct_road': 90.8, 'lr_pct_opp': 9.2, 'la_pct_road': 96.3, 'la_pct_opp': 3.1,
            'dev_cards_road': 3.6, 'dev_cards_opp': 0.6, 'prod_score_road': 1050.7, 'prod_score_opp': 318.9},
    'It3': {'win_rate': 86.7, 'avg_turns': 136.1, 'loss_vp_road': 9.68, 'loss_vp_opp': 8.23,
            'lr_pct_road': 74.3, 'lr_pct_opp': 25.7, 'la_pct_road': 79.6, 'la_pct_opp': 20.3,
            'dev_cards_road': 3.0, 'dev_cards_opp': 1.8, 'prod_score_road': 1066.6, 'prod_score_opp': 732.1},
    'It4': {'win_rate': 68.2, 'avg_turns': 140.2, 'loss_vp_road': 9.54, 'loss_vp_opp': 9.42,
            'lr_pct_road': 47.7, 'lr_pct_opp': 52.3, 'la_pct_road': 78.8, 'la_pct_opp': 21.2,
            'dev_cards_road': 2.9, 'dev_cards_opp': 2.0, 'prod_score_road': 1001.2, 'prod_score_opp': 1007.3},
    'It5': {'win_rate': 47.5, 'avg_turns': 118.9, 'loss_vp_road': 9.06, 'loss_vp_opp': 10.40,
            'lr_pct_road': 77.1, 'lr_pct_opp': 21.8, 'la_pct_road': 33.5, 'la_pct_opp': 66.5,
            'dev_cards_road': 2.2, 'dev_cards_opp': 2.8, 'prod_score_road': 946.5, 'prod_score_opp': 1044.1},
    'OneResource': {'win_rate': 80.4, 'avg_turns': 126.5, 'loss_vp_road': 9.95, 'loss_vp_opp': 7.63,
                    'lr_pct_road': 80.4, 'lr_pct_opp': 19.1, 'la_pct_road': 79.1, 'la_pct_opp': 20.8,
                    'dev_cards_road': 3.0, 'dev_cards_opp': 1.8, 'prod_score_road': 1038.0, 'prod_score_opp': 781.4},
    'Dev': {'win_rate': 72.9, 'avg_turns': 141.6, 'loss_vp_road': 10.80, 'loss_vp_opp': 9.43,
            'lr_pct_road': 92.0, 'lr_pct_opp': 7.8, 'la_pct_road': 25.0, 'la_pct_opp': 75.0,
            'dev_cards_road': 2.1, 'dev_cards_opp': 2.9, 'prod_score_road': 1164.9, 'prod_score_opp': 858.6},
    'CityRush': {'win_rate': 52.9, 'avg_turns': 119.0, 'loss_vp_road': 9.20, 'loss_vp_opp': 10.34,
                 'lr_pct_road': 78.7, 'lr_pct_opp': 20.3, 'la_pct_road': 40.4, 'la_pct_opp': 59.6,
                 'dev_cards_road': 2.3, 'dev_cards_opp': 2.7, 'prod_score_road': 974.9, 'prod_score_opp': 1005.5},
    'Para': {'win_rate': 74.2, 'avg_turns': 134.9, 'loss_vp_road': 8.99, 'loss_vp_opp': 5.14,
             'lr_pct_road': 71.0, 'lr_pct_opp': 28.8, 'la_pct_road': 79.1, 'la_pct_opp': 20.2,
             'dev_cards_road': 2.7, 'dev_cards_opp': 3.2, 'prod_score_road': 1006.6, 'prod_score_opp': 601.1},
    'ParaSettleIt5': {'win_rate': 43.9, 'avg_turns': 118.3, 'loss_vp_road': 9.01, 'loss_vp_opp': 10.84,
                      'lr_pct_road': 78.3, 'lr_pct_opp': 21.0, 'la_pct_road': 28.3, 'la_pct_opp': 71.7,
                      'dev_cards_road': 2.1, 'dev_cards_opp': 2.9, 'prod_score_road': 935.9, 'prod_score_opp': 1059.4},
}

# Kolejność przeciwników (od najsłabszego do najsilniejszego)
OPPONENTS_ORDER = ['It1', 'It2', 'It3', 'It4', 'It5', 'OneResource', 'Dev', 'CityRush', 'Para', 'ParaSettleIt5']


def plot_roadplayer_effectiveness(output_dir: Path):
    """Wykres 1: Skuteczność RoadPlayer w zależności od jakości przeciwnika"""
    fig, ax = plt.subplots(figsize=(12, 7))
    
    opponents = OPPONENTS_ORDER
    win_rates = [ROADPLAYER_DATA[opp]['win_rate'] for opp in opponents]
    
    # Wykres liniowy
    x = np.arange(len(opponents))
    line = ax.plot(x, win_rates, 'o-', linewidth=3, markersize=10, 
                   color='#2ecc71', label='RoadPlayer Win Rate', 
                   markerfacecolor='#2ecc71', markeredgecolor='white', markeredgewidth=2)
    
    # Linia referencyjna na poziomie 50% (remis)
    ax.axhline(y=50.0, color='grey', linestyle='--', linewidth=2, 
               alpha=0.7, label='Linia remisu (50%)')
    
    # Dodaj wartości na punktach
    for i, (opp, wr) in enumerate(zip(opponents, win_rates)):
        ax.text(i, wr + 2, f'{wr:.1f}%', ha='center', va='bottom', 
                fontweight='bold', fontsize=9)
    
    # Konfiguracja osi
    ax.set_xlabel('Przeciwnik', fontsize=13, fontweight='bold')
    ax.set_ylabel('Współczynnik zwycięstw RoadPlayer (%)', fontsize=13, fontweight='bold')

    ax.set_xticks(x)
    ax.set_xticklabels(opponents, rotation=0, ha='center')
    ax.set_ylim([0, 105])
    ax.grid(True, alpha=0.3, linestyle='-', linewidth=0.5)
    ax.legend(loc='upper right', fontsize=11, framealpha=0.9)
    
    plt.tight_layout()
    plt.savefig(output_dir / 'road_effectiveness.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'road_effectiveness.png'}")


def plot_longest_road_comparison(output_dir: Path):
    """Wykres 2: Porównanie dominacji w premii Najdłuższa Droga"""
    fig, ax = plt.subplots(figsize=(12, 7))
    
    opponents = OPPONENTS_ORDER
    lr_road = [ROADPLAYER_DATA[opp]['lr_pct_road'] for opp in opponents]
    lr_opp = [ROADPLAYER_DATA[opp]['lr_pct_opp'] for opp in opponents]
    
    x = np.arange(len(opponents))
    width = 0.35
    
    # Wykres słupkowy grupowy
    bars1 = ax.bar(x - width/2, lr_road, width, label='RoadPlayer', 
                   color='#2ecc71', alpha=0.8, edgecolor='black', linewidth=1.5)
    bars2 = ax.bar(x + width/2, lr_opp, width, label='Przeciwnik', 
                   color='#e74c3c', alpha=0.8, edgecolor='black', linewidth=1.5)
    
    # Dodaj wartości na słupkach
    for bars in [bars1, bars2]:
        for bar in bars:
            height = bar.get_height()
            if height > 0:
                ax.text(bar.get_x() + bar.get_width()/2., height + 1,
                       f'{height:.1f}%', ha='center', va='bottom', 
                       fontweight='bold', fontsize=8)
    
    # Linia referencyjna na poziomie 50%
    ax.axhline(y=50.0, color='gray', linestyle='--', linewidth=1.5, 
               alpha=0.5, label='Linia remisu (50%)')
    
    # Konfiguracja osi
    ax.set_xlabel('Przeciwnik', fontsize=13, fontweight='bold')
    ax.set_ylabel('Częstość zdobycia premii Najdłuższa Droga (%)', fontsize=13, fontweight='bold')

    ax.set_xticks(x)
    ax.set_xticklabels(opponents, rotation=0, ha='center')
    ax.set_ylim([0, 105])
    ax.grid(True, alpha=0.3, linestyle='-', linewidth=0.5, axis='y')
    ax.legend(loc='upper right', fontsize=11, framealpha=0.9)
    
    plt.tight_layout()
    plt.savefig(output_dir / 'road_longest_road.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'road_longest_road.png'}")


def main():
    parser = argparse.ArgumentParser(description='Wizualizacja wyników RoadPlayer')
    parser.add_argument('--output-dir', type=Path, default=Path('plots'),
                       help='Katalog wyjściowy dla wykresów')
    args = parser.parse_args()
    
    args.output_dir.mkdir(exist_ok=True)
    
    print("Generowanie wykresów RoadPlayer...")
    plot_roadplayer_effectiveness(args.output_dir)
    plot_longest_road_comparison(args.output_dir)
    
    print(f"\nWszystkie wykresy zapisane w: {args.output_dir}")
    print("\nWygenerowane wykresy:")
    print("  1. roadplayer_1_effectiveness.png - Skuteczność w zależności od jakości przeciwnika")
    print("  2. roadplayer_2_longest_road.png - Porównanie premii Najdłuższa Droga")


if __name__ == '__main__':
    main()
