#!/usr/bin/env python3
"""
Skrypt do wizualizacji wyników Scenariusza 5: RoadPlayer vs różne boty.

Użycie:
    python utils/visualize_scenario5_road.py --output-dir docs/
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

# Dane z eksperymentów
DATA = {
    'It1': {'win_rate': 100.0, 'avg_turns': 131.2, 'loss_vp_road': 0.00, 'loss_vp_opp': 3.58,
            'lr_road': 95.9, 'lr_opp': 3.9, 'la_road': 96.7, 'la_opp': 2.6,
            'dev_road': 3.5, 'dev_opp': 0.7, 'prod_road': 1059.2, 'prod_opp': 271.2},
    'It2': {'win_rate': 99.2, 'avg_turns': 134.2, 'loss_vp_road': 11.57, 'loss_vp_opp': 3.96,
            'lr_road': 90.8, 'lr_opp': 9.2, 'la_road': 96.3, 'la_opp': 3.1,
            'dev_road': 3.6, 'dev_opp': 0.6, 'prod_road': 1050.7, 'prod_opp': 318.9},
    'It3': {'win_rate': 86.7, 'avg_turns': 136.1, 'loss_vp_road': 9.68, 'loss_vp_opp': 8.23,
            'lr_road': 74.3, 'lr_opp': 25.7, 'la_road': 79.6, 'la_opp': 20.3,
            'dev_road': 3.0, 'dev_opp': 1.8, 'prod_road': 1066.6, 'prod_opp': 732.1},
    'It4': {'win_rate': 68.2, 'avg_turns': 140.2, 'loss_vp_road': 9.54, 'loss_vp_opp': 9.42,
            'lr_road': 47.7, 'lr_opp': 52.3, 'la_road': 78.8, 'la_opp': 21.2,
            'dev_road': 2.9, 'dev_opp': 2.0, 'prod_road': 1001.2, 'prod_opp': 1007.3},
    'It5': {'win_rate': 47.5, 'avg_turns': 118.9, 'loss_vp_road': 9.06, 'loss_vp_opp': 10.40,
            'lr_road': 77.1, 'lr_opp': 21.8, 'la_road': 33.5, 'la_opp': 66.5,
            'dev_road': 2.2, 'dev_opp': 2.8, 'prod_road': 946.5, 'prod_opp': 1044.1},
    'Para': {'win_rate': 74.2, 'avg_turns': 134.9, 'loss_vp_road': 8.99, 'loss_vp_opp': 5.14,
             'lr_road': 71.0, 'lr_opp': 28.8, 'la_road': 79.1, 'la_opp': 20.2,
             'dev_road': 2.7, 'dev_opp': 3.2, 'prod_road': 1006.6, 'prod_opp': 601.1},
    'ParaSettleIt5': {'win_rate': 43.9, 'avg_turns': 118.3, 'loss_vp_road': 9.01, 'loss_vp_opp': 10.84,
                      'lr_road': 78.3, 'lr_opp': 21.0, 'la_road': 28.3, 'la_opp': 71.7,
                      'dev_road': 2.1, 'dev_opp': 2.9, 'prod_road': 935.9, 'prod_opp': 1059.4},
    'AlphaBeta': {'win_rate': 33.7, 'avg_turns': 114.0, 'loss_vp_road': 10.07, 'loss_vp_opp': 5.47,
                  'lr_road': 38.5, 'lr_opp': 61.5, 'la_road': 52.2, 'la_opp': 47.8,
                  'dev_road': 2.4, 'dev_opp': 2.3, 'prod_road': 855.6, 'prod_opp': 1105.8},
}

# Dane OneResourcePlayer dla porównania
ONERESOURCE_DATA = {
    'It1': {'win_rate': 98.3}, 'It2': {'win_rate': 95.5}, 'It3': {'win_rate': 56.8},
    'It4': {'win_rate': 36.4}, 'It5': {'win_rate': 19.0}, 'Para': {'win_rate': 34.8},
    'ParaSettleIt5': {'win_rate': 18.2}, 'AlphaBeta': {'win_rate': 11.9}
}

# Kolejność przeciwników
OPPONENTS = ['It1', 'It2', 'It3', 'It4', 'It5', 'Para', 'ParaSettleIt5', 'AlphaBeta']


def plot_effectiveness_decline(output_dir: Path):
    """Wykres 1: Spadek skuteczności strategii drogowej"""
    fig, ax = plt.subplots(figsize=(12, 6))
    
    opponents = OPPONENTS
    win_rates = [DATA[opp]['win_rate'] for opp in opponents]
    
    x = np.arange(len(opponents))
    ax.plot(x, win_rates, 'o-', linewidth=2.5, markersize=10, color='#3498db', label='Win Rate RoadPlayer')
    
    ax.axhline(y=50.0, color='gray', linestyle='--', linewidth=2, 
               alpha=0.5, label='Linia remisu (50%)')
    
    # Dodaj wartości na punktach
    for i, (opp, wr) in enumerate(zip(opponents, win_rates)):
        ax.text(i, wr + 2, f'{wr:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=9)
    
    ax.set_xlabel('Przeciwnik', fontsize=12, fontweight='bold')
    ax.set_ylabel('Współczynnik zwycięstw RoadPlayer (%)', fontsize=12, fontweight='bold')

    ax.set_xticks(x)
    ax.set_xticklabels(opponents)
    ax.set_ylim([0, 105])
    ax.grid(True, alpha=0.3)
    ax.legend(loc='upper right')
    
    plt.tight_layout()
    plt.savefig(output_dir / 'road_effectiveness.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'road_effectiveness.png'}")


def plot_longest_road_bot_and_opp(output_dir: Path):
    """Wykres 3: Paradoks Najdłuższej Drogi"""
    fig, ax = plt.subplots(figsize=(12, 6))
    
    opponents = OPPONENTS
    lr_road = [DATA[opp]['lr_road'] for opp in opponents]
    lr_opp = [DATA[opp]['lr_opp'] for opp in opponents]
    
    x = np.arange(len(opponents))
    width = 0.35
    
    bars1 = ax.bar(x - width/2, lr_road, width, label='RoadPlayer', 
                   color='#9b59b6', alpha=0.7, edgecolor='black', linewidth=1.5)
    bars2 = ax.bar(x + width/2, lr_opp, width, label='Przeciwnik', 
                   color='#e74c3c', alpha=0.7, edgecolor='black', linewidth=1.5)
    
    # Dodaj wartości na słupkach
    for bars in [bars1, bars2]:
        for bar in bars:
            height = bar.get_height()
            if height > 0:
                ax.text(bar.get_x() + bar.get_width()/2., height + 1,
                       f'{height:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=8)
    
    # Linia remisu
    ax.axhline(y=50.0, color='gray', linestyle='--', linewidth=2, 
               alpha=0.5, label='Linia remisu (50%)')

    ax.set_xlabel('Przeciwnik', fontsize=12, fontweight='bold')
    ax.set_ylabel('Częstość zdobycia premii Najdłuższa Droga (%)', fontsize=12, fontweight='bold')

    ax.set_xticks(x)
    ax.set_xticklabels(opponents, rotation=45, ha='right')
    ax.set_ylim([0, 100])
    ax.grid(True, alpha=0.3, axis='y')
    ax.legend(loc='upper right')
    
    plt.tight_layout()
    plt.savefig(output_dir / 'road_longest_road.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'road_longest_road.png'}")


def plot_largest_army_decline(output_dir: Path):
    """Wykres 4: Spadek wykorzystania Największej Armii"""
    fig, ax = plt.subplots(figsize=(10, 6))
    
    opponents = OPPONENTS
    la_pcts = [DATA[opp]['la_road'] for opp in opponents]
    
    x = np.arange(len(opponents))
    bars = ax.bar(x, la_pcts, color='#e67e22', alpha=0.7, edgecolor='black', linewidth=1.5)
    
    # Dodaj wartości na słupkach
    for bar, val in zip(bars, la_pcts):
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height + 2,
               f'{val:.1f}%', ha='center', va='bottom', fontweight='bold')
    
    # Linia pokazująca "przepaść"
    ax.axvline(x=3.5, color='red', linestyle='--', linewidth=2, alpha=0.5)
    
    ax.set_xlabel('Przeciwnik', fontsize=12, fontweight='bold')
    ax.set_ylabel('Największa Armia RoadPlayer (%)', fontsize=12, fontweight='bold')
    ax.set_title('Wykorzystanie Największej Armii: słabość strategii drogowej\n' +
                 '(Spadek przeciwko zaawansowanym botom)', 
                 fontsize=14, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(opponents, rotation=45, ha='right')
    ax.set_ylim([0, 100])
    ax.grid(True, alpha=0.3, axis='y')
    
    plt.tight_layout()
    plt.savefig(output_dir / 'scenario5_road_4_largest_army_decline.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'scenario5_road_4_largest_army_decline.png'}")


def plot_tempo(output_dir: Path):
    """Wykres 5: Tempo rozgrywki"""
    fig, ax = plt.subplots(figsize=(10, 6))
    
    opponents = OPPONENTS
    avg_turns = [DATA[opp]['avg_turns'] for opp in opponents]
    
    x = np.arange(len(opponents))
    bars = ax.bar(x, avg_turns, color='#16a085', alpha=0.7, edgecolor='black', linewidth=1.5)
    
    # Dodaj wartości na słupkach
    for bar, val in zip(bars, avg_turns):
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height + 2,
               f'{val:.1f}', ha='center', va='bottom', fontweight='bold')
    
    ax.set_xlabel('Przeciwnik', fontsize=12, fontweight='bold')
    ax.set_ylabel('Średnia liczba tur', fontsize=12, fontweight='bold')
    ax.set_title('Tempo rozgrywki: zaawansowane boty wygrywają szybciej', 
                 fontsize=14, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(opponents, rotation=45, ha='right')
    ax.grid(True, alpha=0.3, axis='y')
    
    plt.tight_layout()
    plt.savefig(output_dir / 'scenario5_road_5_tempo.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'scenario5_road_5_tempo.png'}")


def plot_production_comparison(output_dir: Path):
    """Wykres 6: Porównanie produkcji zasobów"""
    fig, ax = plt.subplots(figsize=(12, 6))
    
    opponents = OPPONENTS
    prod_road = [DATA[opp]['prod_road'] for opp in opponents]
    prod_opp = [DATA[opp]['prod_opp'] for opp in opponents]
    
    x = np.arange(len(opponents))
    width = 0.35
    
    bars1 = ax.bar(x - width/2, prod_road, width, label='RoadPlayer', 
                   color='#3498db', alpha=0.7, edgecolor='black', linewidth=1.5)
    bars2 = ax.bar(x + width/2, prod_opp, width, label='Przeciwnik', 
                   color='#e74c3c', alpha=0.7, edgecolor='black', linewidth=1.5)
    
    # Dodaj wartości na słupkach
    for bars in [bars1, bars2]:
        for bar in bars:
            height = bar.get_height()
            if height > 0:
                ax.text(bar.get_x() + bar.get_width()/2., height + 15,
                       f'{int(height)}', ha='center', va='bottom', fontweight='bold', fontsize=8)
    
    # Linia pokazująca "przepaść"
    ax.axvline(x=3.5, color='red', linestyle='--', linewidth=2, alpha=0.5)
    
    ax.set_xlabel('Przeciwnik', fontsize=12, fontweight='bold')
    ax.set_ylabel('Produkcja zasobów (ProdScore)', fontsize=12, fontweight='bold')
    ax.set_title('Porównanie produkcji zasobów: RoadPlayer vs Przeciwnik', 
                 fontsize=14, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(opponents, rotation=45, ha='right')
    ax.grid(True, alpha=0.3, axis='y')
    ax.legend(loc='upper left')
    
    plt.tight_layout()
    plt.savefig(output_dir / 'scenario5_road_6_production.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'scenario5_road_6_production.png'}")


def main():
    parser = argparse.ArgumentParser(description='Wizualizacja wyników Scenariusza 5 - RoadPlayer')
    parser.add_argument('--output-dir', type=Path, default=Path('plots'),
                       help='Katalog wyjściowy dla wykresów')
    args = parser.parse_args()
    
    args.output_dir.mkdir(exist_ok=True)
    
    print("Generowanie wykresów Scenariusza 5 - RoadPlayer...")
    plot_effectiveness_decline(args.output_dir)
    plot_longest_road_bot_and_opp(args.output_dir)
    # plot_largest_army_decline(args.output_dir)
    # plot_tempo(args.output_dir)
    # plot_production_comparison(args.output_dir)
    
    print(f"\nWszystkie wykresy zapisane w: {args.output_dir}")
    print("\nWygenerowane wykresy:")
    print("  1. road_effectiveness.png - Spadek skuteczności")
    print("  3. road_longest_road.png - Paradoks Najdłuższej Drogi")
    # print("  4. scenario5_road_4_largest_army_decline.png - Spadek LA%")
    # print("  5. scenario5_road_5_tempo.png - Tempo rozgrywki")
    # print("  6. scenario5_road_6_production.png - Porównanie produkcji")


if __name__ == '__main__':
    main()
