## Streszczenie
Praca składa się z dwóch zasadniczych części. Pierwszym celem pracy jest zaprojektowanie oraz implementacja silnika gry planszowej Catan w wersji przeciwko jednemu graczowi (1 vs 1) w języku C++. Implementacja została wykonana z naciskiem na wydajność oraz elastyczność, w szczególności poprzez efektywną reprezentację stanu gry, system aplikowania i cofania akcji oraz modelowanie obiektów występujących w grze. Poprawność działania silnika została zweryfikowana za pomocą rozbudowanego zestawu testów jednostkowych i integracyjnych.

Drugim celem pracy, o charakterze badawczym, jest implementacja oraz porównanie botów wykorzystujących różne strategie rozgrywki. Przedstawiono kilka typów graczy automatycznych, w tym graczy losowych, heurystycznych oraz gracza wykorzystującego algorytm przeszukiwania drzewa gry. Skuteczność poszczególnych botów została oceniona na podstawie przeprowadzonych eksperymentów i analizy uzyskanych wyników.

Dodatkowo opracowano moduł rejestracji i odtwarzania rozgrywek, umożliwiający szczegółowe prześledzenie przebiegu gry pomiędzy wybranymi botami krok po kroku. Moduł ten wspiera analizę zachowania graczy automatycznych, ułatwia debugowanie silnika gry oraz stanowi narzędzie pomocnicze w procesie porównywania strategii.

## Streszczenie po angielsku

## Spis treści
## 1. Wstęp
### 1.1. Wprowadzenie do tematyki gier planszowych i sztucznej inteligencji

Sztuczna inteligencja od wielu lat stanowi istotny obszar badań w kontekście gier, w szczególności gier cyfrowych, gdzie znajduje zastosowanie m.in. w projektowaniu przeciwników sterowanych komputerowo, generowaniu treści oraz personalizacji rozgrywki. W przypadku gier planszowych zakres wykorzystania algorytmów sztucznej inteligencji jest węższy, co wynika z ich analogowej natury oraz fizycznych komponentów, jednak w ostatnich latach obserwuje się dynamiczny wzrost zainteresowania tym obszarem badawczym.

Jednym z podstawowych zastosowań sztucznej inteligencji w grach planszowych jest wsparcie procesu projektowania i testowania mechanik gry. Współczesne gry planszowe, w szczególności tzw. eurogry, charakteryzują się dużą złożonością decyzyjną, licznymi zależnościami pomiędzy elementami gry oraz koniecznością zachowania balansu pomiędzy różnymi strategiami. Tradycyjne testowanie takich gier wymaga znacznych nakładów czasu i pracy ludzkich testerów, co czyni automatyczne symulacje rozgrywek atrakcyjną alternatywą.

Algorytmy sztucznej inteligencji umożliwiają symulowanie tysięcy rozgrywek w krótkim czasie, analizowanie drzew decyzyjnych oraz ocenę skuteczności poszczególnych strategii. Metody te pozwalają na identyfikację niezbalansowanych mechanik, dominujących strategii oraz rzadkich, lecz istotnych scenariuszy rozgrywki, które mogłyby zostać pominięte w testach manualnych. W praktyce prowadzi to do bardziej obiektywnej analizy systemu gry i usprawnia proces iteracyjnego doskonalenia zasad.

### 1.2. Gra Catan – charakterystyka i potencjał badawczy

*Catan* (wcześniej znany jako *The Settlers of Catan*) to strategiczna gra planszowa zaprojektowana przez Klausa Teubera, w której gracze rozwijają osadnictwo na wyspie poprzez pozyskiwanie surowców, handel oraz budowę dróg, osad i miast. Rozgrywka łączy elementy planowania ekonomicznego z umiarkowaną losowością wynikającą z rzutów kośćmi decydujących o produkcji zasobów.

#### Zalety gry *Catan*

Jedną z najczęściej wskazywanych zalet gry Catan jest umiejętne połączenie relatywnie prostych zasad z wysokim poziomem satysfakcji oraz znaczną głębią strategiczną. W recenzji opublikowanej na łamach magazynu Pyramid podkreślono, że gra oferuje satysfakcjonujące doświadczenie rozwoju ekonomicznego i wymiany zasobów, przy jednocześnie umiarkowanym czasie rozgrywki, wynoszącym zazwyczaj od półtorej do dwóch godzin. Zwrócono uwagę, że poziom satysfakcji płynący z rozgrywki jest porównywalny z grami o znacznie dłuższym czasie trwania.

Drugim, bardzo istotnym atutem jest wysoka regrywalność: plansza składa się z heksów, a ich układ i przypisane numery produkcji można zmieniać między partiami, co ogranicza powtarzalność i utrudnia wyuczenie jednej „sztywnej” sekwencji optymalnych ruchów. Dzięki temu gra sprzyja analizie adaptacyjnych strategii oraz reagowania na bieżącą sytuację na planszy.

Kolejną cechą wyróżniającą jest istotna rola handlu i negocjacji. Mechanizm wymiany zasobów między graczami sprawia, że *Catan* nie jest jedynie „łamigłówką optymalizacyjną”, lecz grą, w której liczą się także przewidywanie intencji przeciwnika, ocena ryzyka oraz decyzje o współpracy lub blokowaniu. W praktyce gra równoważy konflikt (blokowanie lokalizacji, „robber”) z kooperacją transakcyjną (handel), co buduje złożoną dynamikę interakcji.

*Catan* jest również uznanym tytułem o silnym wpływie kulturowym i branżowym: zdobył prestiżowe nagrody (m.in. Spiel des Jahres w 1995 oraz Game of the Century według Gamescom w 2015) i stał się jednym z symboli „nowoczesnych” gier planszowych.

