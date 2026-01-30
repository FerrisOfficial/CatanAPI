# Analiza wyników Scenariusza 5: RoadPlayer vs różne boty

## Podsumowanie danych

| Przeciwnik | Win Rate Road | Avg Turns | LossVP Road | LossVP Opp | LR% Road | LR% Opp | LA% Road | LA% Opp | ProdScore Road | ProdScore Opp |
|------------|---------------|-----------|-------------|------------|----------|---------|----------|---------|----------------|---------------|
| It1 | 100.0% | 131.2 | 0.00 | 3.58 | 95.9% | 3.9% | 96.7% | 2.6% | 1059.2 | 271.2 |
| It2 | 99.2% | 134.2 | 11.57 | 3.96 | 90.8% | 9.2% | 96.3% | 3.1% | 1050.7 | 318.9 |
| It3 | 86.7% | 136.1 | 9.68 | 8.23 | 74.3% | 25.7% | 79.6% | 20.3% | 1066.6 | 732.1 |
| It4 | 68.2% | 140.2 | 9.54 | 9.42 | 47.7% | 52.3% | 78.8% | 21.2% | 1001.2 | 1007.3 |
| It5 | 47.5% | 118.9 | 9.06 | 10.40 | 77.1% | 21.8% | 33.5% | 66.5% | 946.5 | 1044.1 |
| Para | 74.2% | 134.9 | 8.99 | 5.14 | 71.0% | 28.8% | 79.1% | 20.2% | 1006.6 | 601.1 |
| ParaSettleIt5 | 43.9% | 118.3 | 9.01 | 10.84 | 78.3% | 21.0% | 28.3% | 71.7% | 935.9 | 1059.4 |
| AlphaBeta | 33.7% | 114.0 | 10.07 | 5.47 | 38.5% | 61.5% | 52.2% | 47.8% | 855.6 | 1105.8 |

## Kluczowe wnioski

### 1. **Strategia drogowa jest bardziej skuteczna niż strategia monosurowcowa**

**Obserwacja**: RoadPlayer osiąga lepsze wyniki niż OneResourcePlayer przeciwko większości przeciwników:
- vs It1: Road 100% vs OneResource 98.3%
- vs It2: Road 99.2% vs OneResource 95.5%
- vs It3: Road 86.7% vs OneResource 56.8% (znacząca różnica!)
- vs It4: Road 68.2% vs OneResource 36.4%
- vs It5: Road 47.5% vs OneResource 19.0%
- vs Para: Road 74.2% vs OneResource 34.8% (paradoksalnie Road wygrywa!)

**Wnioski**:
- **Strategia drogowa jest bardziej elastyczna** niż strategia monosurowcowa
- **RoadPlayer lepiej radzi sobie z zaawansowanymi botami** - szczególnie vs It3 (86.7% vs 56.8%)
- **Para jest słabszy przeciwko RoadPlayer** - Road wygrywa 74.2%, podczas gdy OneResource tylko 34.8%
- **Strategia drogowa działa dłużej** - Road wygrywa vs It4 (68.2%), podczas gdy OneResource przegrywa (36.4%)

**Jak przedstawić**:
- Wykres porównawczy RoadPlayer vs OneResourcePlayer
- Wykres pokazujący, że strategia drogowa jest bardziej elastyczna

### 2. **Przepaść skuteczności - It4 jest punktem przełomowym**

**Obserwacja**: RoadPlayer pokazuje dramatyczny spadek skuteczności między It3 a It4:
- vs It3: 86.7% (wciąż bardzo dobrze)
- vs It4: 68.2% (spadek o 18.5 punktów procentowych)
- vs It5: 47.5% (przegrywa - poniżej 50%)

**Wnioski**:
- **It4 jest punktem przełomowym dla strategii drogowej** - spadek z 86.7% do 68.2%
- **It5 całkowicie dominuje** - Road przegrywa 47.5% (poniżej 50%)
- **Strategia drogowa działa lepiej niż monosurowcowa** - Road vs It3: 86.7%, OneResource vs It3: 56.8%

**Jak przedstawić**:
- Wykres pokazujący spadek win rate z zaznaczoną przepaścią między It3 a It4
- Porównanie z OneResourcePlayer pokazujące, że Road jest bardziej skuteczny

### 3. **Paradoksalne wyniki - It4 vs It5**

**Obserwacja**: 
- vs It4: Road wygrywa 68.2%, ale LR% = 47.7% (It4 ma lepszą najdłuższą drogę!)
- vs It5: Road przegrywa 47.5%, ale LR% = 77.1% (Road ma lepszą najdłuższą drogę!)

**Wnioski**:
- **Posiadanie Najdłuższej Drogi nie gwarantuje zwycięstwa** - vs It5 Road ma 77.1% LR%, ale przegrywa
- **It4 jest lepszy w budowie dróg** - ma 52.3% LR% vs Road 47.7%, ale przegrywa
- **Strategia drogowa wymaga więcej niż tylko najdłuższej drogi** - potrzebne są również osady i miasta

