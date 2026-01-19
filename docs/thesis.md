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

### 1.2.1. Plansza, cel gry i podstawowe zasady

Plansza gry *Catan* składa się z heksagonalnych pól reprezentujących różne typy terenu, takich jak lasy, wzgórza, pola uprawne, pastwiska oraz góry, z których każde odpowiada określonemu rodzajowi surowca. Pola te rozmieszczone są w sposób modularny, co powoduje, że każda rozgrywka posiada inną konfigurację przestrzenną. Pomiędzy heksami znajdują się węzły (skrzyżowania), na których gracze mogą budować osady i miasta, oraz krawędzie, na których budowane są drogi.

Każdy typ pola produkcyjnego na planszy odpowiada określonemu surowcowi:
- lasy produkują drewno (*Lumber*),
- wzgórza produkują cegłę (*Brick*),
- pola uprawne produkują zboże (*Grain*),
- pastwiska produkują wełnę (*Wool*),
- góry produkują rudę (*Ore*).

Pola pustynne nie generują zasobów i stanowią początkową lokalizację rozbójnika.

Celem gry jest zdobycie **10 punktów zwycięstwa** (15 w wariancie rozgrywki 1 vs 1). Punkty te są przyznawane głównie za budowę osad (1 punkt zwycięstwa) i miast (2 punkty zwycięstwa). Dodatkowymi źródłami punktów zwycięstwa są premie:
- Premia **Najdłuższa Droga** przyznawana jest graczowi posiadającemu najdłuższy nieprzerwany ciąg połączonych dróg o długości co najmniej pięciu segmentów. Premia ta może zostać odebrana przez innego gracza, jeśli zbuduje on dłuższą drogę.
- Premia **Największa Armia** przyznawana jest graczowi, który jako pierwszy użyje co najmniej trzech kart Rycerz i posiada ich więcej niż pozostali gracze. Podobnie jak w przypadku Najdłuższej Drogi, premia ta jest dynamiczna i może przechodzić pomiędzy graczami w trakcie rozgrywki.

Każda z tych premii zapewnia dodatkowe **2 punkty zwycięstwa**, istotnie wpływając na strategię oraz tempo gry. W dalszej części pracy punkty zwycięstwa będą oznaczane skrótem **VP** (ang. *Victory Points*). Rozgrywka toczy się w turach, a zwycięstwo następuje natychmiast po osiągnięciu wymaganej liczby punktów przez jednego z graczy.

Podstawowy przebieg tury obejmuje rzut dwiema kośćmi sześciennymi, który determinuje produkcję zasobów na planszy. Gracze otrzymują surowce z tych pól, których numer odpowiada wyrzuconej sumie, pod warunkiem że posiadają przy nich osady lub miasta. Następnie możliwe jest prowadzenie handlu (z innymi graczami lub z bankiem) oraz wykonywanie akcji budowy, takich jak wznoszenie dróg, osad, miast lub zakup kart rozwoju.

Istotnym elementem gry jest losowość wynikająca z rzutów kośćmi, która wpływa na tempo pozyskiwania zasobów, jednak decyzje strategiczne — wybór lokalizacji budowy, kierunek rozwoju infrastruktury oraz zarządzanie zasobami — mają kluczowe znaczenie dla długoterminowego sukcesu. Ta kombinacja **niepełnej informacji, losowości i planowania** sprawia, że *Catan* stanowi interesujące środowisko badawcze dla analizy algorytmów decyzyjnych i strategii gry.

### 1.2.2. Produkcja zasobów i miara pipsów

Każde pole produkcyjne (heks) na planszy posiada przypisaną liczbę z zakresu 2–12 (z wyjątkiem 7, który odpowiada aktywacji rozbójnika). Liczby te odpowiadają możliwym sumom wyrzuconym na dwóch sześciennych kościach. Ze względu na różną liczbę kombinacji prowadzących do danej sumy, poszczególne liczby mają odmienne prawdopodobieństwo wystąpienia. W praktyce wprowadza się miarę **pipsów** (z ang. *pips*, dosłownie: oczka na kości), która odzwierciedla częstość występowania danej liczby:

- **2 lub 12**: 1 pipsa (1 kombinacja: 1+1 lub 6+6),
- **3 lub 11**: 2 pipsy (2 kombinacje: 1+2, 2+1 lub 5+6, 6+5),
- **4 lub 10**: 3 pipsy (3 kombinacje),
- **5 lub 9**: 4 pipsy (4 kombinacje),
- **6 lub 8**: 5 pipsów (5 kombinacji).

Liczba 7 nie występuje na heksach produkcyjnych — jej wyrzucenie aktywuje rozbójnika i wymusza odrzucenie połowy kart przez graczy posiadających więcej niż siedem zasobów.

Miara pipsów stanowi użyteczne narzędzie analityczne, ponieważ bezpośrednio odzwierciedla **wartość oczekiwaną produkcji** z danego pola. Heksy o wyższych pipsach generują zasoby częściej, co czyni je bardziej wartościowymi celami w fazie ustawień początkowych oraz podczas oceny potencjału produkcyjnego pozycji gracza. Pojęcie to będzie wykorzystywane w dalszej części pracy przy opisie strategii botów, które oceniają jakość lokalizacji budowy oraz prognozują przyszłą produkcję zasobów.

