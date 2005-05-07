#include <stdio.h>
#include <stdlib.h>																								// per atoi
#include <string.h>																								// per strlen, strcat
#include <unistd.h>																								// per write
#include <sys/types.h>																							// per open
#include <sys/stat.h>																							// per open
#include <fcntl.h>																								// per open
#include <sys/select.h>																							// per select
#include <errno.h>																								// per errno

#include "macro.h"
#include "searchengines.h"
#include "core.h"
#include "bench.h"
#include "parser.h"
#include "socketutils.h"
#include "cmdline.h"
#include "utils.h"


char *query;





// resetStatistic, printStatistic, printFound ---------------------------------
// Reset or print the statistics on the done work
// ----------------------------------------------------------------------------
void resetStatistic() {
	stats.ld_time		= 0;
	stats.rx_time		= 0;
	stats.rx_bytes		= 0;
	stats.tx_time		= 0;
	stats.tx_bytes		= 0;
	stats.sl_time		= 0;
	stats.prs_time		= 0;
	stats.prs_result	= 0;
	stats.req_result	= 0;
	}

void printStatistic() {
	printf("<STATS>\n");
	printf("\t<LOADING time=\"%d\"/>\n",stats.ld_time);
	printf("\t<READ bytes=\"%d\"/>\n", stats.rx_bytes);
	printf("\t<WRITE time=\"%d\" bytes=\"%d\"/>\n",stats.tx_time, stats.tx_bytes);
	printf("\t<SELECT time=\"%d\"/>\n",stats.sl_time);
	printf("\t<PARSER time=\"%d\" results=\"%d\"/>\n",stats.prs_time, stats.prs_result);
	printf("\t<REQUEST query=\"%s\" result=\"%d\"/>\n",query, stats.req_result);
	printf("</STATS>\n");
}

void printFound(struct engineWork e[], short used) {
	short i;
	printf("<FOUND>\n");
	for (i=0; i<used; i++)
		if (e[i].entry.found!=NULL) {
			printf("\t<ENGINE name=\"%s\" found=\"%s\"/>\n", e[i].info->name, e[i].entry.found);
			free(e[i].entry.found);
		}
	printf("</FOUND>\n");
}

void printError(struct engineWork e[], short used) {
	short i;
	printf("<ERROR>\n");
	for (i=0; i<used; i++)
		if (e[i].error!=NULL) {
			printf("\t<ENGINE name=\"%s\" error=\"%s\"/>\n", e[i].info->name, e[i].error);
			free(e[i].error);
		}
	printf("</ERROR>\n");
}

void printRxTimes(struct engineWork e[], short used) {
	short i;
	printf("<RXTIMES>\n");
	for (i=0; i<used; i++)
		printf("\t<ENGINE name=\"%s\" rxTime=\"%d\"/>\n", e[i].info->name, e[i].rxTime);
	printf("</RXTIMES>\n");
	}

void printParsedResults(struct engineWork e[], short used) {
	short i;
	printf("<PARSER>\n");
	for (i=0; i<used; i++)
		printf("\t<ENGINE name=\"%s\" requested=\"%d\" parsed=\"%d\"/>\n", e[i].info->name, e[i].requested, e[i].parsed);
	printf("</PARSER>\n");
	}

// ----------------------------------------------------------------------------






// setReadInterest, setWriteInterest,  setNoInterest --------------------------
// Enable and disable the WRITING or the READING interest for the engine
// ----------------------------------------------------------------------------
void setReadInterest(struct engineWork *e) {
	e->readretry  = 5;
	e->writeretry = 0;
}

void setWriteInterest(struct engineWork *e) {
	e->readretry  = 0;
	e->writeretry = 5;
}

void setNoInterest(struct engineWork *e) {
	e->readretry  = 0;
	e->writeretry = 0;
	socketClose(e->socket);																						// close the engine's socket
}
// ----------------------------------------------------------------------------






