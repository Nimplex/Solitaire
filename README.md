# Pasjans Klondike

(całość dostępna również na githubie: https://github.com/Nimplex/Solitaire)

Konsolowa gra pasjans Klondike napisana w C++ z obsługą różnych poziomów trudności, systemem punktacji i funkcjami cofania ruchów.

## Wymagania systemowe

* System operacyjny: **Windows**
* Kompilator C++ obsługujący standard **C++20** lub nowszy (wymagana biblioteka `std::format`)
* Zainstalowany **CMake**
* Biblioteka `conio.h` (standardowo dostępna na Windows)
* **MinGW (GCC 13 lub nowszy)** – szczegóły poniżej

---

## Instalacja kompilatora (MinGW z obsługą C++20)

Aby korzystać z funkcji takich jak `std::format`, potrzebny jest nowoczesny kompilator C++. Zalecana metoda instalacji:

### Krok 1: Zainstaluj MSYS2

1. Pobierz MSYS2 ze strony: [https://www.msys2.org](https://www.msys2.org)
2. Zainstaluj do folderu, np. `C:\msys64`
3. Uruchom `MSYS2 MSYS` z menu Start

### Krok 2: Zaktualizuj system

```bash
pacman -Syu
# Po aktualizacji zamknij terminal i otwórz ponownie:
pacman -Su
```

### Krok 3: Zainstaluj GCC i CMake

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-make
```

### Krok 4: Dodaj MinGW do zmiennej PATH

Dodaj do zmiennej środowiskowej `PATH`:

```
C:\msys64\ucrt64\bin
```

Dzięki temu polecenia `g++`, `cmake` i `make` będą dostępne w terminalu systemowym (CMD / PowerShell).

---

## Sposób uruchomienia projektu

(w razie problemów zostawiłem skompilowaną wersję w folderze)

### Krok 1: Konfiguracja CMake

W katalogu z projektem:

```bash
cmake -B build -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-static -static-libgcc -static-libstdc++"
```

### Krok 2: Kompilacja

```bash
cmake --build build -- -j
```

### Krok 3: Uruchomienie

```bash
./build/Solitaire.exe
```

---

## Instrukcje rozgrywki

### Cel gry

Celem gry jest przeniesienie wszystkich kart na stosy fundacyjne (górne 4 stosy), układając je w kolejności od Asa do Króla według kolorów.

### Ekran startowy

1. **Wybór poziomu trudności**: Użyj strzałek `←` `→` aby wybrać poziom trudności, zatwierdź `Enter`

   * **Łatwy**: Dobieranie po 1 karcie ze stosu
   * **Ciężki**: Dobieranie po 3 karty ze stosu

2. **Wprowadzenie imienia**: Wpisz swoje imię i zatwierdź `Enter`

### Sterowanie w grze

#### Wybór stosów

* **\[Q]** - Stos główny (stock) - dobieranie kart
* **\[W]** - Stos odrzuconych (waste)
* **\[E]** - Pierwszy stos fundacyjny
* **\[R]** - Drugi stos fundacyjny
* **\[T]** - Trzeci stos fundacyjny
* **\[Y]** - Czwarty stos fundacyjny
* **\[1-7]** - Stosy tableau (główne stosy do układania)

#### Poruszanie się

* **Strzałki `←` `→`** - Wybór karty w stosie (gdy dostępnych jest więcej kart)
* **Enter** - Zatwierdzenie wyboru karty
* **\[Q]** - Anulowanie aktualnego ruchu (podczas wyboru karty lub celu)

#### Dodatkowe funkcje

* **\[U]** - Cofnij ostatni ruch (maksymalnie 3 ruchy wstecz)
* **\[P]** - Restart gry (rozpoczęcie od nowa)

### Ekran wyników

Po ukończeniu gry zobaczysz tabelę najlepszych wyników pokazującą:

* Pozycję w rankingu
* Imię gracza
* Liczbę wykonanych ruchów

Najlepsze wyniki są zapisywane w pliku `scores.txt`.
