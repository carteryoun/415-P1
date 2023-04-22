#include <unistd.h>
#include <stdlib.h>
#include "p1fxns.h"
#include <string.h>
#include <errno.h>

char *p;
int val = -1;

if((p = getenv("VARIABLE_NAME")) != NULL) {
	val = atoi(p);
}