W niniejszej pracy analizowana jest wersja gry Catan w wariancie 1 vs 1, co w naturalny sposób eliminuje aspekt handlu pomiędzy graczami, który w rozgrywce dwuosobowej traci swój negocjacyjny charakter. W klasycznej wersji gry handel stanowi istotny element interakcji społecznej, jednak przy dwóch graczach sprowadza się on do decyzji trywialnych lub symetrycznych, nie wnosząc istotnej wartości strategicznej.

Pominięcie handlu między graczami upraszcza część społeczno-negocjacyjną gry, jednocześnie zwiększając kontrolę eksperymentalną — umożliwia prowadzenie powtarzalnych symulacji, jednoznaczne porównywanie botów oraz przypisywanie obserwowanych różnic w wynikach konkretnym strategiom decyzyjnym. Jednocześnie w wariancie 1 vs 1 zachowane zostają kluczowe własności istotne z punktu widzenia badań nad algorytmami decyzyjnymi, takie jak zarządzanie zasobami, decyzje przestrzenne na planszy, losowość wynikająca z rzutów kośćmi oraz bezpośrednia rywalizacja o punkty zwycięstwa i osiągnięcia specjalne.

#### Scena turniejowa i mistrzostwa

Istotnym potwierdzeniem dojrzałości gry jako dyscypliny rywalizacyjnej jest rozbudowana scena turniejowa. Oficjalna strona CATAN opisuje system mistrzostw obejmujący turnieje krajowe oraz wydarzenia międzynarodowe, takie jak mistrzostwa świata, Europy czy obu Ameryk. Wskazano m.in., że ostatnie mistrzostwa świata odbyły się w kwietniu 2025 w Stuttgarcie, a kolejne planowane są na 2027; jednocześnie zapowiedziano, że następne mistrzostwa Europy mają się odbyć w 2026.

Równolegle funkcjonuje program „CATAN Championship” organizowany w wielu krajach: gracze rywalizują w turniejach kwalifikacyjnych i narodowych, a zwycięzcy uzyskują możliwość udziału w wydarzeniach najwyższej rangi. Opis tej struktury (rundy wstępne, przejście do fazy pucharowej, awans do finałów narodowych) pokazuje, że *Catan* posiada standaryzowane ramy rywalizacji, co jest korzystne także z perspektywy projektowania eksperymentów porównawczych z udziałem botów.

#### Potencjał badawczy (w kontekście botów)

Z perspektywy badań nad algorytmami decyzyjnymi *Catan* jest szczególnie interesujący, ponieważ łączy kilka źródeł złożoności:

* **losowość** (dystrybucja zasobów zależna od rzutów),
* **wieloetapowe decyzje sekwencyjne** (budowa, handel, rozwój),
* **interakcję strategiczną** (blokady, wyścig o przestrzeń i punkty),
* **wiele dróg do zwycięstwa** (różne kombinacje budowy, kart rozwoju i premii).

W ramach niniejszej pracy gra stanowi więc dogodne środowisko eksperymentalne do implementacji silnika, projektowania agentów (botów) i oceny ich skuteczności w powtarzalnych warunkach, w tym z możliwością odtwarzania i analizy przebiegu rozgrywek.

### 1.3. Cel i zakres pracy

cel aplikacyjny: implementacja silnika gry Catan 1 vs 1
cel badawczy: projekt i porównanie botów o różnych strategiach

### 1.4. Podział pracy i wkład autorów

## 2. Przegląd istniejących rozwiązań
### 2.1. Istniejące implementacje Catana
### 2.2. Boty i algorytmy decyzyjne w grach planszowych
### 2.3. Podsumowanie i uzasadnienie autorskiego podejścia

## 3. Analiza problemu i założenia projektowe
### 3.1. Wymagania funkcjonalne gry
### 3.2. Wymagania niefunkcjonalne (wydajność, testowalność)
### 3.3. Ograniczenia wersji 1 vs 1
### 3.4. Model stanu gry i akcji


# 4. Architektura i implementacja systemu

## 4.1. Wybór technologii i narzędzi

Implementacja gry została wykonana w języku **C++**, który zapewnia wysoką wydajność obliczeniową, pełną kontrolę nad zarządzaniem pamięcią oraz możliwość precyzyjnego modelowania struktur danych. Wybór ten był szczególnie istotny ze względu na planowane uruchamianie **dużej liczby symulacji rozgrywek** w celu porównywania strategii botów, co wymaga niskiego narzutu czasowego na pojedynczą symulację.

Język C++ umożliwił:

- zastosowanie **struktur pakowanych bitowo**, minimalizujących rozmiar stanu gry,
- szybkie kopiowanie i porównywanie stanów (istotne przy symulacjach i testach),
- brak konieczności dynamicznych alokacji pamięci w kluczowych fragmentach silnika.

Do testowania poprawności implementacji zasad gry wykorzystano framework **GoogleTest**, który pozwala na:

- tworzenie testów jednostkowych dla pojedynczych reguł (np. budowa drogi, produkcja zasobów),
- testy regresyjne (czy zmiany w kodzie nie psują wcześniej poprawnych mechanizmów),
- automatyczną weryfikację niezmienników stanu gry po wykonaniu akcji.

Testy pełnią istotną rolę w projekcie, ponieważ nawet niewielkie błędy w implementacji zasad mogą prowadzić do nieprawidłowych decyzji botów i zafałszowania wyników porównań strategii.

W celu wsparcia analizy zachowania botów oraz ułatwienia debugowania silnika gry opracowano dodatkowe narzędzie do odtwarzania przebiegu rozgrywek. Narzędzie to zostało zaimplementowane w języku **Python** z wykorzystaniem biblioteki **Tkinter**, która została wybrana ze względu na dostępność w standardowej bibliotece języka, brak zależności zewnętrznych oraz wystarczającą funkcjonalność do tworzenia prostych interfejsów graficznych.

