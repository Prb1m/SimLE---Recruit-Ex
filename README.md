# Zadanie Rekrutacyjne

Klasa "czarnej skrzynki" na zadanie rekrutacyjne koła naukowego SimLE

##  Instalacja

```bash
git clone https://github.com/username/repository.git
cd repository
```

## Uruchomienie

```bash
g++ -std=c++17 main.cpp TelemetryBlackBox.cpp -o program
./program
```

## Funkcje

dodatkowe funkcje:
- calcXORsum - prywatna funkcja do liczenia check sum, powtorzona w mainie do testow
- printBinary - wypisywanie binarnego przedstawienia danej ramki
- recordWithBadXor - funkcja symulująca złe wpisanie check sum

## Pliki

- TelemetryBlackBox.cpp - plik klasy czarnej skrzynki
- TelemetryBlackBox.h   - plik nagłówkowy klasy czarnej skrzynki
- main.cpp - plik zawierający testy oraz wywołanie testów w pętli

Pliku pdf zadania rekrutacyjnego, aby uszanować prawa autorskie.