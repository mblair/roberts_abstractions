#include <stdio.h>

int main(void) {
	float cels;
	float fahr;

	printf("Please enter a temperature in degrees Celsius: ");
	scanf("%f", &cels);

	fahr = 9.0 / 5 * cels + 32;
	printf("%f degrees Celsius is %f degrees Fahrenheit.\n", cels, fahr);

	return 0;
}