Aplikacja umożliwia wizualne prześledzenie rozgrywki krok po kroku na podstawie zapisanego stanu gry, co pozwala na analizę decyzji podejmowanych przez boty oraz weryfikację poprawności działania mechanizmów silnika. Dane o przebiegu gry są zapisywane przez silnik w plikach w formacie **JSONL**, w których każda linia reprezentuje pojedyncze akcje. Takie rozwiązanie pozwala na efektywny zapis danych, ich łatwe przetwarzanie oraz jednoznaczne odtworzenie całej rozgrywki w narzędziu analitycznym.


## 4.2. Struktura projektu

Projekt został podzielony na kilka warstw logicznych:

- **symulację gry** (część rdzeniową),
- **reprezentację akcji i stanu**,
- **implementacje graczy automatycznych (botów)**,
- **narzędzia uruchomieniowe i diagnostyczne**,
- **wydzielony moduł testujący**.

Taki podział ułatwia niezależny rozwój poszczególnych komponentów systemu, w szczególności silnika gry i botów, a także umożliwia automatyczne uruchamianie wielu symulacji w celu porównywania strategii i weryfikacji poprawności implementacji zasad gry.

Centralnym elementem architektury jest **silnik gry**, który odpowiada za:

- przechowywanie kompletnego stanu rozgrywki,
- generowanie legalnych akcji w danym stanie,
- stosowanie akcji i aktualizację stanu zgodnie z zasadami gry.

Logika sterująca przebiegiem rozgrywki (kolejność tur, fazy gry, komunikacja z botami) została oddzielona od logiki modyfikującej stan. Dzięki temu możliwe jest testowanie silnika w izolacji oraz wykorzystywanie go zarówno przez boty deterministyczne, jak i losowe.

Boty działają wyłącznie w oparciu o **odczyt stanu gry** i zwracają decyzje w postaci akcji, nie mając bezpośredniego dostępu do modyfikacji planszy czy zasobów.


## 4.3. Implementacja planszy i elementów gry

Plansza gry została zaimplementowana jako **graf o stałej topologii**, składający się z trzech podstawowych typów elementów:

- **heksów** (pola produkcji surowców),
- **węzłów** (miejsca budowy osad i miast),
- **krawędzi** (miejsca budowy dróg).

Zamiast dynamicznych struktur grafowych zastosowano **stałorozmiarowe tablice identyfikatorów**, odpowiadające dokładnej liczbie elementów występujących w klasycznej wersji gry *Catan*. Topologia planszy jest inicjalizowana jednokrotnie i pozostaje niezmienna w trakcie gry.

Każdy element planszy jest przechowywany w postaci **pakowanej struktury liczbowej**:

- heksy jako **16-bitowe rekordy**,
- węzły jako **64-bitowe rekordy**,
- krawędzie jako **32-bitowe rekordy**.

Takie rozwiązanie umożliwia:

- szybki dostęp do informacji o sąsiedztwie,
- sprawdzanie legalności budowy bez dodatkowych struktur pomocniczych,
- wydajne przetwarzanie stanu podczas produkcji zasobów i ruchu rozbójnika.

Szczególnie istotne jest to, że heksy przechowują zakodowaną informację o **udziale graczy w produkcji**, co pozwala rozdzielać zasoby bez konieczności każdorazowego analizowania przyległych węzłów.


## 4.4. System akcji i cofania ruchów

Wszystkie działania w grze są reprezentowane przez jednolity typ **akcji**, zakodowany jako **64-bitowa wartość**. Akcja zawiera:

- typ operacji (np. budowa, handel, rzut kośćmi),
- identyfikator gracza wykonującego akcję,
- opcjonalne argumenty (np. identyfikatory heksów, węzłów, krawędzi),
- wektor zasobów, jeżeli akcja tego wymaga.

Taka reprezentacja upraszcza interfejs pomiędzy botami a silnikiem gry — bot zawsze zwraca jedną wartość opisującą swoją decyzję, a silnik interpretuje ją zgodnie z typem akcji.

Silnik gry implementuje mechanizm **apply / undo**, który obejmuje:

- zastosowanie akcji do bieżącego stanu,
- zapis minimalnej informacji potrzebnej do cofnięcia skutków,
- możliwość przywrócenia poprzedniego stanu gry.

Mechanizm cofania ruchów jest kluczowy dla:

- botów analizujących wiele wariantów przyszłych stanów,
- testów jednostkowych,
- symulacji deterministycznych.


## 4.5. Zarządzanie stanem gry

Stan gry jest przechowywany w jednej, zwartej strukturze, która zawiera:

- stan planszy (heksy, węzły, krawędzie),
- stan graczy (zasoby, karty, punkty zwycięstwa),
- stan banku,
- pozycję rozbójnika,
- informacje o aktualnej turze i graczu aktywnym.

Stan gracza został zakodowany w postaci **jednej 64-bitowej wartości**, co umożliwia szybkie modyfikacje zasobów, punktów zwycięstwa i liczników kart oraz łatwe porównywanie stanów.

Boty otrzymują dostęp wyłącznie do **odczytu stanu gry**, a po wykonaniu decyzji silnik weryfikuje, czy stan nie został zmieniony w sposób niedozwolony. Zapewnia to spójność symulacji i poprawność porównywania strategii.

Całość tworzy **deterministyczny, testowalny i wydajny system symulacji**, który stanowi solidną podstawę do dalszej części pracy, poświęconej implementacji i analizie graczy automatycznych.


## 5. Testowanie i weryfikacja poprawności
### 5.1. Strategia testowania
### 5.2. Testy jednostkowe i integracyjne
### 5.3. Walidacja zgodności z zasadami gry

## 6. Projekt i implementacja botów


## 6.1. Gracz losowy (baseline)

