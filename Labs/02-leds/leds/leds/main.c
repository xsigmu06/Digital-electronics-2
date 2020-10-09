/***********************************************************************
 * 
 * Alternately toggle two LEDs when a push button is pressed.
 * ATmega328P (Arduino Uno), 16 MHz, AVR 8-bit Toolchain 3.6.2
 *
 * Copyright (c) 2018-2020 Tomas Fryza
 * Dept. of Radio Electronics, Brno University of Technology, Czechia
 * This work is licensed under the terms of the MIT license.
 * 
 **********************************************************************/

/* Defines -----------------------------------------------------------*/
//#define LED_GREEN   PB5     // AVR pin where green LED is connected
#define LED_RED0     PC0
#define LED_RED1     PC1
#define LED_RED2     PC2
#define LED_RED3     PC3
#define LED_RED4     PC4
#define BTN     PD0			// button
#define BLINK_DELAY 250
#ifndef F_CPU
#define F_CPU 16000000      // CPU frequency in Hz required for delay
#endif

/* Includes ----------------------------------------------------------*/
#include <util/delay.h>     // Functions for busy-wait delay loops
#include <avr/io.h>         // AVR device-specific IO definitions

/* Functions ---------------------------------------------------------*/
/**
 * Main function where the program execution begins. Toggle two LEDs 
 * when a push button is pressed.
 */
void knightRider (int i)
{
	switch(i)
	{
		case 0:
			PORTC = PORTC ^ (1<<LED_RED0);
			break;
		case 1:
			PORTC = PORTC ^ (1<<LED_RED0);
			PORTC = PORTC ^ (1<<LED_RED1);
			break;
		case 2:
			PORTC = PORTC ^ (1<<LED_RED1);
			PORTC = PORTC ^ (1<<LED_RED2);
			break;
		case 3:
			PORTC = PORTC ^ (1<<LED_RED2);
			PORTC = PORTC ^ (1<<LED_RED3);
			break;
		case 4:
			PORTC = PORTC ^ (1<<LED_RED3);
			PORTC = PORTC ^ (1<<LED_RED4);
			break;
		default:
			PORTC = PORTC ^ (1<<LED_RED0);
	}
}

int main(void)
{
    /* RED LEDs */
    DDRC = DDRC | (1<<LED_RED0) | (1<<LED_RED1) | (1<<LED_RED2) | (1<<LED_RED3) | (1<<LED_RED4);
    PORTC = PORTC & ~(1<<LED_RED0) & ~(1<<LED_RED1) & ~(1<<LED_RED2) & ~(1<<LED_RED3) & ~(1<<LED_RED4);				
	
	/* BUTTON */
	DDRD = DDRD & ~(1<<BTN);								// & ~(1 ...vstup
	PORTD = PORTD | (1<<BTN);	
	
	uint8_t i = 0;
			
    // Infinite loop
    while (1)
    {
		if(bit_is_clear(PIND, BTN))								// je tlacitko v 1? -> clear... aktivni nula
		{		
			PORTC = PORTC | (1<<LED_RED0);
			for(i = 0; i < 4; i++)
			{
				PORTC = PORTC<<1;
				_delay_ms(BLINK_DELAY);
				//knightRider(i);
			}
			for(i = 4; i > 0; i--)
			{
				PORTC = PORTC>>1;
				_delay_ms(BLINK_DELAY);
				//knightRider(i);
			}
		}
		else
			PORTC = PORTC & ~(1<<LED_RED0);
    }
    return 0;
}