### 1.2.4. Zalety gry *Catan*

Jedną z najczęściej wskazywanych zalet gry Catan jest umiejętne połączenie relatywnie prostych zasad z wysokim poziomem satysfakcji oraz znaczną głębią strategiczną. W recenzji opublikowanej na łamach magazynu Pyramid podkreślono, że gra oferuje satysfakcjonujące doświadczenie rozwoju ekonomicznego i wymiany zasobów, przy jednocześnie umiarkowanym czasie rozgrywki, wynoszącym zazwyczaj od półtorej do dwóch godzin. Zwrócono uwagę, że poziom satysfakcji płynący z rozgrywki jest porównywalny z grami o znacznie dłuższym czasie trwania.

Drugim, bardzo istotnym atutem jest wysoka regrywalność: plansza składa się z heksów, a ich układ i przypisane numery produkcji można zmieniać między partiami, co ogranicza powtarzalność i utrudnia wyuczenie jednej „sztywnej” sekwencji optymalnych ruchów. Dzięki temu gra sprzyja analizie adaptacyjnych strategii oraz reagowania na bieżącą sytuację na planszy.

*Catan* jest uznanym tytułem o silnym wpływie kulturowym i branżowym: zdobył prestiżowe nagrody (m.in. Spiel des Jahres w 1995 oraz Game of the Century według Gamescom w 2015) i stał się jednym z symboli „nowoczesnych” gier planszowych.

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

Taka reprezentacja upraszcza interfejs pomiędzy botami a silnikiem gry — bot zawsze zwraca jedną wartość opisującą swoją decyzję, a silnik interpretuje ją zgodnie z typem akcji. Jednocześnie pozwala ona traktować akcje jako **dane**, które mogą być przechowywane, kopiowane oraz przekazywane pomiędzy komponentami systemu.

Zaprojektowany system akcji został **zainspirowany wzorcem projektowym *Command***. Zgodnie z jego założeniami, każda akcja stanowi samodzielny opis żądanej operacji, oddzielony od logiki jej wykonania. Odpowiedzialność za interpretację i realizację akcji spoczywa na silniku gry, który pełni rolę wykonawcy, natomiast boty oraz logika sterująca rozgrywką jedynie generują i przekazują akcje do wykonania. Takie rozdzielenie zmniejsza sprzężenie pomiędzy komponentami oraz ułatwia rozbudowę systemu o nowe typy działań.

Silnik gry implementuje mechanizm **apply / undo**, który obejmuje:

- zastosowanie akcji do bieżącego stanu gry,
- zapis minimalnej informacji potrzebnej do cofnięcia jej skutków,
- możliwość przywrócenia poprzedniego stanu gry.

W praktyce oznacza to, że każda akcja niesie ze sobą komplet informacji pozwalających nie tylko na jej wykonanie, lecz również na deterministyczne cofnięcie zmian, co jest charakterystyczną cechą implementacji wzorca *Command* z obsługą historii poleceń. Informacje potrzebne do cofania są zapisywane w sposób minimalny, co ogranicza narzut pamięciowy i czasowy.

Mechanizm cofania ruchów jest kluczowy dla:

- botów analizujących wiele wariantów przyszłych stanów gry,
- algorytmów przeszukiwania drzewa decyzji,
- testów jednostkowych oraz regresyjnych,
- symulacji deterministycznych i odtwarzania przebiegu rozgrywki.
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

## Modul do odtwarzania rozgrywek


## 5. Testowanie i weryfikacja poprawności
### 5.1. Strategia testowania
### 5.2. Testy jednostkowe i integracyjne
### 5.3. Walidacja zgodności z zasadami gry

## 6. Projekt i implementacja botów

Celem niniejszego rozdziału jest opis zaprojektowanych i zaimplementowanych graczy automatycznych (botów), które zostały wykorzystane do badań porównawczych w dalszej części pracy. Boty różnią się stopniem złożoności strategii decyzyjnej – od gracza w pełni losowego, pełniącego rolę punktu odniesienia, po boty heurystyczne rozwijane iteracyjnie poprzez stopniowe wzbogacanie funkcji oceny stanu gry.

Architektura botów została zaprojektowana w oparciu o wzorzec projektowy **Strategy**. Silnik gry współpracuje z botami poprzez wspólny interfejs gracza, natomiast konkretne implementacje strategii decyzyjnej są enkapsulowane w klasach poszczególnych botów. Umożliwia to wymienne stosowanie różnych algorytmów podejmowania decyzji bez konieczności modyfikacji logiki silnika, a także ułatwia prowadzenie eksperymentów porównawczych pomiędzy strategiami.


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

Proces projektowania botów heurystycznych przebiegał iteracyjnie. Każda kolejna wersja bota rozszerzała funkcję oceny o nowe elementy, obserwowane jako istotne podczas analizy rozgrywek poprzednich wersji. Takie podejście umożliwia analizę wpływu poszczególnych elementów strategii na skuteczność rozgrywki oraz pozwala na obserwację, w jakim stopniu nawet proste heurystyki poprawiają jakość decyzji względem losowego wyboru akcji.