// engineError ----------------------------------------------------------------
// Set  no  interest  for  the  broken engine, and print the error in the FOUND
// variable
// ----------------------------------------------------------------------------
int engineError(struct engineWork *e, char *error) {
	e->error = malloc(strlen(error)+strlen(strerror(errno))+4);										// reserve memory for error description
	strcpy(e->error,error);																						// copy the passed string
	strcat(e->error," (");																						// insert the separator
	strcat(e->error,strerror(errno));																		// insert the ERRNO description
	strcat(e->error,")");																						// insert the tail chars
	setNoInterest(e);																								// reset interest in the broken engine
	return -1;
}
// ----------------------------------------------------------------------------






// engineInit -----------------------------------------------------------------
// Initialize engine's working parameters
// ----------------------------------------------------------------------------
int engineInit(struct engineWork *e) {
	e->entry.url	= NULL;																						// reset newEntry URL pointer
	e->entry.title	= NULL;																						// reset newEntry TITLE pointer
	e->entry.text	= NULL;																						// reset newEntry TEXT pointer
	e->entry.found	= NULL;																						// reset newEntry FOUND pointer
	e->entry.rank	= 1;																							// reset the new-entry RANK counter
	
	e->rxTime		= 0;																							// reset the RX time counter
	e->error			= NULL;																						// reset the engine ERROR report
	e->parsed		= 0;																							// reset the number of requested result
	e->requested	= 0;																							// reset the number of parsed result
	
	e->socket = createTCPSocket();																			// create engine TCP socket
	if (e->socket==-1) return engineError(e, "createTCPSocket");									// if error, report it and stop engine
	
	setSocketOptions(e->socket);																				// set socket options (NON_BLOCK)	
	setWriteInterest(e);																							// set WRITING INTEREST
	return 1;
}
// ----------------------------------------------------------------------------






// readyToWrite, readyToRead --------------------------------------------------
// Return TRUE if the engine have something to read/write and is ready to do it
// ----------------------------------------------------------------------------
char readyToWrite(struct engineWork e, fd_set *wset) {
	return (e.writeretry>0 && FD_ISSET(e.socket, wset));
}

char readyToRead(struct engineWork e, fd_set *rset) {
	return (e.readretry>0 && FD_ISSET(e.socket, rset));
}
// ----------------------------------------------------------------------------






// search----------------------------------------------------------------------
// Launch the search of the query Q on the given engines, parsing their setting
// ----------------------------------------------------------------------------
int search(char *q, short used, char *argv[]){
	struct engineWork engine[used];																			// array of used search engines
	struct timeval timeout;																						// timeout structure for SELECT
	short maxfd;																									// maximum socket value
	fd_set rset, wset;																							// read & write structures for SELECT
	short readysocket, i;

	query = q;																										// set the global query var to given

	bench(1);																										// start LOADING timer
	resetStatistic();																								// reset STATS structure

	loadEnginesInformations("engines.dat");																// build the usable web engines list

	for (i=0; i<used; i++) {
		engineInit(&engine[i]);																					// create TCP-socket, set W interest
		engineSetup(argv[i], &engine[i]);																	// setup the engine with given arg
		engineConnect(&engine[i]);																				// connect the engine
	}

	stats.ld_time = bench(1);																					// save LOADING time and reset timer
	
	while (stillAny(engine, used)) {																			// while at least one've smthing to R/W
		setTimeout(5000, &timeout);																			// set SELECT timeout
		setSocketSet(engine, used, &rset, &wset);															// prepare the FD_SET structures
		maxfd = maxEngineSocket(engine, used) + 1;														// calculate the maximum between FDs
		
		bench(1);
		readysocket = select(maxfd, &rset, &wset, NULL, &timeout);									// wait for a ready socket
		stats.sl_time += bench(1);																				// save time spent in SELECT
	
		switch (readysocket) {
			case -1:																									// ERROR, then retry
				if (config.verbose) printf("SELECT: failed!\n");
				continue;
			
			case 0:																									// TIMEOUT, decrease retrycount & retry
				if (config.verbose) printf("SELECT: timeout!\n");
				decreaseRetry(engine, used);
				continue;
			
			default:																									// AT LEAST ONE IS READY
				for (i=0; i<used; i++) {																		// look between the used engines
					if (readyToWrite(engine[i], &wset)) {													// if it's socket is ready for writing
						tx(&engine[i]);																			// try to send waiting datas
					}
					if (readyToRead(engine[i], &rset)) {													// if it's socket is ready for reading
						rx(&engine[i]);																			// try to read waiting datas
						if (config.result_parsing) parse(&engine[i]);									// parse what read
					}
				}
				continue;
		}
	}

	printStatistic();																								// print statistics
	printRxTimes(engine, used);																				// print RX times for the engines
	printParsedResults(engine, used);																		// print statistic about the parser
	printFound(engine, used);																					// print grabbed "found results"
	printError(engine, used);																					// print encountered errors
	freeSearchEnginesMemory();																					// free memory of usable web engines
	return 0;
}
// ----------------------------------------------------------------------------






