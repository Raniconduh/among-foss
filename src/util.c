#include "util.h"


/* Generate a random number. */
int random_num(int max) {
	return rand() % (max + 1);
}
