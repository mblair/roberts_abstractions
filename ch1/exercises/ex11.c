// Way to tell us about sqrt and pow.
// You'll need to invoke the compiler like so: clang -g -lm ex11.c
// I hope not all libraries need compiler flags...

#include <stdio.h>
#include <math.h>

#define RECTANGLES	100

int main(void) {

	int i, radius = 2;
	double height, area = 0, width = (float)radius / RECTANGLES;

	double x = width / 2; // The first x value is the midpoint of the first rectangle.

	for(i=0; i < RECTANGLES; i++) {
		height = sqrt(pow(radius, 2) - pow(x, 2));
		area += height * width;
		x += width;
	}

	printf("The approximation of Π with %d rectangles is %.20f.\n", RECTANGLES, area);

	return 0;

}