#### it1 – heurystyka punktów zwycięstwa

Pierwsza wersja bota heurystycznego (it1) opiera się na najbardziej oczywistym kryterium: **maksymalizacji przyrostu punktów zwycięstwa**. Akcje prowadzące bezpośrednio do zdobycia punktów (budowa osady lub miasta) otrzymują najwyższą ocenę.

Strategia ta jest prosta, lecz krótkowzroczna – nie uwzględnia przyszłej produkcji zasobów ani kontroli przestrzeni na planszy.

#### it2 – zarządzanie zasobami i handel z bankiem

Druga iteracja bota wprowadza fundamentalną zmianę: **aktywne wykorzystanie handlu z bankiem** do optymalizacji ścieżki do zakupu struktur.

Bot wykrywa kontrolowane porty (2:1 specyficzne, 3:1 generyczne, 4:1 standardowy bank) i wykorzystuje je do przekształcania nadwyżek surowców w brakujące zasoby. Kluczowym mechanizmem jest ocena **osiągalności celu po uwzględnieniu handlu** poprzez obliczenie, ile surowców pozostanie do zdobycia nawet po optymalnej wymianie nadwyżek.

Bot wybiera cel zakupu nie na podstawie tego, co może kupić natychmiast, lecz **do czego jest najbliżej po serii transakcji**. Może świadomie zrezygnować z budowy drogi, jeśli po wymianie zasobów będzie w stanie zbudować osadę lub miasto. Handel jest rozważany przed akcjami niższego priorytetu (drogi, karty rozwoju), co pozwala systematycznie gromadzić zasoby do celów wysokowartościowych.

Hierarchia priorytetów:
```
Miasto/Osada (natychmiast) → Handel (w kierunku celu) → Droga → Karta rozwoju → Koniec tury
```

#### it3 – optymalizacja fazy początkowej

Trzecia iteracja bota wprowadza dwie istotne zmiany: **świadomą strategię ustawień początkowych** oraz mechanizm aktywnego ograniczania rozwoju przeciwnika poprzez wykorzystanie rozbójnika.

Ocena węzłów początkowych opiera się na **ważonej produkcji zasobów**, w której większe znaczenie przypisane jest surowcom kluczowym dla wczesnej ekspansji. Dodatkowo bot silnie premiuje **różnorodność zasobów**, unikając sytuacji, w których jedna osada produkuje głównie ten sam surowiec.

Drugie ustawienie początkowe dobierane jest w sposób komplementarny względem pierwszego. Bot analizuje, których surowców brakuje w początkowej konfiguracji i preferuje lokalizacje uzupełniające te deficyty, dążąc do możliwie pełnego pokrycia wszystkich typów zasobów.

Istotnym elementem strategii jest **świadome wykorzystanie rozbójnika jako narzędzia kontroli**. Bot wybiera heksy, których blokada powoduje maksymalne ograniczenie produkcji przeciwnika przy jednoczesnym minimalnym wpływie na własne zasoby. Unikane jest blokowanie pustych pól oraz własnych obszarów produkcyjnych.

W fazie głównej gry bot it3 zachowuje mechanizmy zarządzania zasobami wprowadzone w iteracji it2.

#### it4 – inteligentne użycie kart rozwoju i selekcja deterministyczna

Czwarta iteracja bota rozszerza wcześniejsze strategie o **zaawansowane zarządzanie kartami rozwoju** oraz wprowadza **deterministyczny wybór najlepszej akcji budowy**, zastępując losowe rozstrzyganie remisów stosowane w poprzednich wersjach.

Dla każdego typu karty rozwoju zaimplementowana została odrębna logika decyzyjna. Karty typu Rycerz wykorzystywane są przede wszystkim do ograniczania produkcji przeciwnika oraz budowania przewagi w rywalizacji o premię *Największa Armia*. Karty zapewniające dostęp do zasobów analizowane są pod kątem tego, czy umożliwiają realizację istotnych celów rozwojowych, takich jak budowa miasta lub osady. Karty punktów zwycięstwa traktowane są jako bezpośrednie wzmocnienie pozycji punktowej.

Istotnym elementem strategii jest uwzględnianie **ryzyka przed rzutem kośćmi**. Akcje prowadzące do znacznego zwiększenia liczby zasobów na ręce są oceniane ostrożnie, aby ograniczyć ryzyko strat w przypadku wyrzucenia liczby 7. Z kolei działania niewpływające na wielkość ręki uznawane są za bezpieczniejsze w tym kontekście.

W zakresie budowy bot dokonuje deterministycznego wyboru lokalizacji na podstawie **potencjału produkcyjnego** oraz użyteczności strategicznej. Preferowane są węzły zapewniające wysoką i elastyczną produkcję zasobów, szczególnie w środkowej fazie gry, natomiast budowa dróg oceniana jest pod kątem możliwości otwierania kolejnych opcji rozwoju.

