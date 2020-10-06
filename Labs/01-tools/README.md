# Lab 3: User library for GPIO control

## Preparation tasks (done before the lab at home)

Fill in the following table and enter the number of bits and numeric range for the selected data types defined by C.

| **Data type** | **Number of bits** | **Range** | **Description** |
| :-: | :-: | :-: | :-- | 
| `uint8_t`  | 8 | 0, 1, ..., 255 | Unsigned 8-bit integer |
| `int8_t`   |  |  |  |
| `uint16_t` |  |  |  |
| `int16_t`  |  |  |  |
| `float`    |  | -3.4e+38, ..., 3.4e+38 | Single-precision floating-point |
| `void`     |  |  |  |

Any function in C contains a declaration (function prototype), a definition (block of code, body of the function); each declared function can be executed (called).

Study [this article](https://www.programiz.com/c-programming/c-user-defined-functions) and complete the missing sections in the following user defined function declaration, definition, and call.

```C
#include <avr/io.h>

// Function declaration (prototype)
uint16_t calculate(uint8_t, ...    );

int main(void)
{
    uint8_t a = 156;
    uint8_t b = 14;
    uint16_t c;

    // Function call
    c = ...      (a, b);

    while (1)
    {
    }
    return 0;
}

// Function definition (body)
...      calculate(uint8_t x, uint8_t y)
{
    uint16_t result;    // result = x^2 + 2xy + y^2

    result = x*x;
    ...
    ...
    return result;
}
```
