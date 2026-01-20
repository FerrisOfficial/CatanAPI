# Wyjaśnienie pojęć: CI (Przedział Ufności) i Przepaść Kompetencyjna

## CI - Przedział Ufności (Confidence Interval)

### Co to jest?

**Przedział ufności 95% (95% CI)** to zakres wartości, w którym z 95% pewnością znajduje się prawdziwa wartość parametru (np. współczynnik zwycięstw).

### Przykład:

Jeśli It3 wygrał 987 z 1000 rozgrywek (98.7%), to:
- **Obserwowany wynik**: 98.7%
- **95% CI**: np. [97.5%, 99.5%]

To oznacza, że jeśli powtórzymy eksperyment wiele razy, w 95% przypadków prawdziwy współczynnik zwycięstw It3 będzie między 97.5% a 99.5%.

### Dlaczego to ważne?

1. **Pokazuje niepewność**: Nawet przy 1000 rozgrywkach wynik nie jest "pewny" - jest oszacowaniem
2. **Pozwala porównywać**: Jeśli przedziały ufności dwóch botów się nie nakładają, różnica jest statystycznie istotna
3. **Wiarygodność**: Węższy przedział = większa pewność wyniku

### Jak interpretować na wykresie?

- **Zielony obszar** wokół linii = przedział ufności
- **Wąski obszar** = wysoka pewność (np. It5 z 100% win rate)
- **Szeroki obszar** = większa niepewność (rzadziej występuje przy dużych próbkach)

### Wzór (Wilson score interval):

Dla próbki n=1000 i poziomu ufności 95% (z=1.96):
```
CI = (p̂ + z²/(2n) ± z·√(p̂(1-p̂)/n + z²/(4n²))) / (1 + z²/n)
```

## Przepaść Kompetencyjna (Competence Gap)

### Co to jest?

**Przepaść kompetencyjna** to moment, w którym wprowadzenie nowych mechanizmów powoduje **dramatyczny skok** w skuteczności, tworząc wyraźną granicę między "słabszymi" a "silniejszymi" botami.

### W kontekście Twoich wyników:

**Przepaść występuje między It2 a It3:**

- **It1-It2**: RandomPlayer ma jeszcze realne szanse
  - It1: Random wygrywa 36.4% rozgrywek
  - It2: Random wygrywa 23.1% rozgrywek
  
- **It3+**: RandomPlayer praktycznie nie ma szans
  - It3: Random wygrywa tylko 1.0% rozgrywek
  - It4+: Random wygrywa <2% rozgrywek

### Co powoduje przepaść?

**It3 wprowadza kluczowe mechanizmy:**

1. **Strategia ustawień początkowych**
   - It1-It2: losowe lub proste ustawienia
   - It3+: świadomy wybór najlepszych pozycji startowych

2. **Aktywne wykorzystanie rozbójnika**
   - It1-It2: rozbójnik używany przypadkowo
   - It3+: strategiczne blokowanie produkcji przeciwnika

3. **Lepsze zarządzanie zasobami**
   - It3+: bardziej zaawansowana heurystyka handlu i budowy

### Dlaczego to "przepaść"?

- **Przed przepaścią**: Losowość może jeszcze pomóc RandomPlayerowi
- **Po przepaści**: Losowość nie wystarczy - potrzebna jest strategia

To jak różnica między:
- **Amator vs Amator** (It1-It2 vs Random) - losowość może decydować
- **Profesjonalista vs Amator** (It3+ vs Random) - strategia zawsze wygrywa

### Jak to pokazać na wykresie?

1. **Czerwona linia przerywana** między It2 a It3 - wizualnie oddziela "słabsze" od "silniejszych"
2. **Adnotacja** wyjaśniająca, co się zmieniło (strategia ustawień + rozbójnik)
3. **Wykres pokazujący spadek szans Random** - dramatyczny spadek między It2 a It3

### Znaczenie dla pracy:

- **Pokazuje wartość iteracyjnego podejścia**: Każda iteracja dodaje coś ważnego
- **Identyfikuje kluczowe mechanizmy**: Strategia ustawień i rozbójnik są najważniejsze
- **Uzasadnia rozwój**: Pokazuje, że It3 nie jest tylko "trochę lepszy" - to fundamentalna zmiana

## Podsumowanie

- **CI (95%)**: Pokazuje zakres niepewności wyniku - wąski = pewny wynik
- **Przepaść kompetencyjna**: Moment przełomu, gdy boty stają się "profesjonalne" - losowość przestaje wystarczać

Oba pojęcia pomagają zrozumieć nie tylko **czy** boty są lepsze, ale też **jak bardzo** i **kiedy** następuje przełom.
