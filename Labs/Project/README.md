# Ultrasonic sensor HC-SR04

### Team members

Jan Sigmund, Michal Švento

[Link to GitHub project folder](https://github.com/xsigmu06/Digital-electronics-2/tree/master/Labs/Project)

### Project objectives

Description of ultrasonic sensor HC-SR04 control. Parking assistant application using several ultrasonic sensors (Micro > Sensors > HC-SR04, see examples/Arduino/Arduino_SR04); display; distance in centimeters; acoustic signaling with different frequencies according to distance; distance indication on LED bar; sending application status information to the UART.

## Hardware description

### Module

![module](Images/hc-sr04.png)

Ultrasonic sensor HC-SR04 enables measuring distance in range of 2 _cm_ to 4 _m_. It uses sonar to send high frequency impulses at 40 kHz, therefore it is not affected by sunlight or black material. Sound waves can also penetrate through water. Measurement accuracy can be up to 3 _mm_. Measuring angle is 15 degree.


### Pins

Input pin _Trigger_ must receive 10 _us_ long high pulse (5_V_) to generate ultrasonic burst, which is then reflected off an obstacle back to receiver. When the sound wave gets back, a high value (5_V_) will be set on output pin _Echo_. The width of this signal (in _μs_) is proportional to measured distance divided by 2 (the wave travels to object and back), which can be calculated as follows: 

_distance = time / 58 [cm]_

Speed of sound can also be used - for output in _cm_ convert velocity (e.g. 340 _m/s_) to _cm/μs_ and divide by 2:

_distance = time * velocity = time * 0.017 [cm]_

### Timing

The pulse width of _Echo_ high can be between 120 _μs_ (2 _cm_) and 23.5 _ms_ (400 _cm_). If no object is detected, or the distance is greater than 4 _m_, the _Echo_ signal will be max. 38 _ms_ long. To prevent previous pulse interfering with next measurement it is recommended to wait at least 60 _ms_ for each measuring cycle.

![object](Images/object.jpg)
![no_object](Images/no_object.jpg)


## Code description and simulations

Most of the functionality is implemented directly in 'main.c' accessible [here](main.c). Libraries used are native for C, avr libraries and also from creator [Peter Fleury](http://tinyurl.com/peterfleury).

### Interrupts

#### Timer0 
Since the lowest possible value (without further alterations) for interrupt via timer overflow is for timer0 (16 _μs_), it is used for measuring the width of _Echo_ high. This should not diminish accuracy too much, as it is reasonably fast (lowest resolution is 0.272 _cm_). EDIT: The measurement error is up to 5 cm in greater distances. In it's ISR state FSM (Finite State Machine) is used with two states: `TRIG` & `STATE_ECHO_MEAS`.
- `TRIG` state sends 10us Trigger pulse 
- `STATE_ECHO_MEAS` state measures width of Echo signal and calculateds distance in _cm_.

#### Timer1 
ISR is used for displaying the measured Front and Back on LCD and UART, turning on LEDs bars for front and for back sensor individually. Four Leds on each sensor are used. When distance is smaller than 15 cm - 4 Leds are turned on. If distance is smaller than 50 cm a bigger than 15 cm, 3 Leds are turned on. Two Leds are turned on, when distance is smaller than 100 cm and bigger than 50 cm. If distance is between 100 and 125 cm, one led is turned on. If distance is bigger than 125 cm, Leds are turned off.

|distance[cm]|No of turned on LEDs|
| :--: | :--:|
| <15  | 4|
| 15-50| 3 |
|51-100| 2 |
|100 - 125 | 1 | 
| >125 | 0 |

#### Timer2 
In TIM1 overflow interrupt we change also the lenght of pulse for speaker (i.e frequency of sound for speaker speaker) on same distance limits as on LCD. Variable  TIM2_STOP is the value which is changed.  Overflow is disabled when counter reaches the TIM2_STOP value. The overflow on TIM2 is enabled again, when TIM1 overflows. Frequency of speaker depends on closer object. 
### Functions
#### displayResult

This function provides all outputs on LCD and UART. Data are send only when the value of distance is changed. 
We have three states, which can be displayed:

1. Distance is smaller than 2 cm - "*<2 cm*" is displayed
2. Distance between 2 - 400 cm - Absolute value is displayed
3. Distance > 400 cm - "*>400 cm*" is displayed

#### lcd_clear 

The function clears the part of display, where the value of distance is shown.

## Video/Animation

*Write your text here*
![test](Images/finaltest.gif)

## References

HC-SR04 User's_Manual
https://docs.google.com/document/d/1Y-yZnNhMYy7rwhAgyL_pfa39RsB-x2qR4vP8saG73rE/edit#

HC-SR04 Ultrasonic Sensor datasheet
https://components101.com/ultrasonic-sensor-working-pinout-datasheet

HC-SR04
https://os.mbed.com/components/HC-SR04/

Atmega328P datasheet
https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7810-Automotive-Microcontrollers-ATmega328P_Datasheet.pdf

