# Scenariusz 2: Porównanie iteracyjnych heurystyk - Opis i wnioski

## Cel eksperymentu

Weryfikacja hipotezy, że iteracyjne podejście do projektowania botów prowadzi do systematycznej poprawy skuteczności. Eksperyment polegał na bezpośrednich starciach między kolejnymi iteracjami botów heurystycznych, co pozwala ocenić wartość wprowadzanych w każdej wersji ulepszeń.

## Metodologia

Każda para kolejnych iteracji rozegrała 1000 meczów z przełączaniem miejsc (`--switch`), co zapewnia sprawiedliwe porównanie niezależne od pozycji startowej. Analizowano współczynnik zwycięstw, tempo rozgrywki oraz szczegółowe metryki strategiczne.

## Wyniki

### Progresja skuteczności

Wyniki jednoznacznie potwierdzają wartość iteracyjnego podejścia - w każdej parze kolejna iteracja osiąga wyższy współczynnik zwycięstw:

- **It1 vs It2**: It2 wygrywa 61.9% rozgrywek (przewaga +26.1 punktów procentowych)
- **It2 vs It3**: It3 wygrywa 93.6% rozgrywek (przewaga +87.8 punktów procentowych)
- **It3 vs It4**: It4 wygrywa 66.0% rozgrywek (przewaga +33.6 punktów procentowych)
- **It4 vs It5**: It5 wygrywa 74.2% rozgrywek (przewaga +48.5 punktów procentowych)
- **It1 vs It5** (porównanie skokowe): It5 wygrywa 99.9% rozgrywek (tylko 1 przegrana w 1000 meczach)

### Przepaść kompetencyjna It2→It3

Najbardziej dramatyczny skok w skuteczności następuje między It2Player a It3Player. Różnica w współczynniku zwycięstw wynosi 87.8 punktów procentowych, co potwierdza kluczowe znaczenie mechanizmów wprowadzonych w It3:

- **Strategia ustawień początkowych**: Świadomy wybór najlepszych pozycji startowych
- **Aktywne wykorzystanie rozbójnika**: Strategiczne blokowanie produkcji przeciwnika
- **Lepsze zarządzanie zasobami**: Zaawansowana heurystyka handlu i budowy

Wszystkie metryki szczegółowe pokazują dramatyczną poprawę:
- Produkcja zasobów: 400.5 → 1017.3 (wzrost o 154%)
- Wykorzystanie Największej Armii: 5.8% → 91.1% (wzrost 15-krotny)
- Wykorzystanie Najdłuższej Drogi: 31.4% → 68.5% (podwojenie)

### Tempo rozgrywki

Lepsze boty nie tylko wygrywają częściej, ale również szybciej kończą rozgrywki:

- **It1 vs It2**: 339.8 tur (średnia)
- **It2 vs It3**: 181.9 tur (spadek o 46% - prawie 2x szybciej)
- **It3 vs It4**: 184.4 tur (podobne tempo)
- **It4 vs It5**: 134.2 tury (najszybsze rozgrywki)

Warto zauważyć, że rozgrywka It3 vs It4 trwała dłużej (184.4 tur) niż It2 vs It3 (181.9 tur), co może wskazywać na bardziej zbalansowaną walkę między zaawansowanymi botami - It4 "dzielnie próbuje walczyć" z It3, co wydłuża rozgrywkę mimo wyższej skuteczności It4.

### Paradoks It3 vs It4

Interesująca obserwacja: pomimo iż It3Player osiągał lepszy współczynnik zwycięstw przeciwko RandomPlayer niż It4Player (98.7% vs 96.9%), w bezpośrednim starciu It4Player wygrywa 66% rozgrywek przeciwko It3Player.

**Wyjaśnienie**: It4Player, dzięki lepszej produkcji zasobów (ProdScore: 1127.4 vs 817.7) oraz deterministycznemu wyborowi najlepszej akcji, jest lepiej przygotowany do starcia z zaawansowanym przeciwnikiem. To pokazuje różnicę między ogólną skutecznością przeciwko losowemu przeciwnikowi a skutecznością przeciwko konkretnej strategii.

### Porównanie skokowe: It1 vs It5

Porównanie pierwszej i ostatniej iteracji pokazuje pełną skalę postępu:

- **Współczynnik zwycięstw**: It5 wygrywa 99.9% rozgrywek (tylko 1 przegrana)
- **Tempo**: 114.9 tur (najbliżej klasycznych 60-70 tur)
- **Dominacja**: It1 przegrywa przy średnio 14.00 VP It5 (największa dominacja)
- **Wszystkie metryki pokazują dramatyczną różnicę**:
  - Produkcja: 266.5 → 1096.8 (4x wyższa)
  - Największa Armia: 1.1% → 98.2% (89x wyższa)
  - Karty rozwoju: 0.6 → 3.7 (6x więcej)

## Wnioski

1. **Iteracyjne podejście jest skuteczne** - każda kolejna wersja jest obiektywnie lepsza od poprzedniej, co uzasadnia wprowadzane ulepszenia.

2. **It3 wprowadza przełomowe mechanizmy** - strategia ustawień początkowych oraz aktywne wykorzystanie rozbójnika powodują największy skok w skuteczności, tworząc wyraźną "przepaść kompetencyjną" między botami podstawowymi (It1-It2) a zaawansowanymi (It3+).

3. **Lepsze boty są bardziej efektywne** - nie tylko wygrywają częściej, ale również szybciej kończą rozgrywki, co wskazuje na lepszą optymalizację strategii.

4. **Różnica między "ogólną" a "specyficzną" skutecznością** - It4, mimo gorszych wyników przeciwko RandomPlayer, wygrywa z It3 dzięki lepszemu przygotowaniu do starcia z konkretnym przeciwnikiem.

5. **Pełna progresja działa** - porównanie It1 vs It5 pokazuje, że iteracyjne podejście prowadzi do dramatycznej poprawy we wszystkich aspektach strategii.

## Wizualizacje

Eksperyment został zilustrowany następującymi wykresami:

1. **Progresja skuteczności w bezpośrednich starciach** - pokazuje, że każda kolejna iteracja wygrywa z poprzednią
2. **Tempo rozgrywki** - ilustruje przyspieszenie wraz z poprawą jakości botów
3. **Szczegółowa analiza przepaści It2→It3** - wizualizuje kluczowe różnice między iteracjami

Wszystkie wykresy potwierdzają systematyczną poprawę jakości botów wraz z każdą iteracją, uzasadniając wartość iteracyjnego podejścia do projektowania strategii.
