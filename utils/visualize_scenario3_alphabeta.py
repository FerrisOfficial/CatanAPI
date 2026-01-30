#!/usr/bin/env python3
"""
Skrypt do wizualizacji wyników Scenariusza 3: AlphaBetaPlayer vs różne przeciwniki.

Użycie:
    python utils/visualize_ab_phabeta.py --output-dir docs/
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

# Dane z eksperymentów - AlphaBeta vs różni przeciwnicy
DATA = {
    'It1': {
        'win_rate': 99.9, 'avg_turns': 105.0,
        'loss_vp_ab': 10.00, 'loss_vp_opp': 3.38,
        'lr_ab': 96.3, 'lr_opp': 2.7,
        'la_ab': 92.4, 'la_opp': 3.9,
        'dev_ab': 2.4, 'dev_opp': 0.6,
        'prod_ab': 1175.6, 'prod_opp': 262.3
    },
    'It2': {
        'win_rate': 99.5, 'avg_turns': 107.7,
        'loss_vp_ab': 10.20, 'loss_vp_opp': 3.46,
        'lr_ab': 90.6, 'lr_opp': 8.7,
        'la_ab': 91.6, 'la_opp': 3.3,
        'dev_ab': 2.5, 'dev_opp': 0.5,
        'prod_ab': 1180.0, 'prod_opp': 274.4
    },
    'It3': {
        'win_rate': 94.6, 'avg_turns': 118.8,
        'loss_vp_ab': 11.43, 'loss_vp_opp': 7.36,
        'lr_ab': 82.1, 'lr_opp': 17.8,
        'la_ab': 77.1, 'la_opp': 22.3,
        'dev_ab': 2.5, 'dev_opp': 1.8,
        'prod_ab': 1163.8, 'prod_opp': 607.4
    },
    'It4': {
        'win_rate': 80.6, 'avg_turns': 127.4,
        'loss_vp_ab': 9.30, 'loss_vp_opp': 8.56,
        'lr_ab': 57.8, 'lr_opp': 42.2,
        'la_ab': 76.6, 'la_opp': 22.6,
        'dev_ab': 2.6, 'dev_opp': 1.9,
        'prod_ab': 1110.6, 'prod_opp': 869.9
    },
    'It5': {
        'win_rate': 57.8, 'avg_turns': 113.0,
        'loss_vp_ab': 8.40, 'loss_vp_opp': 9.94,
        'lr_ab': 77.3, 'lr_opp': 22.3,
        'la_ab': 32.2, 'la_opp': 67.8,
        'dev_ab': 2.1, 'dev_opp': 2.8,
        'prod_ab': 1063.6, 'prod_opp': 953.9
    },
    'Para': {
        'win_rate': 83.7, 'avg_turns': 119.0,
        'loss_vp_ab': 9.33, 'loss_vp_opp': 4.94,
        'lr_ab': 78.1, 'lr_opp': 21.8,
        'la_ab': 75.3, 'la_opp': 23.9,
        'dev_ab': 2.4, 'dev_opp': 2.4,
        'prod_ab': 1084.5, 'prod_opp': 504.2
    },
    'ParaSettleIt5': {
        'win_rate': 54.2, 'avg_turns': 111.2,
        'loss_vp_ab': 10.33, 'loss_vp_opp': 10.55,
        'lr_ab': 75.4, 'lr_opp': 24.1,
        'la_ab': 30.3, 'la_opp': 69.3,
        'dev_ab': 2.0, 'dev_opp': 2.9,
        'prod_ab': 1032.5, 'prod_opp': 996.2
    }
}

# Kolejność przeciwników według siły
OPPONENTS_ORDER = ['It1', 'It2', 'It3', 'It4', 'It5', 'Para', 'ParaSettleIt5']


def plot_win_rate_progression(output_dir: Path):
    """Wykres 1: Progresja współczynnika zwycięstw AlphaBeta vs siła przeciwnika"""
    fig, ax = plt.subplots(figsize=(12, 7))
    
    opponents = OPPONENTS_ORDER
    win_rates = [DATA[opp]['win_rate'] for opp in opponents]
    
    x = np.arange(len(opponents))
    
    # Niebieska linia z niebieskimi kółkami
    ax.plot(opponents, win_rates, 'o-', linewidth=2.5, markersize=10, 
            color='#3498db', markerfacecolor='#3498db', markeredgecolor='#3498db', 
            markeredgewidth=1.5, label='Współczynnik zwycięstw AlphaBeta')
    
    # Dodaj wartości procentowe nad każdym kółkiem
    for i, (opp, wr) in enumerate(zip(opponents, win_rates)):
        ax.text(i, wr + 2, f'{wr:.1f}%', ha='center', va='bottom', 
               fontweight='bold', fontsize=10, color='#3498db')
    
    # Szara linia remisu na poziomie 50%
    ax.axhline(y=50, color='gray', linestyle='--', linewidth=1.5, alpha=0.7, 
               label='Linia remisu (50%)')
    
    ax.set_xlabel('Przeciwnik', fontsize=12, fontweight='bold')
    ax.set_ylabel('Współczynnik zwycięstw AlphaBeta (%)', fontsize=12, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(opponents)
    ax.set_ylim([0, 105])
    ax.grid(True, alpha=0.3, axis='y')
    ax.legend(loc='best')
    
    plt.tight_layout()
    plt.savefig(output_dir / 'ab_win_rate_progression.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'ab_win_rate_progression.png'}")


def plot_tempo_vs_opponent(output_dir: Path):
    """Wykres 2: Tempo rozgrywki vs siła przeciwnika"""
    fig, ax = plt.subplots(figsize=(12, 7))
    
    opponents = OPPONENTS_ORDER
    avg_turns = [DATA[opp]['avg_turns'] for opp in opponents]
    
    x = np.arange(len(opponents))
    bars = ax.bar(x, avg_turns, color='#9b59b6', alpha=0.7, edgecolor='black', linewidth=1.5)
    
    # Dodaj wartości na słupkach
    for bar, val in zip(bars, avg_turns):
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height + 2,
               f'{val:.1f}', ha='center', va='bottom', fontweight='bold', fontsize=10)
    
    ax.set_xlabel('Przeciwnik', fontsize=12, fontweight='bold')
    ax.set_ylabel('Średnia liczba tur', fontsize=12, fontweight='bold')

    ax.set_xticks(x)
    ax.set_xticklabels(opponents)
    ax.grid(True, alpha=0.3, axis='y')
    
    plt.tight_layout()
    plt.savefig(output_dir / 'ab_tempo.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'ab_tempo.png'}")


def plot_longest_road_dominance(output_dir: Path):
    """Wykres 3a: Dominacja w premii Najdłuższa Droga"""
    fig, ax = plt.subplots(figsize=(12, 7))
    
    opponents = OPPONENTS_ORDER
    lr_ab = [DATA[opp]['lr_ab'] for opp in opponents]
    lr_opp = [DATA[opp]['lr_opp'] for opp in opponents]
    
    x = np.arange(len(opponents))
    width = 0.35
    
    bars1 = ax.bar(x - width/2, lr_ab, width, label='AlphaBeta', 
                   color='#2ecc71', alpha=0.7, edgecolor='black', linewidth=1.5)
    bars2 = ax.bar(x + width/2, lr_opp, width, label='Przeciwnik', 
                   color='#f39c12', alpha=0.7, edgecolor='black', linewidth=1.5)
    
    ax.set_xlabel('Przeciwnik', fontsize=12, fontweight='bold')
    ax.set_ylabel('Wykorzystanie premii (%)', fontsize=12, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(opponents)
    ax.set_ylim([0, 105])
    ax.grid(True, alpha=0.3, axis='y')
    ax.legend()
    
    plt.tight_layout()
    plt.savefig(output_dir / 'ab_longest_road.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'ab_longest_road.png'}")


def plot_largest_army_dominance(output_dir: Path):
    """Wykres 3b: Dominacja w premii Największa Armia"""
    fig, ax = plt.subplots(figsize=(12, 7))
    
    opponents = OPPONENTS_ORDER
    la_ab = [DATA[opp]['la_ab'] for opp in opponents]
    la_opp = [DATA[opp]['la_opp'] for opp in opponents]
    
    x = np.arange(len(opponents))
    width = 0.35
    
    bars1 = ax.bar(x - width/2, la_ab, width, label='AlphaBeta', 
                   color='#3498db', alpha=0.7, edgecolor='black', linewidth=1.5)
    bars2 = ax.bar(x + width/2, la_opp, width, label='Przeciwnik', 
                   color='#e74c3c', alpha=0.7, edgecolor='black', linewidth=1.5)
    
    ax.set_xlabel('Przeciwnik', fontsize=12, fontweight='bold')
    ax.set_ylabel('Wykorzystanie premii (%)', fontsize=12, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(opponents)
    ax.set_ylim([0, 105])
    ax.grid(True, alpha=0.3, axis='y')
    ax.legend()
    
    plt.tight_layout()
    plt.savefig(output_dir / 'ab_largest_army.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'ab_largest_army.png'}")


def plot_production_comparison(output_dir: Path):
    """Wykres 4: Porównanie produkcji zasobów"""
    fig, ax = plt.subplots(figsize=(12, 7))
    
    opponents = OPPONENTS_ORDER
    prod_ab = [DATA[opp]['prod_ab'] for opp in opponents]
    prod_opp = [DATA[opp]['prod_opp'] for opp in opponents]
    
    x = np.arange(len(opponents))
    width = 0.35
    
    bars1 = ax.bar(x - width/2, prod_ab, width, label='AlphaBeta', 
                   color='#2ecc71', alpha=0.7, edgecolor='black', linewidth=1.5)
    bars2 = ax.bar(x + width/2, prod_opp, width, label='Przeciwnik', 
                   color='#e74c3c', alpha=0.7, edgecolor='black', linewidth=1.5)
    
    ax.set_xlabel('Przeciwnik', fontsize=12, fontweight='bold')
    ax.set_ylabel('Produkcja zasobów (ProdScore)', fontsize=12, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(opponents)
    ax.grid(True, alpha=0.3, axis='y')
    ax.legend()
    
    plt.tight_layout()
    plt.savefig(output_dir / 'ab_production.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'ab_production.png'}")


def plot_loss_vp_dominance(output_dir: Path):
    """Wykres 5: Dominacja mierzona przez LossVP"""
    fig, ax = plt.subplots(figsize=(12, 7))
    
    opponents = OPPONENTS_ORDER
    loss_vp_ab = [DATA[opp]['loss_vp_ab'] for opp in opponents]
    loss_vp_opp = [DATA[opp]['loss_vp_opp'] for opp in opponents]
    
    x = np.arange(len(opponents))
    width = 0.35
    
    bars1 = ax.bar(x - width/2, loss_vp_ab, width, label='LossVP AlphaBeta', 
                   color='#3498db', alpha=0.7, edgecolor='black', linewidth=1.5)
    bars2 = ax.bar(x + width/2, loss_vp_opp, width, label='LossVP Przeciwnik', 
                   color='#e74c3c', alpha=0.7, edgecolor='black', linewidth=1.5)
    
    ax.set_xlabel('Przeciwnik', fontsize=12, fontweight='bold')
    ax.set_ylabel('Średnia VP przeciwnika przy przegranej', fontsize=12, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(opponents)
    ax.grid(True, alpha=0.3, axis='y')
    ax.legend()
    
    plt.tight_layout()
    plt.savefig(output_dir / 'ab_dominance.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'ab_dominance.png'}")


def plot_effectiveness_decline(output_dir: Path):
    """Wykres 6: Spadek skuteczności - szczegółowa analiza"""
    fig, axes = plt.subplots(2, 2, figsize=(16, 12))
    
    opponents = OPPONENTS_ORDER
    
    # Win Rate
    ax = axes[0, 0]
    win_rates = [DATA[opp]['win_rate'] for opp in opponents]
    ax.plot(opponents, win_rates, 'o-', linewidth=2.5, markersize=8, color='#e74c3c')
    ax.fill_between(opponents, win_rates, 50, alpha=0.2, color='#e74c3c')
    ax.axhline(y=50, color='gray', linestyle='--', linewidth=1.5, alpha=0.5)
    for i, (opp, wr) in enumerate(zip(opponents, win_rates)):
        ax.text(i, wr + 2, f'{wr:.1f}%', ha='center', fontweight='bold', fontsize=9)
    ax.set_ylabel('Win Rate (%)', fontweight='bold')
    ax.set_title('Współczynnik zwycięstw', fontweight='bold')
    ax.set_ylim([40, 105])
    ax.grid(True, alpha=0.3)
    
    # Tempo
    ax = axes[0, 1]
    avg_turns = [DATA[opp]['avg_turns'] for opp in opponents]
    ax.plot(opponents, avg_turns, 'o-', linewidth=2.5, markersize=8, color='#9b59b6')
    for i, (opp, turns) in enumerate(zip(opponents, avg_turns)):
        ax.text(i, turns + 2, f'{turns:.1f}', ha='center', fontweight='bold', fontsize=9)
    ax.set_ylabel('Średnia liczba tur', fontweight='bold')
    ax.set_title('Tempo rozgrywki', fontweight='bold')
    ax.grid(True, alpha=0.3)
    
    # Produkcja różnica
    ax = axes[1, 0]
    prod_diff = [DATA[opp]['prod_ab'] - DATA[opp]['prod_opp'] for opp in opponents]
    colors = ['#2ecc71' if d > 0 else '#e74c3c' for d in prod_diff]
    bars = ax.bar(opponents, prod_diff, color=colors, alpha=0.7, edgecolor='black', linewidth=1.5)
    ax.axhline(y=0, color='black', linestyle='-', linewidth=1)
    for bar, val in zip(bars, prod_diff):
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height + (20 if height > 0 else -30),
               f'{val:+.1f}', ha='center', fontweight='bold', fontsize=9)
    ax.set_ylabel('Różnica ProdScore (AB - Opp)', fontweight='bold')
    ax.set_title('Przewaga produkcyjna', fontweight='bold')
    ax.grid(True, alpha=0.3, axis='y')
    
    # Longest Road różnica
    ax = axes[1, 1]
    lr_diff = [DATA[opp]['lr_ab'] - DATA[opp]['lr_opp'] for opp in opponents]
    colors = ['#2ecc71' if d > 0 else '#e74c3c' for d in lr_diff]
    bars = ax.bar(opponents, lr_diff, color=colors, alpha=0.7, edgecolor='black', linewidth=1.5)
    ax.axhline(y=0, color='black', linestyle='-', linewidth=1)
    for bar, val in zip(bars, lr_diff):
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height + (3 if height > 0 else -5),
               f'{val:+.1f}%', ha='center', fontweight='bold', fontsize=9)
    ax.set_ylabel('Różnica LR% (AB - Opp)', fontweight='bold')
    ax.set_title('Przewaga w Najdłuższej Drodze', fontweight='bold')
    ax.grid(True, alpha=0.3, axis='y')
    
    plt.suptitle('Szczegółowa analiza spadku skuteczności AlphaBeta', 
                 fontsize=16, fontweight='bold', y=0.995)
    plt.tight_layout()
    plt.savefig(output_dir / 'ab_effectiveness_decline.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'ab_effectiveness_decline.png'}")


def plot_alphabeta_vs_it5_comparison(output_dir: Path):
    """Wykres 7: Szczegółowe porównanie AlphaBeta vs It5"""
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    
    data_ab = DATA['It5']
    bots = ['AlphaBeta', 'It5']
    
    # Win Rate
    ax = axes[0, 0]
    win_rates = [data_ab['win_rate'], 100 - data_ab['win_rate']]
    bars = ax.bar(bots, win_rates, color=['#3498db', '#e74c3c'], alpha=0.7, 
                  edgecolor='black', linewidth=2)
    for bar, val in zip(bars, win_rates):
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height + 1,
               f'{val:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=12)
    ax.set_ylabel('Win Rate (%)', fontweight='bold')
    ax.set_title('Współczynnik zwycięstw', fontweight='bold')
    ax.set_ylim([0, 105])
    ax.grid(True, alpha=0.3, axis='y')
    
    # Produkcja
    ax = axes[0, 1]
    prod_scores = [data_ab['prod_ab'], data_ab['prod_opp']]
    bars = ax.bar(bots, prod_scores, color=['#2ecc71', '#e74c3c'], alpha=0.7,
                  edgecolor='black', linewidth=2)
    for bar, val in zip(bars, prod_scores):
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height + 20,
               f'{val:.1f}', ha='center', va='bottom', fontweight='bold', fontsize=12)
    ax.set_ylabel('Produkcja zasobów', fontweight='bold')
    ax.set_title('ProdScore', fontweight='bold')
    ax.grid(True, alpha=0.3, axis='y')
    
    # Największa Armia
    ax = axes[1, 0]
    la_pcts = [data_ab['la_ab'], data_ab['la_opp']]
    bars = ax.bar(bots, la_pcts, color=['#3498db', '#e74c3c'], alpha=0.7,
                  edgecolor='black', linewidth=2)
    for bar, val in zip(bars, la_pcts):
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height + 2,
               f'{val:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=12)
    ax.set_ylabel('Największa Armia (%)', fontweight='bold')
    ax.set_title('Wykorzystanie premii LA', fontweight='bold')
    ax.set_ylim([0, 75])
    ax.grid(True, alpha=0.3, axis='y')
    
    # LossVP
    ax = axes[1, 1]
    loss_vps = [data_ab['loss_vp_ab'], data_ab['loss_vp_opp']]
    bars = ax.bar(bots, loss_vps, color=['#3498db', '#e74c3c'], alpha=0.7,
                  edgecolor='black', linewidth=2)
    for bar, val in zip(bars, loss_vps):
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height + 0.2,
               f'{val:.2f}', ha='center', va='bottom', fontweight='bold', fontsize=12)
    ax.set_ylabel('Średnia VP przeciwnika przy przegranej', fontweight='bold')
    ax.set_title('Dominacja (LossVP)', fontweight='bold')
    ax.grid(True, alpha=0.3, axis='y')
    
    plt.suptitle('Szczegółowe porównanie AlphaBeta vs It5\n' +
                 'Najbardziej wyrównane starcie (57.8% vs 42.2%)',
                 fontsize=15, fontweight='bold', y=0.995)
    plt.tight_layout()
    plt.savefig(output_dir / 'ab_vs_it5.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'ab_alphabeta_vs_it5.png'}")


def main():
    parser = argparse.ArgumentParser(description='Wizualizacja wyników Scenariusza 3')
    parser.add_argument('--output-dir', type=Path, default=Path('plots'),
                       help='Katalog wyjściowy dla wykresów')
    args = parser.parse_args()
    
    args.output_dir.mkdir(exist_ok=True)
    
    print("Generowanie wykresów Scenariusza 3...")
    plot_win_rate_progression(args.output_dir)
    plot_tempo_vs_opponent(args.output_dir)
    plot_longest_road_dominance(args.output_dir)
    plot_largest_army_dominance(args.output_dir)
    plot_production_comparison(args.output_dir)
    plot_loss_vp_dominance(args.output_dir)
    # plot_effectiveness_decline(args.output_dir)
    # plot_alphabeta_vs_it5_comparison(args.output_dir)
    
    print(f"\nWszystkie wykresy zapisane w: {args.output_dir}")
    print("\nWygenerowane wykresy:")
    print("  1. ab_win_rate_progression.png - Progresja współczynnika zwycięstw")
    print("  2. ab_tempo.png - Tempo rozgrywki")
    print("  3a. ab_longest_road.png - Dominacja w Najdłuższej Drodze")
    print("  3b. ab_largest_army.png - Dominacja w Największej Armii")
    print("  4. ab_production.png - Porównanie produkcji")
    print("  5. ab_dominance.png - Dominacja (LossVP)")
    # print("  6. ab_effectiveness_decline.png - Szczegółowa analiza spadku skuteczności")
    # print("  7. ab_alphabeta_vs_it5.png - Szczegółowe porównanie vs It5")


if __name__ == '__main__':
    main()
