# Lab 6: Display devices, LCD display

## Preparation tasks (done before the lab at home)

Use schematic of the [LCD keypad shield](../../Docs/arduino_shield.pdf) and find out the connection of LCD display. What data and control signals are used? What is the meaning of these signals?

   | **LCD signal(s)** | **AVR pin(s)** | **Description** |
   | :-: | :-: | :-- |
   | RS | PB0 | Register selection signal. Selection between Instruction register (RS=0) and Data register (RS=1) |
   | R/W | GND | Write data signal (R/W=0), read data signal (R/W=1), pin is GND -> only write |
   | E | PB1 | Enable signal, falling edge starts communication |
   | D[3:0] | not used | Data signals, possible for 8 bit communication |
   | D[7:4] | PD7:PD4 | Data signals, 4 bit communication, words are sent in 2 halves (2 E signals needed) |

What is the ASCII table? What are the values for uppercase letters `A` to `Z`, lowercase letters `a` to `z`, and numbers `0` to `9` in this table?

   ASCII (American Standard Code) - character encoding standard for electronic communication
   `A` to `Z` ... 65 to 90 (decimal)
   `a` to `z` ... 97 to 122 (decimal)
   `0` to `9` ... 48 to 57 (decimal)
   https://www.asciitable.com/
