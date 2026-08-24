#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>




int main(int argc, char *argv[]) {

	if (argc != 2) {
		fprintf(stderr, "usage: showip www.host.com");
		return 1;
	}

	int status;
	struct addrinfo *servinfo, hints, *res;

	char ipstr[INET6_ADDRSTRLEN];
	
	memset(&hints, 0, sizeof(hints));
	
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ( (status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0 ) {
		fprintf(stderr, "gaiaddrinfo: %s\n", gai_strerror(status));
		return 2;
	}

	for (struct addrinfo *p=res; p != NULL; p = p->ai_next) {
		char ipv[sizeof("IPvX")];
		void* addr;

		switch (p->ai_family) {
			case AF_INET:
				strcpy(ipv, "IPv4");
				addr = &( ((struct sockaddr_in*) (p->ai_addr))->sin_addr );				
				break;
			case AF_INET6:
				strcpy(ipv, "IPv6");
				addr = &( ((struct sockaddr_in6*) (p->ai_addr))->sin6_addr);
				break;
		}
		
		inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
		
		printf(" %s: %s\n", ipv, ipstr);	
	}

	freeaddrinfo(res);
}