Strategia it4 adaptuje swoje priorytety do fazy rozgrywki, stopniowo przesuwając nacisk z rozwoju infrastruktury na bezpośrednie zdobywanie punktów zwycięstwa w miarę zbliżania się do końca gry.

#### it5 – heurystyka zbalansowana ze symulacją pozycji

Piąta iteracja bota (it5) stanowi rozwinięcie podejścia heurystycznego poprzez wprowadzenie **lokalnej symulacji skutków akcji** (ang. *one-step lookahead*) oraz bardziej zbalansowanej oceny pozycji. W przeciwieństwie do wcześniejszych botów, które w dużej mierze opierały się na statycznych rankingach priorytetów lub prostych punktacjach, it5 podejmuje decyzję na podstawie **porównania jakości stanu gry przed i po wykonaniu rozważanej akcji**.

Podstawowym mechanizmem jest funkcja oceny pozycji `evaluate_position`, która zwraca skalarną wartość opisującą „siłę” aktualnego stanu z perspektywy bota. W każdej turze bot generuje listę akcji legalnych, a następnie dla każdej z nich wykonuje cykl:

- tymczasowe zastosowanie akcji do stanu gry (`applyAction`),
- obliczenie wartości oceny nowego stanu,
- cofnięcie akcji (`undoLastAction`),
- wybór akcji dającej najwyższy wynik.

W praktyce tworzy to prosty, lecz efektywny schemat selekcji: **akcje są porównywane nie po typie, lecz po realnym wpływie na stan gry**, co poprawia jakość decyzji szczególnie w sytuacjach, gdzie kilka ruchów ma podobny „priorytet” (np. alternatywne budowy dróg lub transakcje z bankiem).

Logika wyboru akcji ma strukturę wieloetapową:

1. **Natychmiastowa budowa struktur punktujących** – jeżeli możliwa jest budowa **miasta** lub **osady**, bot symuluje dostępne warianty i wybiera najlepszy wprost na podstawie oceny pozycji. Zapewnia to zgodność z nadrzędnym celem gry (zdobywanie punktów zwycięstwa), przy jednoczesnym wyborze wariantu maksymalizującego globalną jakość stanu.

2. **Ocena akcji wspierających rozwój** – w przypadku braku możliwości natychmiastowej budowy, bot analizuje deterministyczne akcje typu **budowa drogi** oraz **handel z bankiem**. Dodatkowo premiowane są działania, które **odblokowują w kolejnym kroku** możliwość budowy miasta lub osady (np. poprzez uzupełnienie brakujących zasobów).

3. **Zakup karty rozwoju jako heurystyka awaryjna** – zakup karty rozwoju nie jest symulowany ze względu na losowość talii, lecz oceniany heurystycznie w zależności od fazy gry oraz relacji punktów zwycięstwa bota do przeciwnika. Opcja ta jest preferowana jedynie w sytuacjach braku korzystnych akcji deterministycznych.

4. **Fallback do strategii it4** – jeżeli najlepszą ocenioną akcją okazuje się zakończenie tury bez poprawy sytuacji, bot powraca do logiki it4, aby uniknąć pasywnego stylu gry w stanach, w których wcześniejsze heurystyki potrafią znaleźć konstruktywne posunięcie.

Rozwinięty został również mechanizm **odrzucania kart przy wyrzuceniu 7**. it5 nie usuwa zasobów losowo, lecz najpierw identyfikuje najbardziej prawdopodobny cel rozwoju (miasto, osada, droga lub karta rozwoju), a następnie odrzuca te surowce, które są **najmniej istotne dla jego realizacji**. Zasoby krytyczne dla wybranego celu są chronione, natomiast odrzucane są przede wszystkim nadwyżki.

W rezultacie it5 łączy zalety wcześniejszych iteracji (deterministyczna selekcja oraz rozsądne priorytety) z większą świadomością konsekwencji podejmowanych decyzji wynikającą z lokalnej symulacji stanu. Strategia ta pozostaje relatywnie lekka obliczeniowo, a jednocześnie znacząco poprawia jakość decyzji w porównaniu do czysto regułowych heurystyk.

### 6.3. Bot wykorzystujący algorytm alpha-beta

Kolejnym graczem automatycznym jest bot wykorzystujący **algorytm przeszukiwania drzewa gry alpha-beta**. Jego celem jest podejmowanie decyzji na podstawie analizy przyszłych stanów gry, z uwzględnieniem możliwych odpowiedzi przeciwnika.

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

W trakcie przeszukiwania drzewa gry bot nie modeluje jawnie losowych rzutów kośćmi (co znacząco zwiększałoby współczynnik rozgałęzienia), jednak uwzględnia ich wpływ w sposób przybliżony poprzez **wartość oczekiwaną produkcji zasobów**. W tym celu dla każdej pozycji obliczana jest miara produkcji oparta o tzw. *pipsy* – liczbę oczek odpowiadającą prawdopodobieństwu wyrzucenia danego numeru heksu. Węzły (osady i miasta) oceniane są na podstawie sumy wartości `pips × waga_surowca` dla przyległych heksów, przy czym miasta otrzymują podwojony wkład produkcyjny. Taka agregacja stanowi aproksymację przewidywanej liczby otrzymanych zasobów w kolejnych turach i pozwala botowi preferować linie rozgrywki prowadzące do stabilniejszej oraz bardziej wartościowej ekonomicznie produkcji, bez konieczności explicite symulowania wszystkich możliwych wyników rzutów kośćmi.

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

