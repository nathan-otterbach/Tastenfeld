// CPU-Taktfrequenz f�r delay-Funktionen
#define F_CPU 20000000UL  
#include <avr/io.h>
#include <util/delay.h>

// Tastenwerte f�r 4x3 Matrix
const uint8_t tastaturlayout[4][3] = {
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9},
    {0xA, 0, 0xB}
};

/**
 * Scannt die Tastatur nach gedr�ckten Tasten
 * 
 * Funktion: Durchl�uft alle Zeilen/Spalten und erkennt gedr�ckte Tasten
 * Parameter: keine
 * R�ckgabe: uint8_t - Tastenwert aus tastaturlayout oder 0xFF wenn keine Taste gedr�ckt
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
			  
            // Wenn Spalte LOW ist (Taste gedr�ckt)
            if (!(spalten & (1 << pin_spalte))) {			
                if (!(PIND & (1 << pin_spalte))) {
					
                    // Tastenwert zur�ckgeben
                    return tastaturlayout[zeile][spalte];
                }
            }
        }
    }
    
    // Keine Taste gedr�ckt
    return 0xFF;
}

/**
 * Zeigt einen Wert bin�r auf den LEDs an
 * 
 * Funktion: Stellt die unteren 4 Bits des Werts auf PC0-PC3 dar
 * Parameter: uint8_t wert - Anzuzeigender Wert
 * R�ckgabe: keine
 */
void display(uint8_t wert) {
    // Untere 4 Bits von wert auf PC0-PC3 setzen, obere Bits von PORTC unver�ndert lassen
    PORTC = (PORTC & 0xF0) | (wert & 0x0F);
}

/**
 * Hauptprogramm
 */
int main(void) {
    // Port-Konfiguration
    // PB0-PB3 als Ausg�nge (Zeilen)
    DDRB = 0x0F;
	
    // PC0-PC3 als Ausg�nge (LEDs)
    DDRC = 0x0F;
    
    // PD2-PD4 als Eing�nge (Spalten)
    DDRD = 0x00;
	
    // Pull-ups f�r Spalten aktivieren
    PORTD = 0xFF;
    
    // Aktueller Tastenwert
    uint8_t taste = 0xFF;
	
    // Letzter Tastenwert f�r Wiederholungserkennung
    uint8_t temp = 0xFF;
    
    // Hauptschleife
    while (1) {
        // Keypad scannen
        taste = scan_tastatur();
        
        // Wenn neue Taste gedr�ckt
        if (taste != 0xFF && taste != temp) {
			
            // Wert auf LEDs anzeigen
            display(taste);
			
            // Tastenwert speichern
            temp = taste;
			
            // Zur�cksetzen f�r erneute Erkennung
            temp = 0xFF;
        }
    }
    
    return 0;
}