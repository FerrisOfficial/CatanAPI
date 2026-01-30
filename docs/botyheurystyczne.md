### 7.2. Boty heurystyczne

Boty heurystyczne rozwijane były iteracyjnie: każda kolejna wersja rozszerza poprzednią o nowe elementy strategii. Dzięki temu można ocenić wpływ poszczególnych mechanizmów na skuteczność oraz obserwować, jak nawet proste heurystyki poprawiają decyzje względem losowego wyboru akcji.

#### it1 – priorytet punktów zwycięstwa

It1 wybiera akcje według **stałej hierarchii priorytetów**: najpierw budowa miasta, potem osady, drogi, karty rozwoju, na końcu handel i inne. W obrębie każdej kategorii wybór jest **losowy**. Bot nie handluje z bankiem, nie optymalizuje ustawień początkowych ani rozbójnika. Strategia jest prosta i krótkowzroczna – maksymalizuje natychmiastowy przyrost punktów, nie uwzględniając przyszłej produkcji ani pozycji na planszy.

#### it2 – handel z bankiem i cel zakupu

It2 rozszerza it1 o **aktywne wykorzystanie handlu z bankiem**. Wykrywa kontrolowane porty (2:1, 3:1, 4:1) i ocenia, **do którego celu zakupu jest najbliżej po uwzględnieniu możliwych wymian** – tzn. ile brakujących surowców pozostaje po optymalnym „wydaniu” nadwyżek. Cel wybierany jest nie na podstawie tego, co da się kupić od razu, lecz tego, do czego po handlu jest się najbliżej. Bot może odłożyć budowę drogi i najpierw handlować, jeśli po wymianie będzie mógł zbudować osadę lub miasto. Hierarchia: miasto i osada (natychmiast) → handel w kierunku wybranego celu → droga → karta rozwoju → koniec tury.

#### it3 – ustawienia początkowe i rozbójnik

It3 dodaje **świadomą fazę początkową** oraz **celowe ustawianie rozbójnika**. Ustawienia początkowe oceniane są według produkcji (z naciskiem na surowce ważne we wczesnej grze), różnorodności zasobów i dostępu do portów. Drugie ustawienie dobierane jest tak, aby **uzupełniać braki** po pierwszym – bot dąży do pokrycia wszystkich typów surowców. Przy ruchu rozbójnikiem bot wybiera heks, który **maksymalnie ogranicza produkcję przeciwnika** przy **minimalnym wpływie na własną**. W fazie głównej zachowuje logikę it2 (handel i cele zakupu).

#### it4 – karty rozwoju i deterministyczny wybór budowy

It4 wprowadza **inteligentne używanie kart rozwoju** oraz **deterministyczny wybór lokalizacji** zamiast losowego rozstrzygania remisów. Dla każdego typu karty (Rycerz, Monopol, Rozwój itd.) stosowana jest odrębna logika: Rycerze – blokada przeciwnika i premia Największa Armia, karty zasobowe – pod kątem odblokowania budowy miasta lub osady, karty VP – jako bezpośredni zysk punktowy. Uwzględniane jest **ryzyko przed rzutem** (7): unika się akcji, które mocno powiększają rękę przed rzutem. Budowa (miasto, osada, droga) wybierana jest deterministycznie na podstawie **potencjału produkcyjnego** i użyteczności sieci dróg; priorytety dostosowują się do fazy gry.

#### it5 – ocena pozycji i jednokrokowy lookahead

It5 uzupełnia it4 o **jednokrokową symulację** (*one-step lookahead*) i **zunifikowaną ocenę pozycji**. Zamiast sztywnych priorytetów bot dla każdej rozważanej akcji deterministycznej: stosuje ją do kopii stanu, liczy wartość funkcji oceny pozycji, cofa akcję – i wybiera akcję dającą **najwyższą ocenę**. Decyzja opiera się więc na **rzeczywistym wpływie na stan gry**, a nie na samym typie akcji. Najpierw rozważane są budowy miasta i osady (wybór najlepszej lokalizacji), potem drogi i handel; dodatkowo premiowane są ruchy, które **odblokowują** w następnym kroku budowę miasta lub osady.

Zakup karty rozwoju nie jest symulowany (losowość talii) i oceniany heurystycznie; gdy najlepszą opcją jest zakończenie tury bez poprawy, bot **zawraca do logiki it4**. Przy wyrzuceniu 7 it5 **nie odrzuca losowo**: ustala najbliższy cel (miasto, osada, droga, karta), po czym odrzuca surowce najmniej potrzebne do tego celu, chroniąc zasoby krytyczne. It5 łączy deterministyczny wybór i sensowne priorytety z oceną konsekwencji ruchu przy niewielkim koszcie obliczeniowym.