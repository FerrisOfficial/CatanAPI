#!/usr/bin/env python3
"""
Skrypt do wizualizacji wyników Scenariusza 4: Porównanie wszystkich strategii wyspecjalizowanych.

Użycie:
    python utils/visualize_scenario4_all_specialized.py --output-dir docs/
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

# Dane z eksperymentów - wszystkie 3 strategie wyspecjalizowane
DATA = {
    'OneResource': {
        'It1': 98.3, 'It2': 95.5, 'It3': 56.8, 'It4': 36.4, 'It5': 19.0,
        'Para': 34.8, 'ParaSettleIt5': 18.2, 'AlphaBeta': 11.1
    },
    'Dev': {
        'It1': 98.8, 'It2': 97.5, 'It3': 69.1, 'It4': 49.3, 'It5': 27.3,
        'Para': 58.0, 'ParaSettleIt5': 23.6, 'AlphaBeta': 18.3
    },
    'Road': {
        'It1': 100.0, 'It2': 99.2, 'It3': 86.7, 'It4': 68.2, 'It5': 47.5,
        'Para': 74.2, 'ParaSettleIt5': 43.9, 'AlphaBeta': 33.7
    }
}

# Kolejność przeciwników
OPPONENTS = ['It1', 'It2', 'It3', 'It4', 'It5', 'Para', 'ParaSettleIt5', 'AlphaBeta']


def plot_all_specialized_comparison(output_dir: Path):
    """Wykres porównujący wszystkie 3 strategie wyspecjalizowane"""
    fig, ax = plt.subplots(figsize=(14, 8))
    
    x = np.arange(len(OPPONENTS))
    width = 0.25
    
    # Wykresy słupkowe dla każdej strategii
    bars1 = ax.bar(x - width, [DATA['OneResource'][opp] for opp in OPPONENTS], 
                   width, label='OneResourcePlayer', color='#e74c3c', alpha=0.7, 
                   edgecolor='black', linewidth=1.5)
    bars2 = ax.bar(x, [DATA['Dev'][opp] for opp in OPPONENTS], 
                   width, label='DevPlayer', color='#f39c12', alpha=0.7, 
                   edgecolor='black', linewidth=1.5)
    bars3 = ax.bar(x + width, [DATA['Road'][opp] for opp in OPPONENTS], 
                   width, label='RoadPlayer', color='#3498db', alpha=0.7, 
                   edgecolor='black', linewidth=1.5)
    
    # Dodaj wartości na słupkach
    for bars in [bars1, bars2, bars3]:
        for bar in bars:
            height = bar.get_height()
            if height > 0:
                ax.text(bar.get_x() + bar.get_width()/2., height + 1,
                       f'{height:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=8)
    
    # Linia pokazująca granicę równości
    ax.axhline(y=50, color='gray', linestyle='--', linewidth=2, alpha=0.5, label='Granica równości (50%)')
    
    ax.set_xlabel('Przeciwnik', fontsize=14, fontweight='bold')
    ax.set_ylabel('Współczynnik zwycięstw (%)', fontsize=14, fontweight='bold')

    ax.set_xticks(x)
    ax.set_xticklabels(OPPONENTS)
    ax.set_ylim([0, 105])
    ax.grid(True, alpha=0.3, axis='y')
    ax.legend(loc='upper right', fontsize=11)
    
    plt.tight_layout()
    plt.savefig(output_dir / 'scenario4_all_slupki.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'scenario4_all_slupki.png'}")


def plot_all_specialized_lines(output_dir: Path):
    """Wykres liniowy porównujący wszystkie 3 strategie"""
    fig, ax = plt.subplots(figsize=(14, 8))
    
    x = np.arange(len(OPPONENTS))
    
    # Linie dla każdej strategii
    line1 = ax.plot(x, [DATA['OneResource'][opp] for opp in OPPONENTS], 
                   'o-', linewidth=2.5, markersize=10, color='#e74c3c', label='OneResourcePlayer')
    line2 = ax.plot(x, [DATA['Dev'][opp] for opp in OPPONENTS], 
                   's-', linewidth=2.5, markersize=10, color='#f39c12', label='DevPlayer')
    line3 = ax.plot(x, [DATA['Road'][opp] for opp in OPPONENTS], 
                   '^-', linewidth=2.5, markersize=10, color='#3498db', label='RoadPlayer')
    
    # Dodaj wartości na punktach
    for i, opp in enumerate(OPPONENTS):
        ax.text(i, DATA['OneResource'][opp] + 2, f'{DATA["OneResource"][opp]:.1f}%', 
               ha='center', va='bottom', fontweight='bold', fontsize=8, color='#e74c3c')
        ax.text(i, DATA['Dev'][opp] + 2, f'{DATA["Dev"][opp]:.1f}%', 
               ha='center', va='bottom', fontweight='bold', fontsize=8, color='#f39c12')
        ax.text(i, DATA['Road'][opp] + 2, f'{DATA["Road"][opp]:.1f}%', 
               ha='center', va='bottom', fontweight='bold', fontsize=8, color='#3498db')
    
    # Linia pokazująca granicę równości
    ax.axhline(y=50.0, color='gray', linestyle='--', linewidth=2, 
               alpha=0.5, label='Linia remisu (50%)')
    
    # Linie pokazujące przepaści
    ax.axvline(x=2, color='red', linestyle='--', linewidth=2, alpha=0.5, label='Przepaść kompetencyjna')
    ax.axvline(x=3, color='orange', linestyle='--', linewidth=2, alpha=0.5, label='Punkt przełomowy')
    
    ax.set_xlabel('Przeciwnik', fontsize=14, fontweight='bold')
    ax.set_ylabel('Współczynnik zwycięstw (%)', fontsize=14, fontweight='bold')
    ax.set_title('Porównanie skuteczności strategii wyspecjalizowanych', 
                 fontsize=16, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(OPPONENTS)
    ax.set_ylim([0, 105])
    ax.grid(True, alpha=0.3)
    ax.legend(loc='upper right', fontsize=11)
    
    plt.tight_layout()
    plt.savefig(output_dir / 'scenario4_all_specialized_lines.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'scenario4_all_specialized_lines.png'}")


def plot_effectiveness_ranking(output_dir: Path):
    """Wykres pokazujący ranking skuteczności strategii dla każdego przeciwnika"""
    fig, axes = plt.subplots(2, 4, figsize=(16, 8))
    fig.suptitle('Ranking skuteczności strategii wyspecjalizowanych dla każdego przeciwnika', 
                 fontsize=16, fontweight='bold')
    
    strategies = ['OneResource', 'Dev', 'Road']
    colors = {'OneResource': '#e74c3c', 'Dev': '#f39c12', 'Road': '#3498db'}
    labels = {'OneResource': 'OneResource', 'Dev': 'Dev', 'Road': 'Road'}
    
    for idx, opp in enumerate(OPPONENTS):
        row = idx // 4
        col = idx % 4
        ax = axes[row, col]
        
        # Posortuj strategie według win rate
        win_rates = [(DATA[strat][opp], strat) for strat in strategies]
        win_rates.sort(reverse=True)
        
        bars = ax.barh(range(len(win_rates)), [wr[0] for wr in win_rates],
                      color=[colors[wr[1]] for wr in win_rates], alpha=0.7,
                      edgecolor='black', linewidth=1.5)
        
        # Dodaj wartości
        for i, (bar, (wr, strat)) in enumerate(zip(bars, win_rates)):
            width = bar.get_width()
            ax.text(width + 1, bar.get_y() + bar.get_height()/2,
                   f'{wr:.1f}%', ha='left', va='center', fontweight='bold', fontsize=9)
        
        ax.set_yticks(range(len(win_rates)))
        ax.set_yticklabels([labels[wr[1]] for wr in win_rates])
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
    """Wykres pokazujący przepaść kompetencyjną dla wszystkich strategii"""
    fig, ax = plt.subplots(figsize=(14, 8))
    
    x = np.arange(len(OPPONENTS))
    
    # Linie dla każdej strategii
    ax.plot(x, [DATA['OneResource'][opp] for opp in OPPONENTS], 
           'o-', linewidth=2.5, markersize=10, color='#e74c3c', label='OneResourcePlayer')
    ax.plot(x, [DATA['Dev'][opp] for opp in OPPONENTS], 
           's-', linewidth=2.5, markersize=10, color='#f39c12', label='DevPlayer')
    ax.plot(x, [DATA['Road'][opp] for opp in OPPONENTS], 
           '^-', linewidth=2.5, markersize=10, color='#3498db', label='RoadPlayer')
    
    # Linia pokazująca granicę równości
    ax.axhline(y=50.0, color='gray', linestyle='--', linewidth=2, 
               alpha=0.5, label='Linia remisu (50%)')
    
    ax.set_xlabel('Przeciwnik', fontsize=14, fontweight='bold')
    ax.set_ylabel('Współczynnik zwycięstw (%)', fontsize=14, fontweight='bold')

    ax.set_xticks(x)
    ax.set_xticklabels(OPPONENTS)
    ax.set_ylim([0, 105])
    ax.grid(True, alpha=0.3)
    ax.legend(loc='upper right', fontsize=11)
    
    plt.tight_layout()
    plt.savefig(output_dir / 'scenario4_all.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'scenario4_all.png'}")


def main():
    parser = argparse.ArgumentParser(description='Wizualizacja wyników Scenariusza 4 - wszystkie strategie wyspecjalizowane')
    parser.add_argument('--output-dir', type=Path, default=Path('plots'),
                       help='Katalog wyjściowy dla wykresów')
    args = parser.parse_args()
    
    args.output_dir.mkdir(exist_ok=True)
    
    print("Generowanie wykresów Scenariusza 4 - wszystkie strategie wyspecjalizowane...")
    # plot_all_specialized_comparison(args.output_dir)
    # plot_all_specialized_lines(args.output_dir)
    # plot_effectiveness_ranking(args.output_dir)
    plot_competency_gap_comparison(args.output_dir)
    
    print(f"\nWszystkie wykresy zapisane w: {args.output_dir}")
    print("\nWygenerowane wykresy:")
    # print("  1. scenario4_all_slupki.png - Porównanie słupkowe")
    # print("  2. scenario4_all_specialized_lines.png - Porównanie liniowe")
    # print("  3. scenario4_all_specialized_ranking.png - Ranking dla każdego przeciwnika")
    print("  4. scenario4_all.png - Przepaść kompetencyjna dla wszystkich")


if __name__ == '__main__':
    main()
