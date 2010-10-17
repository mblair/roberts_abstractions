#include <stdio.h>

void printTens(int x) {
	switch(x) {
		case 2: printf("twenty"); return;
		case 3: printf("thirty"); return;
		case 4: printf("fourty"); return;
		case 5: printf("fifty"); return;
		case 6: printf("sixty"); return;
		case 7: printf("seventy"); return;
		case 8: printf("eighty"); return;
		case 9: printf("ninety"); return;
	}
}

void printOneDigit(int x) {
	switch(x) {
		case 1: printf("one"); return;
		case 2: printf("two"); return;
		case 3: printf("three"); return;
		case 4: printf("four"); return;
		case 5: printf("five"); return;
		case 6: printf("six"); return;
		case 7: printf("seven"); return;
		case 8: printf("eight"); return;
		case 9: printf("nine"); return;
	}
}

int hundreds(int x) { 
	return x / 100;
}

int tens(int x) { 
	return (x / 10) % 10;
}

int ones(int x) { 
	return x % 10;
}

int thousands(int x) {
	return x / 1000;
}

int lastThree(int x) {
	return x % 1000;
}

int lastTwo(int x) {
	return x % 100;
}

void printNumber(int x) {
	if(x >= 0 && x < 10) {
		printOneDigit(x);
		return;
	}

	if (hundreds(x) > 0) {
		printOneDigit(hundreds(x));
		printf(" hundred ");
	}

	if((lastTwo(x) >= 10 && lastTwo(x) <= 13) || lastTwo(x) == 15 || lastTwo(x) == 18) {
		switch(lastTwo(x)) {
			case 10: printf("ten"); return;
			case 11: printf("eleven"); return;
			case 12: printf("twelve"); return;
			case 13: printf("thirteen"); return;
			case 15: printf("fifteen"); return;
			case 18: printf("eighteen"); return;
		}
	}

	if(lastTwo(x) == 14 || lastTwo(x) == 16 || lastTwo(x) == 17 || lastTwo(x) == 19) { //Yikes, I know. But this is the reusable functions question.
		printOneDigit(x % 10);
		printf("teen");
		return;
	}

	if(tens(x) > 1) {
		printTens(tens(x));
		if (ones(x) > 0) {
			printf("-");
			printOneDigit(ones(x));
		}
		return;
	}
	
	printOneDigit(ones(x));
	return;

}

void printTotal(int x) {
	if(x == 0) {
		printf("zero");
		return;
	}

	if(thousands(x) > 0) {
		printNumber(thousands(x));
		printf(" thousand ");
	}
	printNumber(lastThree(x));
	return;
}

int main(void) {

	int input;
	
	printf("Enter numbers in their decimal form.\n");
	printf("To stop, enter a negative value.");
	
	do {
		printf("\nNumber: ");
		scanf("%d", &input);
		printTotal(input);
	}
	while (input >= 0);

	return 0;
}
