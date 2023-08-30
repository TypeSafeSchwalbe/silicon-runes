# Silicon Runes
*The language of the ancient civilisation that once trapped lightning inside of rocks to trick the rocks into thinking for them.*

Silicon Runes operate on two stacks, one being *the primary stack*, which values are pushed onto by default and where almost all operations are done, and the other being *the secondary stack*, which values can be moved to and from, making the language turing complete.

Values can have one of four types, these being *integers*, *floats*, *strings* and *arrays*.

## Examples

### Hello, world!
```
(Hello, world!)!
```

### Fibonacci Sequence
```
#0 1(':#0>)(:#+'$:!'1-#)@
```
Requires the number of elements of the sequence to print to be the value at the top of the stack, running it would look like this:
```
> 5
> #0 1(':#0>)(:#+'$:!'1-#)@
1
2
3
5
8
```

### FizzBuzz
```
1(:101<)(:3%0=((Fizz)Ip)?:5%0=((Buzz)Ip)?:3%0=0=$:#$'5%0=0=&:($:#$)?0=(()#)?'!1+)@
```

## Instructions

*The first value* shall be the value at the top of stack.
*The second value* shall be the value below the value at the top of the stack.
*The third value* shall be the value below the second value of the stack.

A value shall be *truthy* if it is an integer or float and not equal to zero, or if it is a string or an array and its length is not 0.

### General

`<Space>` or `<Line break>` - Does nothing.

`<Digits>` - Pushes the number as an integer onto the primary stack.

`<Digits>.<Digits>` - Pushes the number as a float onto the primary stack.

`(<Anything>)` - Pushes the content of the parentheses as a string onto the primary stack.

### Stack manipulation

`:` - Creates a copy of the first value on the primary stack, and pushes it onto the primary stack.

`^` - Removes the first value from the primary stack.

`$` - Swaps the places of the first and second values of the primary stack.

`#` - Removes the first value from the primary stack and pushes it onto the secondary stack.

`'` - Removes the first value from the secondary stack and pushes it onto the primary stack.

### I/O

`,` - Receives a line of input and pushes it onto the primary stack as a string.

`!` - Removes the first value from the primary stack and prints it.

### Arithmetic

`+` - Removes the first and second values from the primary stack (expected to be integers or floats), adds them and pushes the sum back onto the primary stack (output type is input type).

`-` - Removes the first and second values from the primary stack (expected to be integers or floats), subtracts the second from the first and pushes the difference back onto the primary stack (output type is input type).

`*` - Removes the first and second values from the primary stack (expected to be integers or floats), multiplies them and pushes the product back onto the primary stack (output type is input type).

`/` - Removes the first and second values from the primary stack (expected to be integers or floats), divides the second by the first and pushes the quotient back onto the primary stack (output type is input type).

`%` - Removes the first and second values from the primary stack (expected to be integers or floats), divides the second by the first and pushes the remainder back onto the primary stack (output type is input type).

`~` - Removes the first value from the primary stack (expected to be an integer or float), negates it mathematically and pushes the result back onto the primary stack (output type is input type).

### Comparisons

`<` - Removes the first and second values from the primary stack (expected to be integers or floats) and pushes `1` onto the primary stack if the second is less than the first, and otherwise pushes `0` onto the primary stack (output type is input type).

`>` - Removes the first and second values from the primary stack (expected to be integers or floats) and pushes `1` onto the primary stack if the second is greater than the first, and otherwise pushes `0` onto the primary stack (output type is input type).

`=` - Removes the first and second values from the primary stack (expected to be integers or floats) and pushes `1` onto the primary stack if the second is equal to the first, and otherwise pushes `0` onto the primary stack (output type is input type).

### Logical 

`&` - Removes the first and second values from the primary stack (expected to be integers or floats) and pushes `1` onto the primary stack if both are not equal to 0, and otherwise pushes `0` onto the primary stack (output type is input type).

