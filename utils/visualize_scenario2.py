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
    ax.set_title('Progresja skuteczności w bezpośrednich starciach\n' +
                 'Każda kolejna iteracja wygrywa z poprzednią', 
                 fontsize=14, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(pair_labels)
    ax.set_ylim([0, 105])
    ax.grid(True, alpha=0.3, axis='y')
    ax.legend(loc='upper left')
    
    plt.tight_layout()
    plt.savefig(output_dir / 'scenario2_1_pairwise_comparison.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'scenario2_1_pairwise_comparison.png'}")


def plot_advantage_gap(output_dir: Path):
    """Wykres 2: Przewaga kolejnej iteracji"""
    fig, ax = plt.subplots(figsize=(10, 6))
    
    pairs = [('It1', 'It2'), ('It2', 'It3'), ('It3', 'It4'), ('It4', 'It5')]
    transitions = ['It1→It2', 'It2→It3', 'It3→It4', 'It4→It5']
    
    advantages = [PAIR_DATA[pair]['bot_b_win_rate'] - PAIR_DATA[pair]['bot_a_win_rate'] 
                  for pair in pairs]
    
    x = np.arange(len(transitions))
    colors = ['#3498db', '#e74c3c', '#3498db', '#3498db']  # It2→It3 na czerwono
    
    bars = ax.bar(x, advantages, color=colors, alpha=0.7, edgecolor='black', linewidth=1.5)
    
    # Dodaj wartości na słupkach
    for bar, val in zip(bars, advantages):
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height + 1,
               f'+{val:.1f}%', ha='center', va='bottom', fontweight='bold')
    
    ax.set_xlabel('Przejście między iteracjami', fontsize=12, fontweight='bold')
    ax.set_ylabel('Przewaga kolejnej iteracji (punkty procentowe)', fontsize=12, fontweight='bold')
    ax.set_title('Przewaga kolejnej iteracji nad poprzednią', 
                 fontsize=14, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(transitions)
    ax.set_ylim([0, max(advantages) * 1.15])
    ax.grid(True, alpha=0.3, axis='y')
    
    plt.tight_layout()
    plt.savefig(output_dir / 'scenario2_2_advantage_gap.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'scenario2_2_advantage_gap.png'}")


def plot_competency_gap_detail(output_dir: Path):
    """Wykres 3: Szczegółowa analiza przepaści It2→It3"""
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    
    data = PAIR_DATA[('It2', 'It3')]
    bots = ['It2', 'It3']
    
    # Win Rate
    ax = axes[0, 0]
    win_rates = [data['bot_a_win_rate'], data['bot_b_win_rate']]
    bars = ax.bar(bots, win_rates, color=['#e74c3c', '#2ecc71'], alpha=0.7, 
                  edgecolor='black', linewidth=2)
    for bar, val in zip(bars, win_rates):
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height + 1,
               f'{val:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=12)
    ax.set_ylabel('Win Rate (%)', fontweight='bold')
    ax.set_title('Współczynnik zwycięstw', fontweight='bold')
    ax.set_ylim([0, 100])
    ax.grid(True, alpha=0.3, axis='y')
    
    # Produkcja zasobów
    ax = axes[0, 1]
    prod_scores = [data['bot_a_prod'], data['bot_b_prod']]
    bars = ax.bar(bots, prod_scores, color=['#e74c3c', '#2ecc71'], alpha=0.7,
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
    la_pcts = [data['bot_a_la'], data['bot_b_la']]
    bars = ax.bar(bots, la_pcts, color=['#e74c3c', '#2ecc71'], alpha=0.7,
                  edgecolor='black', linewidth=2)
    for bar, val in zip(bars, la_pcts):
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height + 2,
               f'{val:.1f}%', ha='center', va='bottom', fontweight='bold', fontsize=12)
    ax.set_ylabel('Największa Armia (%)', fontweight='bold')
    ax.set_title('Wykorzystanie premii LA', fontweight='bold')
    ax.set_ylim([0, 100])
    ax.grid(True, alpha=0.3, axis='y')
    
    # LossVP (dominacja)
    ax = axes[1, 1]
    loss_vps = [data['bot_a_loss_vp'], data['bot_b_loss_vp']]
    bars = ax.bar(bots, loss_vps, color=['#e74c3c', '#2ecc71'], alpha=0.7,
                  edgecolor='black', linewidth=2)
    for bar, val in zip(bars, loss_vps):
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height + 0.2,
               f'{val:.2f}', ha='center', va='bottom', fontweight='bold', fontsize=12)
    ax.set_ylabel('Średnia VP przeciwnika przy przegranej', fontweight='bold')
    ax.set_title('Dominacja (LossVP)', fontweight='bold')
    ax.grid(True, alpha=0.3, axis='y')
    
    plt.tight_layout()
    plt.savefig(output_dir / 'scenario2_3_competency_gap_detail.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'scenario2_3_competency_gap_detail.png'}")


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
    ax.set_title('Tempo rozgrywki w zależności od iteracji', 
                 fontsize=14, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(pair_labels, rotation=45, ha='right')
    ax.grid(True, alpha=0.3, axis='y')
    ax.legend()
    
    plt.tight_layout()
    plt.savefig(output_dir / 'scenario2_4_tempo_progression.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'scenario2_4_tempo_progression.png'}")


def plot_metrics_radar(output_dir: Path):
    """Wykres 5: Radar chart - profil strategii"""
    fig, ax = plt.subplots(figsize=(10, 10), subplot_kw=dict(projection='polar'))
    
    bots = ['It1', 'It2', 'It3', 'It4', 'It5']
    
    # Zbierz dane dla każdego bota (użyj danych z par)
    bot_metrics = {}
    for bot in bots:
        bot_metrics[bot] = {'lr': 0, 'la': 0, 'dev': 0, 'prod': 0, 'win_rate': 0}
    
    # It1 - z pary It1 vs It2
    bot_metrics['It1'] = {
        'lr': PAIR_DATA[('It1', 'It2')]['bot_a_lr'],
        'la': PAIR_DATA[('It1', 'It2')]['bot_a_la'],
        'dev': PAIR_DATA[('It1', 'It2')]['bot_a_dev'],
        'prod': PAIR_DATA[('It1', 'It2')]['bot_a_prod'],
        'win_rate': PAIR_DATA[('It1', 'It2')]['bot_a_win_rate']
    }
    
    # It2 - z pary It2 vs It3 (użyj danych gdy It2 wygrywa)
    bot_metrics['It2'] = {
        'lr': PAIR_DATA[('It2', 'It3')]['bot_a_lr'],
        'la': PAIR_DATA[('It2', 'It3')]['bot_a_la'],
        'dev': PAIR_DATA[('It2', 'It3')]['bot_a_dev'],
        'prod': PAIR_DATA[('It2', 'It3')]['bot_a_prod'],
        'win_rate': PAIR_DATA[('It2', 'It3')]['bot_a_win_rate']
    }
    
    # It3 - z pary It3 vs It4
    bot_metrics['It3'] = {
        'lr': PAIR_DATA[('It3', 'It4')]['bot_a_lr'],
        'la': PAIR_DATA[('It3', 'It4')]['bot_a_la'],
        'dev': PAIR_DATA[('It3', 'It4')]['bot_a_dev'],
        'prod': PAIR_DATA[('It3', 'It4')]['bot_a_prod'],
        'win_rate': PAIR_DATA[('It3', 'It4')]['bot_a_win_rate']
    }
    
    # It4 - z pary It4 vs It5
    bot_metrics['It4'] = {
        'lr': PAIR_DATA[('It4', 'It5')]['bot_a_lr'],
        'la': PAIR_DATA[('It4', 'It5')]['bot_a_la'],
        'dev': PAIR_DATA[('It4', 'It5')]['bot_a_dev'],
        'prod': PAIR_DATA[('It4', 'It5')]['bot_a_prod'],
        'win_rate': PAIR_DATA[('It4', 'It5')]['bot_a_win_rate']
    }
    
    # It5 - z pary It1 vs It5
    bot_metrics['It5'] = {
        'lr': PAIR_DATA[('It1', 'It5')]['bot_b_lr'],
        'la': PAIR_DATA[('It1', 'It5')]['bot_b_la'],
        'dev': PAIR_DATA[('It1', 'It5')]['bot_b_dev'],
        'prod': PAIR_DATA[('It1', 'It5')]['bot_b_prod'],
        'win_rate': PAIR_DATA[('It1', 'It5')]['bot_b_win_rate']
    }
    
    # Normalizuj metryki
    metrics = ['lr', 'la', 'dev', 'prod', 'win_rate']
    metric_labels = ['Najdłuższa\nDroga (%)', 'Największa\nArmia (%)', 
                     'Karty rozwoju\n(x10)', 'Produkcja\n(x10)', 'Win Rate (%)']
    
    max_values = {'lr': 100, 'la': 100, 'dev': 4, 'prod': 1200, 'win_rate': 100}
    
    angles = np.linspace(0, 2 * np.pi, len(metrics), endpoint=False).tolist()
    angles += angles[:1]
    
    colors = plt.cm.Set3(np.linspace(0, 1, len(bots)))
    
    for i, bot in enumerate(bots):
        values = []
        for metric in metrics:
            val = bot_metrics[bot][metric]
            normalized = (val / max_values[metric]) * 100
            values.append(normalized)
        values += values[:1]
        
        ax.plot(angles, values, 'o-', linewidth=2, label=bot, color=colors[i])
        ax.fill(angles, values, alpha=0.15, color=colors[i])
    
    ax.set_xticks(angles[:-1])
    ax.set_xticklabels(metric_labels)
    ax.set_ylim(0, 100)
    ax.set_title('Ewolucja profilu strategicznego botów', 
                 fontsize=14, fontweight='bold', pad=20)
    ax.legend(loc='upper right', bbox_to_anchor=(1.3, 1.1))
    ax.grid(True)
    
    plt.tight_layout()
    plt.savefig(output_dir / 'scenario2_5_radar_chart.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'scenario2_5_radar_chart.png'}")


def plot_dominance_comparison(output_dir: Path):
    """Wykres 6: Dominacja - LossVP"""
    fig, ax = plt.subplots(figsize=(10, 6))
    
    pairs = [('It1', 'It2'), ('It2', 'It3'), ('It3', 'It4'), ('It4', 'It5')]
    pair_labels = ['It1 vs It2', 'It2 vs It3', 'It3 vs It4', 'It4 vs It5']
    
    loss_vps = [PAIR_DATA[pair]['bot_b_loss_vp'] for pair in pairs]
    
    x = np.arange(len(pair_labels))
    bars = ax.bar(x, loss_vps, color='#e67e22', alpha=0.7, edgecolor='black', linewidth=1.5)
    
    # Dodaj wartości na słupkach
    for bar, val in zip(bars, loss_vps):
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height + 0.2,
               f'{val:.2f}', ha='center', va='bottom', fontweight='bold')
    
    ax.set_xlabel('Para botów', fontsize=12, fontweight='bold')
    ax.set_ylabel('Średnia VP przeciwnika przy przegranej', fontsize=12, fontweight='bold')
    ax.set_title('Dominacja kolejnych iteracji\n' +
                 '(Jak bardzo lepszy bot dominuje słabszego)', 
                 fontsize=14, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(pair_labels, rotation=45, ha='right')
    ax.grid(True, alpha=0.3, axis='y')
    
    plt.tight_layout()
    plt.savefig(output_dir / 'scenario2_6_dominance.png', dpi=300, bbox_inches='tight')
    plt.close()
    print(f"Zapisano: {output_dir / 'scenario2_6_dominance.png'}")


def main():
    parser = argparse.ArgumentParser(description='Wizualizacja wyników Scenariusza 2')
    parser.add_argument('--output-dir', type=Path, default=Path('plots'),
                       help='Katalog wyjściowy dla wykresów')
    args = parser.parse_args()
    
    args.output_dir.mkdir(exist_ok=True)
    
    print("Generowanie wykresów Scenariusza 2...")
    plot_pairwise_comparison(args.output_dir)
    plot_advantage_gap(args.output_dir)
    plot_competency_gap_detail(args.output_dir)
    plot_tempo_progression(args.output_dir)
    plot_metrics_radar(args.output_dir)
    plot_dominance_comparison(args.output_dir)
    
    print(f"\nWszystkie wykresy zapisane w: {args.output_dir}")
    print("\nWygenerowane wykresy:")
    print("  1. scenario2_1_pairwise_comparison.png - Progresja w parach")
    print("  2. scenario2_2_advantage_gap.png - Przewaga kolejnej iteracji")
    print("  3. scenario2_3_competency_gap_detail.png - Szczegółowa analiza przepaści")
    print("  4. scenario2_4_tempo_progression.png - Tempo rozgrywki")
    print("  5. scenario2_5_radar_chart.png - Profil strategii (radar)")
    print("  6. scenario2_6_dominance.png - Dominacja botów")


if __name__ == '__main__':
    main()
