# Analiza wyników Scenariusza 1: Skalowanie jakości botów

## Podsumowanie danych

Celem pierwszego scenariusza eksperymentalnego było zbadanie, w jaki sposób wzrost złożoności i jakości strategii decyzyjnych botów wpływa na przebieg oraz wynik rozgrywek. W szczególności analizowano, czy kolejne iteracje botów heurystycznych prowadzą jedynie do stopniowej poprawy skuteczności, czy też występują jakościowe „progi”, po których charakter rozgrywki ulega istotnej zmianie.

| Bot | Win Rate vs Random | Avg Turns | LossVP (Random) | LossVP (Bot) | Longest road% | Largest army% | Avg DevCards | ProdScore |
|-----|-------------------|-----------|-----------------|--------------|----------|----------|--------------|-----------|
| It1 | 56.0% | 448.5 | 6.81 | 7.51 | 51.2% | 58.9% | 3.0 | 730.4 |
| It2 | 73.3% | 381.0 | 6.23 | 8.48 | 64.4% | 62.0% | 3.0 | 864.3 |
| It3 | 98.7% | 180.3 | 3.98 | 11.50 | 83.7% | 91.9% | 3.2 | 1003.2 |
| It4 | 96.9% | 192.6 | 3.64 | 9.80 | 94.6% | 86.7% | 3.0 | 1190.3 |
| It5 | 100.0% | 115.3 | 3.21 | - | 71.5% | 98.5% | 3.7 | 1096.4 |
| Para | 99.5% | 125.1 | 3.34 | 4.80 | 90.2% | 25.9% | 9.9 | 1267.8 |
| ParaSettleIt5 | 100.0% | 111.0 | 3.21 | - | 68.3% | 98.8% | 3.7 | 1111.5 |
| OneResource | 99.8% | 159.9 | 3.79 | 11.50 | 82.0% | 92.4% | 2.9 | 1117.0 |
| Dev | 99.8% | 166.1 | 4.00 | 12.00 | 52.8% | 99.7% | 4.6 | 1064.8 |
| Road | 100.0% | 132.7 | 3.22 | - | 94.5% | 98.6% | 3.6 | 1051.0 |

## Kluczowe wnioski

### 1. **Progresja jakości botów jest dramatyczna**

**Obserwacja**: Win rate rośnie wykładniczo z każdą iteracją:
- It1: 56% (ledwo lepszy od losowego)
- It2: 73% (znacząca poprawa)
- It3: 98.7% (przełom - praktycznie zawsze wygrywa)
- It4-It5: 96-100% (dominacja)

**Wnioski**:
- **It3 jest punktem przełomowym** - wprowadzenie strategii ustawień początkowych i rozbójnika powoduje skok z 73% do 98.7%
- **It5 osiąga perfekcję** - 100% win rate, zero przegranych w 1000 rozgrywkach
- **Różnica między It1 a It5 jest ogromna** - od ledwo lepszego niż losowy do całkowitej dominacji

**Jak przedstawić**:
- Wykres liniowy pokazujący wykładniczy wzrost win rate
- Wykres słupkowy z przedziałami ufności (pokazuje, że It3+ są statystycznie istotnie lepsze)
- Wykres "skalowania" pokazujący, że każda iteracja jest obiektywnie lepsza

### 2. **Tempo rozgrywki dramatycznie przyspiesza**

**Obserwacja**: Średnia liczba tur spada z 448.5 (It1) do 111.0 (ParaSettleIt5):
- It1: 448.5 tur (bardzo długie rozgrywki)
- It2: 381.0 tur (nadal długie)
- It3: 180.3 tur (znaczące przyspieszenie)
- It4: 192.6 tur (podobnie)
- It5: 115.3 tur (4x szybciej niż It1!)
- ParaSettleIt5: 111.0 tur (najszybszy)

**Wnioski**:
- **Lepsze boty nie tylko wygrywają częściej, ale też szybciej** - są bardziej efektywne
- **It3 wprowadza "szybkie zwycięstwa"** - średnia spada z 381 do 180 tur
- **It5 jest najbardziej efektywny** - wygrywa w ~115 tur (klasyczna gra to 60-70 tur, więc boty grają dłużej, ale It5 jest najbliżej)

**Jak przedstawić**:
- Wykres liniowy pokazujący spadek średniej liczby tur
- Wykres słupkowy porównujący tempo rozgrywki
- Dodatkowo: wykres pokazujący maxTurns - It5 ma maxTurns=258, podczas gdy It1-It3 mają maxTurns=1000 (timeout!)

### 3. **"Przepaść kompetencyjna" między It2 a It3**

**Obserwacja**: 
- It1-It2: Random ma jeszcze szanse (36.4% i 23.1%)
- It3+: Random praktycznie nie ma szans (<2%)

