/***********************************************************************
 * 
 * Stopwatch with LCD display output.
 * ATmega328P (Arduino Uno), 16 MHz, AVR 8-bit Toolchain 3.6.2
 *
 * Copyright (c) 2017-2020 Tomas Fryza
 * Dept. of Radio Electronics, Brno University of Technology, Czechia
 * This work is licensed under the terms of the MIT license.
 * 
 **********************************************************************/

/* Includes ----------------------------------------------------------*/
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include "lcd.h"            // Peter Fleury's LCD library
#include <stdlib.h>         // C library. Needed for conversion function

/* Variables ---------------------------------------------------------*/
// Custom character definition using https://omerk.github.io/lcdchargen/
uint8_t customChar[64] = {
    0b00000,
    0b01010,
    0b11111,
    0b01110,
    0b00100,
    0b10101,
    0b01110,
    0b00100,
    
	0b10000,
	0b10000,
	0b10000,
	0b10000,
	0b10000,
	0b10000,
	0b10000,
	0b10000,
    
    0b11000,
    0b11000,
    0b11000,
    0b11000,
    0b11000,
    0b11000,
    0b11000,
    0b11000,

	0b11100,
	0b11100,
	0b11100,
	0b11100,
	0b11100,
	0b11100,
	0b11100,
	0b11100,
    
	0b11110,
	0b11110,
	0b11110,
	0b11110,
	0b11110,
	0b11110,
	0b11110,
	0b11110,
    
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111,
	0b11111                
};

/* Function definitions ----------------------------------------------*/
/**
 * Main function where the program execution begins. Update stopwatch
 * value on LCD display when 8-bit Timer/Counter2 overflows.
 */
int main(void)
{
    // Initialize LCD display
    lcd_init(LCD_DISP_ON);
    
    // Set pointer to beginning of CGRAM memory
    lcd_command(1 << LCD_CGRAM);
    for (uint8_t i = 0; i < 64; i++)
    {
        // Store all new chars to memory line by line
        lcd_data(customChar[i]);
    }
    // Set DDRAM address
    lcd_command(1 << LCD_DDRAM);
    
    // Display first custom character
    lcd_putc(0);    //vlastni symboly se ukladaji ve sloupci 0 radek 0 az 7

    // Put string(s) at LCD display
    lcd_gotoxy(1, 0);       //sloupec 1, radek 0
    lcd_puts("00:00.0");
    //lcd_gotoxy(11, 0);
    //lcd_putc('a');
    //lcd_gotoxy(1, 1);
    //lcd_putc('b');
    lcd_gotoxy(11, 1);
    lcd_putc('c');

    // Configure 8-bit Timer/Counter0 for Stopwatch
    // Enable interrupt and set the overflow prescaler to 16 ms
    TIM0_overflow_16ms();
    TIM0_overflow_interrupt_enable();

    // Configure 8-bit Timer/Counter2 for Stopwatch
    // Enable interrupt and set the overflow prescaler to 16 ms
    TIM2_overflow_16ms();
    TIM2_overflow_interrupt_enable();

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
ISR(TIMER0_OVF_vect)
{
    static uint8_t number_of_overflows = 0;
    static uint8_t tens = 0;    // Tenths of a second
    static uint8_t seg = 0;     // Segment (Vlastni znaky)
    static uint8_t pos = 1;     // Position
        
    if(number_of_overflows > 0 && number_of_overflows < 6)  //5 stavu pro 5 znaku
    {
        seg++;
        lcd_gotoxy(pos, 1);
        lcd_putc(seg);
    }
    
    number_of_overflows++;
    if (number_of_overflows >= 6)
    {
        number_of_overflows = 0;
        tens++;                
        pos++;
        seg = 0;

        if(pos > 10)
        {
            pos = 1;
            tens = 0;
            lcd_gotoxy(pos, 1);
            lcd_puts("         ");     //reset na pozicich 1 az 10
        }
    }        
}
/**
 * ISR starts when Timer/Counter2 overflows. Update the stopwatch on
 * LCD display every sixth overflow, ie approximately every 100 ms
 * (6 x 16 ms = 100 ms).
 */
ISR(TIMER2_OVF_vect)
{
    static uint8_t number_of_overflows = 0;
    static uint8_t tens = 0;        // Tenths of a second
    static uint8_t secs = 0;        // Seconds
    static uint8_t mins = 0;        // minutes

    char lcd_string[2] = "  ";      // String for converting numbers by itoa()
    number_of_overflows++;
    if (number_of_overflows >= 6)
    {
        // Do this every 6 x 16 ms = 100 ms
        number_of_overflows = 0;

        tens++;
        if(tens > 9)
        {
            tens = 0;
            secs++;
        }
        itoa(tens, lcd_string, 10); //prevede int tens do string lcd_string v 10 soustave
        lcd_gotoxy(7, 0);
        lcd_puts(lcd_string);
                
        if(secs < 10)   //pro 0 .. 9 pozice 5
        {            
            lcd_gotoxy(5, 0);          
        }   
        else            //pro 10 .. 59 pozice 4
        {
            lcd_gotoxy(4, 0);
        }        
        if(secs > 59)
        {
            secs = 0;
            mins++;   
            lcd_gotoxy(11, 0);
            lcd_puts("0   "); //reset na pozicich 11 to 14 (druha mocnina sekund)
            lcd_gotoxy(4, 0);    
        }     
        itoa(secs, lcd_string, 10);
        lcd_puts(lcd_string);
        
        lcd_gotoxy(11, 0);
        itoa(secs*secs, lcd_string, 10);    //druha mocnina sekund
        lcd_puts(lcd_string);
        
        if(mins < 10)  
        {
            lcd_gotoxy(2, 0);
        }
        else         
        {            
            lcd_gotoxy(1, 0);            
        }
        if(mins > 59)
        {
            mins = 0;
        }   
        itoa(mins, lcd_string, 10);
        lcd_puts(lcd_string);   
    }
}