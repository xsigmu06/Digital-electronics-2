/*
 * Project_HC-SR04.c
 *
 * This project implements 2 HC-SR04 modules to MCU Atmega328P. The 
 * measured distance is indicated via LCD panel, UART, LEDs and
 * a speaker. All timers are used for interrupt OVF.
 *
 * Created: 23/11/2020 15:13:47
 * Author : Jan Sigmund, Michal Švento
 */ 

/* Define CPU frequency 16 MHz ---------------------------------------*/
#ifndef F_CPU
#define F_CPU 16000000
#endif

/* Includes ----------------------------------------------------------*/
#include <stdlib.h>         // C library. Needed for conversion function
#include <stdbool.h>        // C library for boolean values
#include <string.h>         // C library for working with strings (strcpy)

#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC
#include <util/delay.h>     // Functions for busy-wait delay loops

#include "gpio.h"			// General purpose input output library
#include "timer.h"          // Timer library for AVR-GCC
#include "lcd.h"            // Peter Fleury's LCD library
#include "uart.h"           // Peter Fleury's UART library


/* Defines -----------------------------------------------------------*/
// Pins for HC-SR04 modules
#define trigFront   PB2
#define echoFront   PB3
#define trigBack    PB4
#define echoBack	PB5

// Pins for front LED bars
#define fLED1	PC1
#define fLED2	PC2
#define fLED3	PC3
#define fLED4	PC4
// Pins for back LED bars
#define bLED5	PD3
#define bLED6	PD2
#define bLED7	PD1
#define bLED8	PD0

// Pin for speaker
#define speaker	PC5

/* Variables ---------------------------------------------------------*/
// Char array to feed into LCD display
char lcd_string[5];  

// Floats for calculating distance
volatile float distFront = 0.0;  // max dist: 400 cm
volatile float distBack = 0.0;

// Variable for changing delay of TIM2_interrupt_disable
// Used for different signal width for speaker
uint8_t TIM2_off = 0;   

// FSM declaration
typedef enum {              
    STATE_TRIG,
    STATE_ECHO_MEAS,
} state_t;

/* Function declarations ---------------------------------------------*/
// Displays distance on LCD and UART
void displayResult(volatile float DistanceFront, volatile float DistanceBack);
// Clears LCD display
void lcd_clear(uint8_t pos);

