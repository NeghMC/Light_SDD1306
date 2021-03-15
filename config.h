/*
 * config.h
 *
 * Created: 28.02.2021 14:30:02
 */ 


#ifndef OLED_CONFIG_H_
#define OLED_CONFIG_H_

/* ------------ SETTINGS --------------
   nalezy zakomentowac nieuzywane ustawienie oraz odkomentowac aktywne
 */

/* rozdzielczosc wyswietlacza */
#define OLED_128_64
//#define OLED_128_32

/* czy uzywac bufora (dodatkowe miejsce w RAM zajete o rozmiarze rozdzielczosci ekranu) */
//#define OLED_USE_BUFFER

/* Wybrac jednen z dostepnych interfejsow */
#define OLED_USE_SPI
//#define OLED_USE_I2C // SDA, SCL

/* wybrac rozmiar kolejki elementow do narysowania na raz */
#define OLED_QUEUE_LENGTH 10

/* delay function */
#define OLED_DELAY(ms) _delay_ms(ms)

/* Konfiguracja SPI */

	#define DC_SET			PORTD |= (1<<PD4)
	#define DC_CLEAR		PORTD &= ~(1<<PD4)
	
	// jesli jest tylko cs pin to jest on tez resetem
	#define RST_SET			PORTD |= (1<<PD3)
	#define RST_CLEAR		PORTD &= ~(1<<PD3)

	#define OLED_SEND(buffer,length) spiWrite(buffer, length)

/* Konfiguracja I2C */









// -------------------------------------

#endif
