#include <stdio.h>
#include <strings.h>																								// per bzero
#include <netdb.h>																								// per gethostbyname
#include <sys/types.h>																							// per inet_ntop
#include <sys/socket.h>																							// per inet_ntop
#include <arpa/inet.h>																							// per inet_ntop
#include <stdlib.h>																								// per malloc
#include <string.h>																								// per strcpy, strcat
#include <unistd.h>																								// per close
#include <fcntl.h>																								// per fcntl
#include <ctype.h>																								// per isdigit

#include "utils.h"
#include "macro.h"
#include "searchengines.h"


#ifdef IPV6
	#define AF_INET AF_INET6
#endif






// CreateTCPSocket ------------------------------------------------------------
// Used  to build TCP socket. Can be useful if we will decide to use particular
// flags in the socket costruction.
// ----------------------------------------------------------------------------
int createTCPSocket() {
	return socket(AF_INET, SOCK_STREAM, 0);
}
// ----------------------------------------------------------------------------






// GetHostEntryAddress --------------------------------------------------------
// Provide hostname resolution capabilities. On error it halt and explain.
// ----------------------------------------------------------------------------
struct in_addr *getHostEntryAddress(char *address) {
	#ifdef DNS_DEBUG
	char str[46];																									// INET6_ADDRSTRLEN in <netinet/in.h>
	#endif
	struct hostent *tmp=NULL;
	
	tmp = gethostbyname2(address, AF_INET);
	if (tmp==NULL)
		switch (h_errno) {
			case HOST_NOT_FOUND: printf("Host not found: %s\n",address); exit(-1);
			case NO_ADDRESS: printf("Host valid but without IP address: %s\n",address); exit(-1);
			case NO_RECOVERY: printf("Not recoverable server error with address: %s\n",address); exit(-1);
			case TRY_AGAIN: printf("Server temporary error, try again later: %s\n",address); exit(-1);
		}
	
	#ifdef DNS_DEBUG
	inet_ntop(tmp->h_addrtype, tmp->h_addr_list[0], str, sizeof(str));
	printf("Resolved %s in %s\n",address,str);
	#endif

	return (struct in_addr *) tmp->h_addr_list[0];
}
// ----------------------------------------------------------------------------






// createSocketAddress --------------------------------------------------------
// This  function create a SOCKADDR structure resolving given ADDRESS and PORT.
// A pointer to this structure is returned.
// ----------------------------------------------------------------------------
struct sockaddr *createSocketAddress(char *address, short port) {
	struct sockaddr_in *sa=NULL;
	struct in_addr *ia=NULL;

	sa = cmalloc(sizeof(struct sockaddr_in));
	
	sa->sin_family = AF_INET;
	sa->sin_port = htons(port);
	
	if (isdigit(address[0]))																					// check if the address is "dotted"
		inet_pton(sa->sin_family, address, &sa->sin_addr);
	else {																											// if not, resolve the domain address
		ia = getHostEntryAddress(address);
		memcpy(&sa->sin_addr, ia, sizeof(struct in_addr));
	}
	return (struct sockaddr *) sa;
}
// ----------------------------------------------------------------------------






// SocketConnect --------------------------------------------------------------
// Connect  a  socket descriptor to a SocketAddress, providing a quick and easy
// way to get connected with a server.
// ----------------------------------------------------------------------------
int socketConnect(short socket, struct sockaddr *sa) {
	return connect(socket, sa, sizeof(struct sockaddr));
}
// ----------------------------------------------------------------------------






// SocketClose ----------------------------------------------------------------
// Send the CLOSE signal to the server, and close the socket
// ----------------------------------------------------------------------------
int socketClose(short socket) {
	shutdown(socket, SHUT_RDWR);
	return close(socket);
}
// ----------------------------------------------------------------------------






// setSocketNonBlocking -------------------------------------------------------
// Set the given socket as NON_BLOCK. Return old socket's flags
// ----------------------------------------------------------------------------
int setSocketNonBlocking(short socket) {
	int flags=0;
	
	flags = fcntl(socket, F_GETFL, 0);
	fcntl(socket, F_SETFL, flags|O_NONBLOCK);
	return flags;
}
// ----------------------------------------------------------------------------





// setSocketOptions -----------------------------------------------------------
// Set the socket options
// ----------------------------------------------------------------------------
int setSocketOptions(short socket) {
	return setSocketNonBlocking(socket);
}
// ----------------------------------------------------------------------------