Najprostszym zaimplementowanym graczem automatycznym jest **gracz losowy**, który stanowi punkt odniesienia (baseline) dla wszystkich pozostałych botów. Jego głównym celem nie jest osiąganie wysokich wyników, lecz dostarczenie **minimalnego poziomu kompetencji**, względem którego można mierzyć skuteczność bardziej zaawansowanych strategii.

### Założenia projektowe

Gracz losowy działa według następujących zasad:

- w każdej fazie gry pobiera od silnika listę **wszystkich legalnych akcji** dostępnych w danym stanie,
- wybiera jedną z nich **z jednakowym prawdopodobieństwem**,
- nie analizuje przyszłych konsekwencji decyzji,
- nie wykorzystuje żadnej wiedzy domenowej o grze *Catan*.

Bot ten nie posiada pamięci długoterminowej ani mechanizmu uczenia – każda decyzja podejmowana jest niezależnie od poprzednich ruchów oraz aktualnej sytuacji strategicznej.

### Rola w badaniach

Pomimo swojej prostoty, gracz losowy pełni istotną funkcję w pracy:

- umożliwia weryfikację poprawności działania silnika gry (czy gra „dochodzi do końca” bez błędów),
- pozwala określić **dolną granicę skuteczności** strategii,
- stanowi punkt odniesienia przy ocenie, czy dana heurystyka faktycznie wnosi wartość decyzyjną.

Jeżeli bot heurystyczny nie osiąga statystycznie lepszych wyników niż gracz losowy, oznacza to, że zaprojektowana strategia jest nieskuteczna lub błędnie zaimplementowana.

### Charakterystyka zachowania

W praktyce gracz losowy:

- często buduje struktury w nieoptymalnych lokalizacjach,
- nie planuje ciągłości sieci dróg,
- zużywa zasoby bez długoterminowego celu,
- podejmuje losowe decyzje dotyczące kart rozwoju i budowy.

Pomimo tego, dzięki losowości rzutów kośćmi, bot ten jest w stanie okazjonalnie wygrać pojedyncze rozgrywki, co dodatkowo podkreśla znaczenie przeprowadzania **dużej liczby symulacji** w analizie wyników.


## 6.2. Boty heurystyczne

Drugą grupę graczy automatycznych stanowią **boty heurystyczne**, które podejmują decyzje na podstawie uproszczonej oceny jakości dostępnych akcji. W przeciwieństwie do gracza losowego, boty te wykorzystują wiedzę domenową o mechanice gry *Catan*, jednak nie stosują przeszukiwania drzewa gry ani symulacji przyszłych stanów.

Zastosowano podejście **iteracyjnego ulepszania strategii**, w którym każda kolejna wersja bota rozszerza poprzednią o nowe kryteria oceny.

### 6.2.1. Założenia wspólne

Wszystkie boty heurystyczne (it1–it5) działają według wspólnego schematu:

1. silnik gry generuje listę legalnych akcji,
2. dla każdej akcji bot oblicza **wartość heurystyczną**,
3. wybierana jest akcja o najwyższej ocenie,
4. w przypadku remisu stosowane jest losowe rozstrzygnięcie.

Heurystyka ma charakter **funkcji punktowej**, której wartość jest sumą ważonych składowych opisujących bieżące korzyści i potencjalne ryzyka związane z daną akcją.

Takie podejście pozwala zachować:

- niski koszt obliczeniowy decyzji,
- deterministyczność zachowania (z wyjątkiem remisów),
- łatwość modyfikacji i rozszerzania strategii.

### 6.2.2. Iteracyjne rozszerzanie heurystyk (it1–it5)

Proces projektowania botów heurystycznych przebiegał iteracyjnie. Każda kolejna wersja bota rozszerzała funkcję oceny o nowe elementy, obserwowane jako istotne podczas analizy rozgrywek poprzednich wersji.

#### it1 – heurystyka punktów zwycięstwa

Pierwsza wersja bota heurystycznego (it1) opiera się na najbardziej oczywistym kryterium: **maksymalizacji przyrostu punktów zwycięstwa**. Akcje prowadzące bezpośrednio do zdobycia punktów (np. budowa osady lub miasta) otrzymują najwyższą ocenę.

Strategia ta jest prosta, lecz krótkowzroczna – nie uwzględnia przyszłej produkcji zasobów ani kontroli przestrzeni na planszy.

#### it2 – zarządzanie zasobami i handel z bankiem

Druga iteracja bota wprowadza fundamentalną zmianę: **aktywne wykorzystanie handlu z bankiem** do optymalizacji ścieżki do zakupu struktur.

Bot wykrywa kontrolowane porty (2:1 specyficzne, 3:1 generyczne, 4:1 standardowy bank) i wykorzystuje je do przekształcania nadwyżek surowców w brakujące zasoby. Kluczowym mechanizmem jest ocena **osiągalności celu po uwzględnieniu handlu** poprzez obliczenie, ile surowców pozostanie do zdobycia nawet po optymalnej wymianie nadwyżek.

Bot wybiera cel zakupu nie na podstawie tego, co może kupić natychmiast, lecz **do czego jest najbliżej po serii transakcji**. Może świadomie zrezygnować z budowy drogi, jeśli po wymianie zasobów będzie w stanie zbudować osadę lub miasto. Handel jest rozważany przed akcjami niższego priorytetu (drogi, karty rozwoju), co pozwala systematycznie gromadzić zasoby do celów wysokowartościowych.

Hierarchia priorytetów:
```
Miasto/Osada (natychmiast) → Handel (w kierunku celu) → Droga → Karta rozwoju → Koniec tury
```

Strategia ta redukuje liczbę „martwych tur" i przyspiesza osiąganie celów wysokowartościowych, jednak nadal nie analizuje aspektów przestrzennych planszy.

#### it3 – optymalizacja fazy początkowej i kontrola przeciwnika

