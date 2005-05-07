#include <stdio.h>
#include <unistd.h>																								// per exit
#include <stdlib.h>																								// per exit

#include "core.h"
#include "cmdline.h"
#include "utils.h"

int main(int argc, char *argv[]) {
	char *q=NULL;
	char c;
	short k;

	enoughtParametersCheck(argv[0], argc);
	resetConfiguration();
	c = parseParameters(argv);
	q = extractQuery(argv[c+1]);
	
	search(q, (argc-c-2), &argv[c+2]);
	
	free(q);
	exit(0);
}
