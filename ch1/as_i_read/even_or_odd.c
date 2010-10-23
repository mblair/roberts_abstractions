#include <stdio.h>
//#include "../book_code/unix-xwindows/genlib.h"
//#include "../book_code/unix-xwindows/simpio.h"

int main() {
	int n;

	printf("This program labels a number as even or odd.\n");
	printf("Enter a number: ");
	scanf("%d", &n);
	if (n % 2 == 0) {
		printf("That number is even.\n");
	} else {
		printf("That number is odd.\n");
	}

	return 0;
}