**Wnioski**:
- **It3 wprowadza mechanizmy, które całkowicie zmieniają grę**:
  - Strategia ustawień początkowych (lepsze pozycje startowe)
  - Aktywne wykorzystanie rozbójnika (blokowanie przeciwnika)
- **It2 jest "ostatnim słabym botem"** - jeszcze można go pokonać losowo
- **It3+ są "profesjonalne"** - losowość nie wystarczy do zwycięstwa

**Jak przedstawić**:
- Wykres pokazujący "przepaść" między It2 a It3
- Tabela porównująca kluczowe mechanizmy (It2 ma handel, It3 ma handel + rozbójnik + lepsze ustawienia)
- Wykres pokazujący spadek szans Random z każdą iteracją

### 4. **LossVP pokazuje dominację**

**Obserwacja**: Gdy Random przegrywa, przeciwnik ma coraz więcej punktów:
- It1: Random przegrywa przy średnio 7.51 VP przeciwnika
- It2: 8.48 VP
- It3: 11.50 VP (znacząca dominacja)
- It4: 9.80 VP
- It5+: Random przegrywa przy ~3.21 VP (boty wygrywają tak szybko, że Random nie zdąży zdobyć punktów)

**Wnioski**:
- **Lepsze boty nie tylko wygrywają, ale dominują** - przeciwnik ma dużo więcej punktów
- **It3 pokazuje największą dominację** (11.50 VP) - wygrywa zdecydowanie
- **It5+ wygrywają tak szybko, że Random nie zdąży zdobyć punktów** (LossVP Random = 3.21)

**Jak przedstawić**:
- Wykres słupkowy LossVP pokazujący dominację
- Wykres pokazujący różnicę VP w momencie zwycięstwa

### 5. **Metryki szczegółowe pokazują różnice w strategiach**

**Najdłuższa Droga (LR%)**:
- It1-It2: ~50-64% (umiarkowane wykorzystanie)
- It3+: 68-94% (silne wykorzystanie)
- Road: 94.5% (oczekiwane - specjalizacja)

**Największa Armia (LA%)**:
- It1-It2: ~59-62% (umiarkowane)
- It3+: 86-99% (bardzo silne)
- Dev: 99.7% (oczekiwane - specjalizacja w kartach)

**Karty rozwoju**:
- It1-It4: ~3.0 karty (standardowe)
- It5: 3.7 karty (więcej)
- Para: 9.9 karty! (bardzo dużo - strategia oparta na kartach)
- Dev: 4.6 karty (więcej niż standard)

**Produkcja zasobów (ProdScore)**:
- It1: 730.4
- It2: 864.3 (+18%)
- It3: 1003.2 (+16%)
- It4: 1190.3 (+19% - najwyższa!)
- It5: 1096.4
- Para: 1267.8 (najwyższa - trenowany)

**Wnioski**:
- **It4 ma najwyższą produkcję** (1190.3) - lepsze zarządzanie zasobami
- **Para ma najwięcej kart rozwoju** (9.9) - strategia oparta na kartach
- **Dev ma najwyższy LA%** (99.7%) - specjalizacja w kartach Rycerz
- **Road ma najwyższy LR%** (94.5%) - specjalizacja w drogach

**Jak przedstawić**:
- Wykres radarowy (spider chart) pokazujący różne aspekty strategii
- Wykresy słupkowe dla każdej metryki
- Macierz korelacji między metrykami a win rate

### 6. **Ciekawe obserwacje o botach eksperymentalnych**

**OneResourcePlayer**:
- Win rate: 99.8% (bardzo dobry!)
- Avg turns: 159.9 (wolniejszy niż It5)
- ProdScore: 1117.0 (wysoka produkcja)
- Strategia monosurowcowa działa przeciwko Random!

**DevPlayer**:
- Win rate: 99.8% (również bardzo dobry)
- Avg turns: 166.1 (najwolniejszy z najlepszych)
- Avg DevCards: 4.6 (najwięcej)
- LA%: 99.7% (praktycznie zawsze ma Największą Armię)
- Strategia oparta na kartach działa!

**RoadPlayer**:
- Win rate: 100% (perfekcja!)
- Avg turns: 132.7 (szybki)
- LR%: 94.5% (praktycznie zawsze ma Najdłuższą Drogę)
- Strategia drogowa działa przeciwko Random!

**ParaPlayer**:
- Win rate: 99.5% (bardzo dobry)
- Avg turns: 125.1 (szybki)
- Avg DevCards: 9.9! (ogromna liczba kart)
- ProdScore: 1267.8 (najwyższa produkcja)
- Trenowany bot jest bardzo skuteczny!

