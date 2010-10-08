#include <stdio.h>

int main(void) {
	int i, num, max, sum = 0;

	printf("Please enter an integer: ");
	scanf("%d", &num);

	max = num * 2 - 1;

	for(i=1; i<=max; i+=2) {
		sum += i;
	}

	printf("The sum of the first %d odd integers is %d.\n", num, sum);

	return 0;
}
