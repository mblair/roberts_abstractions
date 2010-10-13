#include <stdio.h>

int Round(float input) {
	if(input > 0) {
		input += 0.5;
	}

	if(input < 0) {
		input -= 0.5;
	}

	return (int)input;
}

int main(void) {
	float input;

	printf("Please enter a floating-point number: ");
	scanf("%f", &input);
	printf("Here's the rounded version: %d\n", Round(input));

	return 0;
}
