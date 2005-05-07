#include "macro.h"

struct engineParser {
	short ic;																										// next command to execute
	char **s;																										// pointer to GRAB command choice
};

struct newEntry {
	char *url;
	char *title;
	char *text;
	char *found;
	short rank;
};

struct engineWork {
	struct engineInformations *info;
	short rpp;																										// results per page
	short startpage;																								// start page
	short endpage;																									// end page
	char ob[obsize+1];																							// output buffer
	char *ob_next;																									// pointer to next data to send
	char ib[ibsize+1];																							// input buffer
	char *ib_free;																									// pointer to free space in input buffer
	char *ib_next;																									// pointer to next data to analyze
	short readretry;																								// read retry count
	short writeretry;																								// write retry count
	short socket;																									// socket
	char *error;																									// encountered error
	int rxTime;																										// read time for the engine
	int requested;																									// number of result requested to engine
	int parsed;																										// number of result correctly parsed
	struct newEntry entry;																						// pointer to newEntry structure
	struct engineParser parser;																				// pointer to engine's parser
};

struct statistic {
	int ld_time;
	int rx_time;
	int rx_bytes;
	int tx_time;
	int tx_bytes;
	int sl_time;
	int prs_time;
	int prs_result;
	int req_result;
	} stats;

void engineSetup(char *arg, struct engineWork *e);

int engineConnect(struct engineWork *e);

char stillAny(struct engineWork e[], short n);

short maxEngineSocket(struct engineWork e[], short n);

void setTimeout(short ms, struct timeval *t);

void setSocketSet(struct engineWork e[], short n, fd_set *rset, fd_set *wset);

void decreaseRetry(struct engineWork e[], short n);

int tx(struct engineWork *e);

int rx(struct engineWork *e);

void printEngineInfo(short i, struct engineWork e, fd_set rset, fd_set wset);

void dumpToDisk(struct engineWork *e);

int search(char *q, short used, char *argv[]);

void setNoInterest(struct engineWork *e);
