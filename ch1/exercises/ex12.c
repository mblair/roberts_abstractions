#include <stdio.h>
#include <stdlib.h>
//#include "../../book_code/unix-xwindows/genlib.h"

void printOneDigit(int x) {
	switch(x) {
		case 0: printf("zero"); break;
		case 1: printf("one"); break;
		case 2: printf("two"); break;
		case 3: printf("three"); break;
		case 4: printf("four"); break;
		case 5: printf("five"); break;
		case 6: printf("six"); break;
		case 7: printf("seven"); break;
		case 8: printf("eight"); break;
		case 9: printf("nine"); break;
		default: exit(1); break;
	}
}

int hundreds(int x) { 
	return x / 100;
}

int tens(int x) { 
	return (x / 10) % 10;
}

int ones(int x) { 
	return x % 10;
}

int thousands(int x) {
	return x / 1000;
}

int lastThree(int x) {
	return x % 1000;
}

void printNumber(int x) {
	if (hundreds(x) > 0) {
		printOneDigit(hundreds(x));
		printf(" hundred");
	}
}

void printTotal(int x) {
	if(thousands(x) > 0) {
		printNumber(thousands(x));
		printf(" thousand ");
	}
	printNumber(lastThree(x));
}

int main(void) {

	int input;
	
	printf("Enter numbers in their decimal form.\n");
	printf("To stop, enter a negative value.");
	
	do {
		printf("\nNumber: ");
		scanf("%d", &input);
		printTotal(input);
	}
	while (input >= 0);

	return 0;
}
