int createTCPSocket();

struct sockaddr *createSocketAddress(char *address, short port);

int socketConnect(short socket, struct sockaddr *address);

int socketClose(short socket);

int setSocketOptions(short socket);
