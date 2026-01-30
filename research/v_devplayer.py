#!/usr/bin/env python3
"""
Skrypt do wizualizacji wyników DevPlayer: Skuteczność w zależności od jakości przeciwnika.

Użycie:
    python utils/visualize_devplayer.py --output-dir docs/
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

# Dane z eksperymentów - DevPlayer vs różni przeciwnicy
DEVPLAYER_DATA = {
    'It1': {'win_rate': 98.8, 'avg_turns': 164.4, 'loss_vp_dev': 11.00, 'loss_vp_opp': 4.57,
            'lr_pct_dev': 52.6, 'lr_pct_opp': 36.1, 'la_pct_dev': 98.9, 'la_pct_opp': 1.0,
            'dev_cards_dev': 4.5, 'dev_cards_opp': 0.6, 'prod_score_dev': 1067.2, 'prod_score_opp': 322.6},
    'It2': {'win_rate': 97.5, 'avg_turns': 163.0, 'loss_vp_dev': 11.56, 'loss_vp_opp': 5.20,
            'lr_pct_dev': 39.4, 'lr_pct_opp': 53.7, 'la_pct_dev': 99.4, 'la_pct_opp': 0.6,
            'dev_cards_dev': 4.6, 'dev_cards_opp': 0.6, 'prod_score_dev': 1083.6, 'prod_score_opp': 373.3},
    'It3': {'win_rate': 68.1, 'avg_turns': 172.1, 'loss_vp_dev': 10.44, 'loss_vp_opp': 10.22,
            'lr_pct_dev': 23.2, 'lr_pct_opp': 76.5, 'la_pct_dev': 90.7, 'la_pct_opp': 9.3,
            'dev_cards_dev': 3.4, 'dev_cards_opp': 1.7, 'prod_score_dev': 1031.3, 'prod_score_opp': 922.0},
    'It4': {'win_rate': 45.8, 'avg_turns': 183.2, 'loss_vp_dev': 10.16, 'loss_vp_opp': 11.05,
            'lr_pct_dev': 8.1, 'lr_pct_opp': 91.9, 'la_pct_dev': 90.8, 'la_pct_opp': 9.2,
            'dev_cards_dev': 3.4, 'dev_cards_opp': 1.8, 'prod_score_dev': 929.1, 'prod_score_opp': 1201.8},
    'It5': {'win_rate': 24.9, 'avg_turns': 129.0, 'loss_vp_dev': 8.61, 'loss_vp_opp': 10.96,
            'lr_pct_dev': 23.6, 'lr_pct_opp': 72.4, 'la_pct_dev': 59.8, 'la_pct_opp': 40.2,
            'dev_cards_dev': 2.6, 'dev_cards_opp': 2.4, 'prod_score_dev': 809.0, 'prod_score_opp': 1133.4},
    'OneResource': {'win_rate': 52.7, 'avg_turns': 147.5, 'loss_vp_dev': 10.38, 'loss_vp_opp': 8.46,
                    'lr_pct_dev': 33.3, 'lr_pct_opp': 63.5, 'la_pct_dev': 89.7, 'la_pct_opp': 10.3,
                    'dev_cards_dev': 3.5, 'dev_cards_opp': 1.6, 'prod_score_dev': 948.1, 'prod_score_opp': 989.7},
    'Road': {'win_rate': 27.1, 'avg_turns': 139.6, 'loss_vp_dev': 9.52, 'loss_vp_opp': 10.98,
             'lr_pct_dev': 6.8, 'lr_pct_opp': 92.8, 'la_pct_dev': 73.9, 'la_pct_opp': 26.1,
             'dev_cards_dev': 3.0, 'dev_cards_opp': 2.1, 'prod_score_dev': 868.4, 'prod_score_opp': 1164.1},
    'CityRush': {'win_rate': 31.0, 'avg_turns': 135.6, 'loss_vp_dev': 8.84, 'loss_vp_opp': 10.87,
                'lr_pct_dev': 28.5, 'lr_pct_opp': 69.1, 'la_pct_dev': 65.0, 'la_pct_opp': 35.0,
                'dev_cards_dev': 2.7, 'dev_cards_opp': 2.3, 'prod_score_dev': 837.0, 'prod_score_opp': 1095.4},
    'Para': {'win_rate': 58.0, 'avg_turns': 160.4, 'loss_vp_dev': 8.46, 'loss_vp_opp': 5.54,
             'lr_pct_dev': 20.9, 'lr_pct_opp': 78.6, 'la_pct_dev': 92.3, 'la_pct_opp': 7.5,
             'dev_cards_dev': 3.2, 'dev_cards_opp': 3.4, 'prod_score_dev': 906.9, 'prod_score_opp': 747.6},
    'ParaSettleIt5': {'win_rate': 23.6, 'avg_turns': 129.2, 'loss_vp_dev': 8.52, 'loss_vp_opp': 11.31,
                      'lr_pct_dev': 23.1, 'lr_pct_opp': 73.2, 'la_pct_dev': 54.4, 'la_pct_opp': 45.6,
                      'dev_cards_dev': 2.6, 'dev_cards_opp': 2.4, 'prod_score_dev': 797.5, 'prod_score_opp': 1141.4},
    'AlphaBeta': {'win_rate': 18.3, 'avg_turns': 122.6, 'loss_vp_dev': 4.64, 'loss_vp_opp': 10.56,
                  'lr_pct_dev': 8.2, 'lr_pct_opp': 91.6, 'la_pct_dev': 74.3, 'la_pct_opp': 25.7,
                  'dev_cards_dev': 2.9, 'dev_cards_opp': 2.0, 'prod_score_dev': 747.3, 'prod_score_opp': 1195.7}
}

# Kolejność przeciwników (od najsłabszego do najsilniejszego)
OPPONENTS_ORDER = ['It1', 'It2', 'It3', 'It4', 'Para', 'It5', 'OneResource', 'Road', 'CityRush', 'ParaSettleIt5', 'AlphaBeta']


def plot_devplayer_effectiveness(output_dir: Path):
    """Wykres 1: Skuteczność DevPlayer w zależności od jakości przeciwnika"""
    fig, ax = plt.subplots(figsize=(12, 7))
    
    opponents = OPPONENTS_ORDER
    win_rates = [DEVPLAYER_DATA[opp]['win_rate'] for opp in opponents]
    
    # Wykres liniowy
    x = np.arange(len(opponents))
    line = ax.plot(x, win_rates, 'o-', linewidth=3, markersize=10, 
                   color='#f39c12', label='DevPlayer Win Rate', 
                   markerfacecolor="#cd840f", markeredgecolor='white', markeredgewidth=2)
    
    # Linia referencyjna na poziomie 50% (remis)
    ax.axhline(y=50.0, color='grey', linestyle='--', linewidth=2, 
               alpha=0.7, label='Linia remisu (50%)')
    
    # Dodaj wartości na punktach
    for i, (opp, wr) in enumerate(zip(opponents, win_rates)):
        ax.text(i, wr + 2, f'{wr:.1f}%', ha='center', va='bottom', 
                fontweight='bold', fontsize=9)
    
    # Konfiguracja osi
    ax.set_xlabel('Przeciwnik', fontsize=13, fontweight='bold')
    ax.set_ylabel('Współczynnik zwycięstw DevPlayer (%)', fontsize=13, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(opponents, rotation=0, ha='center')
    ax.set_ylim([0, 105])
    ax.grid(True, alpha=0.3, linestyle='-', linewidth=0.5)
    ax.legend(loc='upper right', fontsize=11, framealpha=0.9)
    
    plt.tight_layout()
    plt.savefig(output_dir / 'devplayer_effectiveness.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'devplayer_effectiveness.png'}")


def plot_largest_army_comparison(output_dir: Path):
    """Wykres 2: Porównanie przewagi w premii Największa Armia"""
    fig, ax = plt.subplots(figsize=(12, 7))
    
    opponents = OPPONENTS_ORDER
    la_dev = [DEVPLAYER_DATA[opp]['la_pct_dev'] for opp in opponents]
    la_opp = [DEVPLAYER_DATA[opp]['la_pct_opp'] for opp in opponents]
    
    x = np.arange(len(opponents))
    width = 0.35
    
    # Wykres słupkowy grupowy
    bars1 = ax.bar(x - width/2, la_dev, width, label='DevPlayer', 
                   color='#3498db', alpha=0.8, edgecolor='black', linewidth=1.5)
    bars2 = ax.bar(x + width/2, la_opp, width, label='Przeciwnik', 
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
    ax.axhline(y=50.0, color='gray', linestyle='--', linewidth=2, 
               alpha=0.5, label='Linia remisu (50%)')
    
    # Konfiguracja osi
    ax.set_xlabel('Przeciwnik', fontsize=13, fontweight='bold')
    ax.set_ylabel('Częstość zdobycia premii Największa Armia (%)', fontsize=13, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(opponents, rotation=0, ha='center')
    ax.set_ylim([0, 105])
    ax.grid(True, alpha=0.3, linestyle='-', linewidth=0.5, axis='y')
    ax.legend(loc='upper right', fontsize=11, framealpha=0.9)
    
    plt.tight_layout()
    plt.savefig(output_dir / 'devplayer_largest_army.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'devplayer_largest_army.png'}")


def plot_dev_cards_comparison(output_dir: Path):
    """Wykres 3: Porównanie liczby kupionych kart rozwoju"""
    fig, ax = plt.subplots(figsize=(12, 7))
    
    opponents = OPPONENTS_ORDER
    dev_cards_dev = [DEVPLAYER_DATA[opp]['dev_cards_dev'] for opp in opponents]
    dev_cards_opp = [DEVPLAYER_DATA[opp]['dev_cards_opp'] for opp in opponents]
    
    x = np.arange(len(opponents))
    width = 0.35
    
    # Wykres słupkowy grupowy
    bars1 = ax.bar(x - width/2, dev_cards_dev, width, label='DevPlayer', 
                   color='#2ecc71', alpha=0.8, edgecolor='black', linewidth=1.5)
    bars2 = ax.bar(x + width/2, dev_cards_opp, width, label='Przeciwnik', 
                   color='#e67e22', alpha=0.8, edgecolor='black', linewidth=1.5)
    
    # Dodaj wartości na słupkach
    for bars in [bars1, bars2]:
        for bar in bars:
            height = bar.get_height()
            if height > 0:
                ax.text(bar.get_x() + bar.get_width()/2., height + 0.05,
                       f'{height:.1f}', ha='center', va='bottom', 
                       fontweight='bold', fontsize=9)
    
    # Konfiguracja osi
    ax.set_xlabel('Przeciwnik', fontsize=13, fontweight='bold')
    ax.set_ylabel('Średnia liczba kupionych kart rozwoju', fontsize=13, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(opponents, rotation=0, ha='center')
    ax.set_ylim([0, max(max(dev_cards_dev), max(dev_cards_opp)) * 1.2])
    ax.grid(True, alpha=0.3, linestyle='-', linewidth=0.5, axis='y')
    ax.legend(loc='upper right', fontsize=11, framealpha=0.9)
    
    plt.tight_layout()
    plt.savefig(output_dir / 'devplayer_dev_cards.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'devplayer_dev_cards.png'}")


def main():
    parser = argparse.ArgumentParser(description='Wizualizacja wyników DevPlayer')
    parser.add_argument('--output-dir', type=Path, default=Path('plots'),
                       help='Katalog wyjściowy dla wykresów')
    args = parser.parse_args()
    
    args.output_dir.mkdir(exist_ok=True)
    
    print("Generowanie wykresów DevPlayer...")
    plot_devplayer_effectiveness(args.output_dir)
    plot_largest_army_comparison(args.output_dir)
    plot_dev_cards_comparison(args.output_dir)
    
    print(f"\nWszystkie wykresy zapisane w: {args.output_dir}")
    print("Wygenerowane pliki:")
    print("  1. devplayer_effectiveness.png - Skuteczność w zależności od jakości przeciwnika")
    print("  2. devplayer_largest_army.png - Porównanie premii Największa Armia")
    print("  3. devplayer_dev_cards.png - Porównanie liczby kupionych kart rozwoju")


if __name__ == '__main__':
    main()
