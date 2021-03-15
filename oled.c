/*
 * oled.c
 *
 *  Created on: 8 lut 2019
 *      Author: Grzegorz
 */

#include "oled.h"
#include <util/delay.h>


void spi_write(const uint8_t * data, uint16_t length) {
	
}

void oled_init(uint8_t refresh) {

	RST_SET;
	OLED_DELAY(25);
	RST_CLEAR;
	OLED_DELAY(25);
	RST_SET;
	
	const uint8_t config[] = {
		SSD1306_DISPLAYOFF,
		SSD1306_SETDISPLAYCLOCKDIV, refresh,
		SSD1306_SETDISPLAYOFFSET, 0x0,
		SSD1306_SETSTARTLINE | 0x0,
		SSD1306_CHARGEPUMP, 0x14, //wewnetrzne zasilanie 9v, 0x10 - zewnetrzne
		SSD1306_MEMORYMODE, 0x00,
		SSD1306_SEGREMAP | 0x1,
		SSD1306_COMSCANDEC,
		SSD1306_SETCONTRAST, 0xCF, //wewnetrzne zasilanie 9v, 0x9F - zewnetrzne
		SSD1306_SETPRECHARGE, 0xF1,
		
	#if defined OLED_128_32
		SSD1306_SETMULTIPLEX, 0x1F,
		SSD1306_SETCOMPINS, 0x02,
	#elif defined OLED_128_64
		SSD1306_SETMULTIPLEX, 0x3F,
		SSD1306_SETCOMPINS, 0x12,
	#endif
	
		SSD1306_SETVCOMDETECT, 0x40,
		SSD1306_DISPLAYALLON_RESUME,
		SSD1306_NORMALDISPLAY,
		SSD1306_DEACTIVATE_SCROLL,
		SSD1306_DISPLAYON
	};
	
	OLED_SEND(config, sizeof(config));
}

#ifdef OLED_USE_BUFFER
/*
uint8_t oled_buf[OLED_BUFFER_SIZE];

void oled_display() { // @TODO
	oled_send(OLED_CMD, SSD1306_SETLOWCOLUMN | 0x0);
	oled_send(OLED_CMD, SSD1306_SETHIGHCOLUMN | 0x0);
	oled_send(OLED_CMD, SSD1306_SETSTARTLINE | 0x0);
	#if USE_CS == 1
	CS_SET;
	#endif
	DC_SET;
	#if USE_CS == 1
	CS_CLEAR;
	#endif
	for(uint16_t i = 0; i < OLED_BUFFER_SIZE; ++i)
	spiWrite(oled_buf[i]);
	#if USE_CS == 1
	CS_SET;
	#endif
}

static void oled_setPixel_fast(int x, int y, uint8_t color) {
		if(color)	oled_buf[x + (y >> 8) * OLED_WIDTH] |= (1 << (y & 0x7));
		else		oled_buf[x + (y >> 8) * OLED_WIDTH] &= ~(1 << (y & 0x7));

	uint8_t column = x;
	uint8_t row = y >> 3; // y/8	
	
	if(column != oled_lastColumn || row != oled_lastRow) {
		oled_display();
	}
	
	oled_buffer |= (color << (y & 0x7));
	
	oled_lastColumn = column;
	oled_lastRow = row;
}

void oled_setPixel(uint8_t x, uint8_t y, screen_buffer_t * buffer, uint8_t color) {
	if(x < 0 || x >= buffer->w || y < 0 || y >= buffer->h)
		return;

	oled_setPixel_fast(x, y, color);
}

uint8_t oled_readPixel_fast(int x, int y) {
	return oled_buf[x + (y >> 3) * OLED_WIDTH] & (1 << (y & 0x7));
}

void oled_refresh(uint8_t pageStart, uint8_t pageStop, uint8_t columnStart, uint8_t columnStop) {
	uint8_t pageCnt, columnCnt;
	uint8_t * ram_buf_start;

	for(pageCnt = pageStart; pageCnt < pageStop; pageCnt++) {

		oled_send(OLED_CMD, SSD1306_SETLOWCOLUMN | (columnStart & 0x0f)); // automatycna inkrementacja adresu kolumny
		oled_send(OLED_CMD, SSD1306_SETHIGHCOLUMN | (columnStart >> 4 ));
		oled_send(OLED_CMD, SSD1306_SETPAGE | pageCnt);

		ram_buf_start = &oled_buf[pageCnt * 128 + columnStart];

		#if USE_CS == 1
		CS_CLEAR;
		#endif

		for(columnCnt = columnStart; columnCnt < columnStop; columnCnt++)
		spiWrite(*ram_buf_start++);
	}
}

void oled_clear(uint8_t color) {
	uint8_t fill = (color ? 0xff : 0x00);
	#if OLED_USE_BUFFER == 1
	for(uint16_t i = 0; i < OLED_BUFFER_SIZE; ++i)
	oled_buf[i] = fill;
	#else
	oled_send(OLED_CMD, SSD1306_SETPAGE | 0x0);
	oled_send(OLED_CMD, SSD1306_SETLOWCOLUMN | 0x0);
	oled_send(OLED_CMD, SSD1306_SETHIGHCOLUMN | 0x0);
	
	#if USE_CS == 1
	CS_SET;
	#endif
	
	DC_SET;
	
	#if USE_CS == 1
	CS_CLEAR;
	#endif
	
	for(uint16_t i = 0; i < OLED_BUFFER_SIZE; ++i)
	spiWrite(fill);
	
	#if USE_CS == 1
	CS_SET;
	#endif
	#endif
}

*/

