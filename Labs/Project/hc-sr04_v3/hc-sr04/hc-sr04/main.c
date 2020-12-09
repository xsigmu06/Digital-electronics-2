/*
 * hc-sr04.c
 *
 * Created: 18. 11. 2020 12:04:28
 * Authors : Jan Sigmund, Michal Švento
 */ 
/* Definees ----------------------------------------------------------*/
#define TRIGGERREAR PB2
#define ECHOREAR	PB3
#define TRIGGERBACK PB4
#define ECHOBACK	PB5
#define LED1	PC1
#define LED2	PC2
#define LED3	PC3
#define LED4	PC4
#define SPEAKER	PC5

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
char lcd_stringrear[5]="    ";
char lcd_stringback[5]="    ";
uint16_t distancerear;
uint16_t distanceback;
uint16_t smallerdist;






int main(void)
{	
	lcd_init(LCD_DISP_ON);
	lcd_gotoxy(1,0);
	lcd_puts("Front: ");
	lcd_gotoxy(1,1);
	lcd_puts("Back: ");
	
	
	GPIO_config_output(&DDRB,TRIGGERREAR);
	GPIO_write_low(&PORTB,TRIGGERREAR);
	GPIO_config_input_nopull(&DDRB,ECHOREAR);
	GPIO_config_output(&DDRB,TRIGGERREAR);
	GPIO_write_low(&PORTB,TRIGGERREAR);
	GPIO_config_input_nopull(&DDRB,ECHOREAR);
	

	
	GPIO_config_output(&DDRC,LED1);
	GPIO_write_low(&PORTC,LED1);
	GPIO_config_output(&DDRC,LED2);
	GPIO_write_low(&PORTC,LED2);
	GPIO_config_output(&DDRC,LED3);
	GPIO_write_low(&PORTC,LED3);
	GPIO_config_output(&DDRC,LED4);
	GPIO_write_low(&PORTC,LED4);
	
	GPIO_config_output(&DDRC,SPEAKER);
	
	
	
	
	
	TIM0_overflow_16u();
	TIM0_overflow_interrupt_enable();
	
	TIM1_overflow_262ms();
	TIM1_overflow_interrupt_enable();

	TIM2_overflow_16ms();
	TIM2_overflow_interrupt_enable();
	
	//uart_init(UART_BAUD_SELECT(115200,F_CPU));
	
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
uint16_t isbigger (uint16_t distancerear, uint16_t distanceback)
{
	if (distancerear<=distanceback)
	{
		smallerdist=distancerear;
	}
	else
	{
		smallerdist=distanceback;
	}
	
	return smallerdist;
}


ISR(TIMER0_OVF_vect)
{	
	
	static uint16_t number_of_overflows=0;
	static uint16_t lenghtrear=0;
	static uint16_t lenghtback=0;
	
	
	
	
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
					GPIO_write_high(&PORTB,TRIGGERREAR);
					GPIO_write_high(&PORTB,TRIGGERBACK);
					state=STATE_ECHODETECT;
					lenghtrear=0;
					lenghtback=0;
				} 

				break;	
				
		case STATE_ECHODETECT:
				GPIO_write_low(&PORTB,TRIGGERREAR);
				GPIO_write_low(&PORTB,TRIGGERBACK);
				if (number_of_overflows<=4062)
				{
					if (GPIO_read(&PINB,ECHOREAR))
					{
						lenghtrear++;
					}
					if (GPIO_read(&PINB,ECHOBACK))
					{
						lenghtback++;
					}
					
				} 
				else
				{
					state=STATE_ECHOCOUNT;
					
				}
				break;
		case STATE_ECHOCOUNT:
		
				if (lenghtrear<8)
				{
					
				}
				else
				{
					distancerear=lenghtrear*16*0.017;
					itoa(distancerear,lcd_stringrear,10);

				}
				
				if (lenghtback<8)  // smaller than 2cm
				{
				
				}
				else
				{
					distanceback=lenghtback*16*0.017;
					itoa(distanceback,lcd_stringback,10);
				}
				

				smallerdist=isbigger(lenghtrear,lenghtback);
			
				lenghtrear=0;
				lenghtback=0;
				number_of_overflows=0;
				state=STATE_PULSE;

				
		default:
				state=STATE_PULSE;
				
		}
	
	
	
}

ISR(TIMER1_OVF_vect)
{
		//uart_puts(lcd_stringback);
		//uart_puts("\n");
		//uart_puts(lcd_stringrear);
		//uart_puts(" ");
	
			lcd_gotoxy(8,0);
			lcd_puts("    ");
			lcd_gotoxy(8,0);
			lcd_puts(lcd_stringrear);
			lcd_gotoxy(8,1);
			lcd_puts("    ");
			lcd_gotoxy(8,1);
			lcd_puts(lcd_stringback);
		
		
		
			
	// Turn on LEDS
	if(smallerdist <= 10)
	{
		GPIO_write_high(&PORTC, LED1);
		GPIO_write_high(&PORTC, LED2);
		GPIO_write_high(&PORTC, LED3);
		GPIO_write_high(&PORTC, LED4);
	}
	else if(smallerdist<= 50)
	{
		GPIO_write_high(&PORTC, LED1);
		GPIO_write_high(&PORTC, LED2);
		GPIO_write_high(&PORTC, LED3);
		GPIO_write_low(&PORTC, LED4);
	}
	else if(smallerdist <= 100)
	{
		GPIO_write_high(&PORTC, LED1);
		GPIO_write_high(&PORTC, LED2);
		GPIO_write_low(&PORTC, LED3);
		GPIO_write_low(&PORTC, LED4);
	}
	else if(smallerdist <= 200)
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
			
		
		
		
		
	/*if (smallerdist<10)
	{
		TIM1_overflow_33ms();
		GPIO_toggle(&PORTC,SPEAKER);
	}
	
	else if(smallerdist<50)
	{
		
	}
	else if(smallerdist<100)
	{
		
	}
	*/
}



ISR(TIMER2_OVF_vect)
{
	


}


