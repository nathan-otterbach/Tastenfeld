# Dokumentation zum Tastenfeld-Projekt

## 1. Programmübersicht

Das Programm steuert eine 4x3-Tastenmatrix und zeigt die gedrückten Werte auf einer LED-Anzeige in binärer Form an. Es verwendet die folgenden Hardware-Komponenten:

- ATmega32P Mikrocontroller mit 20 MHz Takt
- 4x3 Tastaturmatrix (verbunden mit PB0-PB3 für Zeilen und PD2-PD4 für Spalten)
- LED-Anzeige (verbunden mit PC0-PC3)

## 2. Code-Analyse

```c
// Definition der CPU-Taktfrequenz für delay-Funktionen (20 MHz)
#define F_CPU 20000000UL  

// Einbindung von AVR-spezifischen Header-Dateien
#include <avr/io.h>
#include <util/delay.h>

// Definition der Tastenbelegung eines 4x3-Keypads (Zeilen x Spalten)
// 0xA = Taste '*', 0xC = Taste '#', 0 wird in der Mitte unten angenommen
const uint8_t tastaturlayout[4][3] = {
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9},
    {0xA, 0, 0xC}
};
```

- `F_CPU` definiert die CPU-Taktfrequenz auf 20 MHz, was für die Verzögerungsfunktionen wichtig ist
- Die `tastaturlayout` 2D-Array definiert die Werte für jede Taste in der 4x3-Matrix

### 2.1 Tastatur-Scan-Funktion

```c
/**
 * Tastaturmatrix scannen
 * 
 * Diese Funktion überprüft nacheinander jede Zeile des Keypads, indem sie 
 * jeweils eine Zeile auf LOW setzt (aktive Zeile) und die anderen auf HIGH.
 * Anschließend wird geprüft, ob in einer der drei Spalten ein LOW-Pegel 
 * anliegt, was auf eine gedrückte Taste hinweist.
 * 
 * Rückgabe:
 *  - Der erkannte Tastenwert laut `tastaturlayout`
 *  - 0xFF, falls keine Taste gedrückt ist
 */
uint8_t scan_tastatur(void) {
    for (uint8_t zeile = 0; zeile < 4; zeile++) {
		
        // Setze alle Zeilen-Pins (PB0–PB3) auf HIGH (inaktiv)
        PORTB = 0x0F;

        // Aktive Zeile auf LOW setzen, um diese zu "selektieren"
        PORTB &= ~(1 << zeile);

        // Kurze Verzögerung kann nötig sein (auskommentiert)
        //_delay_us(1); 
		
        // Durchlaufe alle drei Spalten (PD2–PD4)
        for (uint8_t spalte = 0; spalte < 3; spalte++) {
            uint8_t pin_spalte = spalte + 2;

            // Wenn eine Taste gedrückt ist (Spaltenpin auf LOW)
            if (!(PIND & (1 << pin_spalte))) {
                // Gib den Wert aus dem Layout zurück
                return tastaturlayout[zeile][spalte];
            }
        }
    }

    // Keine Taste erkannt
    return 0xFF;
}
```

Die Funktion `scan_tastatur()` durchsucht die Matrix nach gedrückten Tasten:

1. Durchläuft nacheinander alle vier Zeilen
2. Setzt alle Zeilenpins auf HIGH mit dem Befehl `PORTB = 0x0F;`
3. Setzt nur die aktuelle Zeile auf LOW mit `PORTB &= ~(1 << zeile);`
4. Prüft, ob eine der Spalten auf LOW gezogen wurde, was einen Tastendruck anzeigt
5. Bei erkanntem Tastendruck:
   - Rückgabe des entsprechenden Werts aus dem tastaturlayout

### 2.2 Anzeige-Funktion

```c
/**
 * Anzeige auf LED-Ausgängen
 * 
 * Gibt die unteren 4 Bits eines Wertes auf Port C aus (PC0–PC3).
 * Dadurch lassen sich z.B. gedrückte Tasten binär auf LEDs darstellen.
 *
 * Parameter:
 *  - wert: Ein 8-Bit-Wert, von dem die unteren 4 Bits dargestellt werden
 */
void display(uint8_t wert) {
    // Nur PC0–PC3 sind als Ausgänge gesetzt; höherwertige Bits werden ignoriert
    PORTC = wert;
}
```

Die Funktion `display_()` zeigt einen Wert auf den LEDs an:
- Bewahrt den Zustand der oberen 4 Bits von PORTC mit `(PORTC & 0xF0)`
- Isoliert die unteren 4 Bits des Werts mit `(value & 0x0F)`
- Kombiniert beide mit OR, sodass nur die unteren 4 Bits (PC0-PC3) aktualisiert werden

