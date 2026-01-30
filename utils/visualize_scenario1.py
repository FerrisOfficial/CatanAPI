#!/usr/bin/env python3
"""
Skrypt do wizualizacji wyników Scenariusza 1: Skalowanie jakości botów.

Użycie:
    python utils/visualize_scenario1.py --output-dir docs/
"""

import argparse
import sys
from pathlib import Path
from typing import Dict, List
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.patches import Rectangle
import seaborn as sns

# Konfiguracja stylu
sns.set_style("whitegrid")
plt.rcParams['figure.figsize'] = (12, 8)
plt.rcParams['font.size'] = 10

# Dane z eksperymentów
DATA = {
    'It1': {'win_rate': 56.0, 'avg_turns': 448.5, 'loss_vp_random': 6.81, 'loss_vp_bot': 7.51,
            'lr_pct': 51.2, 'la_pct': 58.9, 'dev_cards': 3.0, 'prod_score': 730.4, 'random_wins': 364},
    'It2': {'win_rate': 73.3, 'avg_turns': 381.0, 'loss_vp_random': 6.23, 'loss_vp_bot': 8.48,
            'lr_pct': 64.4, 'la_pct': 62.0, 'dev_cards': 3.0, 'prod_score': 864.3, 'random_wins': 231},
    'It3': {'win_rate': 98.7, 'avg_turns': 180.3, 'loss_vp_random': 3.98, 'loss_vp_bot': 11.50,
            'lr_pct': 83.7, 'la_pct': 91.9, 'dev_cards': 3.2, 'prod_score': 1003.2, 'random_wins': 10},
    'It4': {'win_rate': 96.9, 'avg_turns': 192.6, 'loss_vp_random': 3.64, 'loss_vp_bot': 9.80,
            'lr_pct': 94.6, 'la_pct': 86.7, 'dev_cards': 3.0, 'prod_score': 1190.3, 'random_wins': 15},
    'It5': {'win_rate': 100.0, 'avg_turns': 115.3, 'loss_vp_random': 3.21, 'loss_vp_bot': None,
            'lr_pct': 71.5, 'la_pct': 98.5, 'dev_cards': 3.7, 'prod_score': 1096.4, 'random_wins': 0},
    'Para': {'win_rate': 99.5, 'avg_turns': 125.1, 'loss_vp_random': 3.34, 'loss_vp_bot': 4.80,
             'lr_pct': 90.2, 'la_pct': 25.9, 'dev_cards': 9.9, 'prod_score': 1267.8, 'random_wins': 5},
    'ParaSettleIt5': {'win_rate': 100.0, 'avg_turns': 111.0, 'loss_vp_random': 3.21, 'loss_vp_bot': None,
                      'lr_pct': 68.3, 'la_pct': 98.8, 'dev_cards': 3.7, 'prod_score': 1111.5, 'random_wins': 0},
    'OneResource': {'win_rate': 99.8, 'avg_turns': 159.9, 'loss_vp_random': 3.79, 'loss_vp_bot': 11.50,
                    'lr_pct': 82.0, 'la_pct': 92.4, 'dev_cards': 2.9, 'prod_score': 1117.0, 'random_wins': 2},
    'Dev': {'win_rate': 99.8, 'avg_turns': 166.1, 'loss_vp_random': 4.00, 'loss_vp_bot': 12.00,
            'lr_pct': 52.8, 'la_pct': 99.7, 'dev_cards': 4.6, 'prod_score': 1064.8, 'random_wins': 1},
    'Road': {'win_rate': 100.0, 'avg_turns': 132.7, 'loss_vp_random': 3.22, 'loss_vp_bot': None,
             'lr_pct': 94.5, 'la_pct': 98.6, 'dev_cards': 3.6, 'prod_score': 1051.0, 'random_wins': 0},
    'AlphaBeta': {'win_rate': 100.0, 'avg_turns': 108.7, 'loss_vp_random': 2.97, 'loss_vp_bot': None,
                  'lr_pct': 96.2, 'la_pct': 91.8, 'dev_cards': 2.5, 'prod_score': 1168.2, 'random_wins': 0},
}

# Boty heurystyczne (do wykresów progresji)
BOTS = ['It1', 'It2', 'It3', 'It4', 'It5', 'Para', 'ParaSettleIt5', 'OneResource', 'Dev', 'Road', 'AlphaBeta']


