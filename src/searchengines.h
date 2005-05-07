struct engineInformations {
	char name[DEFAULT_NAME_LENGTH];																			// engine name
	char *host;																										// host = "www.something.com"
	short port;																										// port of the search engine
	struct sockaddr *sa;																							// struct sockaddr for connect()
	short rpp;																										// results for each page
	short pag;																										// desidered number of pages
	short npc;																										// next-page calculation mode
	char query[DEFAULT_QUERY_SIZE];																			// default query string
	char *parser_cmd[MAX_PARSER_INSTRUCTION];																// array of parser commands
};

int loadEnginesInformations(char *filename);

struct engineInformations *ptrToEngineInformations(char *name);

void freeSearchEnginesMemory();