Trzecia iteracja wprowadza **strategię ustawień początkowych** oraz mechanizm aktywnego ograniczania rozwoju przeciwnika poprzez mądre użycie rozbójnika.

**Ocena węzłów początkowych** uwzględnia produkcję ważoną (pipsy × wagi surowców, gdzie brick/lumber = 5, grain/wool = 4, ore = 3), co odzwierciedla potrzeby wczesnej ekspansji. Bot silnie premiuje **różnorodność zasobów** (+25 za każdy unikalny typ) i karze duplikaty na tym samym węźle (-20 za powtórzenie), preferując węzły typu [brick, lumber, grain] nad [lumber, lumber, grain]. Porty są oceniane wyżej w drugim ustawieniu niż w pierwszym.

**Strategia drugiego ustawienia** jest komplementarna: bot analizuje, jakich surowców brakuje z pierwszej osady i premiuje węzły dostarczające nowych typów zasobów (+18 za każdy nowy, +10 dodatkowego za brick/lumber). Celem jest pokrycie wszystkich pięciu typów surowców zamiast kumulacji tych samych.

**Rozbójnik jako narzędzie kontroli**: bot maksymalizuje szkody dla przeciwnika (pipsy × 30 × wartość przeciwnika) przy jednoczesnej minimalizacji własnych strat (pipsy × 45 × własna wartość). Silnie preferowane są heksy, gdzie tylko przeciwnik posiada budynki. Strategia unika blokowania pustych pól i własnej produkcji.

Bot it3 łączy zbalansowany rozwój ekonomiczny z aktywną obroną, jednak nadal dziedziczy zarządzanie zasobami z it2 dla fazy głównej gry.

#### it4 – inteligentne użycie kart rozwoju i selekcja deterministyczna

Czwarta iteracja wprowadza dwie fundamentalne innowacje: **zaawansowane zarządzanie kartami rozwoju** oraz **deterministyczny wybór najlepszej akcji budowy** (w miejsce losowego wyboru z it1–it3).

Bot implementuje odrębną logikę dla każdego typu karty rozwoju. **Knight** oceniany jest według szkód dla przeciwnika (+10000 za zdobycie Largest Army, +2000 jeśli odbiera ją przeciwnikowi, +700 za możliwość kradzieży kart). **Monopoly** akceptowany tylko gdy przeciwnik ma ≥3 kart danego surowca; bot modeluje stan po monopoly i przyznaje +8000 jeśli pozwoli kupić miasto, +6000 za osadę. **Year of Plenty** wybiera pary surowców maksymalizujące redukcję deficytu do najbliższego zakupu (+9000 za unlock miasta).

Kluczowym mechanizmem jest **świadomość ryzyka pre-roll**: karty zwiększające rękę (Monopoly, Year of Plenty) są karane (-4500) jeśli projekcja ręki przekracza 10 kart przed rzutem, co chroni przed stratami przy wyrzuceniu 7. Karty bezpieczne (Knight, Road Building) nie mają tej kary.

**Deterministyczny wybór budowy** preferuje węzły o najwyższej produkcji ważonej (ore=14, grain=13 > brick/lumber=12 > wool=11), co odzwierciedla potrzeby mid-game (miasta, dev cards). Drogi oceniane są według potencjału otwieranych węzłów (3× production score). Bot adaptuje strategię do fazy gry: przy VP≥6 preferuje dev cards nad drogami, chyba że droga ma wyjątkowo wysoki score (>4200).

#### it5 – heurystyka zbalansowana

Piąta i finalna iteracja bota heurystycznego wprowadza dwa kluczowe ulepszenia: **inteligentne zarządzanie zrzucaniem kart** oraz **symulacyjną ocenę akcji** wykorzystującą mechanizm apply/undo silnika gry. Bot it5 reprezentuje najbardziej zaawansowaną strategię heurystyczną w pracy, łącząc wszystkie wcześniejsze mechanizmy z nowymi technikami oceny pozycji.

**Inteligentne zrzucanie kart przy przekroczeniu limitu**

Gdy gracz posiada więcej niż 9 kart i musi zrzucić połowę po wyrzuceniu 7, bot it5 implementuje strategię opartą na **celu zakupu**. Mechanizm działa w trzech krokach:

1. **Wybór najlepszego celu** — bot analizuje wszystkie możliwe zakupy (miasto, osada, droga, karta rozwoju) i wybiera ten, do którego jest najbliżej pod względem deficytu zasobów. W przypadku remisu preferowane są cele wyższej wartości (miasto > osada > droga > karta rozwoju).

2. **Dynamiczne wagi zasobów** — dla wybranego celu bot przypisuje wagi każdemu typowi surowca, odzwierciedlające jego znaczenie dla danego zakupu. Wagi bazowe (brick=10, lumber=10, wool=8, grain=12, ore=13) są modyfikowane w zależności od celu:
   - **Miasto**: +10 ore, +8 grain (priorytet surowców do miast)
   - **Osada**: +7 brick/lumber, +6 wool/grain (zbalansowany mix)
   - **Droga**: +8 brick/lumber (podstawowe surowce)
   - **Karta rozwoju**: +7 ore/grain/wool (elastyczność)

3. **Preferencja nadwyżek** — bot silnie preferuje zrzucanie zasobów, które są nadwyżką względem wybranego celu. Zasoby potrzebne do zakupu są chronione dodatkową karą (+500), co minimalizuje ryzyko zablokowania możliwości zakupu w następnej turze.

Strategia ta zapewnia, że nawet w sytuacji przymusowego zrzucania kart, bot zachowuje spójność z długoterminowym planem zakupów i nie traci kluczowych zasobów.

**Symulacyjna ocena akcji**

Najważniejszą innowacją bota it5 jest wykorzystanie **symulacji akcji** do oceny ich jakości. Zamiast polegać wyłącznie na heurystykach statycznych, bot:

