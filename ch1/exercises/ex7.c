#include <stdio.h>

int IsPerfect(int n) {
	int i, sum = 0;
	for (i=1; i < n; i++) {
		if (n % i == 0) {
			sum += i;
		}
	}
	if (sum == n) {
		return 1;
	} else {
		return 0;
	}
}

int main(void) {
	int i;
	for (i=1; i <= 9999; i++) {
		if(IsPerfect(i)) {
			printf("%d\n", i);
		}
	}

	return 0;
}
