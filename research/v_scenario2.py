#!/usr/bin/env python3
"""
Skrypt do wizualizacji wyników Scenariusza 2: Porównanie iteracyjnych heurystyk.

Użycie:
    python utils/visualize_scenario2.py --output-dir plots/
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

# Dane z eksperymentów - para (bot_a, bot_b): wyniki
PAIR_DATA = {
    ('It1', 'It2'): {
        'bot_a_win_rate': 35.8, 'bot_b_win_rate': 61.9, 'avg_turns': 339.8,
        'bot_a_loss_vp': 6.24, 'bot_b_loss_vp': 7.96,
        'bot_a_lr': 37.4, 'bot_b_lr': 62.6,
        'bot_a_la': 45.8, 'bot_b_la': 52.3,
        'bot_a_dev': 2.3, 'bot_b_dev': 2.6,
        'bot_a_prod': 589.1, 'bot_b_prod': 832.1
    },
    ('It2', 'It3'): {
        'bot_a_win_rate': 5.8, 'bot_b_win_rate': 93.6, 'avg_turns': 181.9,
        'bot_a_loss_vp': 5.06, 'bot_b_loss_vp': 11.28,
        'bot_a_lr': 31.4, 'bot_b_lr': 68.5,
        'bot_a_la': 5.8, 'bot_b_la': 91.1,
        'bot_a_dev': 0.9, 'bot_b_dev': 3.2,
        'bot_a_prod': 400.5, 'bot_b_prod': 1017.3
    },
    ('It3', 'It4'): {
        'bot_a_win_rate': 32.4, 'bot_b_win_rate': 66.0, 'avg_turns': 184.4,
        'bot_a_loss_vp': 9.11, 'bot_b_loss_vp': 9.89,
        'bot_a_lr': 24.6, 'bot_b_lr': 75.4,
        'bot_a_la': 52.5, 'bot_b_la': 46.3,
        'bot_a_dev': 2.5, 'bot_b_dev': 2.5,
        'bot_a_prod': 817.7, 'bot_b_prod': 1127.4
    },
    ('It4', 'It5'): {
        'bot_a_win_rate': 25.7, 'bot_b_win_rate': 74.2, 'avg_turns': 134.2,
        'bot_a_loss_vp': 9.54, 'bot_b_loss_vp': 11.04,
        'bot_a_lr': 82.0, 'bot_b_lr': 17.8,
        'bot_a_la': 11.4, 'bot_b_la': 88.6,
        'bot_a_dev': 1.7, 'bot_b_dev': 3.4,
        'bot_a_prod': 958.1, 'bot_b_prod': 1070.6
    },
    ('It1', 'It5'): {  # Porównanie skokowe
        'bot_a_win_rate': 0.1, 'bot_b_win_rate': 99.9, 'avg_turns': 114.9,
        'bot_a_loss_vp': 3.58, 'bot_b_loss_vp': 14.00,
        'bot_a_lr': 14.0, 'bot_b_lr': 73.5,
        'bot_a_la': 1.1, 'bot_b_la': 98.2,
        'bot_a_dev': 0.6, 'bot_b_dev': 3.7,
        'bot_a_prod': 266.5, 'bot_b_prod': 1096.8
    }
}


def plot_pairwise_comparison(output_dir: Path):
    """Wykres 1: Progresja w parach - win rate każdego bota"""
    fig, ax = plt.subplots(figsize=(12, 6))
    
    pairs = [('It1', 'It2'), ('It2', 'It3'), ('It3', 'It4'), ('It4', 'It5')]
    pair_labels = ['It1 vs It2', 'It2 vs It3', 'It3 vs It4', 'It4 vs It5']
    
    x = np.arange(len(pairs))
    width = 0.35
    
    bot_a_rates = [PAIR_DATA[pair]['bot_a_win_rate'] for pair in pairs]
    bot_b_rates = [PAIR_DATA[pair]['bot_b_win_rate'] for pair in pairs]
    
    bars1 = ax.bar(x - width/2, bot_a_rates, width, label='Poprzednia iteracja', 
                   color='#e74c3c', alpha=0.7, edgecolor='black', linewidth=1.5)
    bars2 = ax.bar(x + width/2, bot_b_rates, width, label='Kolejna iteracja', 
                   color='#2ecc71', alpha=0.7, edgecolor='black', linewidth=1.5)
    
    # Dodaj wartości na słupkach
    for bars in [bars1, bars2]:
        for bar in bars:
            height = bar.get_height()
            if height > 0:
                ax.text(bar.get_x() + bar.get_width()/2., height + 1,
                       f'{height:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=9)
    
    
    ax.set_xlabel('Para botów', fontsize=12, fontweight='bold')
    ax.set_ylabel('Współczynnik zwycięstw (%)', fontsize=12, fontweight='bold')

    ax.set_xticks(x)
    ax.set_xticklabels(pair_labels)
    ax.set_ylim([0, 105])
    ax.grid(True, alpha=0.3, axis='y')
    ax.legend(loc='upper left')
    
    plt.tight_layout()
    plt.savefig(output_dir / 'scenario2_pairwise_comparison.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'scenario2_pairwise_comparison.png'}")



def plot_tempo_progression(output_dir: Path):
    """Wykres 4: Tempo rozgrywki"""
    fig, ax = plt.subplots(figsize=(10, 6))
    
    pairs = [('It1', 'It2'), ('It2', 'It3'), ('It3', 'It4'), ('It4', 'It5')]
    pair_labels = ['It1 vs It2', 'It2 vs It3', 'It3 vs It4', 'It4 vs It5']
    
    avg_turns = [PAIR_DATA[pair]['avg_turns'] for pair in pairs]
    
    x = np.arange(len(pair_labels))
    bars = ax.bar(x, avg_turns, color='#9b59b6', alpha=0.7, edgecolor='black', linewidth=1.5)
    
    # Dodaj wartości na słupkach
    for bar, val in zip(bars, avg_turns):
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height + 5,
               f'{val:.1f}', ha='center', va='bottom', fontweight='bold')
    
    
    ax.set_xlabel('Para botów', fontsize=12, fontweight='bold')
    ax.set_ylabel('Średnia liczba tur', fontsize=12, fontweight='bold')

    ax.set_xticks(x)
    ax.set_xticklabels(pair_labels, rotation=45, ha='right')
    ax.grid(True, alpha=0.3, axis='y')
    ax.legend()
    
    plt.tight_layout()
    plt.savefig(output_dir / 'scenario2_tempo.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'scenario2_tempo.png'}")


def plot_largest_army_pct(output_dir: Path):
    """Wykres: Procent wykorzystania premii Największa Armia (LA%) w parach."""
    fig, ax = plt.subplots(figsize=(12, 6))

    pairs = [('It1', 'It2'), ('It2', 'It3'), ('It3', 'It4'), ('It4', 'It5')]
    pair_labels = ['It1 vs It2', 'It2 vs It3', 'It3 vs It4', 'It4 vs It5']

    x = np.arange(len(pairs))
    width = 0.35

    bot_a_la = [PAIR_DATA[pair]['bot_a_la'] for pair in pairs]
    bot_b_la = [PAIR_DATA[pair]['bot_b_la'] for pair in pairs]

    bars1 = ax.bar(x - width/2, bot_a_la, width, label='Poprzednia iteracja',
                   color='#e67e22', alpha=0.8, edgecolor='black', linewidth=1.2)
    bars2 = ax.bar(x + width/2, bot_b_la, width, label='Kolejna iteracja',
                   color='#1abc9c', alpha=0.8, edgecolor='black', linewidth=1.2)

    for bars in [bars1, bars2]:
        for bar in bars:
            height = bar.get_height()
            if height > 0:
                ax.text(bar.get_x() + bar.get_width()/2., height + 1,
                        f'{height:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=9)

    ax.set_xlabel('Para botów', fontsize=12, fontweight='bold')
    ax.set_ylabel('Największa Armia (%)', fontsize=12, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(pair_labels)
    ax.set_ylim([0, 105])
    ax.grid(True, alpha=0.3, axis='y')
    ax.legend(loc='upper right')
    plt.tight_layout()
    plt.savefig(output_dir / 'scenario2_largest_army.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'scenario2_largest_army.png'}")


def plot_longest_road_pct(output_dir: Path):
    """Wykres: Procent wykorzystania premii Najdłuższa Droga (LR%) w parach."""
    fig, ax = plt.subplots(figsize=(12, 6))

    pairs = [('It1', 'It2'), ('It2', 'It3'), ('It3', 'It4'), ('It4', 'It5')]
    pair_labels = ['It1 vs It2', 'It2 vs It3', 'It3 vs It4', 'It4 vs It5']

    x = np.arange(len(pairs))
    width = 0.35

    bot_a_lr = [PAIR_DATA[pair]['bot_a_lr'] for pair in pairs]
    bot_b_lr = [PAIR_DATA[pair]['bot_b_lr'] for pair in pairs]

    bars1 = ax.bar(x - width/2, bot_a_lr, width, label='Poprzednia iteracja',
                   color='#3498db', alpha=0.8, edgecolor='black', linewidth=1.2)
    bars2 = ax.bar(x + width/2, bot_b_lr, width, label='Kolejna iteracja',
                   color='#9b59b6', alpha=0.8, edgecolor='black', linewidth=1.2)

    for bars in [bars1, bars2]:
        for bar in bars:
            height = bar.get_height()
            if height > 0:
                ax.text(bar.get_x() + bar.get_width()/2., height + 1,
                        f'{height:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=9)

    ax.set_xlabel('Para botów', fontsize=12, fontweight='bold')
    ax.set_ylabel('Najdłuższa Droga (%)', fontsize=12, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(pair_labels)
    ax.set_ylim([0, 105])
    ax.grid(True, alpha=0.3, axis='y')
    ax.legend(loc='upper right')
    plt.tight_layout()
    plt.savefig(output_dir / 'scenario2_longest_road.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'scenario2_longest_road.png'}")




def main():
    parser = argparse.ArgumentParser(description='Wizualizacja wyników Scenariusza 2')
    parser.add_argument('--output-dir', type=Path, default=Path('plots'),
                       help='Katalog wyjściowy dla wykresów')
    args = parser.parse_args()
    
    args.output_dir.mkdir(exist_ok=True)
    
    print("Generowanie wykresów Scenariusza 2...")
    plot_pairwise_comparison(args.output_dir)
    plot_tempo_progression(args.output_dir)
    plot_largest_army_pct(args.output_dir)
    plot_longest_road_pct(args.output_dir)

    print(f"\nWszystkie wykresy zapisane w: {args.output_dir}")
    print("\nWygenerowane wykresy:")
    print("  1. scenario2_pairwise_comparison.png - Progresja w parach")
    print("  2. scenario2_tempo.png - Tempo rozgrywki")
    print("  3. scenario2_largest_army.png - Największa Armia (LA%)")
    print("  4. scenario2_longest_road.png - Najdłuższa Droga (LR%)")


if __name__ == '__main__':
    main()