### 2.3 Hauptprogramm

```c
/**
 * Hauptfunktion
 * 
 * Initialisiert die I/O-Ports:
 *  - PB0–PB3 (Zeilen) als Ausgang
 *  - PD2–PD4 (Spalten) als Eingang mit Pull-up
 *  - PC0–PC3 (LED-Ausgabe) als Ausgang
 * 
 * In der Hauptschleife wird die Tastatur kontinuierlich gescannt.
 * Wird eine Taste erkannt, so wird ihr Wert auf den LEDs dargestellt.
 */
int main(void) {
    // Initialisiere Port B: PB0–PB3 als Ausgang für Zeilenansteuerung
    DDRB = 0x0F;

    // Initialisiere Port C: PC0–PC3 als Ausgang für LED-Anzeige
    DDRC = 0x0F;

    // Initialisiere Port D: PD2–PD4 als Eingang für Spaltenabfrage
    DDRD = 0x00;

    // Aktiviere interne Pull-up-Widerstände an den Spalteneingängen
    PORTD = 0x1C;

    // Variable zum Speichern des aktuellen Tastencodes
    uint8_t taste = 0xFF;

    // Endlosschleife
    while (1) {
        // Scanne Tastatur
        taste = scan_tastatur();

        // Wenn eine Taste erkannt wurde
        if (taste != 0xFF) {
            // Gib den Tastenwert auf den LEDs aus
            display(taste);
        }
    }

    return 0;
}
```

Die `main()`-Funktion:
1. Konfiguriert die Hardware:
   - Zeilenpins als Ausgänge mit Pull-ups
   - Spaltenpins als Eingänge mit Pull-ups
   - Display-Pins als Ausgänge, initial alle LEDs aus
2. Führt eine Endlosschleife aus, die:
   - Kontinuierlich die Tastatur scannt
   - Neue Tastendrücke erkennt
   - Den Wert der gedrückten Taste auf den LEDs anzeigt

## 3. Beispiel: Drücken der Taste 8

Wenn die Taste 8 gedrückt wird, läuft folgender Prozess ab:

1. **Physische Position der Taste 8**:
   - Befindet sich in der 3. Zeile (Index 2) und 2. Spalte (Index 1) der Matrix
   - Entspricht dem Wert 8 laut `tastaturlayout[2][1]`

2. **Scannen der Tastatur**:
   - Die Funktion `scan_tastatur()` durchläuft die Zeilen 0-3
   - Bei Zeile 2 (3. Zeile):
     ```c
     // Setze alle Zeilen-Pins (PB0–PB3) auf HIGH (inaktiv)
     PORTB = 0x0F;

     // Aktive Zeile auf LOW setzen, um diese zu "selektieren"
     PORTB &= ~(1 << 2);
     ```
   - PB2 ist jetzt LOW, PB0, PB1 und PB3 sind HIGH

3. **Erkennung des Tastendrucks**:
   - Die gedrückte Taste 8 verbindet Zeile 3 (PB2) mit Spalte 2 (PD3)
   - Da PB2 LOW ist, wird auch PD3 auf LOW gezogen
   - Bei der Überprüfung:
     ```c
        // Durchlaufe alle drei Spalten (PD2–PD4)
        for (uint8_t spalte = 0; spalte < 3; spalte++) {
            uint8_t pin_spalte = 1 + 2;

            // Wenn eine Taste gedrückt ist (Spaltenpin auf LOW)
            if (!(PIND & (1 << 3))) {
                // Gib den Wert aus dem Layout zurück
                return tastaturlayout[2][1];
            }
        }
     ```
   - Die Bedingung ist erfüllt, da PD3 jetzt LOW ist

4. **Anzeige des Werts**:
   - In der Hauptschleife wird der neue Tastendruck erkannt
   - `display(8)` wird aufgerufen:
     ```c
     // Nur PC0–PC3 sind als Ausgänge gesetzt; höherwertige Bits werden ignoriert
     PORTC = wert;
     ```
   - 8 in Binär ist `1000`
   - Die LEDs werden entsprechend gesetzt:
     - PC3 = 1 (HIGH)
     - PC2 = 0 (LOW)
     - PC1 = 0 (LOW)
     - PC0 = 0 (LOW)
   - Dies zeigt den Wert 8 in Binärform auf den LEDs an

6. **Vorbereitung für den nächsten Tastendruck**:
   - `temp` wird auf 8 gesetzt
   - `temp` wird auf 0xFF zurückgesetzt
   - Die Taste 8 kann nun erneut erkannt werden, wenn sie losgelassen und wieder gedrückt wird
