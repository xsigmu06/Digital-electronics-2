/***********************************************************************
 * 
 * Decimal counter with 7-segment output.
 * ATmega328P (Arduino Uno), 16 MHz, AVR 8-bit Toolchain 3.6.2
 *
 * Copyright (c) 2018-2020 Tomas Fryza
 * Dept. of Radio Electronics, Brno University of Technology, Czechia
 * This work is licensed under the terms of the MIT license.
 * 
 **********************************************************************/

/* Includes ----------------------------------------------------------*/
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include "segment.h"        // Seven-segment display library for AVR-GCC

uint8_t sec_units = 0;
uint8_t sec_decimals = 0;
uint8_t min_units = 0;
uint8_t min_decimals = 0;

/* Function definitions ----------------------------------------------*/
/**
 * Main function where the program execution begins. Display decimal 
 * counter values on SSD (Seven-segment display) when 16-bit 
 * Timer/Counter1 overflows.
 */
int main(void)
{
    // Configure SSD signals
    SEG_init();

    // Test of SSD: display number '3' at position 0
    //SEG_update_shift_regs(4, 1, 1);

	
	// Configure 8-bit Timer/Counter0
	TIM0_overflow_4ms();
	TIM0_overflow_interrupt_enable();
	
    /* Configure 16-bit Timer/Counter1
     * Set prescaler and enable overflow interrupt */
	TIM1_overflow_33ms();
	TIM1_overflow_interrupt_enable();
	
	
    // Enables interrupts by setting the global interrupt mask
	sei();

    // Infinite loop
    while (1)
    {
        /* Empty loop. All subsequent operations are performed exclusively 
         * inside interrupt service routines ISRs */
    }

    // Will never reach this
    return 0;
}

/* Interrupt service routines ----------------------------------------*/
/**
 * ISR starts when Timer/Counter0 overflows. Display values on SSD.
 */
ISR(TIMER0_OVF_vect)
{
	static uint8_t pos = 0;
	//SEG_update_shift_regs(sec_units, pos, 0);
	if(pos == 0)
	{
		SEG_update_shift_regs(sec_units, pos, 0);
		pos = 1;
	}
	else if (pos == 1)
	{
		SEG_update_shift_regs(sec_decimals, pos, 0);
		pos = 2;
	}
	else if (pos == 2)
	{
		SEG_update_shift_regs(min_units, pos, 1);
		
		pos = 3;
	}
	else if (pos == 3)
	{
		SEG_update_shift_regs(min_decimals, pos, 0);
		pos = 0;
	}
	/*if (min_units > 1)
	{
		SEG_clear();
		sei();
	}*/
}

/**
 * ISR starts when Timer/Counter1 overflows. Increment decimal counter.
 */
ISR(TIMER1_OVF_vect)
{
    // WRITE YOUR CODE HERE
	//SEG_update_shift_regs(sec_units, 0, 0);

	/*if(sec_units > 7) 
	{
		sec_units = 0;
		SEG_clear();
	}*/
	sec_units++;
	if(sec_units > 9) 
	{
		sec_units = 0;
		sec_decimals++;
		if(sec_decimals > 5) 
		{
			sec_decimals = 0;
			min_units++;
			if (min_units > 9)
			{
				min_units = 0;
				min_decimals++;
				if(min_decimals > 5)
				{
					min_decimals = 0;
				}
			}
		}
	}
}