// clearInputBuffer, clearOutputBuffer ----------------------------------------
// Clear and initialize INPUT or OUTPUT buffer, and pointers.
// ----------------------------------------------------------------------------
void clearInputBuffer(struct engineWork *e) {
	bzero(e->ib, ibsize);
	e->ib_free = e->ib;
	e->ib_next = e->ib;
}

void clearOutputBuffer(struct engineWork *e) {
	bzero(e->ob, obsize);
	e->ob_next = e->ob;
}
// ----------------------------------------------------------------------------






// dumpToDisk -----------------------------------------------------------------
// Append  the  content of the cache to a file on the disk with the name of the
// engine host
// ----------------------------------------------------------------------------
void dumpToDisk(struct engineWork *e) {
	short f;
	f = open(e->info->host, O_CREAT|O_APPEND|O_WRONLY, S_IRUSR|S_IWUSR);
	write(f, e->ib_free, strlen(e->ib_free));
	close(f);
}
// ----------------------------------------------------------------------------






// enginePageCalc -------------------------------------------------------------
// Calculate the %PAGE index for queries.
//    0) %PAGE = page
//		1) %PAGE = page + 1
//    2) %PAGE = page * rpp
//    3) %PAGE = (page * rpp) + 1
// ----------------------------------------------------------------------------
char *enginePageCalc(char pagecalc, short rpp, short page) {
	short n;
	
	switch (pagecalc) {
		case 0:
			n = (page - 1);
			break;
		case 1:
			n = (page - 1) + 1;
			break;
		case 2:
			n = rpp * (page - 1);
			break;
		case 3:
			n = (rpp * (page - 1)) + 1;
			break;
	}
	return itoa(n);
}
// ----------------------------------------------------------------------------






// createHTTPRequest ----------------------------------------------------------
// This  function make the HTTP REQUEST using the given parameters. Then return
// a pointer to it.
// ----------------------------------------------------------------------------
void createHTTPRequest(struct engineWork *e) {
	char *t1, *t2, *t3, *npage, *rpp;																		// temporary buffers
	extern char *query;
	
	npage = enginePageCalc(e->info->npc, e->rpp, e->startpage);										// calculate page number
	rpp = itoa(e->rpp);																							// convert page number into integer
	t1 = strrpl(e->info->query, "%QUERYSTR", query);													// replace the query string
	t2 = strrpl(t1, "%RESULTS", rpp);																		// replace the desidered result number
	t3 = strrpl(t2, "%PAGE", npage);																			// replace the desidered page number
	
	clearOutputBuffer(e);																						// clear the output buffer
	strcpy(e->ob, t3);																							// put maked HTTP REQUEST into buffer

	if (config.verbose) printf("HTTP REQUEST: socket=%d, engine=%s\n%s",e->socket, e->info->host, t3);
	
	free(t1); free(t2); free(t3); free(npage); free(rpp);
}
// ----------------------------------------------------------------------------






// bufferFreeMemory -----------------------------------------------------------
// Calculate free memory left in the engine's input-buffer
// ----------------------------------------------------------------------------
int bufferFreeMemory(struct engineWork *e) {
	return (e->ib + ibsize) - e->ib_free;
}
// ----------------------------------------------------------------------------






