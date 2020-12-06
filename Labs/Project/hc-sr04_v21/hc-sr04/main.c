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
#define SPEAKER PD3

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
typedef enum //  FSM declaration
{              
    	STATE_PULSE,
    	STATE_ECHO_DETECT,
    	STATE_ECHO_MEAS,
} state_t;
static uint16_t distance = 0;      // measured distance
static uint16_t dif_distance = 0;  // for checking if distance changed
char lcd_string[4]="    ";  

int main(void)
{	         
	//  Initialize LCD display
	lcd_init(LCD_DISP_ON);
    
    //   Initialize UART to asynchronous, 8N1, 9600
    uart_init(UART_BAUD_SELECT(9600, F_CPU));
	
    //   Set trigger pin to output, write low
	GPIO_config_output(&DDRB,TRIGGER);
	GPIO_write_low(&PORTB,TRIGGER);
	
    //  Set echo pin to input, add pullup resistor
	GPIO_config_input_nopull(&DDRB,ECHO);
    
    //  Initialize LEDS
    GPIO_config_output(&DDRC,LED1);
    GPIO_write_low(&PORTC,LED1);
    GPIO_config_output(&DDRC,LED2);
    GPIO_write_low(&PORTC,LED2);
    GPIO_config_output(&DDRC,LED3);
    GPIO_write_low(&PORTC,LED3);
    GPIO_config_output(&DDRC,LED4);
    GPIO_write_low(&PORTC,LED4);
    
    // Initialize Speaker
    // TO DO
	
    //  timer/counter0 compare mode for measuring signals
	TIM0_overflow_16u();    // No prescaler
    TCCR0A |= (1<<WGM01);   // CTC mode
    TIMSK0 |= (1<<OCIE0A);  // Output Compare Match A Interrupt Enable
    //  OCR0A = F_CPU/freq*2*N -1
    OCR0A = 79;
    
    //  timer/counter1 overflow for LEDS, LCD
        
    TIM1_overflow_interrupt_enable();
    
	
    //  timer/counter2 for PWM controlled speaker
    // TO DO
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

// Clear LCD display
void lcd_clear()
{
    lcd_gotoxy(0, 0);
    lcd_puts("                ");
}

/* Interrupt service routines ----------------------------------------*/
/**
 * ISR starts when ... Update Finite State Machine and ...
 * 
 */
ISR(TIMER0_COMPA_vect){	
	static uint16_t counter = 0;    //  counter of interrupts
	static uint16_t echo_dur = 0;   //  duration of ECHO    	

	static state_t state= STATE_PULSE;  // Current state of the FSM
       
	//  FSM
	switch(state)
	{
	    //  Send pulse of length 10 us to TRIGGER pin
	    case STATE_PULSE:
            counter++;
            		
			if(counter < 1)
			{
				GPIO_write_high(&PORTB,TRIGGER);				
			}
			else
			{
				GPIO_write_low(&PORTB,TRIGGER);
                state = STATE_ECHO_DETECT;                
			}	
            		
			break;
			
		//  Detect echo High value, if not detected for 65 ms -> send pulse
		case STATE_ECHO_DETECT:
            counter++;
            
			if (counter<6500)
			{
				if(bit_is_set(PINB,ECHO))
				{
					echo_dur++;
					state = STATE_ECHO_DETECT;
				}	
			}
            else
            {
                counter = 0;
                state = STATE_ECHO_MEAS;
            }               
                          			
		break;

		//  Measure echo High length
		case STATE_ECHO_MEAS:


        distance = ((float)echo_dur*10)*0.17; // distance in mm
        echo_dur = 0;
                
        itoa(distance, lcd_string, 10);
        
        state=STATE_PULSE;
            
            
		break;

		//  If something unexpected happens then move to PULSE
		default:
			state = STATE_PULSE;
		break;
	}
}

/**
 * ISR starts when timer/counter1 overflows.
 * Writes distance on LCD, uart and turns on LEDS.
 */
ISR(TIMER2_OVF_vect)
{          
    if(dif_distance != distance)    // if next meas. distance is different
    {                    
        // Write result on LCD screen
        if(distance < 2)  // distance smaller than minimum possible                    
        {
            lcd_clear();
            lcd_gotoxy(0, 0);
            lcd_puts("Too small");
                        
            /*uart_puts("Distance too small.");
            uart_puts("\n");    // \n ... newline*/
        }
        else if(distance <= 400)
        {
            lcd_clear();
            lcd_gotoxy(0, 0);
            lcd_puts("Dist: ");
			
            lcd_puts(lcd_string);                       
                        
            /*uart_puts("Distance [mm]: ");
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
        else // ?
        {
            lcd_clear();
            lcd_gotoxy(0, 0);
            lcd_puts("Somethings wrong");
                        
            /* uart_puts("Somethings wrong");
            uart_puts("\n");    // \n ... newline*/
        }
        
        //  Turn on LEDS
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
    dif_distance = distance;
}
/**	TIMER for sound signalization
 *
 */
ISR(TIMER1_OVF_vect)
{
	//  Turn on LEDS
	if(distance <= 10)
	{
		GPIO_toggle(&PORTD,SPEAKER);
		TIM1_overflow_4ms();
	}
	else if(distance <= 50)
	{
		TIM1_overflow_33ms();
		GPIO_toggle(&PORTD,SPEAKER);
	}
	else if(distance <= 100)
	{
		GPIO_toggle(&PORTD,SPEAKER);
		TIM1_overflow_1s();
	}
	else if(distance <= 200)
	{
		GPIO_toggle(&PORTD,SPEAKER);
		TIM1_overflow_1s()
	}
	else
	{
		GPIO_toggle(&PORTD,SPEAKER);
		TIM2_stop();	
	}
}