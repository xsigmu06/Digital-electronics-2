/*
 * Project_HC-SR04.c
 *
 * Created: 23/11/2020 15:13:47
 * Author : sigmu
 */ 

/* Includes ----------------------------------------------------------*/
#include <stdlib.h>         // C library. Needed for conversion function
#include <stdbool.h>        // C library for boolean values
#include <string.h>
#include <avr/io.h>         // AVR device-specific IO definitions
#include <avr/interrupt.h>  // Interrupts standard C library for AVR-GCC

#include "gpio.h"			// General purpose input output library
#include "timer.h"          // Timer library for AVR-GCC
#include "lcd.h"            // Peter Fleury's LCD library
#include "uart.h"           // Peter Fleury's UART library


/* Defines -----------------------------------------------------------*/
#ifndef  F_CPU
#define F_CPU 16000000
#endif

#include <util/delay.h>     // Functions for busy-wait delay loops

#define trigFront   PB2
#define echoFront   PB3
#define trigBack    PB4
#define echoBack	PB5

#define LED1	PC1
#define LED2	PC2
#define LED3	PC3
#define LED4	PC4

#define speaker	PC5

/* Variables ---------------------------------------------------------*/
char lcd_string[15] = "Front = 400 cm";

float distFront = 0;  // max dist: 400 cm
float distBack = 0;
uint16_t smallerDist = 0;
uint16_t diffFront = 0, diffBack = 0;

// FSM declaration
typedef enum {              
    STATE_TRIG,
    STATE_ECHO_MEAS,
    //STATE_ECHO_COUNT,
} state_t;

void displayResult(uint16_t DistanceFront,  uint16_t DistanceBack);
void lcd_clear(uint16_t pos);

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
     
     // LEDs
     GPIO_config_output(&DDRC,LED1);
     GPIO_write_low(&PORTC,LED1);
     GPIO_config_output(&DDRC,LED2);
     GPIO_write_low(&PORTC,LED2);
     GPIO_config_output(&DDRC,LED3);
     GPIO_write_low(&PORTC,LED3);
     GPIO_config_output(&DDRC,LED4);
     GPIO_write_low(&PORTC,LED4);
     
     // speaker
     GPIO_config_output(&DDRC, speaker);
     GPIO_write_low(&PORTC, speaker);
     
    // Initialize LCD display
    lcd_init(LCD_DISP_ON);
    lcd_gotoxy(0, 0);
    lcd_puts("Front: ");
    lcd_gotoxy(0, 1);
    lcd_puts(" Back: ");

    // Configure 8-bit Timer/Counter0
    // Enable interrupt and set the overflow prescaler to 16 us
    // Used for displaying result on LCD, uart, LEDs
    TIM0_overflow_16us();
    TIM0_overflow_interrupt_enable();

    // Configure 16-bit Timer/Counter1
    // Enable interrupt and set the overflow prescaler to 262 ms
    // Used for displaying result on LCD, uart, LEDs
    TIM1_overflow_262ms();
    TIM1_overflow_interrupt_enable();    

    // Initialize UART to asynchronous, 8N1, 9600
    uart_init(UART_BAUD_SELECT(9600, F_CPU));

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
 * 
 */