### 6.4 Boty celujące w jedną strategię

### 6.4.1. Bot celujący w jeden zasób

Oprócz botów opisanych wcześniej zaimplementowano również dodatkowego gracza o wąsko wyspecjalizowanej strategii, nazwanego **OneResourcePlayer**. Jego założeniem jest maksymalizacja korzyści z jednego, wybranego surowca poprzez:

- wybór **priorytetowego surowca** na podstawie aktualnej konfiguracji planszy,
- zajęcia **portu 2:1** dla tego surowca już w ustawieniach początkowych,
- prowadzenie rozwoju infrastruktury (osady, miasta, drogi) w kierunku pól oraz portów związanych z tym surowcem,
- wykorzystanie nadprodukcji priorytetowego zasobu do handlu z bankiem po korzystnym kursie 2:1.

Celem tego gracza jest sprawdzenie hipotezy: **czy silna specjalizacja w jeden surowiec i wczesne pozyskanie portu 2:1 może stanowić skuteczną strategię w wariancie 1 vs 1**. Z tego względu bot ten należy traktować jako **bot eksperymentalny**.

#### Wybór priorytetowego surowca

Priorytetowy surowiec wybierany jest automatycznie na podstawie parametrów planszy. Dla każdego surowca obliczane są cechy opisujące jego „atrakcyjność":

- suma pipsów ze wszystkich heksów danego surowca (ogólna dostępność produkcji),
- najlepszy pojedynczy węzeł (ile pipsów danego surowca może generować jedna osada),
- najlepszy węzeł połączony z portem 2:1 dla tego surowca (kluczowe dla strategii),
- liczba heksów z danym surowcem (różnorodność źródeł).

Na tej podstawie konstruowany jest wynik punktowy z wagami: port 2:1 na silnym węźle (+60 za każdy pips), produkcja z najlepszego węzła (+40 za pips), łączna suma pipsów (+50 za pips) oraz liczba heksów (+10). Dodatkowo wprowadzono minimalne rozstrzyganie remisów na korzyść cegły i drewna (+20) z uwagi na ich rolę w budowie dróg.

#### Ustawienie początkowe: wymuszenie portu 2:1

Najważniejszym elementem strategii OneResourcePlayer jest faza początkowa. Bot stara się tak dobrać pierwszą osadę, aby:

1. znajdowała się na węźle z portem 2:1 odpowiadającym wybranemu surowcowi,
2. miała możliwie wysoką produkcję tego surowca (wysokie pipsy),
3. zapewniała sensowną możliwość rozwoju (premiowany jest stopień węzła, tj. liczba dostępnych krawędzi do dalszej rozbudowy dróg).

Jeżeli wśród legalnych akcji nie ma możliwości uzyskania portu 2:1 dla najlepszego surowca (np. z uwagi na ograniczenia legalnych ustawień), bot podejmuje próbę znalezienia **jakiegokolwiek** portu 2:1 dostępnego w danym układzie i dostosowuje wybór priorytetowego surowca. Dopiero w ostateczności wybiera pierwszą legalną akcję.

Drugie ustawienie początkowe również preferuje surowiec priorytetowy (wysokie pipsy, +60 za każdy pips), ale dodatkowo wprowadza heurystykę uzupełniania braków — premiowane są węzły dostarczające **innych** surowców, których bot nie miał w pierwszej osadzie (+10 za każdy nowy typ), aby ograniczyć ryzyko całkowitego zablokowania rozwoju.

#### Logika tury: specjalizacja i konwersja zasobów

W normalnej fazie gry bot rozważa wyłącznie akcje deterministyczne i stosuje następującą kolejność priorytetów:

1. **Budowa miast i osad** – silnie premiowana, szczególnie gdy zwiększa produkcję priorytetowego surowca lub znajduje się na porcie 2:1.

2. **Handel z bankiem** – kluczowy element strategii. Bot **wydaje nadwyżki priorytetowego surowca** (wykorzystując port 2:1) w zamian za zasoby potrzebne do budowy. Transakcje są wysoko oceniane gdy odblokowują możliwość zakupu miasta lub osady.

3. **Budowa drogi** – oceniana przez funkcję potencjału, która bada węzły w zasięgu 1–2 krawędzi i premiuje pipsy priorytetowego surowca w możliwych lokalizacjach osad oraz możliwości dalszej rozbudowy, ignorując miejsca naruszające regułę odległości.

Strategia ta jest celowo **jednostronna**: bot często poświęca równowagę zasobów na rzecz maksymalizacji jednego kanału ekonomicznego (produkcja + port 2:1 → konwersja → budowa).

#### Odrzucanie kart i mechanizm awaryjny

Bot implementuje własną politykę zrzucania kart przy przekroczeniu limitu, wybierając docelowy zakup i silnie chroniąc zarówno priorytetowy surowiec, jak i zasoby krytyczne dla najbliższego celu.

