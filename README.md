# Dokumentation zum Tastenfeld-Projekt

## 1. Programmübersicht

Das Programm steuert eine 4x3-Tastenmatrix und zeigt die gedrückten Werte auf einer LED-Anzeige in binärer Form an. Es verwendet die folgenden Hardware-Komponenten:

- ATmega32P Mikrocontroller mit 20 MHz Takt
- 4x3 Tastaturmatrix (verbunden mit PB0-PB3 für Zeilen und PD2-PD4 für Spalten)
- LED-Anzeige (verbunden mit PC0-PC3)

## 2. Code-Analyse

```c
// CPU-Taktfrequenz für delay-Funktionen
#define F_CPU 20000000UL  
#include <avr/io.h>
#include <util/delay.h>

// Tastenwerte für 4x3 Matrix
const uint8_t tastaturlayout[4][3] = {
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9},
    {0xA, 0, 0xB}
};
```

- `F_CPU` definiert die CPU-Taktfrequenz auf 20 MHz, was für die Verzögerungsfunktionen wichtig ist
- Die `tastaturlayout` 2D-Array definiert die Werte für jede Taste in der 4x3-Matrix

### 2.1 Tastatur-Scan-Funktion

```c
uint8_t scan_tastatur(void) {
    for (uint8_t reihe = 0; reihe < 4; reihe++) {
        // Alle Zeilen (PB0-PB3) auf HIGH setzen
        PORTB = 0x0F;
        // Aktuelle Zeile auf LOW setzen
        PORTB &= ~(1 << reihe);
  
        _delay_us(5);
        
        // Spaltenstatus lesen (PD2-PD4)
        uint8_t spalten = PIND & 0x1C;
        for (uint8_t spalte = 0; spalte < 3; spalte++) {
            // Spalten sind PD2-PD4
            uint8_t pin_spalte = spalte + 2;
            
            // Wenn Spalte LOW ist (Taste gedrückt)
            if (!(spalten & (1 << pin_spalte))) {
                _delay_ms(10);
				
                if (!(PIND & (1 << pin_spalte))) {
                    // Warten bis Taste losgelassen wird
                    while (!(PIND & (1 << pin_spalte))) {
                        _delay_ms(10);
                    }
                    // Tastenwert zurückgeben
                    return tastaturlayout[reihe][spalte];
                }
            }
        }
    }
    
    // Keine Taste gedrückt
    return 0xFF;
}
```

Die Funktion `scan_tastatur()` durchsucht die Matrix nach gedrückten Tasten:

1. Durchläuft nacheinander alle vier Zeilen
2. Setzt alle Zeilenpins auf HIGH mit dem Befehl `PORTB |= 0x0F;`
3. Setzt nur die aktuelle Zeile auf LOW mit `PORTB &= ~(1 << row);`
4. Prüft, ob eine der Spalten auf LOW gezogen wurde, was einen Tastendruck anzeigt
5. Bei erkanntem Tastendruck:
   - Erneute Prüfung nach 10ms
   - Warten auf Tastenloslassen
   - Rückgabe des entsprechenden Werts aus dem tastaturlayout

### 2.2 Anzeige-Funktion

```c
void display(uint8_t wert) {
    // Untere 4 Bits von wert auf PC0-PC3 setzen, obere Bits von PORTC unverändert lassen
    PORTC = (PORTC & 0xF0) | (wert & 0x0F);
}
```

Die Funktion `display_()` zeigt einen Wert auf den LEDs an:
- Bewahrt den Zustand der oberen 4 Bits von PORTC mit `(PORTC & 0xF0)`
- Isoliert die unteren 4 Bits des Werts mit `(value & 0x0F)`
- Kombiniert beide mit OR, sodass nur die unteren 4 Bits (PC0-PC3) aktualisiert werden

### 2.3 Hauptprogramm

```c
int main(void) {
    // Port-Konfiguration
    // PB0-PB3 als Ausgänge (Zeilen)
    DDRB = 0x0F;
    // PC0-PC3 als Ausgänge (LEDs)
    DDRC = 0x0F;
    
    // PD2-PD4 als Eingänge (Spalten)
    DDRD &= ~0x1C;
    // Pull-ups für Spalten aktivieren
    PORTD |= 0x1C;
    
    // Aktueller Tastenwert
    uint8_t taste = 0xFF;
    // Letzter Tastenwert für Wiederholungserkennung
    uint8_t temp = 0xFF;
    
    // Hauptschleife
    while (1) {
        // Keypad scannen
        taste = scan_tastatur();
        
        // Wenn neue Taste gedrückt
        if (taste != 0xFF && taste != temp) {
            // Wert auf LEDs anzeigen
            display(taste);
            // Tastenwert speichern
            temp = taste;
            
            _delay_ms(100);
            // Zurücksetzen für erneute Erkennung
            temp = 0xFF;
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
     PORTB |= 0x0F;  // Alle Zeilen auf HIGH
     PORTB &= ~(1 << 2);  // Nur Zeile 3 (PB2) auf LOW
     ```
   - PB2 ist jetzt LOW, PB0, PB1 und PB3 sind HIGH

3. **Erkennung des Tastendrucks**:
   - Die gedrückte Taste 8 verbindet Zeile 3 (PB2) mit Spalte 2 (PD3)
   - Da PB2 LOW ist, wird auch PD3 auf LOW gezogen
   - Bei der Überprüfung:
     ```c
     cols = PIND & 0x1C;  // Liest Spaltenzustand
     // Bei Spalte 2 (col=1, col_pin=3):
     if (!(cols & (1 << 3))) {  // Prüft, ob PD3 LOW ist
     ```
   - Die Bedingung ist erfüllt, da PD3 jetzt LOW ist

4. **Entprellung und Bestätigung**:
   - Nach 10ms wird erneut geprüft, ob PD3 noch LOW ist
   - Wenn ja, wartet die Funktion, bis die Taste losgelassen wird
   - Danach gibt sie den Wert 8 zurück

5. **Anzeige des Werts**:
   - In der Hauptschleife wird der neue Tastendruck erkannt
   - `display(8)` wird aufgerufen:
     ```c
     PORTC = (PORTC & 0xF0) | (8 & 0x0F);
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
   - Nach 100ms wird `temp` auf 0xFF zurückgesetzt
   - Die Taste 8 kann nun erneut erkannt werden, wenn sie losgelassen und wieder gedrückt wird
