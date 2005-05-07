struct configuration {
	char result_parsing;																							// enable or disable the PARSER
	char dump_to_disk;																							// enable or disable the DUMP TO DISK
	char verbose;																									// print additional informations
	char socket_trace;																							// print informations about sockets
	char parser_print;																							// control execution of parser PRINT cmd
	} config;

void resetConfiguration();

short parseParameters(char *argv[]);

char *extractQuery(char *str);

void enoughtParametersCheck(char *cmd, short argc);