`|` - Removes the first and second values from the primary stack (expected to be integers or floats) and pushes `1` onto the primary stack if one of them is not equal to 0, and otherwise pushes `0` onto the primary stack (output type is input type).

### Control Flow

`?` - Removes the first (expected to be a string) and second values from the primary stack. Executes the removed string as instructions if the second removed value is truthy.

`@` - Removes the first and second values from the primary stack (expected to be strings). Repeatedly executes the first removed string as instructions, then removes the (now) first value from the primary stack. If that value is truthy, it executes the second removed string as instructions and otherwise stops repeating and continues with the next instruction. 

### Interpreter

`IP` - Pushes the number of elements on the primary stack (before execution of the instruction) as an integer onto the primary stack.

`IS` - Pushes the number of elements on the secondary stack as an integer onto the primary stack.

`Ip` - Removes the first value from the primary stack (expected to be a string) and prints it (without a new line).

`Id` - Nicely prints a list of all the values on the primary and on the secondary stack (for debug purposes).

`Ir` - Clears the primary stack and the secondary stack.

### Arrays

`AN` - Creates a new array and pushes it onto the primary stack.

`Ap` - Removes the first value from the primary stack and pushes it onto the array stored in the second value on the primary stack.

`Ag` - Removes the first value from the primary stack (expected to be an integer) and pushes the element at the index defined by the first removed value from the array stored in the second value on the primary stack onto the primary stack.

`As` - Removes the first value and second (expected to be an integer) values from the primary stack and replaces the element at the index defined by the second removed value in the array stored in the third value on the primary stack with the first removed value. 

`Ar` - Removes the first value from the primary stack (expected to be an integer) and removes the element at the index defined by the first removed value in from the array stored in the second value on the primary stack (moves all elements with higher integers one down).

`Al` - Pushes the number of elements of the array stored in the first value on the primary stack onto the primary stack.

### Strings

`Sm` - Removes the first and second values from the primary stack (expected to be strings), appends the first removed string to the end of the second removed string and pushes the merged string onto the primary stack.

`Ss` - Removes the first and second values from the primary stack (expected to be integers), creates a copy of the part of the string stored in the third value on the primary stack starting at (including) the character index defined by the second removed value and ending at (excluding) the character index defined by the first removed value and pushes the substring onto the primary stack.

`Sl` - Pushes the number of characters of the string stored in the first value on the primary stack onto the primary stack.

### Math

`MP` - Pushes `3.14159265358979323846` (Pi) as a float onto the primary stack.

`MT` - Pushes `6.28318530717958647692` (Tau) as a float onto the primary stack.

`ME` - Pushes `2.7182818284590452354` (Euler's number) as a float onto the primary stack.

`MR` - Pushes a random number that is greater or equal to 0 and less than 1 as a float onto the primary stack.

`Mf` - Removes the first value from the primary stack (expected to be an integer) and pushes it onto the primary stack as a float.

`Mu` - Removes the first value from the primary stack (expected to be a float), rounds it towards positive infinity and pushes it onto the primary stack as an integer. 

`Md` - Removes the first value from the primary stack (expected to be a float), rounds it towards negative infinity and pushes it onto the primary stack as an integer.

`Mn` - Removes the first value from the primary stack (expected to be a float), rounds it towards the nearest integer and pushes it onto the primary stack as an integer.

`Ms` - Replaces the first value on the primary stack (expected to be a float) with its sine.

`Mc` - Replaces the first value on the primary stack (expected to be a float) with its cosine.

`Mt` - Replaces the first value on the primary stack (expected to be a float) with its tangent.

`Ma` - Replaces the first value on the primary stack (expected to be a float) with its absolute value.

`Mr` - Replaces the first value on the primary stack (expected to be a float) with its square root.

`Mp` - Removes the first and second values from the primary stack (expected to be floats), raises the second value to the power of the first and pushes it onto the primary stack.
