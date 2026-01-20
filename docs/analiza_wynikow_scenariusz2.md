# Analiza wyników Scenariusza 2: Porównanie iteracyjnych heurystyk

## Podsumowanie danych

| Para | Bot A | Bot B | Win Rate A | Win Rate B | Avg Turns | LossVP A | LossVP B | LR% A | LR% B | LA% A | LA% B | DevCards A | DevCards B | ProdScore A | ProdScore B |
|------|-------|-------|------------|------------|-----------|----------|----------|-------|-------|-------|-------|------------|------------|-------------|-------------|
| It1 vs It2 | It1 | It2 | 35.8% | 61.9% | 339.8 | 6.24 | 7.96 | 37.4% | 62.6% | 45.8% | 52.3% | 2.3 | 2.6 | 589.1 | 832.1 |
| It2 vs It3 | It2 | It3 | 5.8% | 93.6% | 181.9 | 5.06 | 11.28 | 31.4% | 68.5% | 5.8% | 91.1% | 0.9 | 3.2 | 400.5 | 1017.3 |
| It3 vs It4 | It3 | It4 | 32.4% | 66.0% | 184.4 | 9.11 | 9.89 | 24.6% | 75.4% | 52.5% | 46.3% | 2.5 | 2.5 | 817.7 | 1127.4 |
| It4 vs It5 | It4 | It5 | 25.7% | 74.2% | 134.2 | 9.54 | 11.04 | 82.0% | 17.8% | 11.4% | 88.6% | 1.7 | 3.4 | 958.1 | 1070.6 |
| It1 vs It5 | It1 | It5 | 0.1% | 99.9% | 114.9 | 3.58 | 14.00 | 14.0% | 73.5% | 1.1% | 98.2% | 0.6 | 3.7 | 266.5 | 1096.8 |

## Kluczowe wnioski

### 1. **Każda iteracja jest obiektywnie lepsza**

**Obserwacja**: W każdej parze kolejna iteracja osiąga wyższy współczynnik zwycięstw:
- It1 vs It2: It2 wygrywa 61.9% (różnica: +26.1%)
- It2 vs It3: It3 wygrywa 93.6% (różnica: +87.8%!)
- It3 vs It4: It4 wygrywa 66.0% (różnica: +33.6%)
- It4 vs It5: It5 wygrywa 74.2% (różnica: +48.5%)
- It1 vs It5: It5 wygrywa 99.9% (różnica: +99.8% - porównanie skokowe!)

**Wnioski**:
- **Iteracyjne podejście działa** - każda wersja jest lepsza od poprzedniej
- **Największy skok: It2→It3** - potwierdza "przepaść kompetencyjną" z Scenariusza 1
- **It4 jest lepszy od It3** mimo że It3 miał lepszy win rate vs Random (98.7% vs 96.9%)
- **It5 dominuje It4** - wygrywa 74.2% rozgrywek
- **It1 vs It5 pokazuje pełną progresję** - It5 wygrywa 99.9% (tylko 1 przegrana w 1000 rozgrywkach!)

**Jak przedstawić**:
- Wykres słupkowy pokazujący win rate każdego bota w jego parze
- Wykres pokazujący "przewagę" kolejnej iteracji (różnica w win rate)
- Wykres liniowy pokazujący progresję (ale uwaga: to nie jest bezpośrednie porównanie)

### 2. **Dramatyczna przewaga It3 nad It2**

**Obserwacja**: 
- It2 vs It3: It3 wygrywa 93.6% (It2 tylko 5.8%)
- To największa różnica w całym scenariuszu!

**Wnioski**:
- **It3 wprowadza mechanizmy, które całkowicie dominują It2**:
  - Strategia ustawień początkowych
  - Aktywne wykorzystanie rozbójnika
  - Lepsze zarządzanie zasobami
- **It2 jest "ostatnim słabym botem"** - nawet It1 może go czasem pokonać (35.8%)
- **It3+ są "profesjonalne"** - różnice między nimi są mniejsze, ale nadal istotne

**Jak przedstawić**:
- Wykres pokazujący "przepaść" między It2 a It3
- Porównanie metryk: It3 ma 2.5x wyższą produkcję (1017.3 vs 400.5)
- Wykres pokazujący, że It3 dominuje we wszystkich metrykach

### 3. **Tempo rozgrywki przyspiesza z każdą iteracją**

**Obserwacja**: Średnia liczba tur spada:
- It1 vs It2: 339.8 tur
- It2 vs It3: 181.9 tur (prawie 2x szybciej!)
- It3 vs It4: 184.4 tur (podobnie)

**Wnioski**:
- **Lepsze boty wygrywają szybciej** - są bardziej efektywne
- **Największe przyspieszenie: It2→It3** - z 339.8 do 181.9 tur
- **It3+ mają podobne tempo** - różnice są w strategii, nie w szybkości

