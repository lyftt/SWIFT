#include <stdio.h>

#define N 10
//#define LOOP 100000

int main() {
	int f[N];
	int i, l;
//	for (l = 0; l < LOOP; l++) {
	for (i = 0; i < N; i++ ) {
		if (i <= 1) {
			f[i] = i;
		} else {
			f[i] = f[i-2] + f[i-1];
		}
	}
	printf("%d\n", f[N-1]);
//	}
	return 0;
}
