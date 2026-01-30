import random
data = {    
    ("It5", "Random"): 100,
    ("It5", "It1"): 100,
    ("It5", "It2"): 100,
    ("It5", "It3"): 79,
    ("It5", "It4"): 74,
    ("It5", "It5"): 47,
    ("It5", "Para"): 77,
    ("It5", "ParaSettle"): 47,
    ("It5", "AlphaBeta"): 44,
    ("It5", "OneResource"): 83,
    ("It5", "Dev"): 75,
    ("It5", "Road"): 58,
    ("It5", "CityRush"): 51,
}

data2 = {
    ('It5', 'Random'): 100.0, 
    ('It5', 'It1'): 100.0, 
    ('It5', 'It2'): 98.6, 
    ('It5', 'It3'): 77.4, 
    ('It5', 'It4'): 69.6, 
    ('It5', 'It5'): 45.9, 
    ('It5', 'Para'): 76.7, 
    ('It5', 'ParaSettle'): 48.3, 
    ('It5', 'AlphaBeta'): 39.5, 
    ('It5', 'OneResource'): 74.1, 
    ('It5', 'Dev'): 68.0, 
    ('It5', 'Road'): 54.9, 
    ('It5', 'CityRush'): 48.1}

diffs = {
    (a, b): data[(a, b)] - data2[(a, b)]
    for (a, b) in data.keys()
}

import matplotlib.pyplot as plt

bots = [
    "Random", "It1", "It2", "It3", "It4", "It5",
    "Para", "ParaSettle", "AlphaBeta",
    "OneResource", "Dev", "Road", "CityRush"
]
diff = [diffs[('It5', bot)] for bot in bots]
x = range(len(bots))
plt.figure(figsize=(12,6))
plt.bar(x, diff, color=['green' if d>0 else 'red' for d in diff])
plt.axhline(0, color='black', linewidth=1)
plt.yticks(range(-2, 12, 1))
plt.xticks(x, bots, rotation=45, ha='right')
plt.ylabel("Różnica początkowego ustawienia (%)")
plt.title("Porównanie wyników It5 przed i po genetycznym algorytmie")
plt.tight_layout()
plt.show()