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
typedef enum {              // FSM declaration
    	STATE_PULSE,
    	STATE_ECHO_DETECT,
    	STATE_ECHO_MEAS,
	} state_t;

int main(void)
{	
	// Initialize LCD display
	lcd_init(LCD_DISP_ON);
	lcd_gotoxy(1,0);
    
    // Initialize UART to asynchronous, 8N1, 9600
    uart_init(UART_BAUD_SELECT(9600, F_CPU));
	
	GPIO_config_output(&DDRB,TRIGGER);
	GPIO_write_low(&PORTB,TRIGGER);
	
	GPIO_config_input_pullup(&DDRB,ECHO);
	
	//  Timer0 1us overflow interrupt 
	//  OCR0A = F_CPU/freq*2*N -1
	OCR0A=127;
	TIM0_overflow_16u();
	TIM0_CTC();
	TIM0_compare_A();
	
	lcd_init(LCD_DISP_ON);
	
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
ISR(TIMER0_COMPA_vect){
	
	static uint16_t counter = 0;    // counter for interrupts
	static uint16_t echo_dur = 0;   // duration of ECHO
    static uint16_t distance = 0;      // measured distance
    static uint16_t dif_distance = 0;  // for checking if distance changed
	char lcd_string[5]="     ";

	static state_t state= STATE_PULSE;  // Current state of the FSM
       
	// FSM
	switch(state)
	{
	    // Send pulse of length 10 us to TRIGGER pin
	    case STATE_PULSE:
            counter++;
            		
			if(counter <= 10)
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
                
                if(dif_distance != distance)    // if next meas. distance is different
                {
                    // Write result on LCD screen
                    if(distance < 2)  // distance smaller than minimum possible                     // pri nule se zastavi - proc?
                    {
                        lcd_clear();
                        lcd_gotoxy(0, 0);
                        lcd_puts("Too small");
                        
                        /*uart_puts("Distance too small.");
                        uart_puts("\n");    // \n ... newline*/
                    }
                    else if(distance < 400)
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
                        
                       /* uart_puts("Somethings wrong");
                        uart_puts("\n");    // \n ... newline*/
                    }
                }
                 
                distance = dif_distance;
                state=STATE_PULSE;
            }
            
		break;

		// If something unexpected happens then move to PULSE
		default:
			state = STATE_PULSE;
		break;
	}
}



