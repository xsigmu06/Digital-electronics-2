/***********************************************************************
 * 
 * Alternately toggle two LEDs when a push button is pressed. Use 
 * functions from GPIO library.
 * ATmega328P (Arduino Uno), 16 MHz, AVR 8-bit Toolchain 3.6.2
 *
 * Copyright (c) 2019-2020 Tomas Fryza
 * Dept. of Radio Electronics, Brno University of Technology, Czechia
 * This work is licensed under the terms of the MIT license.
 * 
 **********************************************************************/

/* Defines -----------------------------------------------------------*/
#define LED0		PC0     // AVR pin where LED0 is connected
#define LED1		PC1
#define LED2		PC2
#define LED3		PC3
#define LED4		PC4
#define BTN			PD0
#define BLINK_DELAY 500
#ifndef F_CPU
#define F_CPU 16000000      // CPU frequency in Hz required for delay
#endif

/* Includes ----------------------------------------------------------*/
#include <util/delay.h>     // Functions for busy-wait delay loops
#include <avr/io.h>         // AVR device-specific IO definitions
#include "gpio.h"           // GPIO library for AVR-GCC

/* Function definitions ----------------------------------------------*/
/**
 * Main function where the program execution begins. Toggle two LEDs 
 * when a push button is pressed. Functions from user-defined GPIO
 * library is used instead of low-level logic operations.
 */
int main(void)
{
    /* LED 0 */
    GPIO_config_output(&DDRC, LED0);
    GPIO_write_low(&PORTC, LED0);
	
    GPIO_config_output(&DDRC, LED1);
    GPIO_write_low(&PORTC, LED1);
	
    GPIO_config_output(&DDRC, LED2);
    GPIO_write_low(&PORTC, LED2);
	
    GPIO_config_output(&DDRC, LED0);
    GPIO_write_low(&PORTC, LED0);
	
    GPIO_config_output(&DDRC, LED0);
    GPIO_write_low(&PORTC, LED0);
	
    /* push button */
    GPIO_config_input_pullup(&DDRD, BTN);

    // Infinite loop
    while (1)
    {
        // Pause several milliseconds
        _delay_ms(BLINK_DELAY);

		if(!GPIO_read(&PIND, BTN))			// active low BTN, but IF needs 1 to work -> ! ... not
		{
			GPIO_toggle(&PORTB, LED_GREEN);
			GPIO_toggle(&PORTC, LED_RED);
		}
    }

    // Will never reach this
    return 0;
}