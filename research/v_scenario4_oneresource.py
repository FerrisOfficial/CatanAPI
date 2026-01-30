#!/usr/bin/env python3
"""
Skrypt do wizualizacji wyników Scenariusza 4: OneResourcePlayer vs różne boty.

Użycie:
    python utils/visualize_scenario4_oneresource.py --output-dir docs/
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
    'It1': {'win_rate': 98.3, 'avg_turns': 160.1, 'loss_vp_or': 9.00, 'loss_vp_opp': 4.37,
            'lr_or': 82.9, 'lr_opp': 14.7, 'la_or': 89.2, 'la_opp': 8.9,
            'prod_or': 1117.3, 'prod_opp': 321.2},
    'It2': {'win_rate': 95.5, 'avg_turns': 161.1, 'loss_vp_or': 9.82, 'loss_vp_opp': 4.93,
            'lr_or': 72.8, 'lr_opp': 26.1, 'la_or': 89.3, 'la_opp': 8.8,
            'prod_or': 1115.0, 'prod_opp': 387.6},
    'It3': {'win_rate': 56.8, 'avg_turns': 148.6, 'loss_vp_or': 8.43, 'loss_vp_opp': 9.36,
            'lr_or': 43.4, 'lr_opp': 56.5, 'la_or': 51.9, 'la_opp': 47.6,
            'prod_or': 968.4, 'prod_opp': 854.7},
    'It4': {'win_rate': 36.4, 'avg_turns': 146.6, 'loss_vp_or': 8.23, 'loss_vp_opp': 10.09,
            'lr_or': 22.2, 'lr_opp': 77.8, 'la_or': 46.9, 'la_opp': 51.8,
            'prod_or': 859.0, 'prod_opp': 1119.4},
    'It5': {'win_rate': 19.0, 'avg_turns': 116.9, 'loss_vp_or': 7.24, 'loss_vp_opp': 11.21,
            'lr_or': 39.0, 'lr_opp': 55.8, 'la_or': 12.1, 'la_opp': 87.8,
            'prod_or': 745.2, 'prod_opp': 1065.1},
    'Dev': {'win_rate': 51.1, 'avg_turns': 149.2, 'loss_vp_or': 8.74, 'loss_vp_opp': 10.14,
            'lr_or': 67.4, 'lr_opp': 30.1, 'la_or': 12.5, 'la_opp': 87.5,
            'prod_or': 1014.6, 'prod_opp': 928.9},
    'Road': {'win_rate': 22.5, 'avg_turns': 125.9, 'loss_vp_or': 7.45, 'loss_vp_opp': 10.28,
             'lr_or': 18.3, 'lr_opp': 81.3, 'la_or': 22.1, 'la_opp': 77.6,
             'prod_or': 784.8, 'prod_opp': 1038.0},
    'CityRush': {'win_rate': 25.3, 'avg_turns': 117.3, 'loss_vp_or': 7.28, 'loss_vp_opp': 10.96,
                 'lr_or': 42.9, 'lr_opp': 51.5, 'la_or': 16.4, 'la_opp': 83.5,
                 'prod_or': 793.1, 'prod_opp': 1034.4},
    'Para': {'win_rate': 34.8, 'avg_turns': 127.8, 'loss_vp_or': 7.73, 'loss_vp_opp': 6.14,
             'lr_or': 32.2, 'lr_opp': 67.2, 'la_or': 60.6, 'la_opp': 33.5,
             'prod_or': 795.7, 'prod_opp': 962.1},
    'ParaSettleIt5': {'win_rate': 18.2, 'avg_turns': 113.2, 'loss_vp_or': 7.19, 'loss_vp_opp': 11.24,
                      'lr_or': 38.9, 'lr_opp': 54.7, 'la_or': 11.1, 'la_opp': 88.7,
                      'prod_or': 735.0, 'prod_opp': 1089.8},
    'AlphaBeta': {'win_rate': 11.1, 'avg_turns': 111.1, 'loss_vp_or': 5.43, 'loss_vp_opp': 10.60,
                  'lr_or': 14.6, 'lr_opp': 85.2, 'la_or': 23.2, 'la_opp': 76.8,
                  'prod_or': 680.6, 'prod_opp': 1129.1},
}

# Kolejność przeciwników
OPPONENTS = ['It1', 'It2', 'It3', 'It4', 'It5', 'Para', 'ParaSettleIt5', 'Dev', 'Road', 'CityRush', 'AlphaBeta']


def plot_effectiveness_decline(output_dir: Path):
    """Wykres 1: Spadek skuteczności strategii monosurowcowej"""
    fig, ax = plt.subplots(figsize=(12, 6))
    
    opponents = OPPONENTS
    win_rates = [DATA[opp]['win_rate'] for opp in opponents]
    
    x = np.arange(len(opponents))
    ax.plot(x, win_rates, 'o-', linewidth=2.5, markersize=10, color='#e74c3c', label='Win Rate OneResourcePlayer')
    
    # Linia pokazująca "przepaść" między It2 a It3
    ax.axhline(y=50, color='gray', linestyle='--', linewidth=1, alpha=0.5, label='Granica równości (50%)')
    
    # Dodaj wartości na punktach
    for i, (opp, wr) in enumerate(zip(opponents, win_rates)):
        ax.text(i, wr + 2, f'{wr:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=9)
    
    ax.set_xlabel('Przeciwnik', fontsize=12, fontweight='bold')
    ax.set_ylabel('Współczynnik zwycięstw OneResourcePlayer (%)', fontsize=12, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(opponents)
    ax.set_ylim([0, 105])
    ax.grid(True, alpha=0.3)
    ax.legend(loc='upper right')
    
    plt.tight_layout()
    plt.savefig(output_dir / 'orBot.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'orBot.png'}")


def plot_longest_road_decline(output_dir: Path):
    """Wykres 2: Spadek wykorzystania Najdłuższej Drogi"""
    fig, ax = plt.subplots(figsize=(10, 6))
    
    opponents = OPPONENTS
    lr_or = [DATA[opp]['lr_or'] for opp in opponents]
    lr_opp = [DATA[opp]['lr_opp'] for opp in opponents]
    
    x = np.arange(len(opponents))
    width = 0.35
    
    bars1 = ax.bar(x - width/2, lr_or, width, label='OneResourcePlayer', 
                   color='#e74c3c', alpha=0.7, edgecolor='black', linewidth=1.5)
    bars2 = ax.bar(x + width/2, lr_opp, width, label='Przeciwnik', 
                   color='#3498db', alpha=0.7, edgecolor='black', linewidth=1.5)
    
    # Dodaj wartości na słupkach
    for bars in [bars1, bars2]:
        for bar in bars:
            height = bar.get_height()
            if height > 0:
                ax.text(bar.get_x() + bar.get_width()/2., height + 1,
                       f'{height:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=8)
    
    # Linia pokazująca "przepaść"
    ax.axvline(x=2.5, color='red', linestyle='--', linewidth=2, alpha=0.5)
    
    ax.set_xlabel('Przeciwnik', fontsize=12, fontweight='bold')
    ax.set_ylabel('Najdłuższa Droga (%)', fontsize=12, fontweight='bold')
    ax.set_title('Wykorzystanie premii Najdłuższa Droga: spadek dominacji OneResourcePlayer', 
                 fontsize=14, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(opponents, rotation=45, ha='right')
    ax.set_ylim([0, 100])
    ax.grid(True, alpha=0.3, axis='y')
    ax.legend(loc='upper right')
    
    plt.tight_layout()
    plt.savefig(output_dir / 'scenario4_oneresource_2_longest_road.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'scenario4_oneresource_2_longest_road.png'}")


def plot_largest_army_decline(output_dir: Path):
    """Wykres 3: Spadek wykorzystania Największej Armii"""
    fig, ax = plt.subplots(figsize=(10, 6))
    
    opponents = OPPONENTS
    la_or = [DATA[opp]['la_or'] for opp in opponents]
    la_opp = [DATA[opp]['la_opp'] for opp in opponents]
    
    x = np.arange(len(opponents))
    width = 0.35
    
    bars1 = ax.bar(x - width/2, la_or, width, label='OneResourcePlayer', 
                   color='#e74c3c', alpha=0.7, edgecolor='black', linewidth=1.5)
    bars2 = ax.bar(x + width/2, la_opp, width, label='Przeciwnik', 
                   color='#3498db', alpha=0.7, edgecolor='black', linewidth=1.5)
    
    # Dodaj wartości na słupkach
    for bars in [bars1, bars2]:
        for bar in bars:
            height = bar.get_height()
            if height > 0:
                ax.text(bar.get_x() + bar.get_width()/2., height + 1,
                       f'{height:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=8)
    
    # Linia pokazująca "przepaść"
    ax.axvline(x=2.5, color='red', linestyle='--', linewidth=2, alpha=0.5)
    
    ax.set_xlabel('Przeciwnik', fontsize=12, fontweight='bold')
    ax.set_ylabel('Największa Armia (%)', fontsize=12, fontweight='bold')
    ax.set_title('Wykorzystanie premii Największa Armia: dramatyczny spadek przeciwko zaawansowanym botom', 
                 fontsize=14, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(opponents, rotation=45, ha='right')
    ax.set_ylim([0, 100])
    ax.grid(True, alpha=0.3, axis='y')
    ax.legend(loc='upper right')
    
    plt.tight_layout()
    plt.savefig(output_dir / 'scenario4_oneresource_3_largest_army.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'scenario4_oneresource_3_largest_army.png'}")


def plot_production_comparison(output_dir: Path):
    """Wykres 4: Porównanie produkcji zasobów"""
    fig, ax = plt.subplots(figsize=(12, 6))
    
    opponents = OPPONENTS
    prod_or = [DATA[opp]['prod_or'] for opp in opponents]
    prod_opp = [DATA[opp]['prod_opp'] for opp in opponents]
    
    x = np.arange(len(opponents))
    width = 0.35
    
    bars1 = ax.bar(x - width/2, prod_or, width, label='OneResourcePlayer', 
                   color='#e74c3c', alpha=0.7, edgecolor='black', linewidth=1.5)
    bars2 = ax.bar(x + width/2, prod_opp, width, label='Przeciwnik', 
                   color='#3498db', alpha=0.7, edgecolor='black', linewidth=1.5)
    
    # Dodaj wartości na słupkach
    for bars in [bars1, bars2]:
        for bar in bars:
            height = bar.get_height()
            if height > 0:
                ax.text(bar.get_x() + bar.get_width()/2., height + 20,
                       f'{int(height)}', ha='center', va='bottom', fontweight='bold', fontsize=8)
    
    # Linia pokazująca "przepaść"
    ax.axvline(x=2.5, color='red', linestyle='--', linewidth=2, alpha=0.5)
    
    ax.set_xlabel('Przeciwnik', fontsize=12, fontweight='bold')
    ax.set_ylabel('Produkcja zasobów (ProdScore)', fontsize=12, fontweight='bold')
    ax.set_title('Porównanie produkcji zasobów: OneResourcePlayer vs Przeciwnik\n' +
                 '(Zaawansowane boty mają wyższą produkcję)', 
                 fontsize=14, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(opponents, rotation=45, ha='right')
    ax.grid(True, alpha=0.3, axis='y')
    ax.legend(loc='upper left')
    
    plt.tight_layout()
    plt.savefig(output_dir / 'scenario4_oneresource_4_production.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'scenario4_oneresource_4_production.png'}")


def plot_loss_vp_comparison(output_dir: Path):
    """Wykres 5: Porównanie LossVP"""
    fig, ax = plt.subplots(figsize=(10, 6))
    
    opponents = OPPONENTS
    loss_vp_or = [DATA[opp]['loss_vp_or'] for opp in opponents]
    loss_vp_opp = [DATA[opp]['loss_vp_opp'] for opp in opponents]
    
    x = np.arange(len(opponents))
    width = 0.35
    
    bars1 = ax.bar(x - width/2, loss_vp_or, width, label='OneResourcePlayer (średnie VP przy przegranej)', 
                   color='#e74c3c', alpha=0.7, edgecolor='black', linewidth=1.5)
    bars2 = ax.bar(x + width/2, loss_vp_opp, width, label='Przeciwnik (średnie VP przy przegranej)', 
                   color='#3498db', alpha=0.7, edgecolor='black', linewidth=1.5)
    
    # Dodaj wartości na słupkach
    for bars in [bars1, bars2]:
        for bar in bars:
            height = bar.get_height()
            if height > 0:
                ax.text(bar.get_x() + bar.get_width()/2., height + 0.2,
                       f'{height:.2f}', ha='center', va='bottom', fontweight='bold', fontsize=8)
    
    # Linia pokazująca "przepaść"
    ax.axvline(x=2.5, color='red', linestyle='--', linewidth=2, alpha=0.5)
    
    ax.set_xlabel('Przeciwnik', fontsize=12, fontweight='bold')
    ax.set_ylabel('Średnie VP przy przegranej', fontsize=12, fontweight='bold')
    ax.set_title('Porównanie LossVP: przeciwnicy mają wyższe VP przy przegranej OneResourcePlayer', 
                 fontsize=14, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(opponents, rotation=45, ha='right')
    ax.grid(True, alpha=0.3, axis='y')
    ax.legend(loc='upper left')
    
    plt.tight_layout()
    plt.savefig(output_dir / 'scenario4_oneresource_5_loss_vp.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'scenario4_oneresource_5_loss_vp.png'}")


def plot_tempo(output_dir: Path):
    """Wykres 6: Tempo rozgrywki"""
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
    
    # Linia pokazująca "przepaść"
    ax.axvline(x=2.5, color='red', linestyle='--', linewidth=2, alpha=0.5)
    
    ax.set_xlabel('Przeciwnik', fontsize=12, fontweight='bold')
    ax.set_ylabel('Średnia liczba tur', fontsize=12, fontweight='bold')
    ax.set_title('Tempo rozgrywki: zaawansowane boty wygrywają szybciej', 
                 fontsize=14, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(opponents, rotation=45, ha='right')
    ax.grid(True, alpha=0.3, axis='y')
    
    plt.tight_layout()
    plt.savefig(output_dir / 'scenario4_oneresource_6_tempo.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'scenario4_oneresource_6_tempo.png'}")


def main():
    parser = argparse.ArgumentParser(description='Wizualizacja wyników Scenariusza 4 - OneResourcePlayer')
    parser.add_argument('--output-dir', type=Path, default=Path('plots'),
                       help='Katalog wyjściowy dla wykresów')
    args = parser.parse_args()
    
    args.output_dir.mkdir(exist_ok=True)
    
    print("Generowanie wykresów Scenariusza 4 - OneResourcePlayer...")
    plot_effectiveness_decline(args.output_dir)
    # plot_longest_road_decline(args.output_dir)
    # plot_largest_army_decline(args.output_dir)
    # plot_production_comparison(args.output_dir)
    # plot_loss_vp_comparison(args.output_dir)
    # plot_tempo(args.output_dir)
    
    print(f"\nWszystkie wykresy zapisane w: {args.output_dir}")
    print("\nWygenerowane wykresy:")
    print("  1. orBot.png - Spadek skuteczności")
    # print("  2. scenario4_oneresource_2_longest_road.png - Najdłuższa Droga")
    # print("  3. scenario4_oneresource_3_largest_army.png - Największa Armia")
    # print("  4. scenario4_oneresource_4_production.png - Porównanie produkcji")
    # print("  5. scenario4_oneresource_5_loss_vp.png - Porównanie LossVP")
    # print("  6. scenario4_oneresource_6_tempo.png - Tempo rozgrywki")


if __name__ == '__main__':
    main()