1. **Symuluje każdą deterministyczną akcję** — dla akcji budowy miasta, osady, drogi oraz handlu z bankiem, bot tymczasowo stosuje akcję (`applyAction()`), ocenia wynikową pozycję za pomocą funkcji `evaluate_position()`, a następnie cofa akcję (`undoLastAction()`).

2. **Premiuje akcje odblokowujące** — akcje handlu i budowy dróg otrzymują dodatkowy bonus (+8000 za odblokowanie możliwości budowy miasta, +5000 za osadę), jeśli po ich wykonaniu bot mógłby natychmiast zbudować strukturę wysokiej wartości. Mechanizm ten pozwala botowi planować sekwencje akcji zamiast oceniać je w izolacji.

3. **Hierarchia priorytetów z symulacją**:
   - **Miasta i osady** — wszystkie legalne akcje są symulowane, wybierana jest ta o najwyższej ocenie pozycji
   - **Drogi i handel** — również oceniane przez symulację, z dodatkowymi bonusami za odblokowanie
   - **Karty rozwoju** — oceniane heurystycznie (bez symulacji, ze względu na losowość), z modyfikacjami zależnymi od fazy gry i pozycji względem przeciwnika

**Adaptacja do fazy gry i pozycji**

Bot it5 uwzględnia kontekst rozgrywki przy podejmowaniu decyzji:

- **Karty rozwoju** są preferowane gdy bot jest w tyle względem przeciwnika (+2500 bonus) lub w fazie środkowej gry, ale karane w fazie końcowej (VP≥8, -500) oraz gdy dostępne są lepsze opcje deterministyczne (handel -500, droga -200).

- **Fallback do it4** — jeśli symulacja nie wskazuje wyraźnie lepszej akcji niż obecna pozycja, bot deleguje decyzję do strategii it4, zapewniając stabilność zachowania.

- **Ochrona przed regresją** — bot unika wyboru `EndTurn`, jeśli ocena pozycji po zakończeniu tury byłaby gorsza niż obecna, ponownie korzystając z fallbacku do it4.

**Podsumowanie strategii it5**

Bot it5 łączy wszystkie mechanizmy z poprzednich iteracji (handel, kontrola przestrzeni, zarządzanie kartami rozwoju) z nowymi technikami: inteligentnym zrzucaniem kart oraz symulacyjną oceną akcji. Dzięki wykorzystaniu mechanizmu apply/undo silnika gry, bot może oceniać konsekwencje akcji w sposób bardziej precyzyjny niż czysto heurystyczne podejście, zachowując jednocześnie niski koszt obliczeniowy w porównaniu do pełnego przeszukiwania drzewa gry. Strategia ta stanowi punkt odniesienia dla bota wykorzystującego algorytm alpha-beta, opisanego w kolejnym podrozdziale.



### 6.3. Bot wykorzystujący algorytm alpha-beta

Kolejny, bardziej zaawansowanym graczem automatycznym jest bot wykorzystujący **algorytm przeszukiwania drzewa gry alpha-beta**. Jego celem jest podejmowanie decyzji na podstawie analizy przyszłych stanów gry, z uwzględnieniem możliwych odpowiedzi przeciwnika.

W przeciwieństwie do botów heurystycznych, które oceniają jedynie pojedynczy ruch, bot alpha-beta eksploruje sekwencje akcji, traktując grę jako **dwuosobową grę o sumie zerowej** i zakładając racjonalne zachowanie przeciwnika.

#### Podstawowa wersja algorytmu

Początkowa implementacja bota opierała się na klasycznym algorytmie **minimax z obcinaniem alpha-beta**, przeszukującym drzewo gry do stałej głębokości. W węzłach drzewa:

- gracz sterowany przez bota pełni rolę **maksymalizującą**,
- przeciwnik traktowany jest jako gracz **minimalizujący** wartość funkcji oceny.

Po osiągnięciu maksymalnej głębokości przeszukiwania lub stanu terminalnego, pozycja oceniana jest za pomocą **rozbudowanej funkcji heurystycznej**, uwzględniającej m.in.:

- różnicę punktów zwycięstwa,
- produkcję zasobów,
- potencjał przyszłych osad,
- stan zasobów na ręce,
- długość najdłuższej drogi,
- liczbę kart rozwoju.

Głębokość przeszukiwania została ograniczona do niewielkiej wartości (maksymalnie 3), co wynika z dużego współczynnika rozgałęzienia drzewa gry *Catan*, szczególnie w fazach obejmujących liczne akcje handlu.


#### Sortowanie akcji i poprawa skuteczności obcinania

W praktyce okazało się, że naiwne przeszukiwanie drzewa, w którym akcje rozważane są w kolejności generowanej przez silnik gry, prowadzi do znacznych kosztów obliczeniowych i ogranicza efektywność obcinania alpha-beta.

W celu przyspieszenia działania algorytmu wprowadzono **heurystyczne sortowanie akcji** przed ich eksploracją. Każdej legalnej, deterministycznej akcji przypisywana jest szybka, przybliżona ocena jakości, która:

- preferuje akcje budowy miasta i osady,
- promuje działania prowadzące do zdobycia punktów zwycięstwa,
- wstępnie ocenia użyteczność handlu z bankiem.

Następnie:

- w węzłach maksymalizujących akcje są rozważane w **kolejności malejącej** oceny,
- w węzłach minimalizujących akcje są rozważane w **kolejności rosnącej** oceny.

Takie uporządkowanie powoduje, że w węzłach maksymalizujących najsilniejsze ruchy analizowane są jako pierwsze, natomiast w węzłach minimalizujących priorytetowo rozważane są ruchy najbardziej niekorzystne dla gracza maksymalizującego. Dzięki temu znacznie częściej spełniony zostaje warunek obcinania (`beta ≤ alpha`), co prowadzi do istotnej redukcji liczby odwiedzanych węzłów drzewa gry.

