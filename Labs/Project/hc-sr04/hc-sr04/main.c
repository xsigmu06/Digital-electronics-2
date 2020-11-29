/*
 * hc-sr04.c
 *
 * Created: 18. 11. 2020 12:04:28
 * Authors : Jan Sigmund, Michal Švento
 */ 
/* Definees ----------------------------------------------------------*/
#define TRIGGER PB2
#define ECHO	PB3
#ifndef F_CPU
#define F_CPU 16000000
#endif

/* Includes ----------------------------------------------------------*/
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include "timer.h"          // Timer library for AVR-GCC
#include "lcd.h"            // Peter Fleury's LCD library
#include <stdlib.h>         // C library. Needed for conversion function
#include "uart.h"           // Peter Fleury's UART library
#include "gpio.h"			// General purpose input output library

/* Variables ---------------------------------------------------------*/








int main(void)
{	
	//initialize display
	//lcd_init(LCD_DISP_ON);
	//lcd_gotoxy(1,0);
	//lcd_puts("Distance");
	
	
	GPIO_config_output(&DDRB,TRIGGER);
	GPIO_write_low(&PORTB,TRIGGER);
	
	GPIO_config_input_pullup(&DDRB,ECHO);
	
	// Timer0 1us overflow interrupt 
	// F_CPU/freq*2*N -1
	OCR0A=127;
	TIM0_overflow_16u();
	TIM0_CTC();
	TIM0_overflow_COMPA();
	
	
	lcd_init(LCD_DISP_ON);
	
	
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

ISR(TIMER0_COMPA_vect){
	
	static uint16_t number_of_overflows=0;
	static float distance=0;
	static uint16_t length=0;
	char	lcd_string[5]="     ";
	number_of_overflows++;
	
	typedef enum {              // FSM declaration
		STATE_PULSE,
		STATE_ECHODETECT,
		STATE_ECHOCOUNT,
	} state_t;
	
	static state_t state= STATE_PULSE;
	
	
	// FSM
	switch (state)
	{
	    // Start trigger pulse
	    case STATE_PULSE:
			
			if(number_of_overflows<=10)
			{
				GPIO_write_high(&PORTB,TRIGGER);
				
			}
			else
			{
				GPIO_write_low(&PORTB,TRIGGER);
				state=STATE_ECHODETECT;
			}
			
			break;
			
			// Detect the echo impulse and count the length of echo
			case STATE_ECHODETECT:
				if (number_of_overflows<65000)
				{
					if (bit_is_set(PINB,ECHO))
					{
						length++;
					}
					
				} 
				else
				{
					state=STATE_ECHOCOUNT;
					
				}
			
			break;

			//
			case STATE_ECHOCOUNT:
			
				distance=length*0.17;
				
				if(distance<2)
				{
					
					lcd_gotoxy(0,0);
					lcd_puts("                ");
					lcd_gotoxy(0,0);
					lcd_puts("smaller than 2");
				}
				if (distance<400)
				{
					itoa(distance,lcd_string,10);
					lcd_gotoxy(0,0);
					lcd_puts("                ");
					lcd_gotoxy(0,0);
					lcd_puts("Dist");
					lcd_puts(lcd_string);
				}
				if (distance>400)
				{
					lcd_gotoxy(0,0);
					lcd_puts("                ");
					lcd_gotoxy(0,0);
					lcd_puts("No object or > 400");
				}
				
				number_of_overflows=0;
				length=0;
				state=STATE_PULSE;
			break;

			// If something unexpected happens then move to PULSE
			default:
				state = STATE_PULSE;
			break;
	}

}