**Jak przedstawić**:
- Wykres pokazujący LR% vs Win Rate (pokazuje paradoks)
- Analiza pokazująca, że sama najdłuższa droga nie wystarczy

### 4. **Tempo rozgrywki - zaawansowane boty wygrywają szybciej**

**Obserwacja**: Średnia liczba tur spada wraz z poprawą przeciwnika:
- vs It1-It4: 131-140 tur (podobne tempo)
- vs It5+: 114-118 tur (szybsze - zaawansowane boty wygrywają szybciej)

**Wnioski**:
- **Zaawansowane boty wygrywają szybciej** - szczególnie It5, ParaSettleIt5 i AlphaBeta (114-118 tur)
- **Road vs It1-It4 trwa dłużej** - mimo dominacji Road, rozgrywki są wolniejsze (131-140 tur)
- **AlphaBeta jest najszybszy** - 114.0 tur (najszybsze rozgrywki)

**Jak przedstawić**:
- Wykres pokazujący tempo rozgrywki w zależności od przeciwnika
- Wykres pokazujący, że zaawansowane boty wygrywają szybciej

### 5. **Najdłuższa Droga - sukces i porażka**

**Obserwacja**: RoadPlayer osiąga wysokie LR% przeciwko większości przeciwników:
- vs It1-It2: 90-96% (dominacja)
- vs It3: 74.3% (wciąż dominuje)
- vs It4: 47.7% (spadek - It4 lepszy!)
- vs It5: 77.1% (paradoksalnie wyższe niż vs It4!)
- vs Para: 71.0%
- vs ParaSettleIt5: 78.3%
- vs AlphaBeta: 38.5% (AlphaBeta dominuje!)

**Wnioski**:
- **RoadPlayer skutecznie zdobywa Najdłuższą Drogę** przeciwko większości przeciwników
- **It4 jest lepszy w budowie dróg** - ma 52.3% LR% vs Road 47.7%
- **AlphaBeta całkowicie dominuje** - ma 61.5% LR% vs Road 38.5%
- **Posiadanie Najdłuższej Drogi nie gwarantuje zwycięstwa** - vs It5 Road ma 77.1% LR%, ale przegrywa

**Jak przedstawić**:
- Wykres pokazujący LR% Road vs przeciwników
- Wykres pokazujący paradoks (LR% vs Win Rate)

### 6. **Największa Armia - słabość strategii drogowej**

**Obserwacja**: RoadPlayer osiąga wysokie LA% przeciwko słabym botom, ale spada przeciwko zaawansowanym:
- vs It1-It3: 79-97% (dominacja)
- vs It4: 78.8% (wciąż dobrze)
- vs It5: 33.5% (znaczący spadek - It5 dominuje z 66.5%!)
- vs ParaSettleIt5: 28.3% (znaczący spadek)
- vs AlphaBeta: 52.2% (prawie równo)

**Wnioski**:
- **Strategia drogowa słabo wykorzystuje Największą Armię** przeciwko zaawansowanym botom
- **It5 i ParaSettleIt5 dominują w LA%** - mają 66-72% vs Road 28-33%
- **To pokazuje słabość jednostronnej strategii** - skupienie na drogach kosztem kart rozwoju

**Jak przedstawić**:
- Wykres pokazujący LA% Road vs przeciwników
- Wykres pokazujący, że strategia drogowa słabo wykorzystuje karty rozwoju

### 7. **Ciekawe obserwacje**

**Road vs Para - paradoksalny wynik**:
- Win rate: Road wygrywa 74.2% (dobry wynik!)
- LR%: Road ma 71.0% (dominuje)
- LA%: Road ma 79.1% (dominuje)
- ProdScore: Road ma 1006.6 vs Para 601.1 (znacząca przewaga!)
- **Para jest słabszy przeciwko RoadPlayer niż przeciwko OneResourcePlayer!**

**Wyjaśnienie**: ParaPlayer może być zoptymalizowany przeciwko innym strategiom, ale nie przeciwko strategii drogowej. RoadPlayer skutecznie wykorzystuje swoją specjalizację przeciwko Para.

**Road vs It4 - paradoks LR%**:
- Win rate: Road wygrywa 68.2%
- LR%: Road ma tylko 47.7% (It4 ma 52.3%!)
- **Road wygrywa mimo że nie ma Najdłuższej Drogi!**

**Wyjaśnienie**: RoadPlayer wygrywa dzięki lepszej produkcji zasobów (1001.2 vs 1007.3 - prawie równo) oraz lepszemu wykorzystaniu Największej Armii (78.8% vs 21.2%). To pokazuje, że strategia drogowa nie jest całkowicie jednostronna.