W praktyce umożliwia to przeszukiwanie głębszych drzew decyzyjnych przy tym samym budżecie czasowym, bez pogorszenia jakości podejmowanych decyzji.

#### Adaptacja algorytmu do fazy gry

Kolejnym istotnym rozszerzeniem algorytmu było wprowadzenie mechanizmu **dynamicznej adaptacji strategii do fazy gry**. Faza gry określana jest na podstawie maksymalnej liczby punktów zwycięstwa posiadanych przez dowolnego gracza:

- **faza początkowa** – mniej niż 5 punktów zwycięstwa,
- **faza środkowa** – od 5 do 7 punktów,
- **faza końcowa** – 8 lub więcej punktów.

W zależności od fazy gry modyfikowane są:

- wagi składowych funkcji oceny,
- maksymalna głębokość przeszukiwania,
- liczba rozważanych akcji handlu,
- priorytety typów akcji (np. budowa vs. handel).

W fazie początkowej algorytm preferuje rozwój produkcji i elastyczność zasobów, przy jednoczesnym ograniczeniu głębokości przeszukiwania ze względu na bardzo dużą liczbę dostępnych akcji. W fazie środkowej równoważone są aspekty ekonomiczne i punktowe. Natomiast w fazie końcowej algorytm kładzie silny nacisk na **natychmiastowe zdobywanie punktów zwycięstwa**, zwiększając wagę budowy miast i osad oraz pogłębiając przeszukiwanie drzewa gry.


#### Selekcja i filtrowanie akcji

Aby dodatkowo ograniczyć złożoność obliczeniową, bot alpha-beta rozważa wyłącznie **akcje deterministyczne**, pomijając działania o losowym efekcie (np. dobór kart rozwoju), które nie są bezpośrednio modelowane w drzewie gry.

Akcje handlu z bankiem są dodatkowo filtrowane — w każdej turze analizowana jest jedynie ograniczona liczba najlepiej ocenionych transakcji, przy czym limit ten zależy od aktualnej fazy gry. W fazie końcowej liczba rozważanych wymian jest celowo zmniejszana, aby skoncentrować obliczenia na akcjach prowadzących bezpośrednio do zakończenia rozgrywki.

#### Integracja z botami heurystycznymi

W sytuacjach, w których:

- dostępne są wyłącznie akcje niedeterministyczne,
- najlepszą decyzją według algorytmu okazuje się zakończenie tury,
- lub wynik przeszukiwania nie poprawia bieżącej oceny pozycji,

bot alpha-beta stosuje **mechanizm awaryjny**, delegując decyzję do najbardziej zaawansowanego bota heurystycznego (it5). Takie rozwiązanie zapewnia stabilność zachowania i zapobiega podejmowaniu decyzji ewidentnie gorszych od strategii heurystycznej.

### 6.4. Bot celujący w jeden zasób (strategia monosurowcowa – bot eksperymentalny)

Oprócz botów opisanych wcześniej zaimplementowano również dodatkowego gracza o wąsko wyspecjalizowanej strategii, nazwanego roboczo **OneResourcePlayer**. Jego założeniem jest maksymalizacja korzyści z jednego, wybranego surowca poprzez:

- wybór **priorytetowego surowca** na podstawie aktualnej konfiguracji planszy,
- dążenie do zajęcia **portu 2:1** dla tego surowca już w ustawieniach początkowych,
- prowadzenie rozwoju infrastruktury (osady, miasta, drogi) w kierunku pól oraz portów związanych z tym surowcem,
- wykonywanie transakcji z bankiem głównie wtedy, gdy zwiększają liczbę kart priorytetowego surowca.

W odróżnieniu od bota alpha-beta, celem tego gracza nie jest optymalna gra w sensie minimaksowym, lecz sprawdzenie hipotezy: **czy silna specjalizacja w jeden surowiec i wczesne pozyskanie portu 2:1 może stanowić skuteczną strategię w wariancie 1 vs 1**. Z tego względu bot ten należy traktować jako **bot eksperymentalny**.

#### Wybór priorytetowego surowca

Priorytetowy surowiec wybierany jest automatycznie na podstawie parametrów planszy. Dla każdego surowca obliczane są cechy opisujące jego „atrakcyjność”:

- suma oczek (pips) ze wszystkich heksów danego surowca (ogólna dostępność),
- najlepszy pojedynczy węzeł (ile pipsów danego surowca może generować jedna osada),
- najlepszy węzeł połączony z portem 2:1 dla tego surowca,
- liczba heksów z danym surowcem.

Na tej podstawie konstruowany jest wynik punktowy, w którym najwyżej premiowane są przypadki umożliwiające połączenie **dobrego źródła produkcji** z **portem 2:1**. Dodatkowo wprowadzono minimalne rozstrzyganie remisów na korzyść cegły i drewna (z uwagi na ich rolę w budowie dróg).

#### Ustawienie początkowe: wymuszenie portu 2:1

Najważniejszym elementem strategii OneResourcePlayer jest faza początkowa. Bot stara się tak dobrać pierwszą osadę, aby:

1. znajdowała się na węźle z portem 2:1 odpowiadającym wybranemu surowcowi,
2. miała możliwie wysoką produkcję tego surowca (wysokie pipsy),
3. zapewniała sensowną możliwość rozwoju (premiowany jest „stopień” węzła, tj. liczba dostępnych krawędzi do dalszej rozbudowy dróg).

Jeżeli wśród legalnych akcji nie ma możliwości uzyskania portu 2:1 dla najlepszego surowca (np. z uwagi na ograniczenia legalnych ustawień), bot podejmuje próbę znalezienia **jakiegokolwiek** portu 2:1 dostępnego w danym układzie, a dopiero w ostateczności wybiera pierwszą legalną akcję.

