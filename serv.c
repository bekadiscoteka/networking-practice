#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#define PORT "3046"
#define BACKLOG 10
#define BUFSIZE 128


void sigchld_handler(int s)
{
    (void)s; // quiet unused variable warning

    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

void *get_in_addr( struct sockaddr_storage *s ) {  
	if (s->ss_family == AF_INET) 
		return &( (struct sockaddr_in *) s )->sin_addr; 
	else 
		return &( (struct sockaddr_in6 *) s )->sin6_addr;
}

int main(void) {

	char hostname[BUFSIZE];
	if (gethostname(hostname, BUFSIZE) == -1) {
		perror("server: gethostname");
		exit(1);
	}
	printf("HOST: %s\n", hostname);

	struct addrinfo hints, *res;
	memset( &hints, 0, sizeof hints );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	
	int status;
	if ( (status = getaddrinfo(NULL, PORT, &hints, &res)) != 0) {
		fprintf(stderr, "addr error: %s", gai_strerror(status));
		return 1;
	}

	int sockfd;
	for ( ; res != NULL; res = res->ai_next ) {
		if ( (sockfd = socket( res->ai_family, res->ai_socktype, res->ai_protocol)) == -1 ) {
			perror("server: socket");
			continue;
		}


		int yes = 1;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
			perror("setsockopt");
			exit(1);
		}
		
		if ( bind( sockfd, res->ai_addr, res->ai_addrlen ) == -1 ) {
			perror("server: bind");	
			continue;
		}

		break;
	}


	if (res == NULL) {
		fprintf(stderr, "run out of results");
		exit(2);
	}

	printf("socket is ready\n");

	if ( listen(sockfd, BACKLOG) == -1 ) {
		perror("server listen");
		exit(3);
	}


	printf("listening!\n");

	//freeaddrinfo(res);
	
	struct sigaction sa;
	sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

	printf("sigaction ready\n");

	int newfd;
	struct sockaddr_storage theiraddr;
	socklen_t theiraddr_len = sizeof(theiraddr);

	printf("waiting for connection...\n");


	while (1) {


		if ( (newfd = accept( sockfd, (struct sockaddr*) &theiraddr, &theiraddr_len )) == -1 ) {
			perror("server accept");
			continue;
		}

		
		getpeername( newfd, (struct sockaddr *) &theiraddr, &theiraddr_len );
		char theiraddr_char[INET6_ADDRSTRLEN];
		if (inet_ntop(theiraddr.ss_family, get_in_addr(&theiraddr), theiraddr_char, INET6_ADDRSTRLEN) == NULL) {
			fprintf(stderr, "inet_ntop() fail");	
			exit(1);
		}
		printf("connection established with %s\n", theiraddr_char);
		
		if (fork() == 0) {
			close(sockfd);
			if (send(newfd, "Hello, World!", 13, 0) == -1)
				perror("server: send");
			close(newfd);
			exit(0);
		}
		close(newfd);
	}

	return 0;	
}
