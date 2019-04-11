#include <pthread.h>
#include <stdio.h>
#include "simos.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <pthread.h>



//===============================================================
// The interface to interact with clients
// current implementation is to get one client input from stdin
// --------------------------
// Should change to create server socket to accept client connections
// and then poll client ports to get their inputs.
// -- Best is to use the select function to get client inputs
// Also, during execution, the terminal output (term.c) should be
// sent back to the client and client prints on its terminal.
//===============================================================
/*
void one_submission()
{
	char fname[100];

	printf("Submission file: ");
	scanf("%s", &fname);
	if (Debug) printf("File name: %s has been submitted\n", fname);
	submit_process(fname);

}

void *process_submissions()
{
	char action;
	char fname[100];

	while (systemActive) one_submission();
	printf("Client submission interface loop has ended!\n");
}*/

int sockfd;
fd_set active_fd_set;

void error(char *msg)
{
	perror(msg);
	exit(1);
}

void send_client_result(int sfd, char* result)//send client a string
{
	char buffer[256];
	int ret;

	bzero(buffer, sizeof(buffer));
	sprintf(buffer, "%s", result);
	printf("sending %s\n", buffer);
	ret = send(sfd, buffer, sizeof(buffer), 0);
	if (ret < 0) error("Server ERROR writing to socket");
}

void read_from_client(int fd)
{
	char buffer[256];
	int ret, result;
	char *filename, *token;
	request_t *req;
	struct sockaddr_in cli_addr;
	socklen_t cli_addrlen = sizeof(cli_addr);

	bzero(buffer, sizeof(buffer));
	token = NULL;
	filename = NULL;
	req = NULL;

	ret = recv(fd, buffer, sizeof(buffer), 0);
	if (ret < 0) error("Server ERROR reading from socket");
	token = strtok(buffer, " ");
	if (ret == 0) return;
	filename = (char *)malloc(strlen(token));
	strcpy(filename, token);

	if (strcmp(filename, "nullfile") == 0)
	{
		close(fd);
		FD_CLR(fd, &active_fd_set);
	}
	else
	{
		getpeername(fd, (struct sockaddr *)&cli_addr, &cli_addrlen);
		req = malloc(sizeof(request_t));
		req->sockfd = fd;
		req->filename = filename;
		enqueue(*req);
		printf("Received file name %s\n", filename);
	}
}

void accept_client()
{
	int newsockfd, clilen;
	struct sockaddr_in cli_addr;

	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
	if (newsockfd < 0 &&systemActive==1) error("ERROR accepting");
	else if(systemActive==1)
	{
		printf("Accept client socket %d, %d\n", newsockfd, (int)cli_addr.sin_port);
		FD_SET(newsockfd, &active_fd_set);
	}
}

void initialize_socket(int portno)
{

	struct sockaddr_in serv_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) error("ERROR opening socket");
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR binding");
	listen(sockfd, 5);
}

void socket_select()
{
	//printf("in select\n"); for debugging
	int i;
	fd_set read_fd_set;

	FD_ZERO(&active_fd_set);
	FD_SET(sockfd, &active_fd_set);

	while (systemActive==1)//keep polling for active clients
	{ /* Block until input arrives on one or more active sockets. */
		read_fd_set = active_fd_set;
		if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
		{
			perror("select"); exit(EXIT_FAILURE);
		}

		/* Service all the sockets with input pending. */
		for (i = 0; i < FD_SETSIZE; ++i)
			if (FD_ISSET(i, &read_fd_set))
			{
				if (i == sockfd) accept_client();//receive client info
				else read_from_client(i);//get filename
			}
	}

}
void *client_reqhandler(void *arg)
{
	char *port = (char *)arg;
	int portno;

	portno = atoi(port);
	initialize_socket(portno);
	socket_select();
}

// submission thread is not activated due to input competition
pthread_t submissionThread;

void start_client_reqhandler(char *port)
{
	int ret;
	ret = pthread_create(&submissionThread, NULL, client_reqhandler, (void *)port);
	if (ret < 0) printf("%s\n", "thread creation problem");
}

void end_client_submission()
{
	int ret=-2;
	/*
	int i = 0;
	fd_set write_fd_set;
	while (systemActive==0)//keep polling for active clients
	{ // Block until input arrives on one or more active sockets.
	write_fd_set = active_fd_set;
	if (select(FD_SETSIZE, NULL, &write_fd_set, NULL, NULL) < 0)
	{
		printf("no active clients waiting\n");
		break;
	}

	//* Service all the sockets with input pending. 
	for (i = 0; i < FD_SETSIZE; ++i) {
		if (FD_ISSET(i, &write_fd_set)) {
			send_client_result(i, EOF);
			break;
		}
	}
}
	
	*/
	
	shutdown(sockfd,0);
	ret = pthread_join(submissionThread, NULL);
	printf("Client submission interface has finished %d\n", ret);
}



