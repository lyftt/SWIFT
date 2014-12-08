#include <stdio.h>

#define N 10

int main() {
	int f[N];
	int i;
	for (i = 0; i < N; i++ ) {
		f[i] = i;
	}
	printf("%d\n", f[0]);
	return 0;
}
