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