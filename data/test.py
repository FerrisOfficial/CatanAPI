import matplotlib.pyplot as plt

# Para: RandomPlayer vs RandomPlayer
#   RandomPlayer (pierwszy) vs RandomPlayer (drugi)
#     Wygrane pierwszego: 48 (48.0%)
#     Wygrane drugiego:   44 (44.0%)
#     Brak zwycięzcy:      8 (8.0%)
#     Średnia liczba tur:  489.7, max tur: 1000
#   RandomPlayer (pierwszy) vs RandomPlayer (drugi)
#     Wygrane pierwszego: 53 (53.0%)
#     Wygrane drugiego:   38 (38.0%)
#     Brak zwycięzcy:      9 (9.0%)
#     Średnia liczba tur:  527.3, max tur: 1000

with open('C:\\Users\\Maciej\\CatanAPI\\data\\fise_results_100.txt', 'r', encoding='utf-8') as file:
    data = file.read()
lines = data.split('\n')

class Data:
    def __init__(self):
        self.bota_name = ""
        self.botb_name = ""
        self.bota1_wins = 0
        self.bota2_wins = 0
        self.botb1_wins = 0
        self.botb2_wins = 0

segment = []
for i in range(0, len(lines)):
    if lines[i].startswith('Para: '):
        segment.append(Data())
        segment[-1].bota_name = lines[i][6:lines[i].index(' vs')]
        segment[-1].botb_name = lines[i][lines[i].index('vs ') + 3:]
        i += 2
        segment[-1].bota1_wins = float(lines[i][lines[i].index(': ') + 2:lines[i].index(' (')])
        i += 1
        segment[-1].botb2_wins = float(lines[i][lines[i].index(': ') + 2:lines[i].index(' (')])
        i += 4
        segment[-1].botb1_wins = float(lines[i][lines[i].index(': ') + 2:lines[i].index(' (')])
        i += 1
        segment[-1].bota2_wins = float(lines[i][lines[i].index(': ') + 2:lines[i].index(' (')])

for data in segment:
    print(f"{data.bota_name} vs {data.botb_name}:")
    print(f"  {data.bota_name} \n\twins as first: {data.bota1_wins} \n\twins as second: {data.bota2_wins}")
    print(f"  {data.botb_name} \n\twins as first: {data.botb1_wins} \n\twins as second: {data.botb2_wins}")
    print()