def plot_progression(output_dir: Path):
    """Wykres 1: Progresja jakości botów heurystycznych"""
    fig, ax = plt.subplots(figsize=(10, 6))
    
    bots = BOTS
    win_rates = [DATA[bot]['win_rate'] for bot in bots]
    random_wins = [DATA[bot]['random_wins'] for bot in bots]
    
    # Przedziały ufności 95% (Wilson score interval)
    n = 1000
    z = 1.96
    ci_lower = []
    ci_upper = []
    for wr in win_rates:
        p = wr / 100.0
        denominator = 1 + (z**2 / n)
        centre = (p + (z**2 / (2 * n))) / denominator
        std_adj = np.sqrt((p * (1 - p) + z**2 / (4 * n)) / n) / denominator
        ci_lower.append((centre - z * std_adj) * 100)
        ci_upper.append((centre + z * std_adj) * 100)
    
    x = np.arange(len(bots))
    ax.plot(x, win_rates, 'o-', linewidth=2, markersize=8, label='Win Rate', color='#2ecc71')
    
    ax.set_xlabel('Iteracja bota heurystycznego', fontsize=12, fontweight='bold')
    ax.set_ylabel('Współczynnik zwycięstw (%)', fontsize=12, fontweight='bold')

    ax.set_xticks(x)
    ax.set_xticklabels(bots)
    ax.set_ylim([0, 105])
    ax.grid(True, alpha=0.3)
    ax.legend(loc='lower right')

    
    plt.tight_layout()
    plt.savefig(output_dir / 'rp_progression.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'rp_progression.png'}")


def plot_tempo(output_dir: Path):
    """Wykres 2: Tempo rozgrywki"""
    fig, ax = plt.subplots(figsize=(12, 6))
    
    bots = BOTS
    avg_turns = [DATA[bot]['avg_turns'] for bot in bots]
    
    x = np.arange(len(bots))
    bars = ax.bar(x, avg_turns, color='#16a085', alpha=0.7, edgecolor='black', linewidth=1.5)
    
    # Dodaj wartości na słupkach
    for bar, val in zip(bars, avg_turns):
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height + 5,
               f'{val:.1f}', ha='center', va='bottom', fontweight='bold')
    
    ax.set_xlabel('Bot', fontsize=12, fontweight='bold')
    ax.set_ylabel('Średnia liczba tur', fontsize=12, fontweight='bold')

    ax.set_xticks(x)
    ax.set_xticklabels(bots, rotation=45, ha='right')
    ax.grid(True, alpha=0.3, axis='y')
    
    plt.tight_layout()
    plt.savefig(output_dir / 'rp_tempo.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'rp_tempo.png'}")


def plot_competency_gap(output_dir: Path):
    """Wykres 3: Przepaść kompetencyjna - spadek szans Random"""
    fig, ax = plt.subplots(figsize=(10, 6))
    
    bots = BOTS
    random_wins = [DATA[bot]['random_wins'] for bot in bots]
    random_win_rate = [w / 10.0 for w in random_wins]  # z 1000 rozgrywek
    
    x = np.arange(len(bots))
    bars = ax.bar(x, random_win_rate, color=['#e74c3c', '#e74c3c', '#95a5a6', '#95a5a6', '#95a5a6'], 
                   alpha=0.7, edgecolor='black', linewidth=1.5)
    
    # Linia pokazująca "przepaść"
    ax.axvline(x=1.5, color='red', linestyle='--', linewidth=3, alpha=0.7, label='Przepaść kompetencyjna')
    
    # Dodaj wartości na słupkach
    for i, (bar, val) in enumerate(zip(bars, random_win_rate)):
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height + 1,
                f'{val:.1f}%', ha='center', va='bottom', fontweight='bold')
    
    ax.set_xlabel('Iteracja bota heurystycznego', fontsize=12, fontweight='bold')
    ax.set_ylabel('Współczynnik zwycięstw RandomPlayer (%)', fontsize=12, fontweight='bold')

    ax.set_xticks(x)
    ax.set_xticklabels(bots)
    ax.set_ylim([0, max(random_win_rate) * 1.2])
    ax.grid(True, alpha=0.3, axis='y')
    ax.legend(loc='upper right')
    
    plt.tight_layout()
    plt.savefig(output_dir / '3_competency_gap.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / '3_competency_gap.png'}")


def plot_dominance(output_dir: Path):
    """Wykres 4: Dominacja - LossVP przeciwnika"""
    fig, ax = plt.subplots(figsize=(12, 6))
    
    bots = BOTS
    loss_vp_bot = [DATA[bot]['loss_vp_bot'] if DATA[bot]['loss_vp_bot'] is not None else 0 
                   for bot in bots]
    
    x = np.arange(len(bots))
    bars = ax.bar(x, loss_vp_bot, color='#9b59b6', alpha=0.7, edgecolor='black', linewidth=1.5)
    
    # Dodaj wartości na słupkach
    for bar, val in zip(bars, loss_vp_bot):
        if val > 0:
            height = bar.get_height()
            ax.text(bar.get_x() + bar.get_width()/2., height + 0.2,
                    f'{val:.2f}', ha='center', va='bottom', fontweight='bold')
    
    ax.set_xlabel('Iteracja bota heurystycznego', fontsize=12, fontweight='bold')
    ax.set_ylabel('Średnia liczba VP przeciwnika przy przegranej', fontsize=12, fontweight='bold')

    ax.set_xticks(x)
    ax.set_xticklabels(bots)
    ax.set_ylim([0, max(loss_vp_bot) * 1.15])
    ax.grid(True, alpha=0.3, axis='y')
    
    plt.tight_layout()
    plt.savefig(output_dir / 'rp_dominance.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'rp_dominance.png'}")


def plot_radar_chart(output_dir: Path):
    """Wykres 5: Radar chart - profil strategii"""
    fig, ax = plt.subplots(figsize=(10, 10), subplot_kw=dict(projection='polar'))
    
    # Wybierz kilka botów do porównania
    selected_bots = ['It1', 'It3', 'It5', 'Para', 'Dev', 'Road']
    
    # Normalizuj metryki do skali 0-100
    metrics = ['lr_pct', 'la_pct', 'dev_cards', 'prod_score', 'win_rate']
    metric_labels = ['Najdłuższa\nDroga (%)', 'Największa\nArmia (%)', 
                     'Karty rozwoju\n(x10)', 'Produkcja\n(x10)', 'Win Rate (%)']
    
    # Normalizacja
    max_values = {
        'lr_pct': 100,
        'la_pct': 100,
        'dev_cards': 10,
        'prod_score': 1300,
        'win_rate': 100
    }
    
    angles = np.linspace(0, 2 * np.pi, len(metrics), endpoint=False).tolist()
    angles += angles[:1]  # Zamknij wykres
    
    colors = plt.cm.Set3(np.linspace(0, 1, len(selected_bots)))
    
    for i, bot in enumerate(selected_bots):
        values = []
        for metric in metrics:
            val = DATA[bot][metric]
            normalized = (val / max_values[metric]) * 100
            values.append(normalized)
        values += values[:1]  # Zamknij wykres
        
        ax.plot(angles, values, 'o-', linewidth=2, label=bot, color=colors[i])
        ax.fill(angles, values, alpha=0.25, color=colors[i])
    
    ax.set_xticks(angles[:-1])
    ax.set_xticklabels(metric_labels)
    ax.set_ylim(0, 100)
    ax.set_title('Porównanie profili strategicznych botów', 
                 fontsize=14, fontweight='bold', pad=20)
    ax.legend(loc='upper right', bbox_to_anchor=(1.3, 1.1))
    ax.grid(True)
    
    plt.tight_layout()
    plt.savefig(output_dir / '5_radar_chart.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / '5_radar_chart.png'}")


def plot_efficiency(output_dir: Path):
    """Wykres 6: Efektywność - zwycięstwa na turę"""
    fig, ax = plt.subplots(figsize=(10, 6))
    
    bots = list(DATA.keys())
    efficiency = [DATA[bot]['win_rate'] / DATA[bot]['avg_turns'] * 100 for bot in bots]
    
    # Sortuj według efektywności
    sorted_data = sorted(zip(bots, efficiency), key=lambda x: x[1], reverse=True)
    bots_sorted, efficiency_sorted = zip(*sorted_data)
    
    colors = ['#2ecc71' if bot in BOTS else '#e74c3c' for bot in bots_sorted]
    
    bars = ax.barh(range(len(bots_sorted)), efficiency_sorted, color=colors, alpha=0.7, 
                   edgecolor='black', linewidth=1.5)
    
    # Dodaj wartości
    for i, (bar, val) in enumerate(zip(bars, efficiency_sorted)):
        width = bar.get_width()
        ax.text(width + 0.01, bar.get_y() + bar.get_height()/2,
                f'{val:.3f}', ha='left', va='center', fontweight='bold')
    
    ax.set_xlabel('Efektywność (Win Rate / Avg Turns × 100)', fontsize=12, fontweight='bold')
    ax.set_ylabel('Bot', fontsize=12, fontweight='bold')
    ax.set_title('Efektywność botów (zwycięstwa na turę)', 
                 fontsize=14, fontweight='bold')
    ax.set_yticks(range(len(bots_sorted)))
    ax.set_yticklabels(bots_sorted)
    ax.grid(True, alpha=0.3, axis='x')
    
    # Legenda
    from matplotlib.patches import Patch
    legend_elements = [
        Patch(facecolor='#2ecc71', label='Boty heurystyczne'),
        Patch(facecolor='#e74c3c', label='Boty eksperymentalne')
    ]
    ax.legend(handles=legend_elements, loc='lower right')
    
    plt.tight_layout()
    plt.savefig(output_dir / '6_efficiency.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / '6_efficiency.png'}")


def main():
    parser = argparse.ArgumentParser(description='Wizualizacja wyników Scenariusza 1')
    parser.add_argument('--output-dir', type=Path, default=Path('plots'),
                       help='Katalog wyjściowy dla wykresów')
    args = parser.parse_args()
    
    args.output_dir.mkdir(exist_ok=True)
    
    print("Generowanie wykresu...")
    plot_progression(args.output_dir)
    plot_dominance(args.output_dir)
    # plot_competency_gap(args.output_dir)
    plot_tempo(args.output_dir)
    # plot_radar_chart(args.output_dir)
    # plot_efficiency(args.output_dir)
    
    print(f"\nWykres zapisany w: {args.output_dir}")


if __name__ == '__main__':
    main()
