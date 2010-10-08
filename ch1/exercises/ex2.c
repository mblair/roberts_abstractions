#include <stdio.h>

int main(void) {
	float meters, inches, part_inches;
	int feet, whole_inches;

	printf("Please enter a distance in meters: ");
	scanf("%f", &meters);
	inches = meters / 0.0254;
	feet = inches / 12;
	whole_inches = (int)inches % 12;
	part_inches = inches - (feet * 12 + whole_inches);
	inches = whole_inches + part_inches;

	printf("%f meters is %d feet and %f inches.\n", meters, feet, inches);
	return 0;
}
