#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <err.h>
#include <sys/socket.h>
#include <errno.h>

#define PORT "4950"
#define BUFSIZE 128

void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) 
		return ((struct sockaddr_in*) sa)->sin_addr; 
	else if (sa->sa_family == AF_INET6) 
		return ((struct sockaddr_in6*) sa)->sin6_addr;
	else 
		errx(1, "get_in_addr()/undefined family");
}

int main(void) {
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype	= SOCK_DGRAM;
	hints.ai_family		= AF_UNSPEC;
	hints.ai_flag		= AF_PASSIVE;

	int status = getaddrinfo( NULL, PORT, &hints, &res );  
	if (status > 0) errx(1, "%s", gai_strerror(status));
		
	
	struct addrinfo *p;
	int sockfd;
	for ( p = res; p != NULL; p->ai_next ) {
		sockfd = socket( p->ai_family, p->ai_socktype, p->ai_protocol );
		if (sockfd == -1) {
			perror("listener/socket");
			continue;
		}
		
		if ( bind(sockfd, p->ai_addr, p->ai_addrlen) == -1 ) {
			perror("listener/bind");
			continue;
		}

		break;
	}

	if (p == NULL) errx(1, "couldn't open socket and bind addr");
	//freeaddrinfo(res);
	
	struct sockaddr_storage theiraddr;
	sockaddr_len theiraddrlen = sizeof theiraddr; 
	char buf[BUFSIZE];
	ssize_t msgsize;
	if ( (msgsize=recvfrom(sockfd, buf, BUFSIZE, 0, &theiraddr, &theiraddrlen)) == -1) 
		err(1, "listener/recvfrom");
	

	char addrstr[INET6_ADDRSTRLEN];
	printf("listener: received message from: %s\n", inet_ntop( ((struct sockaddr *) &theiraddr)->sa_family, get_in_addr(&theiraddr), addrstr, INET6_ADDRSTRLEN));

	printf("package is %zd bytes long\n", msgsize);
	printf("package contains %s", buf);
	return 0;
}