/* Function definitions ----------------------------------------------*/
int main(void)
{
    // Output pins (Trigger)
	GPIO_config_output(&DDRB, trigFront);
	GPIO_write_low(&PORTB, trigFront);
	GPIO_config_output(&DDRB, trigBack);
	GPIO_write_low(&PORTB, trigBack);
	
	// Input pins (Echo)
	GPIO_config_input_nopull(&DDRB, echoFront);
	GPIO_config_input_nopull(&DDRB, echoBack);
	
	// Front LEDs
	GPIO_config_output(&DDRC, fLED1);	
	GPIO_config_output(&DDRC, fLED2);
    GPIO_config_output(&DDRC, fLED3);
    GPIO_config_output(&DDRC, fLED4);
    GPIO_write_low(&PORTC, fLED1);
	GPIO_write_low(&PORTC, fLED2    );
    GPIO_write_low(&PORTC, fLED3);
    GPIO_write_low(&PORTC, fLED4);

	// Back LEDs
	GPIO_config_output(&DDRD, bLED5);	
	GPIO_config_output(&DDRD, bLED6);
    GPIO_config_output(&DDRD, bLED7);
    GPIO_config_output(&DDRD, bLED8);
    GPIO_write_low(&PORTD, bLED8); 
    GPIO_write_low(&PORTD, bLED7);
    GPIO_write_low(&PORTD, bLED5);
	GPIO_write_low(&PORTD, bLED6);	

    // Speaker
    GPIO_config_output(&DDRC, speaker);
    GPIO_write_low(&PORTC, speaker);
     
    // Initialize LCD display
    lcd_init(LCD_DISP_ON);
    lcd_gotoxy(0, 0);
    lcd_puts("Front:       cm");
    lcd_gotoxy(0, 1);
    lcd_puts(" Back:       cm");
    
    // Initialize UART to asynchronous, 8N1, 9600
    uart_init(UART_BAUD_SELECT(9600, F_CPU));
    uart_puts("Welcome.\n\n");
    //uart_puts("Please, fasten your seatbelt.\n\n");

    // Configure 8-bit Timer/Counter0
    // Enable interrupt and set the overflow prescaler to 16 us
    // Used for measuring and calculating distance
    TIM0_overflow_16us();
    TIM0_overflow_interrupt_enable();

    // Configure 16-bit Timer/Counter1
    // Enable interrupt and set the overflow prescaler to 262 ms
    // Used for displaying result on LCD, uart, LEDs
    TIM1_overflow_262ms();
    TIM1_overflow_interrupt_enable();    
    
    // Timer/Counter2 is used for speaker
    // It is being configured inside Timer/Counter2  

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
 * ISR starts when Timer/Counter0 overflows. 
 * Timer0 OVF controls sending 10 us Trigger signal and 
 * measurement of Echo signal.
 */
ISR(TIMER0_OVF_vect)
{
    static uint16_t number_of_overflows = 0;
    static uint16_t lenFront = 0;
    static uint16_t lenBack = 0;
    static state_t state= STATE_TRIG;
    
    number_of_overflows++;
    
    switch(state)
    {
        // Send 10 us signals to Trigger pins
        case STATE_TRIG:            
            GPIO_write_high(&PORTB, trigFront);
            GPIO_write_high(&PORTB, trigBack);
            _delay_us(10);
            GPIO_write_low(&PORTB, trigFront);
            GPIO_write_low(&PORTB, trigBack);
            
            state = STATE_ECHO_MEAS;

            break;
        // Measure length of H Echo signals
        // Recommended to be 60 ms long process
        case STATE_ECHO_MEAS:
            if (number_of_overflows <= 3750)  // 60 ms
            {
                if (GPIO_read(&PINB, echoFront))
                {
                    lenFront++;
                }
                if (GPIO_read(&PINB, echoBack))
                {
                    lenBack++;
                }
            }
            else
            {
                // Calculate distance in cm
                number_of_overflows = 0;
                distFront = (float)lenFront * 0.017 * 16.0; // 16.0 us
                distBack = (float)lenBack * 0.017 * 16.0;
                lenFront = 0;
                lenBack = 0;
                state = STATE_TRIG;
            } 
            
            break;   
            
        default:
            state=STATE_TRIG;        
    }
}

/**
 * ISR starts when Timer/Counter1 overflows.
 * Timer1 OVF controls displaying results
 * on LCD, UART, LEDs and signalizing
 * closing distance with speaker
 */
ISR(TIMER1_OVF_vect)
{
    // Default prescaler for lower tone frequency of speaker
    TIM2_overflow_4ms();
    // For comparing smaller distance of 2 modules
    static uint16_t smallerDist = 0;
    
    // Function call to display on LCD, uart
    displayResult(distFront, distBack);
    
    // Decide what module the object is closer to
    if(distFront < distBack)
    {
        smallerDist = distFront;
    }
    else
    {
        smallerDist = distBack;
    }
	
    // Generate tone based on distance	
	if(smallerDist <= 15)
	{	
        // Change frequency of disabling TIM2 interrupt
		TIM2_off = 250;
        // Increase frequency of speaker tone
		TIM2_overflow_2ms();
		TIM2_overflow_interrupt_enable();
	}
	else if(smallerDist <= 50)
	{		
		TIM2_off = 50;
		TIM2_overflow_interrupt_enable();
	}
	else if(smallerDist <= 100)
	{
		TIM2_off = 10;
		TIM2_overflow_interrupt_enable();
	}
	else if(smallerDist <= 125)
	{		
		TIM2_off = 5;
		TIM2_overflow_interrupt_enable();
	}
	else
	{		
		TIM2_overflow_interrupt_disable();
	}
	
	// Turn front LEDs on/off based on distance
	if(distFront <= 15)
	{
		GPIO_write_high(&PORTC, fLED1);
		GPIO_write_high(&PORTC, fLED2);
		GPIO_write_high(&PORTC, fLED3);
		GPIO_write_high(&PORTC, fLED4);
	}
	else if(distFront <= 50)
	{
		GPIO_write_high(&PORTC, fLED1);
		GPIO_write_high(&PORTC, fLED2);
		GPIO_write_high(&PORTC, fLED3);
		GPIO_write_low(&PORTC, fLED4);

	}
	else if(distFront <= 100)
	{
		GPIO_write_high(&PORTC, fLED1);
		GPIO_write_high(&PORTC, fLED2);
		GPIO_write_low(&PORTC, fLED3);
		GPIO_write_low(&PORTC, fLED4);
	}
	else if(distFront <= 125)
	{
		GPIO_write_high(&PORTC, fLED1);
		GPIO_write_low(&PORTC, fLED2);
		GPIO_write_low(&PORTC, fLED3);
		GPIO_write_low(&PORTC, fLED4);
	}
	else
	{
		GPIO_write_low(&PORTC, fLED1);
		GPIO_write_low(&PORTC, fLED2);
		GPIO_write_low(&PORTC, fLED3);
		GPIO_write_low(&PORTC, fLED4);
	}
	
    // Turn back LEDs on/off based on distance	       
	if(distBack <= 15)
    {
        GPIO_write_high(&PORTD, bLED5);
        GPIO_write_high(&PORTD, bLED6);
        GPIO_write_high(&PORTD, bLED7);
        GPIO_write_high(&PORTD, bLED8);
    }
    else if(distBack <= 50)
    {
        GPIO_write_high(&PORTD, bLED5);
        GPIO_write_high(&PORTD, bLED6);
        GPIO_write_high(&PORTD, bLED7);
        GPIO_write_low(&PORTD, bLED8);

    }
    else if(distBack <= 100)
    {
        GPIO_write_high(&PORTD, bLED5);
        GPIO_write_high(&PORTD, bLED6);
        GPIO_write_low(&PORTD, bLED7);
        GPIO_write_low(&PORTD, bLED8);
    }
    else if(distBack <= 125)
    {
        GPIO_write_high(&PORTD, bLED5);
        GPIO_write_low(&PORTD, bLED6);
        GPIO_write_low(&PORTD, bLED7);
        GPIO_write_low(&PORTD, bLED8);
    }
    else
    {
        GPIO_write_low(&PORTD, bLED5);
        GPIO_write_low(&PORTD, bLED6);
        GPIO_write_low(&PORTD, bLED7);
        GPIO_write_low(&PORTD, bLED8);
    }
}
/**
 * ISR starts when Timer/Counter2 overflows.
 * Timer2 OVF controls tone frequency of 
 * speaker
 */
ISR(TIMER2_OVF_vect)
{
    static uint8_t counter = 0;
    counter++;

    GPIO_toggle(&PORTC, speaker);

    // TIM2_off is changed in TIM1 OVF
    if(counter >= TIM2_off)
    {
        TIM2_overflow_interrupt_disable();
        counter = 0;
    }
}

// Function definition to clear LCD display
void lcd_clear(uint8_t pos)
{
    lcd_gotoxy(7, pos);
    lcd_puts("     ");   // clear distance symbols
}

// Function definition to displays result on LCD and uart
void displayResult(volatile float DistanceFront, volatile float DistanceBack)
{    
    static float dist = 0.0, diffFront = 0.0, diffBack = 0.0;
    // For changing position of cursor to write on LCD's two rows 
    uint8_t pos = 0;
    // For changing UART output based on module
    char side[5]= "Front"; 
    // For deciding whether measured distance changed
    bool changed = false;
    
    // Decide which of the 2 modules measured new distance
     if (diffFront != DistanceFront)
    {	
        pos = 0;
        diffFront = DistanceFront;
        dist = DistanceFront;
        strcpy(side, "Front");  // for assigning to array type
        changed = true;
    }
    if (diffBack != DistanceBack)
    {  
        pos = 1;
        diffBack = DistanceBack;
        dist = DistanceBack;
        strcpy(side, "Back");
        changed = true;
    }    
    
    // If one distance changed write out the result
    if (changed)
    {
        changed = false;
        itoa(dist, lcd_string, 10);
            
	    // Write result on LCD screen
	    if(dist < 2)  // Distance smaller than minimum possible
	    {
		    lcd_clear(pos);
		    lcd_gotoxy(7, pos);                                   
		    lcd_puts("<2");
			
            uart_puts(side);		    
		    uart_puts(" object too close.");
		    uart_puts("\n");   
	    }       
	    else if(dist <= 400)
	    {
		    lcd_clear(pos);
		    lcd_gotoxy(8, pos);		    
		    lcd_puts(lcd_string);  
			
            uart_puts(side);		    
		    uart_puts(" distance: ");
		    uart_puts(lcd_string);
            uart_puts("  cm");
		    uart_puts("\n");    
	    }
	    else if(dist > 400)   // Distance greater than maximum possible
	    {
		    lcd_clear(pos);
		    lcd_gotoxy(7, pos);
		    lcd_puts(">400");
					    
            uart_puts(side);
		    uart_puts(" object too far.");     
		    uart_puts("\n"); 
	    }      
    }             
}