// tx -------------------------------------------------------------------------
// If  there  still  is  something to send in the output-buffer, send it, else,
// prepare to receive data
// ----------------------------------------------------------------------------
int tx(struct engineWork *e) {
	short k=strlen(e->ob_next);

	bench(0);
	if (e->ob_next[0]!='\0') {																					// if there is something ready to send
		k=write(e->socket, e->ob_next, k);																	// try to send it on the socket
		
		if (k==-1) return engineError(e, "write");														// if error in WRITE, report and stop
		
		e->ob_next += k;																							// move ptr according to how much sent
		stats.tx_bytes += k;																						// increment sent bytes counter
	
	} else {
		clearInputBuffer(e);																						// prepare INPUT buffer to receive data
		clearParserInformations(e);																			// reset the parse script controller
		setReadInterest(e);																						// set READING interest
	}
	
	if (config.verbose) printf("TX: socket=%d, write=%d\n", e->socket, k);
	stats.tx_time += bench(0);
	return 1;
}
// ----------------------------------------------------------------------------






// requestNewPage -------------------------------------------------------------
// Close current TCP connection with the server, increase the page counter, and
// ask for the new page, opening a new connection with the server.
// ----------------------------------------------------------------------------
int requestNewPage(struct engineWork *e) {
	short k = e->socket;

	e->startpage++;																								// choose new page
	e->socket = createTCPSocket();																			// create new socket
	if (e->socket==-1) return engineError(e, "createTCPSocket");									// if error, report it and stop engine
	
	engineConnect(e);																								// connect the engine to web server
	createHTTPRequest(e);																						// create new request in output buffer
	clearFoundCounter(e);																						// clear FOUND counter
	setWriteInterest(e);																							// set WRITING interest
	
	if (config.socket_trace) printf("SK: engine=%s, old_socket=%d, new_socket=%d\n",e->info->name, k, e->socket);
	return 1;
}
// ----------------------------------------------------------------------------






// rx -------------------------------------------------------------------------
// Receive  as  much  data  is  possible  from  the  server and store it in the
// engine's input-buffer
// ----------------------------------------------------------------------------
int rx(struct engineWork *e) {
	int k=0;
	int free = bufferFreeMemory(e);

	bench(0);
	k = read(e->socket, e->ib_free, free);																	// read as much data is possible
	
	if (k==-1) return engineError(e, "read");																// if error in READ, report it and stop
	
	stats.rx_bytes += k;																							// increment the receive bytes counter
	if (config.dump_to_disk) dumpToDisk(e);																// if requested, dump to disk
	e->ib_free += k;																								// move the FREE-pointer forward

	if (k==0) {																										// if READ returned 0, data finished
		socketClose(e->socket);																					// close the engine's socket
		if (config.socket_trace) printf("SK: engine=%s, closing_socket=%d\n", e->info->name, e->socket);
				
		if (e->startpage < e->endpage)																		// if it's not last page, ask next
			requestNewPage(e);
		else
			setNoInterest(e);																						// if it's the last page, stop reading
	}
		
	if (config.verbose) printf("RX: socket=%d, free=%d, read=%d\n", e->socket, free, k);
			
	e->rxTime += bench(0);																						// save the RX time for engine
	return k;
}
// ----------------------------------------------------------------------------






// decreaseRetry --------------------------------------------------------------
// Scroll  all working-engine's structures and decrease the retry counter after
// a SELECT timeout
// ----------------------------------------------------------------------------
void decreaseRetry (struct engineWork e[], short n) {
	short i;
	for (i=0; i<n; i++) {
		if (e[i].readretry>0) e[i].readretry--;
		if (e[i].writeretry>0) e[i].writeretry--;
	}
}
// ----------------------------------------------------------------------------






// setSocketSet ---------------------------------------------------------------
// After  clearing  the  FD_SET structures, set their bits checking the engines
// retry counters.
// ----------------------------------------------------------------------------
void setSocketSet(struct engineWork e[], short n, fd_set *rset, fd_set *wset) {
	short i;
	
	FD_ZERO(rset); FD_ZERO(wset);
	for (i=0; i<n; i++) {
		if (e[i].readretry>0) {
			FD_SET(e[i].socket, rset);
			continue;
		}
		if (e[i].writeretry>0) {
			FD_SET(e[i].socket, wset);
			continue;
		}
	}
}
// ----------------------------------------------------------------------------






