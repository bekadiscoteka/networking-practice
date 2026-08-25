#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <err.h>
#include <errno.h>

#define PORT "3490"
#define BUFSIZE 128

void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) 
		return &( ((struct sockaddr_in *) sa)->sin_addr );
	else if (sa->sa_family == AF_INET6) 
		return &( ((struct sockaddr_in6 *) sa)->sin6_addr );
	else errx(1, "socket is not INET type");
}

int main(int argc, char *argv[]) {
	struct addrinfo hints, **res;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	if (argc != 2) 
		errx(1, "usage: client <hostname>");
	
	int status;
	if ( (status = getaddrinfo(argv[1], PORT, &hints, res)) != 0) 
		errx(1, "getaddrinfo fail: %s", gai_strerror(status));
	
	printf("addr found\n");
	
	char straddr[INET6_ADDRSTRLEN];
	struct addrinfo *p;
	int sockfd;
	for (p = *res; p != NULL; p = p->ai_next) {
		if ( (sockfd=socket( p->ai_family, p->ai_socktype, p->ai_protocol)) == -1 ) {
			warn("client/socket");
			continue;
		}

		if ( connect( sockfd, p->ai_addr, p->ai_addrlen ) == -1 ) {
			warn("client/connect");
			continue;
		}

		if (inet_ntop(p->ai_family, get_in_addr(p->ai_addr), straddr, INET6_ADDRSTRLEN) == NULL) 
			err(1, "client/inet_ntop");

		break;
	}

	if ( p == NULL ) 
		errx(1, "couldn't establish connection with: %s:%s\n", straddr, PORT);

	printf("connection established with: %s:%s\n", straddr, PORT);

	char buf[BUFSIZE];
    if (recv(sockfd, buf, BUFSIZE, 0) == -1)
		err(1, "client/recv");

	printf("received message: %s", buf);	
	return 0;
}
