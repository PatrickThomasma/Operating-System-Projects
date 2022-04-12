#include <stdlib.h>

int main (void argc, void argv[]) {
	char *p;
	int val = -1;
	if ((p = getenv("VARIABLE_NAME")) != NULL)
		val = atoi(p);

	return 0;
}
