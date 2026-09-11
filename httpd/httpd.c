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
#include <fcntl.h>
#include <sys/stat.h>


/* defines */
#define HOSTADDR "0.0.0.0"
#define PATH_404 "404.html"
#define PATH_405 "405.html"

/* structures */
struct sHttpParse {
	char method[8];
	char host[128];
};

typedef struct sHttpParse httpparse_t;

/* return parsed http on success, 0 on error */
httpparse_t *http_parse(char *s) {
	httpparse_t *hp = malloc(sizeof(struct sHttpParse));
	memset(hp, 0, sizeof(struct sHttpParse));
	
	char *p;
	for (p=s; p != NULL && *p != ' '; p++);
	if (p == NULL) {
		fprintf(stderr, "invalid HTTP format\n");	
		return 0;
	}
	
	
	*p = '\0';
	strcpy(hp->method, s);

	for (++p, s=p; p != NULL && *p!=' '; p++);
	if (p == NULL) {
		fprintf(stderr, "invalid HTTP host format\n");
		return 0;
	}

	*p = '\0';
	strcpy(hp->host, s);

	return hp;
}

void http_status(int c, int status, char *sstatus) {
	char buf[128];
	sprintf(buf, "HTTP/1.1 %d %s\nServer: Little Brother\n", status, sstatus);
	if (write(c, buf, strlen(buf)) == -1)
		fprintf(stderr, "%s\n", "error on writing HTTP status response");
	
	return;
}

void http_content(int c, int respf, long length) {
	char buf[128];
	ssize_t size;

	sprintf(buf, 
		"%s%ld\n\n", 
		"Content-Type: text/html; charset=UTF-8\n" 
		"Content-Length: ", 

		length
	);

	size = strlen(buf);
	do {
		write(c, buf, size);
	}
	while ( (size=read(respf, buf, 128)) != EOF );
	
	return;
}

void http_resp(int c, int status, char msg[], char dir[]) {
	struct stat st;
	int pagef = open(dir, O_RDONLY);
	if (pagef == -1) {
		int notfoundf = open(PATH_404, O_RDONLY);
		stat(PATH_404, &st);

		http_status(c, 404, "Not Found");
		http_content(c, notfoundf, st.st_size); 

		close(notfoundf);
	} else {
		stat(dir, &st);
		
		http_status(c, status, msg);		
		http_content(c, pagef, st.st_size);

		close(pagef);
	}
	
	return;
}


/* answers for GET responses */
void cli_conn(int c) {

	char buf[512];
	ssize_t rsize;
	httpparse_t *p;

	while (1) {

		if ( (rsize=read(c, buf, 512)) == -1) {
			fprintf(stderr, "%s\n", strerror(errno));
			return;
		}

		buf[rsize] = '\0';
		
		if ((p = http_parse(buf)) == NULL) 
			return;

		//printf("Method: %s\nHost: %s\n", p->method, p->host);
		
		if (strcmp(p->method, "GET") == 0) {
			char dir[128];
			strcpy(dir, ".");
			strcat(dir, p->host);

			http_resp(c, 200, "OK", dir);
		} 
		else 
			http_resp(c, 405, "Method Not Allowed", PATH_405);
	}
	
	free(p);
	close(c);
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
	sa.sin_addr.s_addr = inet_addr(HOSTADDR);
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
	
	printf("server is listening on %s:%s\n", HOSTADDR, port);

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

	close(s);

	return 0;
}