ISR(TIMER0_OVF_vect)
{
    static uint16_t number_of_overflows=0;
    static uint16_t lenFront = 0;
    static uint16_t lenBack = 0;
    static state_t state= STATE_TRIG;
    
    number_of_overflows++;
    
    switch(state)
    {
        case STATE_TRIG:            
            GPIO_write_high(&PORTB, trigFront);
            GPIO_write_high(&PORTB, trigBack);
            _delay_us(10);
            GPIO_write_low(&PORTB, trigFront);
            GPIO_write_low(&PORTB, trigBack);
            
            state = STATE_ECHO_MEAS;

            break;
        
        case STATE_ECHO_MEAS:
            if (number_of_overflows<=3750)  // 60 ms
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
                number_of_overflows = 0;
                distFront = lenFront * 0.017 * 16; // 16 us
                distBack = lenBack * 0.017 * 16;
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
 * 
 */
ISR(TIMER1_OVF_vect)
{
    GPIO_toggle(&PORTC, speaker);
    // Display on LCD, uart
    displayResult(distFront, distBack);
    
    if(distFront < distBack)
    {
        smallerDist = distFront;
    }
    else
    {
        smallerDist = distBack;
    }
            
    // Display on LEDs, change frequency of speaker tone with TIM2 prescaler        
    if(smallerDist <= 50)
    {
        GPIO_write_high(&PORTC, LED1);
        GPIO_write_high(&PORTC, LED2);
        GPIO_write_high(&PORTC, LED3);
        GPIO_write_high(&PORTC, LED4);
        
        TIM2_overflow_1ms();    
        TIM2_overflow_interrupt_enable();    
    }
    else if(smallerDist <= 75)
    {
        GPIO_write_high(&PORTC, LED1);
        GPIO_write_high(&PORTC, LED2);
        GPIO_write_high(&PORTC, LED3);
        GPIO_write_low(&PORTC, LED4);
        
        TIM2_overflow_2ms();
        TIM2_overflow_interrupt_enable();   
    }
    else if(smallerDist <= 100)
    {
        GPIO_write_high(&PORTC, LED1);
        GPIO_write_high(&PORTC, LED2);
        GPIO_write_low(&PORTC, LED3);
        GPIO_write_low(&PORTC, LED4);
        
        TIM2_overflow_4ms();
        TIM2_overflow_interrupt_enable();   
    }
    else if(smallerDist <= 125)
    {
        GPIO_write_high(&PORTC, LED1);
        GPIO_write_low(&PORTC, LED2);
        GPIO_write_low(&PORTC, LED3);
        GPIO_write_low(&PORTC, LED4);
        
        /*TIM2_overflow_16ms(); // too low freq.
        TIM2_overflow_interrupt_enable(); */ 
        TIM2_overflow_interrupt_disable(); 
    }
    else
    {
        GPIO_write_low(&PORTC, LED1);
        GPIO_write_low(&PORTC, LED2);
        GPIO_write_low(&PORTC, LED3);
        GPIO_write_low(&PORTC, LED4);
        
        TIM2_overflow_interrupt_disable();
    }
}

ISR(TIMER2_OVF_vect)
{
    GPIO_toggle(&PORTC, speaker);
    /*static uint8_t counter = 0;
    counter++;
    
    if(counter >= 10)
    {
        GPIO_toggle(&PORTC, speaker);
        counter = 0;
    }*/
}

// clears LCD
void lcd_clear(uint16_t pos)
{
    lcd_gotoxy(7, pos);
    lcd_puts("                ");   // clear all 16 symbols in a row
}

// displays result on LCD, uart
void displayResult(uint16_t DistanceFront,  uint16_t DistanceBack)
{    
    uint16_t dist = 0;
    uint16_t pos = 0;
    char side[5]= "";
    bool changed = false;
    
    // if one distance changed
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
    
    if (changed)
    {
        changed = false;
        itoa(dist, lcd_string, 10);
    
	    // Write result on LCD screen
	    if(dist < 2)  // Distance smaller than minimum possible
	    {
		    lcd_clear(pos);
		    lcd_gotoxy(7, pos);                                   
		    lcd_puts(" < 2 cm");
			
            uart_puts(side);		    
		    uart_puts(" object too close.");
		    uart_puts("\n");   
	    }
	    else if(dist <= 400)
	    {
		    lcd_clear(pos);
		    lcd_gotoxy(7, pos);		    
		    lcd_puts(lcd_string);
            lcd_puts(" cm");                       
			
            uart_puts(side);		    
		    uart_puts(" distance: ");
		    uart_puts(lcd_string);
            uart_puts(" cm");
		    uart_puts("\n");    
	    }
	    else if(dist > 400)   // Distance greater than maximum possible
	    {
		    lcd_clear(pos);
		    lcd_gotoxy(0, pos);
		    lcd_puts(" > 400 cm");
					    
            uart_puts(side);
		    uart_puts(" distance is greater than 400 cm.");
		    uart_puts("\n"); 
	    }
	    else
	    {
		    lcd_clear(pos);
		    lcd_gotoxy(0, pos);
		    lcd_puts("Somethings wrong");
					    
		    uart_puts("Somethings wrong");
		    uart_puts("\n"); 
	    }          
    }             
}