**Jak przedstawić**:
- Wykres liniowy pokazujący spadek średniej liczby tur
- Wykres pokazujący korelację między win rate a tempem

### 4. **LossVP pokazuje dominację**

**Obserwacja**: Gdy słabszy bot przegrywa, przeciwnik ma więcej punktów:
- It1 vs It2: It1 przegrywa przy 7.96 VP It2
- It2 vs It3: It2 przegrywa przy 11.28 VP It3 (największa dominacja!)
- It3 vs It4: It3 przegrywa przy 9.89 VP It4

**Wnioski**:
- **It3 dominuje It2 najbardziej** - przeciwnik ma średnio 11.28 VP przy przegranej
- **Lepsze boty nie tylko wygrywają, ale dominują** - przeciwnik ma dużo więcej punktów
- **Różnice między It3+ są mniejsze** - wygrywają bardziej "blisko"

**Jak przedstawić**:
- Wykres słupkowy LossVP pokazujący dominację
- Wykres pokazujący różnicę VP w momencie zwycięstwa

### 5. **Metryki szczegółowe pokazują różnice w strategiach**

**Najdłuższa Droga (LR%)**:
- It1: 37.4% → It2: 62.6% (znaczący wzrost)
- It2: 31.4% → It3: 68.5% (podwojenie!)
- It3: 24.6% → It4: 75.4% (3x wzrost!)

**Największa Armia (LA%)**:
- It1: 45.8% → It2: 52.3% (niewielki wzrost)
- It2: 5.8% → It3: 91.1% (dramatyczny wzrost!)
- It3: 52.5% → It4: 46.3% (spadek - It4 skupia się na innych aspektach)

**Karty rozwoju**:
- It1: 2.3 → It2: 2.6 (niewielki wzrost)
- It2: 0.9 → It3: 3.2 (znaczący wzrost)
- It3: 2.5 → It4: 2.5 (bez zmian)

**Produkcja zasobów (ProdScore)**:
- It1: 589.1 → It2: 832.1 (+41%)
- It2: 400.5 → It3: 1017.3 (+154%!)
- It3: 817.7 → It4: 1127.4 (+38%)

**Wnioski**:
- **It3 wprowadza największe zmiany we wszystkich metrykach**:
  - LR%: 31.4% → 68.5% (2x)
  - LA%: 5.8% → 91.1% (15x!)
  - ProdScore: 400.5 → 1017.3 (2.5x)
- **It4 skupia się na produkcji i drogach**:
  - ProdScore: najwyższa (1127.4)
  - LR%: najwyższa (75.4%)
  - LA%: spadek (46.3% vs 52.5% It3)
- **Każda iteracja poprawia różne aspekty** - nie ma jednej "magicznej metryki"

**Jak przedstawić**:
- Wykres radarowy pokazujący profil każdego bota
- Wykresy słupkowe dla każdej metryki pokazujące progresję
- Macierz korelacji między metrykami a win rate

### 6. **Porównanie skokowe: It1 vs It5**

**Obserwacja**: 
- It1 vs It5: It5 wygrywa 99.9% (tylko 1 przegrana!)
- Avg turns: 114.9 (najszybsze z wszystkich par)
- LossVP: It1 przegrywa przy średnio 14.00 VP It5 (największa dominacja!)

**Wnioski**:
- **Pełna progresja działa** - It5 jest praktycznie niepokonany przez It1
- **Najszybsze rozgrywki** - It5 wygrywa w średnio 114.9 tur (najbliżej klasycznych 60-70 tur)
- **Największa dominacja** - przeciwnik ma średnio 14 VP przy przegranej
- **Wszystkie metryki pokazują dramatyczną różnicę**:
  - LR%: 14.0% vs 73.5% (5x wyższa)
  - LA%: 1.1% vs 98.2% (89x wyższa!)
  - ProdScore: 266.5 vs 1096.8 (4x wyższa)
  - DevCards: 0.6 vs 3.7 (6x więcej)

**Jak przedstawić**:
- Wykres pokazujący "pełną progresję" od It1 do It5
- Porównanie wszystkich metryk pokazujące skalę poprawy
- Wykres pokazujący, że It5 jest "praktycznie perfekcyjny" przeciwko It1

### 7. **Ciekawe obserwacje**

**It3 vs It4 - paradoks?**:
- It3 miał lepszy win rate vs Random (98.7% vs 96.9%)
- Ale It4 wygrywa 66% rozgrywek przeciwko It3!

**Wyjaśnienie**:
- **It4 jest lepszy w bezpośrednim starciu** - lepsza strategia przeciwko konkretnemu przeciwnikowi
- **It3 może być lepszy vs Random**, ale It4 ma lepsze mechanizmy przeciwko It3:
  - Lepsza produkcja (1127.4 vs 817.7)
  - Lepsze wykorzystanie dróg (75.4% vs 24.6%)
  - Deterministyczny wybór najlepszej akcji
