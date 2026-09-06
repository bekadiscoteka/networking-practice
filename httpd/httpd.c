/* httpd.c */
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <err.h>
#include <string.h>
#include <unistd.h>

#define LOCALHOST "127.0.0.0"

void cli_conn(int c) {
	return;
}

/* return client sockfd, on fail return -1 */
int cli_accept(int s) {
	struct sockaddr_in sa;
	socklen_t socklen = sizeof(sa);
	memset(&sa, 0, socklen);
	int c = accept(s, (struct sockaddr*)&sa, &socklen);
	if (c == -1)
		return -1;
	return c;
}

/* return socket descriptor on success, -1 on fail */
int init_srv(int portno, int backlog) {

	int s;
	if ((s=socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		return -1;
	}

	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr(LOCALHOST);
	sa.sin_port = htons(portno); 

	if (bind(s, (struct sockaddr*)&sa, sizeof(sa)) == -1) {
		close(s);
		return -1;
	}

	if (listen(s, backlog) == -1) {
		close(s);
		return -1;
	}

	return s;
}

int main(int argc, char *argv[]) {

	if (argc != 2) {
		fprintf(stderr, "usage: %s <PORT>\n", argv[0]);
		return 1;
	}

	int s;
	char *port = argv[1];
	
	if ((s = init_srv(atoi(port), 10)) == -1) { 
		strerror(errno);
		return 1;
	}
	
	printf("server is listening on %s:%s\n", LOCALHOST, port);

	int c;
	while (1) {

		if (!(c=cli_accept(s))) {
			strerror(errno);
			continue;
		}

		printf("Incoming connection\n");

		if (!fork()) 
			cli_conn(c);
		
	}

	return 0;
}
