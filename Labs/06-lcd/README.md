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

   - ASCII (American Standard Code) - character encoding standard for electronic communication
   - `A` to `Z` ... 65 to 90 (decimal)
   - `a` to `z` ... 97 to 122 (decimal)
   - `0` to `9` ... 48 to 57 (decimal)
   - https://www.asciitable.com/

## Library for HD44780 based LCDs

In the lab, we are using [LCD library for HD44780 based LCDs](http://www.peterfleury.epizy.com/avr-software.html) developed by Peter Fleury. Use online manual of LCD library and add the input parameters and description of the functions to the following table.

   | **Function name** | **Function parameters** | **Description** | **Example** |
   | :-- | :-- | :-- | :-- |
   | `lcd_init` | `LCD_DISP_OFF`<br>`LCD_DISP_ON`<br>`LCD_DISP_ON_CURSOR`<br>`LCD_DISP_ON_CURSOR_BLINK` | Initialize display and select type of cursor. | `lcd_init(LCD_DISP_OFF);` |
   | `lcd_clrscr` |none | Clear display and set cursor to home position. | `lcd_clrscr();` |
   | `lcd_gotoxy` | `x` horizontal position <br> (0: left most position) <br> `y` vertical position <br> (0: first line)| Set cursor to specified position. | `lcd_gotoxy(x,y);` |
   | `lcd_putc` | `c`	character to be displayed | Display character at current cursor position. | `lcd_putc(c);` |
   | `lcd_puts` | `s`	string to be displayed | Display string without auto linefeed. | `lcd_puts(s);` |
   | `lcd_command` | `cmd` instruction to send to LCD controller, see HD44780 data sheet | Send LCD controller instruction command. | `lcd_command(cmd);` |
   | `lcd_data` | `data`	byte to send to LCD controller, see HD44780 data sheet | Send data byte to LCD controller. | `lcd_data(data);` |
