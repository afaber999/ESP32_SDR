/*
 * lcd.c
 *
 * Created: Mar 2021
 * Author: Arjan te Marvelde
 * 
 * Grove 16x2 LCD, HD44780 chip with JHD1804 I2C interface
 * Display RAM addresses 0x00-0x1f for top row and 0x40-0x5f for bottom row
 * Character Generator addresses are 0x00-0x07
 *
 * Character 0x00:
 *         +-+-+-+-+-+
 * 000000  | |1| | | |
 *         +-+-+-+-+-+
 * 000001  |1| | | | |
 *         +-+-+-+-+-+
 * 000010  | |1| | | |
 *         +-+-+-+-+-+
 * 000011  |1| | | | |
 *         +-+-+-+-+-+
 * 000100  | |1| | | |
 *         +-+-+-+-+-+
 * 000101  |1| | | | |
 *         +-+-+-+-+-+
 * 000110  | |1| | | |
 *         +-+-+-+-+-+
 * 000111  | | | | | |	<= do not use, cursor 
 *         +-+-+-+-+-+
 *
 */
#include <Arduino.h>
#include <Wire.h>
#include <stdio.h>
#include <string.h>
#include "lcd.h"
#include "i2c_interface.h"



// commands
#define LCD_CLEARDISPLAY 	0x01					// Note: LCD_ENTRYINC is set
#define LCD_RETURNHOME 		0x02
#define LCD_ENTRYMODESET 	0x04
#define LCD_DISPLAYCONTROL 	0x08
#define LCD_CURSORSHIFT 	0x10
#define LCD_FUNCTIONSET 	0x20
#define LCD_SETCGRAMADDR 	0x40
#define LCD_SETDDRAMADDR 	0x80

// flags for display entry mode: LCD_ENTRYMODESET
#define LCD_ENTRYSHIFT		0x01
#define LCD_ENTRYNOSHIFT	0x00
#define LCD_ENTRYINC		0x02					// Also applies to CGRAM writes
#define LCD_ENTRYDEC		0x00					// Also applies to CGRAM writes

// flags for display on/off control: LCD_DISPLAYCONTROL
#define LCD_DISPLAYON 		0x04
#define LCD_DISPLAYOFF 		0x00
#define LCD_CURSORON 		0x02
#define LCD_CURSOROFF 		0x00
#define LCD_BLINKON 		0x01
#define LCD_BLINKOFF 		0x00

// flags for display/cursor shift: LCD_CURSORSHIFT
#define LCD_DISPLAYMOVE 	0x08
#define LCD_CURSORMOVE 		0x00
#define LCD_MOVERIGHT 		0x04
#define LCD_MOVELEFT 		0x00

// flags for function set: LCD_FUNCTIONSET
#define LCD_8BITMODE 		0x10
#define LCD_4BITMODE 		0x00
#define LCD_2LINE 			0x08
#define LCD_1LINE 			0x00
#define LCD_5x10DOTS 		0x04
#define LCD_5x8DOTS 		0x00

#define LCD_DELAY			100
#define LCD_COMMAND			0x80
#define LCD_DATA			0x40

/* I2C address and pins */
#define I2C_LCD 		0x3E
#define I2C0_SDA 		16
#define I2C0_SCL 		17


uint8_t cgram[65] = 								// Write CGRAM
{ 													// 8x8 bytes
0x80,
0x08, 0x10, 0x08, 0x10, 0x08, 0x10, 0x08, 0x00,
0x08, 0x10, 0x08, 0x10, 0x08, 0x10, 0x0b, 0x00, 
0x08, 0x10, 0x08, 0x10, 0x08, 0x13, 0x0b, 0x00,
0x08, 0x10, 0x08, 0x10, 0x0b, 0x13, 0x0b, 0x00,
0x08, 0x10, 0x08, 0x13, 0x0b, 0x13, 0x0b, 0x00,
0x08, 0x10, 0x0b, 0x13, 0x0b, 0x13, 0x0b, 0x00,
0x08, 0x13, 0x0b, 0x13, 0x0b, 0x13, 0x0b, 0x00,
0x0b, 0x13, 0x0b, 0x13, 0x0b, 0x13, 0x0b, 0x00
};


