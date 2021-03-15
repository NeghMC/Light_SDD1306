/*
 * oled.h
 *
 *  Created on: 8 lut 2019
 *      Author: Grzegorz
 */

#ifndef OLED_H_
#define OLED_H_

#include <avr/io.h>
#include <stdint.h>

#define SWAP(a, b) do{int16_t temp = a; a = b; b = temp;}while(0)
#define TO_BOOL(x) (x != 0)?1:0

#define OLED_BLACK		0
#define OLED_WHITE		1
#define OLED_DATA		0
#define OLED_CMD		1

//typedef enum {page0, page1, page2, page3, page4, page5, page6, page7} oled_page;


#define REFRESH_MIN	0x80
#define REFRESH_MID	0xB0
#define REFRESH_MAX	0xF0

#define SSD1306_MEMORYMODE          0x20 ///< See datasheet
#define SSD1306_COLUMNADDR          0x21 ///< See datasheet
#define SSD1306_PAGEADDR            0x22 ///< See datasheet
#define SSD1306_SETCONTRAST         0x81 ///< See datasheet
#define SSD1306_CHARGEPUMP          0x8D ///< See datasheet
#define SSD1306_SEGREMAP            0xA0 ///< See datasheet
#define SSD1306_DISPLAYALLON_RESUME 0xA4 ///< See datasheet
#define SSD1306_DISPLAYALLON        0xA5 ///< Not currently used
#define SSD1306_NORMALDISPLAY       0xA6 ///< See datasheet
#define SSD1306_INVERTDISPLAY       0xA7 ///< See datasheet
#define SSD1306_SETMULTIPLEX        0xA8 ///< See datasheet
#define SSD1306_DISPLAYOFF          0xAE ///< See datasheet
#define SSD1306_DISPLAYON           0xAF ///< See datasheet
#define SSD1306_SETPAGE				0xB0
#define SSD1306_COMSCANINC          0xC0 ///< Not currently used
#define SSD1306_COMSCANDEC          0xC8 ///< See datasheet
#define SSD1306_SETDISPLAYOFFSET    0xD3 ///< See datasheet
#define SSD1306_SETDISPLAYCLOCKDIV  0xD5 ///< See datasheet
#define SSD1306_SETPRECHARGE        0xD9 ///< See datasheet
#define SSD1306_SETCOMPINS          0xDA ///< See datasheet
#define SSD1306_SETVCOMDETECT       0xDB ///< See datasheet

#define SSD1306_SETLOWCOLUMN        0x00 ///< Not currently used
#define SSD1306_SETHIGHCOLUMN       0x10 ///< Not currently used
#define SSD1306_SETSTARTLINE        0x40 ///< See datasheet

#define SSD1306_EXTERNALVCC         0x01 ///< External display voltage source
#define SSD1306_SWITCHCAPVCC        0x02 ///< Gen. display voltage from 3.3V

#define SSD1306_RIGHT_HORIZONTAL_SCROLL              0x26 ///< Init rt scroll
#define SSD1306_LEFT_HORIZONTAL_SCROLL               0x27 ///< Init left scroll
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29 ///< Init diag scroll
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL  0x2A ///< Init diag scroll
#define SSD1306_DEACTIVATE_SCROLL                    0x2E ///< Stop scroll
#define SSD1306_ACTIVATE_SCROLL                      0x2F ///< Start scroll
#define SSD1306_SET_VERTICAL_SCROLL_AREA             0xA3 ///< Set scroll range

#include "./config.h"

#ifdef OLED_128_64
#define OLED_HEIGHT 64
#endif
#ifdef OLED_128_32
#define OLED_HEIGHT 32
#endif

#define OLED_WIDTH		128

#define OLED_BUFFER_SIZE	(OLED_WIDTH*OLED_HEIGHT/8)
#define OLED_PAGE_SIZE		(OLED_WIDTH * 8)

#define OLED_SEND_CMD(buffer,length) \
	DC_CLEAR; OLED_SEND(buffer,length)
#define OLED_SEND_DATA(buffer,length) \
	DC_SET; OLED_SEND(buffer,length)


extern const __flash unsigned char font[];

void oled_init(uint8_t refresh);

typedef struct {
	uint8_t width, hight;
	uint8_t content[];
} bitmap_t;

typedef struct {
	uint8_t x1, y1, x2, y2, color:1;
} oled_line_t;

typedef struct {
	uint8_t x, y, w, h, color:1;
} oled_fill_rectangle_t;

typedef struct {
	uint8_t x, y, color:1;
	uint8_t size:4;
	char c;
} oled_character_t;

typedef struct {
	uint8_t x, y, color:1;
	uint8_t size:4;
	const __memx char * str;
} oled_string_t;

typedef struct {
	uint8_t x, y, color:1;
	const bitmap_t __memx * b;
} oled_bitmap_t;


#ifdef OLED_USE_BUFFER
/*
void oled_display();
void oled_refresh(uint8_t pageStart, uint8_t pageStop, uint8_t columnStart, uint8_t columnStop);
void oled_setPixel(int x, int y, uint8_t pix);
void oled_clear(uint8_t color);
void oled_drawLine(int x1, int y1, int x2, int y2, uint8_t color);
void oled_fillRect(int x, int y, int w, int h, uint8_t color);
void oled_putc(int x, int y, char c, uint8_t size, uint8_t color);
void oled_puts(int x, int y, char * s, uint8_t size, uint8_t color);
void oled_putn(int x, int y, char * s, uint8_t size, uint8_t color);
void oled_display();*/

#else /* ------------------------- NO BUFFER --------------------------- */


typedef struct {
	enum {
		NULL,
		LINE,
		FILL_RECTANGLE,
		CHARACTER,
		STRING,
		BITMAP
	} type;
	
	union {
		oled_line_t line;
		oled_fill_rectangle_t filled_rectangle;
		oled_character_t character;
		oled_string_t string;
		oled_bitmap_t bitmap;
	};
} oled_to_draw_object_t;

void oled_drawFillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);
void oled_drawBitmap(uint8_t x, uint8_t y, uint8_t color, const __memx bitmap_t * b);
void oled_drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color);
void oled_drawChar(uint8_t x, uint8_t y, uint8_t color, uint8_t size, char c);
void oled_drawString(uint8_t x, uint8_t y, uint8_t color, uint8_t size, const __memx char * str);

void oled_display();

#endif

#endif /* OLED_H_ */
