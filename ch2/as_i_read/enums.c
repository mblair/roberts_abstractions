#include <stdio.h>

typedef enum {
	Red, Green, Blue, Yellow, Cyan, Magenta
} colorT;

typedef int colorIntT;

colorT favorite = Blue;
colorIntT favoriteInt = 2;

int main(void) {
	printf("My favorite color is %d, also known as %d\n", favorite, favoriteInt);
	return 0;
}
