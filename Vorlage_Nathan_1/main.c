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

/**
 * Scannt die Tastatur nach gedrückten Tasten
 * 
 * Funktion: Durchläuft alle Zeilen/Spalten und erkennt gedrückte Tasten
 * Parameter: keine
 * Rückgabe: uint8_t - Tastenwert aus tastaturlayout oder 0xFF wenn keine Taste gedrückt
 */
uint8_t scan_tastatur(void) {
    for (uint8_t zeile = 0; zeile < 4; zeile++) {
        // Alle Zeilen (PB0-PB3) auf HIGH setzen
        PORTB = 0x0F;
		
        // Aktuelle Zeile auf LOW setzen
        PORTB &= ~(1 << zeile);
        _delay_us(1);   
		  
        // Spaltenstatus lesen (PD2-PD4)
        uint8_t spalten = PIND & 0x1C;
        for (uint8_t spalte = 0; spalte < 3; spalte++) {
			
            // Spalten sind PD2-PD4
            uint8_t pin_spalte = spalte + 2;   
			  
            // Wenn Spalte LOW ist (Taste gedrückt)
            if (!(spalten & (1 << pin_spalte))) {			
                if (!(PIND & (1 << pin_spalte))) {
					
                    // Tastenwert zurückgeben
                    return tastaturlayout[zeile][spalte];
                }
            }
        }
    }
    
    // Keine Taste gedrückt
    return 0xFF;
}

/**
 * Zeigt einen Wert binär auf den LEDs an
 * 
 * Funktion: Stellt die unteren 4 Bits des Werts auf PC0-PC3 dar
 * Parameter: uint8_t wert - Anzuzeigender Wert
 * Rückgabe: keine
 */
void display(uint8_t wert) {
    // Untere 4 Bits von wert auf PC0-PC3 setzen, obere Bits von PORTC unverändert lassen
    PORTC = (PORTC & 0xF0) | (wert & 0x0F);
}

/**
 * Hauptprogramm
 */
int main(void) {
    // Port-Konfiguration
    // PB0-PB3 als Ausgänge (Zeilen)
    DDRB = 0x0F;
	
    // PC0-PC3 als Ausgänge (LEDs)
    DDRC = 0x0F;
    
    // PD2-PD4 als Eingänge (Spalten)
    DDRD = 0x00;
	
    // Pull-ups für Spalten aktivieren
    PORTD = 0xFF;
    
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
			
            // Zurücksetzen für erneute Erkennung
            temp = 0xFF;
        }
    }
    
    return 0;
}