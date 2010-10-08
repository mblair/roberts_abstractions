#include <stdio.h>

#define SENTINEL 0

int main(void) {
	int i, max = 0;

	printf("This program finds the largest integer in a list.\n");
	printf("Enter 0 to signal the end of the list.\n");

	while(1) {
		printf(" ? ");
		scanf("%d", &i);
		if (i == SENTINEL) {
			break;
		}

		if (i > max) {
			max = i;
		}
	}

	printf("The largest value is %d.\n", max);

	return 0;
}
