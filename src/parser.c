#include <stdio.h>
#include <string.h>																								// per strlen, strncpy
#include <strings.h>																								// per bzero
#include <stdlib.h>																								// per malloc

#include "macro.h"
#include "core.h"
#include "parser.h"
#include "utils.h"
#include "searchengines.h"
#include "bench.h"
#include "cmdline.h"






// clearParserInformations, clearNewEntry, clearAboutCornet -------------------
// Reset the PARSER or the NEWENTRY or the ABOUTCOUNTER structures and pointers
// ----------------------------------------------------------------------------
void clearParserInformations(struct engineWork *e) {
	e->parser.ic = 0;
	e->parser.s = NULL;
}

void clearNewEntry(struct engineWork *e) {
	if (e->entry.url!=NULL) 	{free(e->entry.url);		e->entry.url = NULL;}
	if (e->entry.title!=NULL)	{free(e->entry.title);	e->entry.title = NULL;}
	if (e->entry.text!=NULL)	{free(e->entry.text);	e->entry.text = NULL;}
}

void clearFoundCounter(struct engineWork *e) {
	if (e->entry.found!=NULL)	{free(e->entry.found);	e->entry.found = NULL;}
}
// ----------------------------------------------------------------------------






// printGrabbedEntry ----------------------------------------------------------
// Print grabbed data, until now.
// ----------------------------------------------------------------------------
void printGrabbedEntry(struct engineWork *e) {
	
	if (config.parser_print) {
		printf("<URL name=\"%s\"/>\n", e->entry.url);
		printf("\t<TITLE label=\"%s\"/>\n",e->entry.title);
		printf("\t<SNIPPET content=\"%s\"/>\n",e->entry.text);
		printf("\t<ENGINE name=\"%s\" rank=\"%d\"/>\n", e->info->name, e->entry.rank);
		printf("</URL>\n");
	}
	
	stats.prs_result++;																							// increase parsed result counter
	e->entry.rank++;																								// increase the rank of next NEWENTRY
	e->parsed++;																									// increase number of parsed entries
	clearNewEntry(e);																								// free mem occupied by old NEWENTRY
}
// ----------------------------------------------------------------------------






// depureString ---------------------------------------------------------------
// Move  the  given  pointer  to a new string cleaned by HTML TAGs. It free the
// memory pointed and occupied by the old string.
// ----------------------------------------------------------------------------
void depureString(char **str) {
	char *t, *s = *str;																							// easy pointer to the string
	short a=0,b=0;																									// temporary counter variables
	char skip='\0';
	
	t = cmalloc(strlen(s)+1);																					// allocate memory for the new string
	
	while (a<strlen(s)) {
		if (skip && s[a]!=skip) {a++; continue;}															// if SKIP char not found, go to next
		if (skip && s[a]==skip) {a++; skip='\0'; continue;}											// if SKIP char found, analyze
		if (s[a]=='<') {skip='>'; a++; continue;}															// if '<' found, look for '>'
		if (s[a]=='&') {skip=';'; a++; continue;}															// if '&' found, look for ';'
		if (s[a]=='\n') {a++; continue; }																	// skip '\n'
		t[b]=s[a];																									// if it's a normal char, collect it
		a++; b++;
	}
	
	*str = strndup(t, strlen(t));																				// alloc & point to exact required mem
	free(t); free(s);
}
// ----------------------------------------------------------------------------






// cmdUntil -------------------------------------------------------------------
// Divide the given string STR into token, using the delimiter DELIM, and after
// look into the buffer for first of them, grabbing text until it's position.
// ----------------------------------------------------------------------------
int cmdUntil(char *str, struct engineWork *e) {
	char delim = '|';																								// delimiter characters
	short len, j;
	char *tmp = strdup(str);																					// duplicate of str, needed for strsep
	char *t = tmp;																									// temporary pointer, needed for strsep
	char *k, *q, *p=NULL;																						// temporary vars, and first match ptr
	
	while (t!=NULL) {
		k = strsep2(&t, delim);																					// extract first token
		q = strstr(e->ib_next, k);																				// look if it's present in the buffer
		if (q==NULL) continue;																					// if not found, try with the next
		if (p==NULL || q<p) {p=q; j=strlen(k);}															// if it's the closest, update "p" & "j"
	}
	free(tmp);
	if (p==NULL) return 0;																						// if no-one found, retry
	len = p - e->ib_next;																						// calculate the length of the GRAB
	t = strndup(e->ib_next, len);																				// copy GRAB in a safe place & save ptr
	strAttach(e->parser.s, t);																					// copy it in the memory
	free(t);
	e->ib_next += len + j;																						// move the NEXT pointer forward
	return 1;
}
// ----------------------------------------------------------------------------






