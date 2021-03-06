/***********************************************************************
 * 
 * Seven-segment display library for AVR-GCC.
 * ATmega328P (Arduino Uno), 16 MHz, AVR 8-bit Toolchain 3.6.2
 *
 * Copyright (c) 2019-2020 Tomas Fryza
 * Dept. of Radio Electronics, Brno University of Technology, Czechia
 * This work is licensed under the terms of the MIT license.
 *
 **********************************************************************/

#define F_CPU 16000000

/* Includes ----------------------------------------------------------*/
#include <util/delay.h>
#include "gpio.h"
#include "segment.h"

/* Variables ---------------------------------------------------------*/
// Active-low digits 0 to 9
uint8_t segment_value[] = {
	//figure 8 snake
	/*0b01111111,	//A
	0b10111111,	//B
	0b11111101,	//G
	0b11110111,	//E
	0b11101111,	//D
	0b11011111,	//C
	0b11111101,	//G
	0b11111011,	//F*/
	// abcdefgDP
	0b00000011,		// Digit 0
	0b10011111,     // Digit 1
	0b00100101,     // Digit 2
	0b00001101,     // Digit 3
	0b10011001,		// Digit 4	
	0b01001001,		// Digit 5
	0b01000001,		// Digit 6
	0b00011111,		// Digit 7
	0b00000001,		// Digit 8
	0b00011001,		// Digit 9
	};

// Active-high position 0 to 3
uint8_t segment_position[] = {
	// p3p2p1p0....
	0b00010000,		// Position 0
	0b00100000,		// Position 1
	0b01000000,		// Position 2
	0b10000000		// Position 3
	};

/* Function definitions ----------------------------------------------*/
void SEG_init(void)
{
    /* Configuration of SSD signals */
    GPIO_config_output(&DDRD, SEGMENT_LATCH);
    GPIO_config_output(&DDRD, SEGMENT_CLK);
    GPIO_config_output(&DDRB, SEGMENT_DATA);
}

/*--------------------------------------------------------------------*/
void SEG_update_shift_regs(uint8_t segments, uint8_t position, bool DP)
{
    uint8_t bit_number;
	if(!DP)	
	{
		segments = segment_value[segments];     // 0, 1, ..., 9
	}
	else
	{	// if parameter DP is true change last bit to 0 to turn on decimal point
		segments = segment_value[segments] & 0b11111110;     // 0., 1., ..., 9.
	}
	position = segment_position[position];  // 0, 1, 2, 3
	
    // Pull LATCH, CLK, and DATA low
	GPIO_write_low(&PORTD, SEGMENT_LATCH);
	GPIO_write_low(&PORTD, SEGMENT_CLK);
	GPIO_write_low(&PORTB, SEGMENT_DATA);
	
    // Wait 1 us
	_delay_us(1);
	
    // Loop through the 1st byte (segments)
    // a b c d e f g DP (active low values)
    for (bit_number = 0; bit_number < 8; bit_number++)
    {
        // Output DATA value (bit 0 of "segments")
		//zpusob 1: modulo
		if(segments % 2 == 0)
		{
			GPIO_write_low(&PORTB, SEGMENT_DATA);
		}
		else
		{
			GPIO_write_high(&PORTB, SEGMENT_DATA);
		}
		
		SEG_clk_2us();
		
        // Shift "segments"
        segments = segments >> 1;
    }

    // Loop through the 2nd byte (position)
    // p3 p2 p1 p0 . . . . (active high values)
    for (bit_number = 0; bit_number < 8; bit_number++)
    {
        // Output DATA value (bit 0 of "position")
        //zpusob 2: AND ... efektivnejsi (rychlejsi operace nez zbytek po deleni)
		if((position & 1) == 0)
		{
			GPIO_write_low(&PORTB, SEGMENT_DATA);
		}
		else
		{
			GPIO_write_high(&PORTB, SEGMENT_DATA);
		}
        
        SEG_clk_2us();
		
        // Shift "position"
        position = position >> 1;
    }

    // Pull LATCH high
	GPIO_write_high(&PORTD, SEGMENT_LATCH);
	
    // Wait 1 us
	_delay_us(1);
}

/*--------------------------------------------------------------------*/
void SEG_clear(void)
{
	// Pull LATCH, CLK, and DATA low
	GPIO_write_low(&PORTD, SEGMENT_LATCH);
	GPIO_write_low(&PORTD, SEGMENT_CLK);
	GPIO_write_low(&PORTB, SEGMENT_DATA);
	
	uint8_t bit_number;
	
	for (bit_number = 0; bit_number < 8; bit_number++)
	{
		// Turn segment off
		GPIO_write_high(&PORTB, SEGMENT_DATA);
		
		SEG_clk_2us();
	}
    // Pull LATCH high
    GPIO_write_high(&PORTD, SEGMENT_LATCH);
	 
    // Wait 1 us
    _delay_us(1);
}

/*--------------------------------------------------------------------*/
void SEG_clk_2us(void)
{
        // Wait 1 us
        _delay_us(1);
        
        // Pull CLK high
        GPIO_write_high(&PORTD, SEGMENT_CLK);
        
        // Wait 1 us
        _delay_us(1);
        
        // Pull CLK low
        GPIO_write_low(&PORTD, SEGMENT_CLK);
}