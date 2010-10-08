### 1. What is the difference between a source file and an object file?

A source file contains...source code. An object file is a compiled 
  version of that source file, before it is linked with libraries and such.

### 2. What characters are used to mark comments in a C program?

`/*` & `*/` are introduced in this book. `//` works for single-line comments.

### 3. In an `#include` line, the name of the library header file can be 
  enclosed in either angle brackets or double quotation marks. What is the 
  difference between the two forms of punctuation?

Angle brackets are for system libraries. Double quotation marks are for your 
  own header files.

### 4. How would you define a constant called CentimetersPerInch with the value 2.54?

Well, I'd call it `CENTIMETERS_PER_INCH` for starters. Here you go:

`#define CENTIMETERS_PER_INCH 2.54`

### 5. What is the name of the function that must be defined in every C program?

	main()

### 6. What is the purpose of the special character `\n` that appears at the end of most strings passed to printf?

It ends the current line of output and begins another.

### 7. What four properties are established when you declare a variable?

Type, scope, name, lifetime (isn't this tied to scope?)

### 8. Indicate which of the following are legal variable names in C:

* `x` -> yes
* `formula1` -> yes
* `average_rainfall` -> yes
* `%correct` -> no, has to start with a letter
* `short` -> nope, that's a keyword
* `tiny` -> yes
* `total output` -> no, can't have spaces
* `aReasonablyLongVariableName` -> yes, hello Java
* `12MonthTotal` -> no, has to start with a letter
* `marginal-cost` -> no hyphens
* `b4hand` -> yes
* `_stk_depth` -> yes (had to test this one, see [here](vars.c))

### 9. What are the two attributes that define a data type?

Domain and operations.

### 10. What is the difference between the types `short`, `int`, and `long`?

The size of an implementation's long has to be greater than or equal to that 
  of an int, which has to be greater than or equal to that of a short. 
  Clang's limits.h gives 32767, 2,147,483,647 and 9.2e18, respectively 
  (see [here](ints.c))

### 11. What does ASCII stand for?

American Standard Code for Information Interchange. Yep, had to look that 
  one up.

### 12. List all possible values of type `bool`.

{ TRUE, FALSE }

### 13. What statements would include in a program to read a value from the user and store it in the variable `x`, which is declared as a double?

This book would lead me to use GetReal() from the book's [I/O library](../book_code/unix-xwindows/simpio.h). [Yea, get real is right](double.c):

	scanf("%lf", &x);

### 14. Suppose that a function contains the following declarations:
* `int i;`
* `long l;`
* `float f;`
* `double d;`
* `char c;`
* `string s;`
### Write a series of printf calls that display the valyes of each of these variables on the screen.

* `printf("%d", i);`
* `printf("%ld", l);`
* `printf("%f", f);`
* `printf("%lf", d);`
* `printf("%c", c);`
* `printf("$s", s);`

### 15. Indicate the values and types of the following expressions:

* `2 + 3` = 5, int
* `19 / 5` = 3, int
* `19.0 / 5` = 3.80, float
* `3 * 6.0` = 18.0, float
* `19 % 5` = 4, int
* `2 % 7` = 2, int

### 16. What is the difference between the unary minus operator and the binary subtraction operator?

Unary minus negates a value. Binary substraction subtracts one value from another.

### 17. What does the term _truncation_ mean?

In the context of C, _truncation_ is a loss of precision:

	int x;
	x = 2.5 / 1;
	printf("%d", x); // Prints 2.00000

### 18. By applying the appropriate precedence rules, calculate the result of each of the following expressions:

* `6 + 5 / 4 - 3` = 4
* `2 + 2 * (2 * 2 - 2) % 2 / 2` = 2
* `10 + 9 * ((8 + 7) % 6) + 5 * 4 % 3 * 2 + 1` = 42
* `1 + 2 + (3 + 4) * ((5 * 6 % 7 * 8) - 9) - 10` = 42

### 19. How do you specify a shorthand assignment operation?

* `operand1 (op)= operand2`
* `x *= 2` doubles x.

### 20. What is the difference between the expressions `++x` and `x++`?

`++x` is incremented before its value is returned, where `x++` returns the value of `x`, then increments it.

### 21. What does the term _short-circuit evaluation_ mean?

When a boolean expression is evaluated, the evaluation completes as early as its value becomes clear. So in the following:

	int x = 1;
	int y = 2;
	if (x == 1 || y == 3) ...

the expression `y == 3` is never evaluated, because only one of the expressions needs to be true, and the first is indeed true.

### 22. Write out the general synctactic form for each of the following control statements: `if`, `switch`, `while`, `for`.

	//This does not repeat.
	if(conditional_expression) {
		statements;
	} else {
		statements;
	}

	switch(expression) {
		case const1:
			statement;
			break;
		case const2:
			statement;
			break;
		default:
			statement;
			break;`
	}
	
	//This repeats until the conditional_expression is false.
	while(conditional_expression) {
		statements;
	}
	
	for(initialization; conditional_expression; step) {
		statements;
	}

### 23. Describe in English the general operation of the switch statement.

Evaluate the expression inside of switch's parentheses. Run through each case statement, comparing the case's constant to the switch value. If equal, execute the associated statements. If not, move on to the next case statement. If you hit no matching values, execute `default`'s statements if it has been specified. If at any time you hit a `break;` statement, stop executing the switch statement.

### 24. What is a sentinel?

A sentinel is a value that you use in a `while` loop to break out of the loop. It allows you to enclose all of the loop's functions inside of the `while` loop, because you often need to read a value before you evaluate a conditional expression. Here's an example:

	while (1) {
		printf("Please enter an integer. Enter 0 when you're done: ");
		scanf("%d", x);
		if (x == 0) {
			break;
		}
		//do something with x.
	}

### 25. What `for` loop control line would you use in each of the following situations:

* Counting from 1 to 100

`for (i=0;i<100;i++)`

* Counting by sevens starting at 0 until the number is three digits long
	
`for(i=0;i<100;i+=7)`

* Counting backward by twos from 100 to 0

`for(i=100;i>=0;i-=2)`

### 26. What is a function prototype?

A function prototype specifies a function's return values and its arguments. I am still not entirely clear on why one needs to include a prototype, but perhaps I'll learn later.

### 27. In your own words, describe what happens when you call a function in C.

* The calling program evaluates the function's arguments.
* A stack frame is created, containing space for all of the function's local variables.
* The function's parameters are assigned their values. Type conversions are performed if necessary.
* The function's statements are executed.
* The `return` expression is evaluated and returned as the value of the function, if there is a return expression (some functions don't return anything). A type conversion (from the return value's type to the function's declared type) is performed if necessary.
* The stack frame goes bye bye.
* The calling program goes on its merry way.

### 28. What is meant by the term stepwise refinement?

As you create your program, you will probably need to break it up into parts, enclosed in functions. As you implement everything, you might need to continue breaking things down into smaller functions until every piece is simple. Gotta love procedural programming.
