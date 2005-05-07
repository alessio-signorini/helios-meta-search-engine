#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cmdline.h"
#include "utils.h"






// resetConfiguration ---------------------------------------------------------
// Reset the configuration datas, enabling the PARSER and disabling the rest.
// ----------------------------------------------------------------------------
void resetConfiguration() {
	config.result_parsing	= 1;
	config.dump_to_disk		= 0;	
	config.verbose				= 0;
	config.socket_trace		= 0;
	config.parser_print		= 1;
}
// ----------------------------------------------------------------------------






// parseParameters ------------------------------------------------------------
// Cycle  on  the  given  ARGV  array  while  found  parameters , modifying the
// configuration.
// ----------------------------------------------------------------------------
short parseParameters(char *argv[]) {
	short i = 1;
	while (argv[i][0]=='-') {
		if (config.verbose) printf("Param[%d]: %s\n",i,argv[i]);
		if (argv[i][1]=='p') config.result_parsing	= 0;
		if (argv[i][1]=='d') config.dump_to_disk		= 1;
		if (argv[i][1]=='v') config.verbose				= 1;
		if (argv[i][1]=='s') config.socket_trace		= 1;
		if (argv[i][1]=='n') config.parser_print		= 0;
		i++;
	}
	return (i-1);
}






// extractQuery ---------------------------------------------------------------
// If  the  query is exact, return a complex query like "word1+...+wordN", else
// return a standard query string.
// ----------------------------------------------------------------------------
char *extractQuery(char *str) {
	const char d[] = "%22";																						// query delimitier
	char dl = strlen(d);																							// length of query-delimitier
	short len = strlen(str);																					// length of query
	char *q;
	
	if (chrrpl(str,' ','+')==0) return strdup(str);														// if SPACE !found, it's simple query
	
	q = malloc(len+dl+dl+1);																					// allocate enought memory
	strcpy(q, d);																									// copy the DELIMITIER at the beginning
	strcpy(q+dl, str);																							// copy the QUERY in the middle
	strcpy(q+dl+len, d);																							// copy the DELIMITIER at the end
	q[len+dl+dl]='\0';																							// insert terminating char
	return q;
}
// ----------------------------------------------------------------------------






// enoughtParametersCheck -----------------------------------------------------
// Fail if there aren't enought parameters, just to evoid segmentation faults.
// ----------------------------------------------------------------------------
void enoughtParametersCheck(char *cmd, short argc) {
	if (argc>2) return;
	printf("Helios v4.1g -----------------------------------------------\n");
	printf("   Usage: %s <flags> <query> <engine1> [... <engineN>]\n",cmd);
	exit(-1);
}
// ----------------------------------------------------------------------------
