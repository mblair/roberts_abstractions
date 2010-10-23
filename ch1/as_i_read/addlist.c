#include <stdio.h>

#define SENTINEL 0

int main() {
	int value, total;

	printf("This program adds a list of numbers.\n");
	printf("Use %d to signal the end of the list.\n", SENTINEL);
	total = 0;
	while (1) {
		printf(" ? ");
		scanf("%d", &value);
		if(value == SENTINEL) break;
		total += value;
	}
	printf("The total is %d.\n", total);
}
