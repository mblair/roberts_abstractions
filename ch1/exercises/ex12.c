#include <stdio.h>
//#include "../../book_code/unix-xwindows/genlib.h"

int hundreds(int x) { return 0;}
int tens(int x) { return 0;}
int ones(int x) { return 0;}

int thousands(int x) {
	return (int)x / 1000;
}

int lastThree(int x) {
	return x % 1000;
}

void printNumber(int x) {}

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
	printf("To stop, enter a negative value.\n");
	
	do {
		printf("Number: ");
		scanf("%d", &input);
		printTotal(input);
	}
	while (input >= 0);

	return 0;
}
