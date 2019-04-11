#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>
#include <cstdint>
#include <arpa/inet.h>

void error(char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, client_id, portno, ret;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char buffer[256];

    if (argc < 4) {
      fprintf(stderr,"usage %s client_id hostname port\n", argv[0]);
      exit(0);
    }
	client_id=atoi(argv[1]);

    portno = atoi(argv[3]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) error("ERROR opening socket");
    server = gethostbyname(argv[2]);
    if (server == NULL) error ("ERROR, no such host");
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
          (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
      error("ERROR connecting");



int end=0;

    
while(end==0){
	for (int i = 0; i < strlen(argv[1]); i++) {//for client_id
		buffer[i] = argv[1][i];
	}
	ret = write(sockfd, buffer, strlen(buffer));
	if (ret < 0) error("ERROR writing to socket");
	bzero(buffer, 256);

printf ("Enter file name: ");
    //scanf ("%s", buffer);
	fgets(buffer, sizeof(buffer), stdin);
	buffer[strlen(buffer)-1]='\0';
	int n=strcmp("nullfile", buffer);
	if(strlen(buffer)==0||n==0){
		printf("Nullfile entered");
		end=1;
		bzero(buffer, 256);
		break;
	}

	printf("entered file name was: %s\n",buffer);
	
	ret = write (sockfd, buffer, strlen(buffer));
	if (ret < 0) error("ERROR writing to socket");
	if( access( buffer, F_OK ) ==0 ) {
	printf("File exists\n");	

	int sum=0;
		ret = read(sockfd, &sum, sizeof(sum));
		if (ret < 0) error("ERROR reading from socket");
		printf("sum: %d\n", sum);
	} 
	
	
	
	else {
	printf("File doesn't exist\n");
	end=1;
	bzero(buffer,256);
	break;
	}
}
    bzero (buffer, 256);


/*
int net_tmp;

ret = read (sockfd, &net_tmp, sizeof(net_tmp));
   	 if (ret < 0) error ("ERROR reading from socket");
	sum=ntohl(net_tmp);	*/

    
return 0;
}