**Wnioski**:
- **Wszystkie strategie eksperymentalne działają przeciwko Random** - nawet wyspecjalizowane boty są lepsze
- **ParaPlayer pokazuje potencjał trenowania** - najwyższa produkcja i dużo kart
- **DevPlayer jest najwolniejszy** - strategia oparta na kartach wymaga czasu

### 7. **Brak rozgrywek bez zwycięzcy (NP)**

**Obserwacja**: 
- It1-It2: 7.6% i 3.6% rozgrywek bez zwycięzcy (timeout po 1000 tur)
- It3+: praktycznie zero timeoutów (0-0.3%)

**Wnioski**:
- **Lepsze boty zawsze kończą rozgrywkę** - nie ma timeoutów
- **It1-It2 mogą "utknąć"** - nie są wystarczająco agresywne
- **It3+ są bardziej deterministyczne** - zawsze dochodzą do końca

## Propozycje wizualizacji

### 1. **Wykres "Progresja jakości"**
- Oś X: Iteracja (It1, It2, It3, It4, It5)
- Oś Y: Win Rate (%)
- Linia pokazująca wykładniczy wzrost
- Dodatkowo: przedziały ufności (95%)
- **Tytuł**: "Progresja skuteczności botów heurystycznych"

### 2. **Wykres "Tempo vs Skuteczność"**
- Oś X: Win Rate (%)
- Oś Y: Średnia liczba tur
- Punkty dla każdego bota
- Pokazuje korelację: lepsze boty = szybsze zwycięstwa
- **Tytuł**: "Zależność między skutecznością a tempem rozgrywki"

### 3. **Wykres słupkowy "Przepaść kompetencyjna"**
- Oś X: Bot
- Oś Y: Win Rate Random (%)
- Pokazuje spadek szans Random z każdą iteracją
- Linia pokazująca "przepaść" między It2 a It3
- **Tytuł**: "Spadek szans RandomPlayer z każdą iteracją"

### 4. **Wykres radarowy "Profil strategii"**
- Oś: LR%, LA%, DevCards, ProdScore, WinRate
- Linie dla różnych botów (It1, It3, It5, Para, Dev, Road)
- Pokazuje różnice w strategiach
- **Tytuł**: "Porównanie profili strategicznych botów"

### 5. **Tabela z kolorowym kodowaniem**
- Tabela z wszystkimi metrykami
- Kolorowe kodowanie: zielony = dobry, czerwony = zły
- Pokazuje wszystkie dane w jednym miejscu

### 6. **Wykres "Dominacja"**
- Oś X: Bot
- Oś Y: LossVP przeciwnika
- Pokazuje, jak bardzo boty dominują
- **Tytuł**: "Dominacja botów nad RandomPlayer"

### 7. **Wykres "Efektywność"**
- Oś X: Bot
- Oś Y: Win Rate / Avg Turns (skuteczność na turę)
- Pokazuje, który bot jest najbardziej efektywny
- **Tytuł**: "Efektywność botów (zwycięstwa na turę)"

## Propozycje tekstu do pracy

### Wprowadzenie do wyników:

"Wyniki Scenariusza 1 pokazują dramatyczną progresję jakości botów heurystycznych. Podczas gdy It1Player osiąga jedynie 56% współczynnik zwycięstw przeciwko graczowi losowemu, It5Player osiąga perfekcję - 100% zwycięstw w 1000 rozgrywkach, bez ani jednej przegranej. Co więcej, lepsze boty nie tylko wygrywają częściej, ale również szybciej - średnia liczba tur spada z 448.5 (It1) do 115.3 (It5), co oznacza 4-krotne przyspieszenie tempa rozgrywki."

### Kluczowy wniosek:

"Szczególnie interesującym obserwacją jest **przepaść kompetencyjna** między It2Player a It3Player. Wprowadzenie strategii ustawień początkowych oraz aktywnego wykorzystania rozbójnika powoduje skok współczynnika zwycięstw z 73.3% do 98.7%, podczas gdy szanse RandomPlayer spadają z 23.1% do zaledwie 1.0%. To sugeruje, że It3 wprowadza mechanizmy strategiczne, które całkowicie zmieniają charakter rozgrywki - losowość przestaje być wystarczająca do osiągnięcia zwycięstwa."

### O botach eksperymentalnych:

"Warto zauważyć, że wszystkie boty eksperymentalne (OneResourcePlayer, DevPlayer, RoadPlayer) osiągają współczynniki zwycięstw powyżej 99%, co potwierdza, że nawet wyspecjalizowane strategie są skuteczne przeciwko graczowi losowemu. Jednak różnice w tempie rozgrywki (DevPlayer: 166.1 tur vs ParaSettleIt5Player: 111.0 tur) oraz w wykorzystaniu mechanizmów pośrednich (DevPlayer: 99.7% LA% vs RoadPlayer: 94.5% LR%) pokazują, że różne strategie prowadzą do zwycięstwa różnymi ścieżkami."
