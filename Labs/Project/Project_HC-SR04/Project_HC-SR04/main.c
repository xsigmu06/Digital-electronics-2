/*
 * Project_HC-SR04.c
 *
 * Created: 23/11/2020 15:13:47
 * Author : sigmu
 */ 

/* Define CPU frequency 16 MHz ---------------------------------------*/
#ifndef  F_CPU
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
#define trigFront   PB2
#define echoFront   PB3
#define trigBack    PB4
#define echoBack	PB5

#define LED1	PC1
#define LED2	PC2
#define LED3	PC3
#define LED4	PC4

#define LED5	PD3
#define LED6	PD2
#define LED7	PD1
#define LED8	PD0


#define speaker	PC5

/* Variables ---------------------------------------------------------*/
char lcd_string[5];     //"Front = 400 cm";

volatile float distFront = 0.0;  // max dist: 400 cm
volatile float distBack = 0.0;

uint8_t TIM2_off = 0;   

// FSM declaration
typedef enum {              
    STATE_TRIG,
    STATE_ECHO_MEAS,
} state_t;

void displayResult(volatile float DistanceFront, volatile float DistanceBack);
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
	
	// LEDsfront
	GPIO_config_output(&DDRC,LED1);
	GPIO_write_low(&PORTC,LED1);
	GPIO_config_output(&DDRC,LED2);
	GPIO_write_low(&PORTC,LED2);
	GPIO_config_output(&DDRC,LED3);
	GPIO_write_low(&PORTC,LED3);
	GPIO_config_output(&DDRC,LED4);
	GPIO_write_low(&PORTC,LED4);
	
	// LEDsback
	GPIO_config_output(&DDRD,LED5);
	GPIO_write_low(&PORTD,LED5);
	GPIO_config_output(&DDRD,LED6);
	GPIO_write_low(&PORTD,LED6);	
	GPIO_config_output(&DDRD,LED7);
	GPIO_write_low(&PORTD,LED7);
	GPIO_config_output(&DDRD,LED8);
	GPIO_write_low(&PORTD,LED8); 
	 
     // speaker
    GPIO_config_output(&DDRC, speaker);
    GPIO_write_low(&PORTC, speaker);
     
    // Initialize LCD display
    lcd_init(LCD_DISP_ON);
    lcd_gotoxy(0, 0);
    lcd_puts("Front: Standby..");
    lcd_gotoxy(0, 1);
    lcd_puts(" Back: Standby..");
    
    // Initialize UART to asynchronous, 8N1, 9600
    uart_init(UART_BAUD_SELECT(9600, F_CPU));
    uart_puts("Welcome.\n\n");
    //uart_puts("Please, fasten your seatbelt.\n\n");

    // Configure 8-bit Timer/Counter0
    // Enable interrupt and set the overflow prescaler to 16 us
    // Used for measuring and calculating distance
    TIM0_overflow_16us();
    TIM0_overflow_interrupt_enable();
    
    // Timer0 1us overflow interrupt
    // F_CPU/freq*2*N -1
    //OCR0A = 127;    
    //TIM0_CTC();
    //TIM0_overflow_COMPA();
	//TIM0_overflow_16us();
	


    // Configure 16-bit Timer/Counter1
    // Enable interrupt and set the overflow prescaler to 262 ms
    // Used for displaying result on LCD, uart, LEDs
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

/* Interrupt service routines ----------------------------------------*/
/**
 * ISR starts when Timer/Counter0 overflows. 
 * 
 */
