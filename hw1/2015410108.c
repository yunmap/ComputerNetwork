#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>

#define SERVER_PORT 47500
//socket(), connect(), send() 만 사용하면 될 듯.
int main(void) {
	struct sockaddr_in server; //server address
	char buf[] = "2015410108";
	int s;
	
	bzero((char *)&server, sizeof(server));
	server.sin_family = AF_INET; //connect internet
	//bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);
	server.sin_port = htons(SERVER_PORT);

	if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		perror("simplex-talk : socket");
		exit(1);
	}
	if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0) {
		perror("simplex-talk : connect");
		close(s);
		exit(1);
	}
	//memset(buf, "2015410108", 10)
	buf[10] = '\0';
	int len = strlen(buf) + 1;
	if (send(s, "2015410108", len, 0)) {
		perror("simplex-talk : send");
		close(s);
		exit(1);
	}
}