#else /* ------------------------- NO BUFFER --------------------------- */

static uint8_t oled_page[OLED_WIDTH];
static uint8_t startY, endY;

static oled_to_draw_object_t queue[OLED_QUEUE_LENGTH];
static uint8_t head;

static void setPixel(uint8_t x, uint8_t y, uint8_t color) {
	if(x < 0 || x >= OLED_WIDTH || y < startY || y >= endY)
	return;
	
	if(color)	oled_page[x] |= (1 << (y-startY));
	else		oled_page[x] &= ~(1 << (y-startY));
}

static void drawFillRect(oled_fill_rectangle_t * obj) {
	for(uint8_t i = 0; i <= obj->w; ++i)
		for(uint8_t j = 0; j <= obj->h; ++j)
			setPixel(obj->x + i, obj->y + j, obj->color);
}

void oled_drawFillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color) {
	if(head >= OLED_QUEUE_LENGTH) return;
	queue[head].type = FILL_RECTANGLE;
	queue[head].filled_rectangle = (oled_fill_rectangle_t){x, y, w, h, color};
	head++;
}

static void drawBitmap(oled_bitmap_t * obj) {

	uint8_t i, j;
	uint16_t k = 0;

	for(j = 0; j < obj->b->hight; j++) {
		for(i = 0; i < obj->b->width; i++) {
			setPixel(obj->x + i, obj->y + j, obj->b->content[k / 8] & (1<<(k % 8)));
			k++;
		}
	}
}

void oled_drawBitmap(uint8_t x, uint8_t y, uint8_t color, const __memx bitmap_t * b) {
	if(head >= OLED_QUEUE_LENGTH) return;
	queue[head].type = BITMAP;
	queue[head].bitmap = (oled_bitmap_t){x, y, color, b};
	head++;
}

// algorytm Bresenhama
void drawLine(oled_line_t * obj) {

	register int8_t dx = obj->x2 - obj->x1;
	register int8_t dy = obj->y2 - obj->y1;
	register uint8_t x1 = obj->x1, x2 = obj->x2, y1 = obj->y1, y2 = obj->y2;
	register const int8_t x_step = (dx < 0) ? -1 : 1;
	register const int8_t y_step = (dy < 0) ? -1 : 1;

	dx = abs(dx);
	dy = abs(dy);

	if(dx < dy) { // sprowadzamy do warunku, os nachylona maksymalnie 45 stopni do OX
		SWAP(x1, y1);
		SWAP(x2, y2);
		SWAP(dx, dy);
	}

	setPixel(x1, y1, obj->color);
	int8_t error = dx >> 1;
	for(uint8_t i = dx; i; i--) {
		x1 += x_step;
		error -= dy;
		if(error < 0) {
			y1 += y_step;
			error += dx;
		}
		setPixel(x1, y1, obj->color);
	}
}

