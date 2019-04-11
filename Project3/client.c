#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>

void error(char *msg)
{ perror(msg);
  exit(0);
}

void main(int argc, char *argv[]) {
  int sockfd, portno, ret;
  struct sockaddr_in serv_addr;
  struct hostent *server;
  char buffer[256], cin[256];
  char *client_id, *result, *token = NULL;

  if (argc < 3)
  { fprintf(stderr, "Usage: Server-host-name Server-port-number\n");
    exit(0);
  }

  portno = atoi(argv[2]);
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) error("ERROR opening socket");

  server = gethostbyname(argv[1]);
  if (server == NULL) error("ERROR, no such host");
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *)server->h_addr,
        (char *)&serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(portno);

  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	  error("ERROR connecting");
  else
	  printf("connected\n");
  int end = 0;
  while (end==0)
  { bzero(buffer, sizeof(buffer));
    bzero(cin, sizeof(cin));
    result = NULL;
    token = NULL;

	printf("Enter file name: ");
	fgets(buffer, sizeof(buffer), stdin);
	buffer[strlen(buffer) - 1] = '\0';
	int n = strcmp("nullfile", buffer);
	if (strlen(buffer) == 0 || n == 0) {
		printf("Nullfile entered");
		bzero(buffer, 256);
		break;
	}
	printf("entered file name was: %s\n", buffer);

	ret = write(sockfd, buffer, strlen(buffer));
	char outputbuffer[256];
	bzero(outputbuffer, sizeof(outputbuffer));
	if (ret < 0) error("ERROR writing to socket");
	// add another while loop, stopped if received completion message from the server 
	if (access(buffer, F_OK) == 0) {
		ret = recv(sockfd, &outputbuffer, sizeof(outputbuffer),0);
		while (ret> 0) {
			printf("%s------------------------------------------\n", outputbuffer);
			if (outputbuffer[0]=='P') { break; }
			bzero(outputbuffer, sizeof(outputbuffer));
			ret = read(sockfd, &outputbuffer, sizeof(outputbuffer));
		}
			if (ret < 0) error("ERROR reading from socket");
			if (ret == 0) { 
				printf("End");
			break; }
		}
		else {
			printf("File doesn't exist\n");
			end = 1;
			bzero(buffer, 256);
			break;
		}
	if (ret < 0) error("Client ERROR reading from socket");  
  }
}
