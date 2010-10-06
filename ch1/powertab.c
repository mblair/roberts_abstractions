/*
 * File: powertab.c
 * ----------------
 * This program generates a table comparing values of the functions n^2 and 2^n.
 */

#include <stdio.h>
#include "../book_code/unix-xwindows/genlib.h"

/*
 * Constants
 * ---------
 * LowerLimit -- Starting value for the table
 * UpperLimit -- Final value for the table
 */

#define LowerLimit 	0
#define UpperLimit 12

/* Private function prototypes */

static int RaiseIntToPower(int n, int k);

/* Main prototype */

int main() {
	int n;

	printf("    |  2 |   N\n");
	printf("  N | N  |  2\n");
	printf("----+----+-----\n");
	for (n = LowerLimit; n <= UpperLimit; n++) {
		printf(" %2d | %3d | %4d\n", n, 
		  RaiseIntToPower(n, 2),
		  RaiseIntToPower(2, n));
	}
	
	return 0;
}

/*
 * Function: RaiseIntToPower
 * Usage: p = RaiseIntToPower(n, k);
 * ---------------------------------
 * This function returns n to the kth power.
 */

static int RaiseIntToPower(int n, int k) {
	int i, result;
	result = 1;
	for (i=0; i < k; i++) {
		result *= n;
	}
	
	return result;
}