//ISR(TIMER0_OVF_vect)
ISR(TIMER0_OVF_vect)
{
    static uint16_t number_of_overflows = 0;
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
            if (number_of_overflows <= 65000)  // 60 ms
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
                distFront = (float)lenFront * 0.017 * 16.0; // 16 us
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
 * 
 */
ISR(TIMER1_OVF_vect)
{
    TIM2_overflow_4ms();
    static uint16_t smallerDist = 0;
    
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
    
	
	
	// change frequency of speaker tone with TIM2 prescaler
	// change frequency of disabling TIM2
	if(smallerDist <= 15)
	{	
		TIM2_off = 200;
		TIM2_overflow_2ms();
		TIM2_overflow_interrupt_enable();
	}
	else if(smallerDist <= 50)
	{		
		TIM2_off = 50;
		TIM2_overflow_4ms();
		TIM2_overflow_interrupt_enable();
	}
	else if(smallerDist <= 100)
	{
		TIM2_off = 10;
		TIM2_overflow_4ms();
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
	
	// Display on LEDs, Front

	if(distFront <= 15)
	{
		GPIO_write_high(&PORTC, LED1);
		GPIO_write_high(&PORTC, LED2);
		GPIO_write_high(&PORTC, LED3);
		GPIO_write_high(&PORTC, LED4);
	}
	else if(distFront <= 50)
	{
		GPIO_write_high(&PORTC, LED1);
		GPIO_write_high(&PORTC, LED2);
		GPIO_write_high(&PORTC, LED3);
		GPIO_write_low(&PORTC, LED4);

	}
	else if(distFront <= 100)
	{
		GPIO_write_high(&PORTC, LED1);
		GPIO_write_high(&PORTC, LED2);
		GPIO_write_low(&PORTC, LED3);
		GPIO_write_low(&PORTC, LED4);
	}
	else if(distFront <= 125)
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
		       
    // Display on LEDs, Back 

	if(distBack <= 15)
    {
        GPIO_write_high(&PORTD, LED5);
        GPIO_write_high(&PORTD, LED6);
        GPIO_write_high(&PORTD, LED7);
        GPIO_write_high(&PORTD, LED8);
    }
    else if(distBack <= 50)
    {
        GPIO_write_high(&PORTD, LED5);
        GPIO_write_high(&PORTD, LED6);
        GPIO_write_high(&PORTD, LED7);
        GPIO_write_low(&PORTD, LED8);

    }
    else if(distBack <= 100)
    {
        GPIO_write_high(&PORTD, LED5);
        GPIO_write_high(&PORTD, LED6);
        GPIO_write_low(&PORTD, LED7);
        GPIO_write_low(&PORTD, LED8);
    }
    else if(distBack <= 125)
    {
        GPIO_write_high(&PORTD, LED5);
        GPIO_write_low(&PORTD, LED6);
        GPIO_write_low(&PORTD, LED7);
        GPIO_write_low(&PORTD, LED8);
    }
    else
    {
        GPIO_write_low(&PORTD, LED5);
        GPIO_write_low(&PORTD, LED6);
        GPIO_write_low(&PORTD, LED7);
        GPIO_write_low(&PORTD, LED8);
    }
	

	
	
}

ISR(TIMER2_OVF_vect)
{
    //GPIO_toggle(&PORTC, speaker);
    static uint8_t counter = 0;
    counter++;

    GPIO_toggle(&PORTC, speaker);

    if(counter >= TIM2_off)
    {
        TIM2_overflow_interrupt_disable();
        counter = 0;
    }
}

// clears LCD
void lcd_clear(uint8_t pos)
{
    lcd_gotoxy(7, pos);
    lcd_puts("                ");   // clear all 16 symbols in a row
}

// displays result on LCD, uart
void displayResult(volatile float DistanceFront, volatile float DistanceBack)
{    
    static float dist = 0.0, diffFront = 0.0, diffBack = 0.0;
    uint8_t pos = 0;
    char side[5]= "Front"; 
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
        //sprintf(lcd_string, "%0.2f", dist);
        
    
	    // Write result on LCD screen
	    if(dist < 2)  // Distance smaller than minimum possible
	    {
		    lcd_clear(pos);
		    lcd_gotoxy(7, pos);                                   
		    lcd_puts("<2 cm");
			
            uart_puts(side);		    
		    uart_puts(" object too close.");
		    uart_puts("\n");   
	    }
	    else if(dist <= 400)
	    {
		    lcd_clear(pos);
		    lcd_gotoxy(8, pos);		    
		    lcd_puts(lcd_string);
            lcd_puts("  cm");                       
			
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
		    lcd_puts(">400 cm");
					    
            uart_puts(side);
		    uart_puts(" object too far.");     
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