#### Mechanizm awaryjny: powrót do bota zbalansowanego

W sytuacji, gdy żaden z ruchów nie daje wyraźnej korzyści w ramach strategii monosurowcowej (wszystkie deterministyczne akcje mają gorsze oceny niż It5Player w analogicznej sytuacji), bot nie wykonuje losowych działań. Zamiast tego stosowany jest **fallback do It5Player**, czyli najbardziej zbalansowanego bota heurystycznego. Zapobiega to „utknięciu" strategii w stanach, w których dążenie do jednego surowca przestaje być racjonalne (np. brak portu 2:1, zablokowanie kluczowych lokalizacji przez przeciwnika).

### 6.4.2. Bot celujący w karty rozwoju

DevPlayer stanowi drugą strategię eksperymentalną, opartą na hipotezie, że **priorytetowe skupienie się na zakupie kart rozwoju** oraz szybkie osiąganie związanych z nimi bonusów (takich jak premia *Największa Armia* oraz karty punktów zwycięstwa) może w określonych warunkach stanowić skuteczną alternatywę dla klasycznej ekspansji terytorialnej.

#### Ustawienie początkowe i logika tury

W fazie ustawień początkowych bot preferuje lokalizacje zapewniające stabilną produkcję surowców niezbędnych do zakupu kart rozwoju, w szczególności rudę, zboże i wełnę, kluczowych dla zakupu kart rozwoju. Premiowana jest różnorodność zasobów oraz dostęp do portów ułatwiających ich wymianę (porty 3:1 i 2:1). Drugie ustawienie wybierane jest w sposób komplementarny, tak aby uzupełnić ewentualne braki w produkcji kluczowych surowców.

W fazie głównej strategia koncentruje się na zakupie kart rozwoju zawsze, gdy jest to możliwe. Jeżeli bezpośredni zakup nie jest dostępny, bot podejmuje działania przygotowawcze — w szczególności handel z bankiem lub ograniczoną rozbudowę infrastruktury — których celem jest umożliwienie zakupu karty w kolejnej turze. Odstępstwo od tej zasady występuje jedynie w sytuacjach, gdy dostępna jest bezpośrednia akcja prowadząca do zakończenia gry poprzez zdobycie brakujących punktów zwycięstwa.

W przypadku gdy strategia oparta na kartach rozwoju przestaje przynosić oczekiwane efekty (np. brak postępu punktowego lub ograniczone możliwości zakupu kart), bot przechodzi do bardziej zbalansowanej strategii heurystycznej, wykorzystując mechanizm awaryjny oparty na it5.

#### Polityka odrzucania i ograniczenia strategii

Przy konieczności odrzucenia kart w wyniku wyrzucenia liczby 7 bot w pierwszej kolejności chroni surowce kluczowe dla zakupu kart rozwoju, natomiast pozbywa się nadwyżek zasobów o mniejszym znaczeniu dla realizowanej strategii. Takie podejście ogranicza ryzyko utraty postępu w kierunku kolejnych zakupów.

Ze względu na silne uzależnienie od losowości talii kart rozwoju oraz rzutów kośćmi, strategia ta charakteryzuje się większą wariancją wyników w porównaniu do botów heurystycznych i algorytmu alpha-beta. W ramach pracy DevPlayer pełni rolę **strategii porównawczej**, umożliwiającej ocenę skuteczności podejścia opartego na rozwoju pośrednim i wysokiej nieprzewidywalności.

### 6.4.3. Bot celujący w najdłuższą drogę

RoadPlayer stanowi trzecią strategię eksperymentalną, której celem jest weryfikacja hipotezy, że **agresywna rozbudowa sieci dróg oraz zdobycie premii Najdłuższa Droga** (2 punkty zwycięstwa) może stanowić efektywną alternatywę dla klasycznego podejścia opartego na szybkim rozwoju osad i miast.

#### Ustawienie początkowe i logika tury

W fazie ustawień początkowych bot preferuje lokalizacje zapewniające wysoką i stabilną produkcję surowców niezbędnych do budowy dróg, w szczególności cegły i drewna. Premiowana jest również różnorodność zasobów, wysoki stopień węzłów (większa liczba możliwych kierunków ekspansji) oraz dostęp do portów ułatwiających wymianę surowców związanych z infrastrukturą drogową. Drugie ustawienie wybierane jest w sposób komplementarny, tak aby ograniczyć ryzyko niedoborów kluczowych zasobów.

W fazie głównej strategia konsekwentnie faworyzuje **budowę dróg** jako podstawową akcję rozwojową. Bot preferuje drogi, które wydłużają istniejącą sieć, zwiększają jej spójność oraz otwierają dostęp do kolejnych obszarów planszy. Jednocześnie uwzględniany jest potencjał węzłów dostępnych po rozbudowie, co pozwala unikać sytuacji, w których sieć dróg rozwija się kosztem całkowitej izolacji ekonomicznej.