// setTimeout -----------------------------------------------------------------
// Set the TIMEVAL structure to the specified timeout value
// ----------------------------------------------------------------------------
void setTimeout(short ms, struct timeval *t) {
	t->tv_sec = ms / 1000;
	t->tv_usec = ms % 1000;
}
// ----------------------------------------------------------------------------






// maxEngineSocket ------------------------------------------------------------
// Find maximum socket value in the given engineWork array
// ----------------------------------------------------------------------------
short maxEngineSocket(struct engineWork e[], short n) {
	short i, max=0;
	for (i=0; i<n; i++)
		if (max<e[i].socket) max=e[i].socket;
	return max;
}
// ----------------------------------------------------------------------------






// stillAny -------------------------------------------------------------------
// Return TRUE if there is at least one engine still at work
// ----------------------------------------------------------------------------
char stillAny(struct engineWork e[], short n) {
	short i;
	
	for (i=0; i<n; i++)
		if (e[i].readretry>0 || e[i].writeretry>0) return 1;
	return 0;
}
// ----------------------------------------------------------------------------






// engineSetup ----------------------------------------------------------------
// Fill  the  engine  work structure with parameter's data, or with the default
// engine preferences if no data were given
// ----------------------------------------------------------------------------
void engineSetup(char *arg, struct engineWork *e) {
	char *name, *results, *pages, *startpage;

	name			= strsep2(&arg, '=');																		// look for '=' to grab the name
	results		= strsep2(&arg, ',');																		// look for ',' to grab results4page
	pages 		= strsep2(&arg, ',');																		// look for ',' to grab pages number
	startpage	= arg;																							// last num, if exist, is start-page

	e->info = ptrToEngineInformations(name);																// set the pointer to engine infos
	
	if (results!=NULL)	e->rpp = atoi(results);															// if results4page specified, use it
		else					e->rpp = e->info->rpp;															// if not specified, use default
	
	if (startpage!=NULL)	e->startpage = atoi(startpage);												// if start page specified, use it
		else					e->startpage = 1;																	// if not specified, start from first
	
	if (pages!=NULL)		e->endpage = atoi(pages) + e->startpage - 1;								// if number of pages specified, use
		else					e->endpage = e->info->pag + e->startpage - 1;							// if not specified, use default
	
	e->requested = e->rpp * (e->endpage - e->startpage + 1);											// calc number of requested results
	stats.req_result += e->requested;																		// calc total requested results
	e->entry.rank = ((e->startpage-1)*e->rpp) + 1;														// calculate first entry rank
	
	createHTTPRequest(e);
}
// ----------------------------------------------------------------------------






// engineConnect --------------------------------------------------------------
// Connect the engine with it's socket, using default engine informations
// ----------------------------------------------------------------------------
int engineConnect(struct engineWork *e) {
	return socketConnect(e->socket, e->info->sa);
}
// ----------------------------------------------------------------------------





// printEngineInfo ------------------------------------------------------------
// Print to screen the avaiable engine's informations
// ----------------------------------------------------------------------------
void printEngineInfo(short i, struct engineWork e, fd_set rset, fd_set wset) {
	printf("\n");
	printf("=== Engine %d ===\n",i);
	printf("host\t\t: %s\n",e.info->host);
	printf("port\t\t: %d\n",e.info->port);
	printf("rpp\t\t: %d\n", e.rpp);
	printf("startpage\t: %d\n",e.startpage);
	printf("endpage\t\t: %d\n",e.endpage);
	printf("readretry\t: %d\n",e.readretry);
	printf("writeretry\t: %d\n",e.writeretry);
	printf("socket\t\t: %d\n",e.socket);
	printf("wset\t\t: %d\n",FD_ISSET(e.socket, &wset));
	printf("rset\t\t: %d\n",FD_ISSET(e.socket, &rset));
}
// ----------------------------------------------------------------------------
