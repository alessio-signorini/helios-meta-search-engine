#include <stdio.h>
#include <stdlib.h>																								// per atoi
#include <string.h>																								// per strcmp, strcpy, strcat
#include <strings.h>																								// per bzero

#include "macro.h"
#include "utils.h"
#include "socketutils.h"
#include "searchengines.h"


struct engineInformations engine[MAX_ENGINE_NUMBER];
short nengines = 0;





// retriveEngineInformations --------------------------------------------------
// Return a pointer to the requested engine informations structure if exist,
// else abort and ask to check parameters
// ----------------------------------------------------------------------------
struct engineInformations *ptrToEngineInformations(char *name) {
	short i=0;
	
	while (i<MAX_ENGINE_NUMBER && engine[i].host!=NULL)
		if (strcmp(engine[i].name, name)==0) return &engine[i]; else i++;
	printf("ERROR: Engine '%s'not found! Please check parameters!\n",name);
	exit(-1);
}
// ----------------------------------------------------------------------------






// createEngineSocket ---------------------------------------------------------
// This  function  create the engine's SocketAddress, and save it in the engine
// structure
// ----------------------------------------------------------------------------
void setEngineSocketAddress(struct engineInformations *e) {
	e->sa = createSocketAddress(e->host, e->port);
}
// ----------------------------------------------------------------------------






// clearEngineData ------------------------------------------------------------
// This  function  clear  the  engine's  datas  before replacing them with read
// values. Just to avoid problems with uncleared vars.
// ----------------------------------------------------------------------------
void clearEngineInformations(struct engineInformations *e) {
	e->host		= NULL;
	e->port		= 80;
	e->sa			= NULL;
	e->rpp		= 10;
	e->pag		= 1;
	e->npc		= 0;
	
	bzero(e->query, sizeof(e->query));
	bzero(e->name, sizeof(e->name));
}
// ----------------------------------------------------------------------------






// engineSetOption ------------------------------------------------------------
// If the OPTION NAME (option) exist in the ENGINE STRUCTURE, set its value
// with the VALUE (value), for the given (i) ENGINE.
// ----------------------------------------------------------------------------
void engineSetOption(struct engineInformations *e, char *option, char *value) {
	if (strcmp("host",option)==0)						e->host		= value;
	if (strcmp("port",option)==0)						e->port		= atoi(value), free(value);
	if (strcmp("desidered_pages",option)==0)		e->pag		= atoi(value), free(value);
	if (strcmp("next_page_calc",option)==0)		e->npc		= atoi(value), free(value);
	if (strcmp("result_per_page",option)==0)		e->rpp		= atoi(value), free(value);
	free(option);
}
// ----------------------------------------------------------------------------






// loadParserScript -----------------------------------------------------------
// Read the file <engine>.prs and save into memory the parser script.
// ----------------------------------------------------------------------------
void loadParserScript(struct engineInformations *e) {
	FILE *f;
	short i=0;
	char buffer[bfsize+1];
	char *filename;
	
	filename = join3("parser/", e->name, ".prs");														// create the <engine>.prs filename
	
	if ((f = fopen(filename,"r"))==NULL) {																	// exit if opening problems occur
		printf("   Error opening '%s'. Exit!",filename); exit(-1); }
	
	while (fgets(buffer, bfsize, f)!=NULL) {
		buffer[strlen(buffer)-1]='\0';																		// replace the final '\n' with a '\0'
		e->parser_cmd[i] = strdup(buffer);																	// put the cmd in the commmand list
		i++;
	}
	
	e->parser_cmd[i] = NULL;																					// set last pointer to NULL

	fclose(f);
	free(filename);
}
// ----------------------------------------------------------------------------






// LoadEnginesInformations ----------------------------------------------------
// Read  CONFIG  FILE  (filename)  line  by  line,  parsing  the content. Lines
// starting  with  '#'  are ignored because considered comments. Lines starting
// with '[' are considered the start of new search engine options.
// Options are expected with the format
//    <option_name> <tab_chars> = <tab_char> <option_value>
// ----------------------------------------------------------------------------
int loadEnginesInformations(char *filename) {
	FILE *f;
	short x;																											// temporary var for '\t' check
	short n=-1;																										// search engine index
	short onl, ovl;																								// opt name length, opt value length
	char *pov;																										// pointer to option value start
	char *on, *ov;																									// option name, option value
	char buffer[bfsize+1];																						// read buffer
	short i;

	if ((f = fopen(filename,"r"))==NULL) {																	// exit if opening problems occur
		printf("   Error opening '%s'. Exit!",filename); exit(-1); }
	
	while (n<MAX_ENGINE_NUMBER && fgets(buffer, bfsize, f)!=NULL)
		switch(buffer[0]) {
			case '#': case '\n':																					// skip comment and empty lines
				continue;
			
			case '[':																								// [ mean new engine
				n++;
				clearEngineInformations(&engine[n]);
				strncpy(engine[n].name, buffer+1, strlen(buffer)-3);									// -3 becouse of ']' + '\n' + '\0'
				continue;
			
			case '{':																								// { mean query data
				while (fgets(buffer, bfsize, f)!=NULL) {
					if (buffer[0]=='}') break;																	// } mean end of query data
					x = (buffer[0]=='\t');																		// x=1 if initial char is a TAB
					buffer[strlen(buffer)-1]='\r';															// replace '\n' char with '\r'
					strcat(engine[n].query, buffer+x);														// add string considering the TAB
					strcat(engine[n].query, "\n");															// add '\n' char at the end
				}
				continue;
			
			default:
				onl = strchr(buffer,'\t') - buffer;															// find option name length
				pov = strchr(buffer,'=') + 2;																	// find option value start position
				ovl = strchr(pov,'\n') - pov;																	// find option value length

				on = strndup(buffer, onl);																		// save the option name
				ov = strndup(pov, ovl);																			// save the option value

				engineSetOption(&engine[n],on,ov);															// set engine.option = value
				continue;
		}

	fclose(f);
	n++;
	
	for (i=0; i<n; i++) {
		loadParserScript(&engine[i]);																			// load the engine's parser script
		setEngineSocketAddress(&engine[i]);																	// calculate the engine's INET address
	}
	
	nengines = n;
	return n;																										// return number of inserted engines
}
// ----------------------------------------------------------------------------






// freeSearchEngineMemory -----------------------------------------------------
// Free the memory occupied by datas of the search engine structure
// ----------------------------------------------------------------------------
void freeSearchEnginesMemory() {
	short i,ii=0;
	
	
	for (i=0; i<nengines; i++) {
		free(engine[i].host);
		free(engine[i].sa);
		ii=0;
		while ((engine[i].parser_cmd[ii]!=NULL) & (ii<MAX_PARSER_INSTRUCTION)) {
			free(engine[i].parser_cmd[ii]);
			ii++;
		}
	}
}
// ----------------------------------------------------------------------------