void oled_drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color) {
	if(head >= OLED_QUEUE_LENGTH) return;
	queue[head].type = LINE;
	queue[head].line = (oled_line_t){x1, y1, x2, y2, color};
	head++;
}

static uint8_t drawChar(oled_character_t * obj) { // @TODO nie ma spacji
	uint8_t line, j = 0;

	for(int8_t i = 0; i < 5; ++i) { // oryginalny font jest 5 na 8
		line = font[(obj->c * 5) + i]; // pobieramy pionow¹ linijkê fonta

		if(line != 0) { // rysujemy tylko 'znacace' linie
			for(uint8_t k = 0; k < 8; k++) {
				if(line & 0x1) { // jeœli rysujemy pixel fonta
					if(obj->size == 1)
					setPixel(obj->x + j * obj->size, obj->y + k * obj->size, obj->color);
					else {
						oled_fill_rectangle_t rec = {obj->x + j * obj->size, obj->y + k * obj->size, obj->size, obj->size, obj->color};
						drawFillRect(&rec);
					}
				}
				line >>= 1;
			}
			j++;
		}
	}
	return j;
	//cursor += (j ? (j*size) : size); // jeœli j == 0 to cursor += 3;
}

void oled_drawChar(uint8_t x, uint8_t y, uint8_t color, uint8_t size, char c) {
	if(head >= OLED_QUEUE_LENGTH) return;
	queue[head].type = CHARACTER;
	queue[head].character = (oled_character_t){x, y, color, size, c};
	head++;
}

static void drawString(oled_string_t * obj) {

	const __memx char * str = obj->str;
	uint8_t x = obj->x;
	while(*str) {
		oled_character_t c = {x, obj->y, obj->color, obj->size, *str};
		uint8_t j = drawChar(&c);
		x += (j ? (j* obj->size) : obj->size);
		str++;
	}
}

void oled_drawString(uint8_t x, uint8_t y, uint8_t color, uint8_t size, const __memx char * str) {
	if(head >= OLED_QUEUE_LENGTH) return;
	queue[head].type = STRING;
	queue[head].string = (oled_string_t){x, y, color, size, str};
	head++;
}


void oled_display() {
	for(uint8_t page = 0 ; page < OLED_HEIGHT / 8; ++page) {
		startY = page * 8;
		endY = (page + 1) * 8;
		for(uint8_t obj = 0; obj < head; ++obj) {
			switch(queue[obj].type) {
				case LINE:
					drawLine(&queue[obj].line);
					break;
				case FILL_RECTANGLE:
					drawFillRect(&queue[obj].filled_rectangle);
					break;
				case CHARACTER:
					drawChar(&queue[obj].character);
					break;
				case STRING:
					drawString(&queue[obj].string);
					break;
				case BITMAP:
					drawBitmap(&queue[obj].bitmap);
					break;
				default:
					// error
					break;
			}
		}
		const uint8_t config[] = {
			SSD1306_SETPAGE | page,
			SSD1306_SETLOWCOLUMN | 0x0,
			SSD1306_SETHIGHCOLUMN | 0x0
		};
		OLED_SEND_CMD(config, sizeof(config));
		OLED_SEND_DATA(oled_page, OLED_WIDTH);
		memset(oled_page, 0, OLED_WIDTH);
	}
}
	
	/*
	uint8_t pageEnd = pageStart + pageNumber;
	
	for(uint8_t pageCnt = pageStart; pageCnt < pageEnd; pageCnt++) {
		const uint8_t config[] = {
			SSD1306_SETPAGE | pageCnt,
			SSD1306_SETLOWCOLUMN | (columnStart & 0x0f),
			SSD1306_SETHIGHCOLUMN | (columnStart >> 4)
		};
		OLED_SEND(config, sizeof(config));
		OLED_SEND(buffer, width);
		buffer += width;
	}*?
}
*/
#endif