Aby zachować minimalną równowagę strategiczną, RoadPlayer nie rezygnuje całkowicie z budowy osad i miast. Akcje te są podejmowane wtedy, gdy bezpośrednio prowadzą do zdobycia punktów zwycięstwa lub gdy umożliwiają dalszą, efektywną rozbudowę sieci dróg. Zakup kart rozwoju traktowany jest drugorzędnie i rozważany głównie w sytuacjach, gdy inne formy ekspansji są chwilowo niedostępne.

#### Polityka odrzucania i ograniczenia strategii

Przy konieczności odrzucania kart w wyniku wyrzucenia liczby 7 bot w pierwszej kolejności chroni surowce bezpośrednio związane z budową dróg, natomiast usuwa nadwyżki zasobów o mniejszym znaczeniu dla realizowanej strategii. W sytuacjach, w których żadna dostępna akcja nie prowadzi do poprawy pozycji, stosowany jest mechanizm awaryjny oparty na bardziej zbalansowanej strategii heurystycznej.

Strategia RoadPlayer jest **wrażliwa na ograniczenia przestrzenne planszy**. Skuteczne blokowanie kluczowych węzłów przez przeciwnika lub przerwanie ciągłości sieci znacząco obniża jej skuteczność. Ponadto jednostronna koncentracja na infrastrukturze drogowej może prowadzić do utraty tempa zdobywania punktów zwycięstwa, szczególnie w starciu z botami preferującymi rozwój ekonomiczny i budowę miast. Wyniki uzyskane przez RoadPlayer potwierdzają, że silna specjalizacja w jednym aspekcie gry nie gwarantuje przewagi nad strategiami zbalansowanymi.


## 6.5. Bot oparty na algorytmie genetycznym

## 7. Eksperymenty i analiza wyników

### 7.1. Metodologia porównania botów

Wszystkie eksperymenty zostały przeprowadzone zgodnie z następującym protokołem:

- **Liczba rozgrywek**: Każda para botów rozegrała **1000 rozgrywek**, co zapewnia statystyczną istotność wyników.

- **Eliminacja efektu miejsca**: W celu wyeliminowania wpływu kolejności ustawień początkowych (gracz rozpoczynający ma niewielką przewagę), zastosowano mechanizm **przełączania miejsc** (`--switch`). W każdej parze rozgrywek gracze zamieniają się miejscami, co zapewnia sprawiedliwe porównanie niezależne od pozycji startowej.

- **Losowość planszy**: Każda rozgrywka wykorzystuje losowo wygenerowaną planszę zgodnie z zasadami gry *Catan*, co eliminuje możliwość optymalizacji strategii pod konkretną konfigurację terenu.

### 7.1.1. Metryki oceny skuteczności

W celu kompleksowej oceny skuteczności botów wprowadzono następujące metryki:

**Metryki podstawowe:**
- **Procent wygranych rozgrywek**: Win Rate
- **Średnia liczba tur do zwycięstwa**: Avg Turns
- **Średnia liczba punktów przeciwnika przy przegranej**: LossVP

**Metryki dodatkowe**
- Procent nierozegranych rozgrywek: NW (No Winner) 
- Częstość zdobycia premii Najdłuższa Droga i Najwięcksza Armia: Longest road% i Largest army%
- Średnia liczba zakupionych kart rozwoju: Avg DevCards
- Średnia produkcja zasobów: ProdScore

### 7.2. Scenariusze testowe

### 7.2.1 Scenariusz 1: Skalowanie jakości botów

Celem pierwszego scenariusza eksperymentalnego było zbadanie, w jaki sposób wzrost złożoności i jakości strategii decyzyjnych botów wpływa na przebieg oraz wynik rozgrywek. W szczególności analizowano, czy kolejne iteracje botów heurystycznych prowadzą jedynie do stopniowej poprawy skuteczności, czy też występują jakościowe „progi”, po których charakter rozgrywki ulega istotnej zmianie.

#### Podsumowanie danych

| Bot | Win Rate vs Random | NW% | Avg Turns | LossVP (Random) | LossVP (Bot) | Longest road% | Largest army% | Avg DevCards | ProdScore |
|-----|-------------------|-----|-----------|-----------------|--------------|---------------|---------------|--------------|-----------|
| It1 | 56.0% | 7.6% | 448.5 | 6.81 | 7.51 | 51.2% | 58.9% | 3.0 | 730.4 |
| It2 | 73.3% | 3.6% | 381.0 | 6.23 | 8.48 | 64.4% | 62.0% | 3.0 | 864.3 |
| It3 | 98.7% | 0.3% | 180.3 | 3.98 | 11.50 | 83.7% | 91.9% | 3.2 | 1003.2 |
| It4 | 96.9% | 1.6% | 192.6 | 3.64 | 9.80 | 94.6% | 86.7% | 3.0 | 1190.3 |
| It5 | 100.0% | 0.0% | 115.3 | 3.21 | – | 71.5% | 98.5% | 3.7 | 1096.4 |
| Para | 99.5% | 0.0% | 125.1 | 3.34 | 4.80 | 90.2% | 25.9% | 9.9 | 1267.8 |
| ParaSettleIt5 | 100.0% | 0.0% | 111.0 | 3.21 | – | 68.3% | 98.8% | 3.7 | 1111.5 |
| OneResourcePlayer | 99.8% | 0.0% | 159.9 | 3.79 | 11.50 | 82.0% | 92.4% | 2.9 | 1117.0 |
| DevPlayer | 99.8% | 0.1% | 166.1 | 4.00 | 12.00 | 52.8% | 99.7% | 4.6 | 1064.8 |
| RoadPlayer | 100.0% | 0.0% | 132.7 | 3.22 | – | 94.5% | 98.6% | 3.6 | 1051.0 |