void lcd_init(void)
{ 
	uint8_t txdata[8];
	
	/* I2C0 initialisation at 400Khz. */
	//i2c_init(i2c0, 400*1000);
	//gpio_set_function(I2C0_SDA, GPIO_FUNC_I2C);
	//gpio_set_function(I2C0_SCL, GPIO_FUNC_I2C);
	//gpio_pull_up(I2C0_SDA);
	//gpio_pull_up(I2C0_SCL);
	
	// delay(50);
	txdata[0] = LCD_COMMAND; 
	
	/* Initialize function set (see datasheet fig 23)*/
	txdata[1] = LCD_FUNCTIONSET | LCD_8BITMODE | LCD_2LINE | LCD_5x8DOTS;
	i2c_write_blocking(I2C_LCD, txdata, 2);
	delayMicroseconds(4500);
	i2c_write_blocking(I2C_LCD, txdata, 2);
	delayMicroseconds(100);
	i2c_write_blocking(I2C_LCD, txdata, 2);
	delayMicroseconds(LCD_DELAY);
	i2c_write_blocking(I2C_LCD, txdata, 2);
	delayMicroseconds(LCD_DELAY);

	/* Initialize display control */
	txdata[1] = LCD_DISPLAYCONTROL | LCD_DISPLAYOFF | LCD_CURSOROFF | LCD_BLINKOFF;
	i2c_write_blocking(I2C_LCD, txdata, 2);
	delayMicroseconds(LCD_DELAY);
	
	/* Display clear */
	txdata[1] = LCD_CLEARDISPLAY;
	i2c_write_blocking(I2C_LCD, txdata, 2);
	delayMicroseconds(1530);

	/* Initialize entry mode set */ 
	txdata[1] = LCD_ENTRYMODESET | LCD_ENTRYINC | LCD_ENTRYNOSHIFT;
	i2c_write_blocking(I2C_LCD, txdata, 2);
	delayMicroseconds(LCD_DELAY);
	
	/* Load CGRAM */
	txdata[1] = 0x40; 								//Set CGRAM address 0
	i2c_write_blocking(I2C_LCD, txdata, 2);
	delayMicroseconds(LCD_DELAY);
	i2c_write_blocking(I2C_LCD, cgram, 65);
	delayMicroseconds(LCD_DELAY);
	
	/* Initialize display control */
	txdata[1] = LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	i2c_write_blocking(I2C_LCD, txdata, 2);
	delayMicroseconds(LCD_DELAY);
	
	/* Display clear once more */
	txdata[1] = LCD_CLEARDISPLAY;
	i2c_write_blocking(I2C_LCD, txdata, 2);
	delayMicroseconds(1530);
}

void lcd_clear(void)
{
	uint8_t txdata[3];

	txdata[0] = LCD_COMMAND; 
	txdata[1] = LCD_CLEARDISPLAY;
	i2c_write_blocking(I2C_LCD, txdata, 2);
	delayMicroseconds(1530);
}

void lcd_curxy(uint8_t x, uint8_t y, bool on)
{
	uint8_t txdata[3];

	x &= 0x0f;
	y &= 0x01;
	txdata[0] = LCD_COMMAND; 
	txdata[1] = x | 0x80 | (y==1?0x40:0x00);
	i2c_write_blocking(I2C_LCD, txdata, 2);
	delayMicroseconds(LCD_DELAY);
	
	txdata[0] = LCD_COMMAND; 
	txdata[1] = LCD_DISPLAYCONTROL | LCD_DISPLAYON | (on?LCD_CURSORON:LCD_CURSOROFF) | LCD_BLINKOFF;
	i2c_write_blocking(I2C_LCD, txdata, 2);
	delayMicroseconds(LCD_DELAY);
}

void lcd_putxy(uint8_t x, uint8_t y, uint8_t c)
{
	uint8_t txdata[3];

	x &= 0x0f;
	y &= 0x01;
	txdata[0] = LCD_COMMAND; 
	txdata[1] = x | 0x80 | (y==1?0x40:0x00);
	i2c_write_blocking(I2C_LCD, txdata, 2);
	delayMicroseconds(LCD_DELAY);

	txdata[0] = LCD_DATA; 
	txdata[1] = c;
	i2c_write_blocking(I2C_LCD, txdata, 2);
	delayMicroseconds(LCD_DELAY);
}

void lcd_writexy(uint8_t x, uint8_t y, const char *s)
{
	uint8_t i, len;
	uint8_t txdata[18];

	x &= 0x0f;
	y &= 0x01;
	txdata[0] = LCD_COMMAND; 
	txdata[1] = x | 0x80 | ((y==1)?0x40:0x00);
	i2c_write_blocking(I2C_LCD, txdata, 2);
	delayMicroseconds(LCD_DELAY);

	len = (uint8_t)strlen(s);
	len = (len>(16-x))?(16-x):len;
	txdata[0] = LCD_DATA; 
	for(i=0; i<len; i++) txdata[i+1]=s[i];
	i2c_write_blocking(I2C_LCD, txdata, len+1);
	delayMicroseconds(LCD_DELAY);
}


void lcd_test(void)
{
	char chr[17];
	int i, j;
	
	chr[16] = 0; 
	lcd_clear();
	for (i=0; i<16; i++)
	{
		for(j=0; j<16; j++) {
            chr[j] = (char)(16*i+j);
        }
		lcd_writexy(0, 0, chr);
		delay(800);
		lcd_writexy(0, 1, chr);
	}
	lcd_clear();
}