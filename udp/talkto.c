#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <err.h>
#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#define PORT "4950"
#define BUFSIZE 128

void *get_in_addr(struct sockaddr *sa) {
	if (sa->sa_family == AF_INET) 
		return &((struct sockaddr_in*) sa)->sin_addr; 
	else if (sa->sa_family == AF_INET6) 
		return &((struct sockaddr_in6*) sa)->sin6_addr;
	else 
		errx(1, "get_in_addr()/undefined family");
}

int main(int argc, char *argv[]) {

	if (argc != 3) {
		errx(1, "argc=%d\nusage: talkto <hostname or ip> <msg>", argc);
	}

	struct addrinfo		hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype	= SOCK_DGRAM;
	hints.ai_family		= AF_INET;
	int status			= getaddrinfo( argv[1], PORT, &hints, &res );  
	if (status > 0) 
		errx(1, "%s", gai_strerror(status));
	
	struct addrinfo *p;
	int sockfd;
	for ( p = res; p != NULL; p=p->ai_next ) {
		sockfd = socket( p->ai_family, p->ai_socktype, p->ai_protocol );
		if (sockfd == -1) {
			perror("talker/socket");
			continue;
		}
		
		break;
	}

	if (p == NULL) 
		errx(1, "couldn't open the socket");
	
	ssize_t nbytes;
	if ( (nbytes=sendto(sockfd, argv[2], strlen(argv[2]), 0, p->ai_addr, p->ai_addrlen)) == -1 ) 
		err(1, "talker/sendto()");
	printf("talker successfully has sent %zd bytes to %s\n", nbytes, argv[1]);	
	freeaddrinfo(res);
	close(sockfd);

	return 0;
}

