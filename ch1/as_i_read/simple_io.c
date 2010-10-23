#include <stdio.h>
#include "../book_code/unix-xwindows/genlib.h"
#include "../book_code/unix-xwindows/simpio.h"

int main() {
	double n1, n2, n3, average;

	printf("This program averages three numbers.\n");
	printf("1st number: ");
	n1 = GetReal();
	printf("2nd number: ");
	n2 = GetReal();
	printf("3rd number: ");
	n3 = GetReal();
	average = (n1 + n2 + n3) / 3;
	printf("The average is %g\n", average);

	return 0;
}
