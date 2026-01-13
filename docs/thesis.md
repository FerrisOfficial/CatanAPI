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

## 4.1. Wybór technologii i narzędzi (C++, GoogleTest)

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

Celem niniejszego rozdziału jest opis zaprojektowanych i zaimplementowanych graczy automatycznych (botów), które zostały wykorzystane do badań porównawczych w dalszej części pracy. Boty różnią się stopniem złożoności strategii decyzyjnej – od gracza w pełni losowego, pełniącego rolę punktu odniesienia, po boty heurystyczne rozwijane iteracyjnie poprzez stopniowe wzbogacanie funkcji oceny stanu gry.

Takie podejście umożliwia analizę wpływu poszczególnych elementów strategii na skuteczność rozgrywki oraz pozwala na obserwację, w jakim stopniu nawet proste heurystyki poprawiają jakość decyzji względem losowego wyboru akcji.


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

#### it2 – uwzględnienie produkcji zasobów

W drugiej iteracji dodano ocenę potencjalnej **produkcji surowców** wynikającej z budowy osady lub miasta. Akcje zwiększające dostęp do heksów o wysokim prawdopodobieństwie produkcji (numery 6 i 8) są premiowane.

Bot zaczyna w ten sposób preferować lokalizacje zapewniające stabilny dopływ zasobów, co przekłada się na większą liczbę możliwych akcji w kolejnych turach.

#### it3 – kontrola przestrzeni i sieci dróg

Trzecia wersja heurystyki rozszerza ocenę o aspekty **przestrzenne**:

- ciągłość sieci dróg,
- blokowanie potencjalnych lokalizacji przeciwnika,
- dostęp do nowych węzłów budowy.

Bot it3 nie tylko rozwija własną infrastrukturę, ale również aktywnie ogranicza możliwości rozwoju przeciwnika.

#### it4 – zarządzanie zasobami i kosztami

W czwartej iteracji uwzględniono bieżący stan zasobów gracza. Akcje, które prowadzą do nadmiernego gromadzenia jednego typu surowca kosztem innych, są karane niższą oceną.

Celem tej heurystyki jest:

- zmniejszenie liczby „martwych tur”,
- utrzymanie elastyczności decyzyjnej,
- lepsze wykorzystanie banku zasobów.

#### it5 – heurystyka zbalansowana

Ostatnia wersja bota heurystycznego (it5) łączy wszystkie wcześniejsze kryteria w jedną, zbalansowaną funkcję oceny. Wagi poszczególnych składowych zostały dobrane eksperymentalnie na podstawie wyników symulacji.

Bot ten reprezentuje **najbardziej zaawansowaną strategię heurystyczną** w pracy i stanowi bezpośredni punkt odniesienia dla bota wykorzystującego algorytm przeszukiwania drzewa gry, opisanego w kolejnym podrozdziale.

### 6.3. Bot wykorzystujący algorytm alpha-beta
### 6.4. Porównanie złożoności i założeń strategii

## 7. Eksperymenty i analiza wyników
(Wspomnieć o tym że normalna gra w Catana trwa 60-70 tur i porównać to z botami)
### 7.1. Metodologia porównania botów
### 7.2. Scenariusze testowe
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