- **To pokazuje różnicę między "ogólną skutecznością" a "skutecznością przeciwko konkretnemu przeciwnikowi"**

**It2 vs It3 - największa przepaść**:
- Win rate: 5.8% vs 93.6% (różnica 87.8%!)
- Avg turns: 339.8 → 181.9 (2x szybciej)
- ProdScore: 400.5 → 1017.3 (2.5x wyższa)
- LA%: 5.8% → 91.1% (15x wyższa!)

**Wnioski**:
- **It2→It3 to największy skok w całej progresji**
- **Wszystkie metryki pokazują dramatyczną poprawę**
- **To potwierdza, że strategia ustawień + rozbójnik są kluczowe**

## Propozycje wizualizacji

### 1. **Wykres "Progresja w parach"**
- Oś X: Para (It1 vs It2, It2 vs It3, It3 vs It4, It4 vs It5)
- Oś Y: Win Rate (%)
- Dwa słupki dla każdej pary (Bot A i Bot B)
- Pokazuje, że każda kolejna iteracja wygrywa
- **Tytuł**: "Progresja skuteczności w bezpośrednich starciach"

### 2. **Wykres "Przewaga kolejnej iteracji"**
- Oś X: Przejście (It1→It2, It2→It3, It3→It4, It4→It5)
- Oś Y: Różnica w Win Rate (%)
- Pokazuje, jak bardzo każda iteracja jest lepsza
- **Tytuł**: "Przewaga kolejnej iteracji nad poprzednią"

### 3. **Wykres "Przepaść It2→It3"**
- Oś X: Bot
- Oś Y: Win Rate vs następnej iteracji (%)
- Pokazuje dramatyczny spadek It2 (5.8%) vs It3
- **Tytuł**: "Przepaść kompetencyjna: It2 vs It3"

### 4. **Wykres "Tempo rozgrywki"**
- Oś X: Para
- Oś Y: Średnia liczba tur
- Pokazuje przyspieszenie z każdą iteracją
- **Tytuł**: "Tempo rozgrywki w zależności od iteracji"

### 5. **Wykres radarowy "Profil strategii"**
- Oś: LR%, LA%, DevCards, ProdScore, WinRate
- Linie dla It1, It2, It3, It4, It5
- Pokazuje, jak zmienia się profil strategii
- **Tytuł**: "Ewolucja profilu strategicznego botów"

### 6. **Macierz wyników**
- Tabela z wszystkimi metrykami
- Kolorowe kodowanie pokazujące progresję
- Pokazuje wszystkie dane w jednym miejscu

### 7. **Wykres "Dominacja"**
- Oś X: Para
- Oś Y: LossVP (średnia VP przeciwnika przy przegranej)
- Pokazuje, jak bardzo lepsze boty dominują
- **Tytuł**: "Dominacja kolejnych iteracji"

## Propozycje tekstu do pracy

### Wprowadzenie do wyników:

"Wyniki Scenariusza 2 potwierdzają wartość iteracyjnego podejścia do projektowania botów. W każdej parze bezpośrednich starć kolejna iteracja osiąga statystycznie istotnie wyższy współczynnik zwycięstw, co uzasadnia wprowadzane w każdej wersji ulepszenia. Szczególnie interesująca jest obserwacja, że największy skok w skuteczności następuje między It2Player a It3Player - różnica w współczynniku zwycięstw wynosi 87.8 punktów procentowych (93.6% vs 5.8%), co potwierdza kluczowe znaczenie strategii ustawień początkowych oraz aktywnego wykorzystania rozbójnika."

### Kluczowy wniosek:

"Warto zauważyć, że pomimo iż It3Player osiągał lepszy współczynnik zwycięstw przeciwko RandomPlayer niż It4Player (98.7% vs 96.9%), w bezpośrednim starciu It4Player wygrywa 66% rozgrywek przeciwko It3Player. To pokazuje różnicę między ogólną skutecznością przeciwko losowemu przeciwnikowi a skutecznością przeciwko konkretnej strategii. It4Player, dzięki lepszej produkcji zasobów (ProdScore: 1127.4 vs 817.7) oraz deterministycznemu wyborowi najlepszej akcji, jest lepiej przygotowany do starcia z zaawansowanym przeciwnikiem."

### O progresji:

"Analiza metryk szczegółowych pokazuje, że każda iteracja poprawia różne aspekty strategii. It2Player wprowadza handel z bankiem, co zwiększa produkcję zasobów o 41%. It3Player wprowadza strategię ustawień początkowych i rozbójnika, co powoduje dramatyczny wzrost we wszystkich metrykach - produkcja wzrasta o 154%, wykorzystanie Największej Armii z 5.8% do 91.1%. It4Player skupia się na optymalizacji produkcji i dróg, osiągając najwyższą produkcję (1127.4) oraz najwyższe wykorzystanie Najdłuższej Drogi (75.4%)."
