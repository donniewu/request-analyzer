#include "size-cache.h"
#include <stdio.h>

int main()
{
	srand(time(NULL));

	for (int c = 0; c < 1000; c++) {
		SizeCache sc;
		sc.setThreshold(100);

		for (int i = 0; i < 1010; i++) {
			if (sc.write(rand() % 1000, rand() % 5 + 1) < 0) {
				printf("wtf\n");
				printf("%s\n", sc.dump().c_str());
				return -1;
			}
		}
		//printf("%s\n", sc.dump().c_str());
	}
}
