#include <stdio.h>

int main(void) {
	long int input, reversed = 0;

	printf("Please enter a positive integer: ");
	scanf("%ld", &input);

	printf("The reverse of %ld is ", input);

	while (input > 0) {
		reversed *= 10;
		reversed += input % 10;
		input /= 10;
	}

	printf("%ld.\n", reversed);
	
	return 0;
}