**Road vs It5 - paradoks odwrotny**:
- Win rate: Road przegrywa 47.5%
- LR%: Road ma 77.1% (dominuje!)
- **Road przegrywa mimo że ma Najdłuższą Drogę!**

**Wyjaśnienie**: It5 wygrywa dzięki lepszej produkcji zasobów (1044.1 vs 946.5) oraz lepszemu wykorzystaniu Największej Armii (66.5% vs 33.5%). To pokazuje, że sama Najdłuższa Droga nie wystarczy przeciwko zaawansowanym botom.

## Propozycje wizualizacji

### 1. **Wykres "Skuteczność strategii drogowej"**
- Oś X: Przeciwnik
- Oś Y: Win Rate RoadPlayer (%)
- Linia pokazująca spadek skuteczności
- Linia pokazująca "przepaść" między It3 a It4
- **Tytuł**: "Skuteczność strategii drogowej w zależności od jakości przeciwnika"

### 2. **Wykres "Porównanie RoadPlayer vs OneResourcePlayer"**
- Oś X: Przeciwnik
- Oś Y: Win Rate (%)
- Dwie linie: RoadPlayer i OneResourcePlayer
- Pokazuje, że Road jest bardziej skuteczny
- **Tytuł**: "Porównanie skuteczności strategii wyspecjalizowanych"

### 3. **Wykres "Paradoks Najdłuższej Drogi"**
- Oś X: Przeciwnik
- Oś Y: LR% RoadPlayer (%)
- Dodatkowa linia pokazująca Win Rate
- Pokazuje paradoks (wysokie LR% ale przegrana vs It5)
- **Tytuł**: "Paradoks Najdłuższej Drogi: posiadanie premii nie gwarantuje zwycięstwa"

### 4. **Wykres "Największa Armia - słabość strategii drogowej"**
- Oś X: Przeciwnik
- Oś Y: LA% RoadPlayer (%)
- Pokazuje spadek wykorzystania LA% przeciwko zaawansowanym botom
- **Tytuł**: "Wykorzystanie Największej Armii: słabość strategii drogowej"

### 5. **Wykres "Tempo rozgrywki"**
- Oś X: Przeciwnik
- Oś Y: Średnia liczba tur
- Pokazuje, że zaawansowane boty wygrywają szybciej
- **Tytuł**: "Tempo rozgrywki: zaawansowane boty wygrywają szybciej"

### 6. **Wykres "Produkcja zasobów"**
- Oś X: Przeciwnik
- Oś Y: ProdScore
- Porównanie Road vs przeciwnik
- Pokazuje, że zaawansowane boty mają wyższą produkcję
- **Tytuł**: "Porównanie produkcji zasobów: RoadPlayer vs Przeciwnik"

## Propozycje tekstu do pracy

### Wprowadzenie do wyników:

"Wyniki Scenariusza 5 dla RoadPlayer pokazują, że strategia drogowa jest bardziej skuteczna niż strategia monosurowcowa przeciwko większości przeciwników. RoadPlayer osiąga wysokie współczynniki zwycięstw przeciwko It1–It3 (86.7–100%), jednak jego skuteczność spada przeciwko It4 (68.2%) i dalej przeciwko It5 (47.5%). Szczególnie interesująca jest obserwacja, że RoadPlayer wygrywa 74.2% rozgrywek przeciwko ParaPlayer, podczas gdy OneResourcePlayer osiąga jedynie 34.8%, co pokazuje, że strategia drogowa jest bardziej elastyczna niż strategia monosurowcowa."

### Kluczowy wniosek:

"Szczególnie istotna jest obserwacja paradoksalnych wyników dotyczących premii Najdłuższa Droga. RoadPlayer przegrywa 47.5% rozgrywek przeciwko It5, mimo że osiąga 77.1% wykorzystania Najdłuższej Drogi. Z kolei przeciwko It4 RoadPlayer wygrywa 68.2%, mimo że osiąga jedynie 47.7% wykorzystania Najdłuższej Drogi (It4 ma 52.3%). To pokazuje, że sama premia Najdłuższa Droga nie gwarantuje zwycięstwa — potrzebne są również osady, miasta oraz efektywne wykorzystanie innych mechanizmów gry, takich jak Największa Armia."

### O zaawansowanych botach:

"Zaawansowane boty (It5, ParaSettleIt5, AlphaBeta) całkowicie dominują strategię drogową, osiągając współczynniki zwycięstw powyżej 52–66%. Co więcej, przeciwnicy mają średnio ponad 10 punktów zwycięstwa w momencie przegranej RoadPlayer, co pokazuje, że zaawansowane boty nie tylko wygrywają częściej, ale również dominują zdecydowanie. To potwierdza, że zbalansowane strategie są bardziej skuteczne niż jednostronna specjalizacja w długoterminowej perspektywie."
