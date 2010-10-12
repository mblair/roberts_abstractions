#include <stdio.h>

int main(void) {
	int input, i = 0;
	
	printf("Please enter a positive integer: ");
	fflush(stdout);
	scanf("%d", &input);


	while (input > 1) {
		for(i=2; i <= input; i++) {
			if(input % i == 0) {
				if (input / i == 1) {	
					printf("%d\n", i);
				} else {
					printf("%d * ", i);
				}
				fflush(stdout);
				input /= i;
				break;
			}
		}
	}


	return 0;
}
