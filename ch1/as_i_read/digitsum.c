#include <stdio.h>

int DigitSum(int n) {
	int sum = 0;

	while (n > 0) {
		sum += n % 10;
		n /= 10;
	}

	return (sum);
}

int main(void) {
	int num;

	printf("Please enter an integer: ");
	scanf("%d", &num);
	printf("The sum of the digits in %d is %d.\n", num, DigitSum(num));

	return 0;
}