#### Kluczowe wnioski
- Nieliniowy charakter progresji jakości

Wzrost skuteczności botów heurystycznych nie ma charakteru liniowego. Iteracje It1 i It2 prowadzą do stopniowej poprawy współczynnika zwycięstw (56% -> 73%), natomiast It3 powoduje jakościowy skok skuteczności do 98.7%. Przekroczenie tego progu kompetencyjnego wynika z wprowadzenia strategii optymalnych ustawień początkowych, zapewniających lepsze pozycje startowe, oraz aktywnego wykorzystania rozbójnika do blokowania produkcji przeciwnika. Od tego momentu bot przejmuje kontrolę nad przebiegiem rozgrywki, a kolejne iteracje (It4, It5) stabilizują tę dominację, osiągając 96–100% zwycięstw.

1_progression.png

- Zależność jakości strategii od tempa gry

Wraz ze wzrostem jakości botów obserwowany jest istotny spadek średniej liczby tur. It1 rozgrywa partie trwające średnio 448 tur, It3 skraca je do około 180 tur, natomiast It5 do około 115 tur. Lepsze boty nie tylko wygrywają częściej, lecz także szybciej, co świadczy o większej efektywności decyzyjnej i zdolności do domykania gry.

- Metryka LossVP

Analiza średniej liczby punktów zwycięstwa przegranego gracza (LossVP) pokazuje istotne różnice jakościowe pomiędzy botami. Dla It1 i It2 gracz losowy przegrywa, osiągając ponad 6 VP, natomiast od It3 wartość ta spada do około 3–4 VP. Oznacza to, że boty wyższej jakości nie tylko wygrywają, lecz także skutecznie ograniczają rozwój przeciwnika. Wysokie wartości LossVP po stronie botów wyspecjalizowanych (Dev, OneResource) wskazują na większą wariancję ich strategii.

- Metryki dodatkowe

Metryki dodatkowe jednoznacznie wskazują na istnienie odmiennych strategii botów. Wraz ze wzrostem iteracji rośnie częstość zdobywania premii Najdłuższej Drogi oraz Największej Armii, co świadczy o coraz efektywniejszym wykorzystaniu kart rozwoju. Boty It1–It2 uzyskują Najdłuższą Drogę w około 50–64% gier, natomiast od It3 wartość ta wzrasta do 68–94%. RoadPlayer, zgodnie ze swoją specjalizacją, zdobywa tę premię w 94.5% rozgrywek.

Analogicznie, premia Największej Armii jest osiągana przez boty It1–It2 w około 59–62% gier, podczas gdy boty It3 i wyższe w ponad 86% przypadków. DevPlayer niemal zawsze uzyskuje tę premię (99.7%), co jest bezpośrednim efektem strategii intensywnego zakupu kart rozwoju.

Jako dodatkową informację warto zauważyć, że boty It1 i It2 kończą odpowiednio 7.6% i 3.6% rozgrywek bez zwycięzcy (timeout po 1000 turach). To wskazuje na niewystarczającą agresywność strategii. Od iteracji It3 zjawisko to praktycznie zanika (0–0.3%), co potwierdza, że boty wyższej jakości podejmują bardziej deterministyczne decyzje i konsekwentnie domykają rozgrywkę.

### Scenariusz 2: Porównanie iteracyjnych heurystyk

- It1Player vs It2Player
- It2Player vs It3Player
- It3Player vs It4Player
- It4Player vs It5Player
- It1Player vs It5Player (porównanie skokowe)

#### Scenariusz 3: OneResourcePlayer

- OneResourcePlayer vs
- OneResourcePlayer vs
- OneResourcePlayer vs
- OneResourcePlayer vs
- OneResourcePlayer vs
- OneResourcePlayer vs ParaSettleIt5Player
- OneResourcePlayer vs AlphaBetaPlayer

#### Scenariusz 4: DevPlayer

- DevPlayer vs
- DevPlayer vs
- DevPlayer vs
- DevPlayer vs
- DevPlayer vs
- DevPlayer vs ParaSettleIt5Player
- DevPlayer vs AlphaBetaPlayer

#### Scenariusz 5: RoadPlayer

- RoadPlayer vs
- RoadPlayer vs
- RoadPlayer vs
- RoadPlayer vs
- RoadPlayer vs
- RoadPlayer vs ParaSettleIt5Player
- RoadPlayer vs AlphaBetaPlayer

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
- Command - wzorzec projektowy https://refactoring.guru/design-patterns/command
- Strategy - wzorzec projektowy https://refactoring.guru/design-patterns/strategy