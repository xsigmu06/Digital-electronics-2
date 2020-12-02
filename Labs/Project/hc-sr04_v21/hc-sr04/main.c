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
#include <stdlib.h>         // C library. Needed for conversion function
#include "gpio.h"			// General purpose input output library
#include "timer.h"          // Timer library for AVR-GCC
#include "lcd.h"            // Peter Fleury's LCD library
#include "uart.h"           // Peter Fleury's UART library

#ifndef F_CPU
#define F_CPU 16000000
#endif

/* Variables ---------------------------------------------------------*/
typedef enum {              // FSM declaration
    	STATE_PULSE,
    	STATE_ECHO_DETECT,
    	STATE_ECHO_MEAS,
	} state_t;
static uint16_t distance=0;      // measured distance


int main(void)
{	
    GPIO_config_output(&DDRC, LED1);
    GPIO_write_low(&PORTC, LED1);
    GPIO_config_output(&DDRC, LED2);
    GPIO_write_low(&PORTC, LED2);
    GPIO_config_output(&DDRC, LED3);
    GPIO_write_low(&PORTC, LED3);
    GPIO_config_output(&DDRC, LED4);
    GPIO_write_low(&PORTC, LED4);    
    
	// Initialize LCD display
	lcd_init(LCD_DISP_ON);
    
    // Initialize UART to asynchronous, 8N1, 9600
    //uart_init(UART_BAUD_SELECT(9600, F_CPU));
	
    // Set trigger pin to output, write low
	GPIO_config_output(&DDRB,TRIGGER);
	GPIO_write_low(&PORTB,TRIGGER);
	
    // Set echo pin to input, add pullup resistor
	GPIO_config_input_pullup(&DDRB,ECHO);
	
    // Define prescaler
	TIM0_overflow_16u();
    
    //  OCR0A = F_CPU/freq*2*N -1
    TCCR0A |= (1<<WGM01);   // CTC mode
    TIMSK0 |= (1<<OCIE0A);  // Output Compare Match A Interrupt Enable
    OCR0A = 7;
    
    TIM1_overflow_262ms();
    
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

void lcd_clear()
{
    lcd_gotoxy(0, 0);
    lcd_puts("                ");   // clear all 16 symbols in a row
}

/* Interrupt service routines ----------------------------------------*/
/**
 * ISR starts when ... Update Finite State Machine and ...
 * 
 */
ISR(TIMER0_COMPA_vect)
{
	
	static uint16_t counter = 0;    // counter for interrupts
	static uint16_t echo_dur = 0;   // duration of ECHO
	char lcd_string[5]="     ";
	static uint16_t dif_distance=0;	

   

	static state_t state= STATE_PULSE;  // Current state of the FSM
  
  
  
	// FSM
	switch(state)
	{
	    // Send pulse of length 10 us to TRIGGER pin
	    case STATE_PULSE:
            counter++;
            		
			if(counter < 10)
			{
				GPIO_write_high(&PORTB,TRIGGER);				
			}
			else
			{
				GPIO_write_low(&PORTB,TRIGGER);
				counter = 0;
                state = STATE_ECHO_DETECT;                
			}	
            		
			break;
			
		// Detect echo High value, if not detected for 65 ms -> send pulse
		case STATE_ECHO_DETECT:
            counter++;
            
			if(bit_is_set(PINB,ECHO))
			{
                counter = 0;
                echo_dur++;
    			state = STATE_ECHO_MEAS;                
			}
            else if(counter >= 65000)
            {
                counter = 0;
                state = STATE_PULSE;
            }               
                          			
		break;

		// Measure echo High length
		case STATE_ECHO_MEAS:
			if(bit_is_set(PINB,ECHO))
			{
    			echo_dur++;
			}
            else
            {
                distance = echo_dur*0.17; // distance in mm
                echo_dur = 0;
                
                itoa(distance, lcd_string, 10);
				if (dif_distance != distance)
				{
				
					// Write result on LCD screen
					if(distance < 2)  // distance smaller than minimum possible                     // pri nule se zastavi - proc?
					{
					    lcd_clear();
					    lcd_gotoxy(0, 0);
					    lcd_puts("Too small");
					    
					    /*uart_puts("Distance too small.");
					    uart_puts("\n");    //*/
					}
					else if(distance <= 400)
					{
					    lcd_clear();
					    lcd_gotoxy(0, 0);
					    lcd_puts("Dist: ");
					    lcd_puts(lcd_string);                       
					    
					   /* uart_puts("Distance [mm]: ");
					    uart_puts(lcd_string);
					    uart_puts("\n");    // \n ... newline*/
					}
					else if(distance > 400)   // distance greater than maximum possible
					{
					    lcd_clear();
					    lcd_gotoxy(0, 0);
					    lcd_puts("No object or > 400");
					    
					    /*uart_puts("No object detected or distance > 400 cm.");
					    uart_puts("\n");    // \n ... newline*/
					}
					else // mohlo by nastat nebo ne?
					{
					    lcd_clear();
					    lcd_gotoxy(0, 0);
					    lcd_puts("Somethings wrong");
					    
					   /*uart_puts("Somethings wrong");
					   uart_puts("\n");    // \n ... newline*/
					}
				}
			dif_distance = distance;
			state=STATE_PULSE;
			}
		break;

		// If something unexpected happens then move to PULSE
		default:
			state = STATE_PULSE;
		break;
		
		}
	
}

ISR(TIMER1_OVF_vect)
{


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