// cmdIf ----------------------------------------------------------------------
// Extract  first  number  L,  and  pattern T in the given STR string. Look for
// the  pattern  P  into  the buffer. Skip follow instruction if it's not found
// in the range L.
// ----------------------------------------------------------------------------
int cmdIf(char *str, struct engineWork *e) {
	char *p, *t;
	short m = strlen(e->ib_next);
	short l = atoi(str);
	short d;
	
	t = strchr(str, ' ') + 1;
	p = strstr(e->ib_next, t);																					// look into buffer for pattern

	if (p==NULL) {
		if (m<l) return 0;																						// if !found but !enought buffer, retry
	} else {
		d = (p - e->ib_next);																					// calculate the distance
		if (d<l) return 1;																						// if found and (position<range), ok
	}
	
	return 2;																										// if !found or out of range, skip
/*	
	l = atoi(strsep2(&t, ' '));																				// extract the search RANGE
	p = strstr(e->ib_next, t);																					// extract the pattern to look for
	if (p==NULL && m>=l) {free(tmp); return 2;}															// if !found & enought data, skip inst
	if (p==NULL && m<l) {free(tmp); return 0;}															// if !found but !enought data, retry
	if ((p - e->ib_next)<l) k=1; else k=2;																	// if !found within the RANGE skip inst
	free(tmp);
*/
}
// ----------------------------------------------------------------------------






// changePtr ------------------------------------------------------------------
// ----------------------------------------------------------------------------
int changePtr(char *p, struct engineWork *e) {
	if (strcmp(p, "URL")==0)	{e->parser.s = &e->entry.url;	 	return 1;}
	if (strcmp(p, "TITLE")==0)	{e->parser.s = &e->entry.title;	return 1;}
	if (strcmp(p, "TEXT")==0)	{e->parser.s = &e->entry.text;	return 1;}
	if (strcmp(p, "FOUND")==0)	{e->parser.s = &e->entry.found;	return 1;}
	printf("Malformed PARSER SCRIPT, grab to <unknown>! Exit!\n");
	exit(-1);
}
// ----------------------------------------------------------------------------







// executeCmd -----------------------------------------------------------------
// Execute the given PARSER COMMAND. Accepted commands are
//    l = LOOK FOR <tag>
//    + = MOVE FORWARD OF <n>
//    - = MOVE BACKWARD OF <n>
//    g = GRAB <target>
//    u = UNTIL <tag>
//    c = CLEAN
//    j = JUMP TO <instruction>
//    w = WRITE <text>
//    i = IF <range> <tag>
//    p = PRINT
// ----------------------------------------------------------------------------
char executeCmd(char *cmd, struct engineWork *e) {
	char *p;
	
	if (cmd==NULL) {printf("Malformed PARSER SCRIPT, inexpected reach of end! Exit!\n"); exit(-1);}
	
	switch(cmd[0]) {
		case 'l':																									// LOOK FOR <tag>
			p = strstr(e->ib_next, cmd+2);																	// calculate position of <tag>
			if (p==NULL) return 0;																				// if not found, abort
			e->ib_next = p;																						// move the NEXT pointer to <tag>
			return 1;
			break;
	
		case '+':																									// MOVE FORWARD OF <n>
			e->ib_next += atoi(cmd+2);																			// move the NEXT pointer of <n> chars
			return 1;
			break;
			
		case '-':																									// MOVE BACKWARD OF <n>
			e->ib_next -= atoi(cmd+2);																			// move NEXT pointer back of <n> chars
			return 1;
			break;
			
		case 'g':																									// GRAB <target>
			return changePtr(cmd+2, e);
			break;
			
		case 'u':																									// UNTIL <tag>
			return cmdUntil(cmd+2, e);
			break;

		case 'i':																									// IF <range> <pattern>
			return cmdIf(cmd+2, e);
			break;
				
		case 'c':																									// CLEAN
			depureString(e->parser.s);																			// clean the last grabbed string
			return 1;
			break;
	
		case 'j':																									// JUMP <instruction>
			e->parser.ic = atoi(cmd+2)-1;																		// set parses IC to the given instr.
			return 1;
			break;

		case 'p':																									// PRINT
			printGrabbedEntry(e);																				// print what grabbed until now
			return 1;
			break;
		
		case 'w':																									// WRITE <text>
			strAttach(e->parser.s, cmd+2);																	// attach to <target> the given <text>
			return 1;
			break;

		case 'q':																									// WRITE <text>
			setNoInterest(e);
			return 1;
			break;
					
		default:
			printf("Malformed PARSER SCRIPT COMMAND '%s'! Exit!\n",cmd);
			exit(-1);
			break;
	}
	return 0;
}
// ----------------------------------------------------------------------------






// nextCmd --------------------------------------------------------------------
// Return next command ready to be executed
// ----------------------------------------------------------------------------
char *nextCmd(struct engineWork  *e) {
	return e->info->parser_cmd[e->parser.ic];																// return NEXT COMMAND to execute
}
// ----------------------------------------------------------------------------






// parse ----------------------------------------------------------------------
// Execute the parsing SCRIPT
// ----------------------------------------------------------------------------
void parse(struct engineWork *e) {
	char t=1;

	bench(0);
	while (t) {
		t = executeCmd(nextCmd(e), e);																		// execute NEXT CMD and save result
		e->parser.ic += t;																						// increment the IC if done
	}
	stats.prs_time += bench(0);
}
// ----------------------------------------------------------------------------
