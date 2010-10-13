#include <stdio.h>

#define TERMS	10000

int main(void) {
	int i, counter = 1;
	double sum = 0;

	for(i=1; i <= TERMS; i += 2) {
		if(counter % 2 == 1) {
			sum += 1.0 / i;
		}
		else {
			sum -= 1.0 / i;
		}

		counter++;
	}

	printf("%.20lf\n", sum*4);
	return 0;
}
