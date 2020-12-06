/*
 * hc-sr04.c
 *
 * Created: 18. 11. 2020 12:04:28
 * Authors : Jan Sigmund, Michal Švento
 */ 
/* Definees ----------------------------------------------------------*/
#define TRIGGER PB2
#define ECHO	PB3
#define LED1	PC1
#define LED2	PC2
#define LED3	PC3
#define LED4	PC4

/* Includes ----------------------------------------------------------*/
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include "lcd.h"            // Peter Fleury's LCD library
#include <stdlib.h>         // C library. Needed for conversion function
#include "uart.h"           // Peter Fleury's UART library
#include "gpio.h"			// General purpose input output library

#ifndef F_CPU
#define F_CPU 16000000
#endif

/* Variables ---------------------------------------------------------*/
char lcd_string[5]="    ";
static uint16_t distance;






int main(void)
{	

	GPIO_config_output(&DDRB,TRIGGER);
	GPIO_write_low(&PORTB,TRIGGER);
	GPIO_config_output(&DDRC,LED1);
	GPIO_write_low(&PORTC,LED1);
	GPIO_config_output(&DDRC,LED2);
	GPIO_write_low(&PORTC,LED2);
	GPIO_config_output(&DDRC,LED3);
	GPIO_write_low(&PORTC,LED3);
	GPIO_config_output(&DDRC,LED4);
	GPIO_write_low(&PORTC,LED4);
	
	GPIO_config_input_nopull(&DDRB,ECHO);
	
	
	
	TIM0_overflow_16u();
	TIM0_overflow_interrupt_enable();
	
	TIM1_overflow_1s();
	TIM1_overflow_interrupt_enable();
	
	uart_init(UART_BAUD_SELECT(9600,F_CPU));
	
	//enable global interrupt
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

/*
*
*/

ISR(TIMER0_OVF_vect)
{	
	
	static uint16_t number_of_overflows=0;
	static uint16_t comparelenght=0;
	static uint16_t lenght=0;
	
	
	
	
	typedef enum {              // FSM declaration
		STATE_PULSE,
		STATE_ECHODETECT,
		STATE_ECHOCOUNT,
	} state_t;
	
	static state_t state= STATE_PULSE;
	
	number_of_overflows++;
	switch(state)
	{
		case STATE_PULSE:
				if (number_of_overflows<=1)
				{
					GPIO_write_high(&PORTB,TRIGGER);
					state=STATE_ECHODETECT;
					lenght=0;
				} 

				break;	
				
		case STATE_ECHODETECT:
				GPIO_write_low(&PORTB,TRIGGER);
				if (number_of_overflows<=4062)
				{
					if (GPIO_read(&PINB,ECHO))
					{
						lenght++;
					}
				} 
				else
				{
					state=STATE_ECHOCOUNT;
					
				}
				break;
		case STATE_ECHOCOUNT:
				if (lenght!=comparelenght)
				{
					distance=lenght*16*0.017;
					itoa(distance,lcd_string,10);
					GPIO_toggle(&PORTB,LED3);
					
					comparelenght=lenght;
				}
				
				
				lenght=0;
				number_of_overflows=0;
				state=STATE_PULSE;

				
		default:
				state=STATE_PULSE;
				
		}
	
	
	
}



ISR(TIMER1_OVF_vect)
{

	
	uart_puts(lcd_string);
	uart_puts("\n");
	
	
	// Turn on LEDS
	if(distance <= 10)
	{
		GPIO_write_high(&PORTC, LED1);
		GPIO_write_high(&PORTC, LED2);
		GPIO_write_high(&PORTC, LED3);
		GPIO_write_high(&PORTC, LED4);
	}
	else if(distance <= 50)
	{
		GPIO_write_high(&PORTC, LED1);
		GPIO_write_high(&PORTC, LED2);
		GPIO_write_high(&PORTC, LED3);
		GPIO_write_low(&PORTC, LED4);
	}
	else if(distance <= 100)
	{
		GPIO_write_high(&PORTC, LED1);
		GPIO_write_high(&PORTC, LED2);
		GPIO_write_low(&PORTC, LED3);
		GPIO_write_low(&PORTC, LED4);
	}
	else if(distance <= 200)
	{
		GPIO_write_high(&PORTC, LED1);
		GPIO_write_low(&PORTC, LED2);
		GPIO_write_low(&PORTC, LED3);
		GPIO_write_low(&PORTC, LED4);
	}
	else
	{
		GPIO_write_low(&PORTC, LED1);
		GPIO_write_low(&PORTC, LED2);
		GPIO_write_low(&PORTC, LED3);
		GPIO_write_low(&PORTC, LED4);
	}
	
}