Drugie ustawienie początkowe również preferuje surowiec priorytetowy (wysokie pipsy), ale dodatkowo wprowadza słabą heurystykę „uzupełniania braków” — premiowane są węzły dostarczające innych surowców, których bot nie miał w pierwszej osadzie, aby ograniczyć ryzyko całkowitego zablokowania rozwoju.


#### Logika tury: specjalizacja zamiast balansu

W normalnej fazie gry bot rozważa wyłącznie akcje deterministyczne i stosuje następującą kolejność priorytetów:

1. **budowa miasta** – silnie premiowana, jeśli zwiększa produkcję priorytetowego surowca oraz (dodatkowo) leży na porcie 2:1,
2. **budowa osady** – analogicznie premiowana w zależności od pipsów surowca priorytetowego oraz portu,
3. **handel z bankiem** – wykonywany wyłącznie wtedy, gdy po wymianie liczba kart priorytetowego surowca wzrasta (czyli strategia aktywnie „pompuje” jeden surowiec),
4. **budowa drogi** – oceniana przez funkcję potencjału, która bada węzły w zasięgu 1–2 krawędzi i premiuje:
   - pipsy priorytetowego surowca w możliwych lokalizacjach osad,
   - potencjalny dostęp do portu 2:1 dla priorytetowego surowca,
   - możliwości dalszej rozbudowy (stopień węzła),
   - przy jednoczesnym ignorowaniu miejsc, gdzie nie da się legalnie postawić osady (reguła odległości).

Warto zauważyć, że strategia ta jest celowo **jednostronna**: bot często poświęca równowagę zasobów na rzecz maksymalizacji jednego kanału ekonomicznego (produkcja + port 2:1).

#### Odrzucanie kart przy rozbójniku

Bot implementuje również własną politykę zrzucania kart w sytuacji przekroczenia limitu (powyżej 9). Mechanizm ten:

- wybiera „docelowy zakup” (miasto / osada / droga / karta rozwoju) poprzez minimalizację deficytu zasobów,
- jednocześnie stosuje mocny bias na zachowanie surowca priorytetowego,
- unika zrzucania zasobów niezbędnych do najbardziej sensownego zakupu.

Dzięki temu bot stara się utrzymać spójność strategii nawet w sytuacjach przymusowej redukcji ręki.


#### Mechanizm awaryjny: powrót do bota zbalansowanego

W sytuacji, gdy żaden z ruchów nie daje wyraźnej korzyści w ramach strategii monosurowcowej, bot nie wykonuje losowych działań. Zamiast tego stosowany jest fallback do **It5Player**, czyli najbardziej zbalansowanego bota heurystycznego. Zapobiega to „utknięciu” strategii w stanach, w których dążenie do jednego surowca przestaje być racjonalne.


#### Ocena wstępna i charakter eksperymentalny

W późniejszych eksperymentach porównawczych okazało się, że strategia monosurowcowa **nie daje stabilnej przewagi** nad najlepszymi botami heurystycznymi ani nad botem alpha-beta. W szczególności:

- nadmierna specjalizacja zwiększa ryzyko zablokowania rozwoju przy niekorzystnym rozkładzie rzutów,
- strategia jest wrażliwa na to, czy port 2:1 faktycznie zostanie osiągnięty na silnym węźle,
- przeciwnik może częściowo kontrować plan poprzez blokowanie kluczowych lokalizacji ekspansji.

W związku z tym OneResourcePlayer należy traktować przede wszystkim jako **bot eksperymentalny**, zaprojektowany w celu przetestowania konkretnej hipotezy strategicznej i lepszego zrozumienia dynamiki gry w wariancie 1 vs 1, a nie jako docelowo najsilniejszego przeciwnika.

## 7. Eksperymenty i analiza wyników
(Wspomnieć o tym że normalna gra w Catana trwa 60-70 tur i porównać to z botami)
### 7.1. Metodologia porównania botów
### 7.2. Scenariusze testowe
### Scenariusz 1: Skalowanie jakości botów
Każdy bot gra z losowym graczem

Cel:
pokazanie, że każdy kolejny bot jest obiektywnie lepszy od baseline.

Porównania:
Random vs it1
Random vs it5
Random vs AlphaBeta

### Scenariusz 2: Porównanie heurystyk
Boty heurystyczne między sobą
Cel:
uzasadnienie iteracyjnego podejścia.

Porównania:
it1 vs it3
it3 vs it5

### Scenariusz 3: Bot eksperymentalny
OneResource vs it5 / AlphaBeta

Cel:
pokazanie, że ciekawa hipoteza ≠ najlepszy wynik.
To bardzo dobrze wygląda:
„strategia monosurowcowa jest interesująca, ale niestabilna”.

### Scenariusz 4: Najlepszy vs najlepszy
AlphaBeta vs it5
Cel:
pokazanie, czy koszt obliczeniowy alpha-beta się opłaca.
### 7.3. Wyniki eksperymentów
### 7.4. Analiza i interpretacja wyników

## 8. Podsumowanie i wnioski
### 8.1. Ocena realizacji celów pracy
### 8.2. Wnioski z części badawczej
### 8.3. Możliwości dalszego rozwoju systemu

## Bibliografia
- Catan - Game Rules https://www.catan.com/understand-catan/game-rules
- Catan - Championships https://www.catan.com/catan-fans/championships
- Catan - Wikipedia https://en.wikipedia.org/wiki/Catan
- https://www.artofcatan.com/p/was-it-luck-or-skill
- GoogleTest User’s Guide https://google.github.io/googletest/reference/testing.html
- tkinter — Python interface to Tcl/Tk https://docs.python.org/3/library/tkinter.html
- https://www.alcumena.fundacjapsc.pl/index.php/alcumena/article/download/337/182/710