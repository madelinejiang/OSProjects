#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <sstream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <cstdint>
#include <arpa/inet.h>


#include <list>
#include <iterator>

static int parentpipe = 0;
struct QueueNode {
	int client_id;
	char* file_name;
	int sockfd;
	int portno;
	int sum = 0;
	int status = 0;

	QueueNode(int c_id, char* f_name, int sfd, int p_no) {
		client_id = c_id;
		file_name = f_name;
		sockfd = sfd;
		portno = p_no;
	}
};

/*Creates ReadyQueue list as a global variable....................*/
std::list <QueueNode> ReadyQueue;
/*.....................................*/

void error(char *msg)
{
	perror(msg);
	exit(1);
}


void summation(int x) {
	int tosum[256];
	FILE* inputFile;
	if (ReadyQueue.begin() == ReadyQueue.end()) {
		printf("Nothing in queue\n");
		return;
	}
	else {
		for (std::list<QueueNode>::iterator i = ReadyQueue.begin(); i != ReadyQueue.end(); ) {
			inputFile = fopen(i->file_name, "r");
			printf("client id is %i. file name is %s\n", i->client_id, i->file_name);
			if (inputFile == NULL) {
				error("Error could not find File");
				i=ReadyQueue.erase(i);
			}
			else {
				int count = 0;
				int sum = 0;
				int first_num;
				fscanf(inputFile, "%d", &first_num);

				while (count < first_num || fscanf(inputFile, "%d", &tosum[count]) != EOF) {
					fscanf(inputFile, "%d", &tosum[count]);
					sum += tosum[count];
					count++;
					sleep(x);
				}

				printf("%d %d\n", i->client_id, sum);
				i->sum = sum;
				fclose(inputFile);

				int ret = write(i->sockfd, &(i->sum), sizeof(sum));
				if (ret < 0)
					error("ERROR writing to socket for sum");
				i=ReadyQueue.erase(i);
			}
		}
	}
}

void q_execution(int sig_num)
{
	printf("A user signal is caught %d\n", sig_num);
	printf("Dequeue code here\n");
	fflush(stdout);
	if (ReadyQueue.begin() == ReadyQueue.end()) {
		char cstr[100];
		std::string tmp = "Empty Ready Queue!";
		strcpy(cstr, &tmp[0]);
		write(parentpipe, cstr, 100 + 1);
	}
	else {
		for (std::list<QueueNode>::iterator i = ReadyQueue.begin(); i != ReadyQueue.end(); i++) {

			std::stringstream qline;
			qline << i->client_id << ", " << i->file_name << ", " << i->sockfd << ", " << i->portno;
			std::string tmp = qline.str();
			char cstr[100];
			strcpy(cstr, &tmp[0]);
			write(parentpipe, cstr, 100 + 1);

		}
	}
	char cstr[100];
	cstr[0] = 'f';
	write(parentpipe, cstr, 100 + 1);

	fflush(stdout);

}


int main(int argc, char *argv[])
{
	setbuf(stdin, NULL);
	int pid;
	int sleepval;
	int pipefd_ptoc[2];
	int pipefd_ctop[2];
	char option[2];
	bool isCready = false;
	bool isAready = false;
	char buffer[256];
	char ctopbuffer[256];


	bzero(buffer, 256);
	if (argc < 3)
	{
		fprintf(stderr, "ERROR, no port provided. Usage: .exe portno\n sleepvalue");
		exit(1);
	}
	pipe(pipefd_ptoc);
	pipe(pipefd_ctop);
	parentpipe = pipefd_ctop[1];
	pid = fork();


	if (pid >= 0)
	{
		if (pid != 0)//parent
		{
			close(pipefd_ptoc[0]);//close unused read end: this pipe is to write to child
			close(pipefd_ctop[1]);//close unused write end:this pipe is to read from child

			printf("Admin: ret-pid=%d; PID=%d;\n", pid, getpid());
			sleep(5);


			while (1) {
				//sleep(.5);//let child run
				isAready = false;
				if (isCready == true) {
					isCready = false;
					sleep(.5);
					printf("Enter in a character:");
					fscanf(stdin, "%s", option); //error check for more than one character entered

					if (option[0] != 't'&& option[0] != 'x' && option[0] != 'q') {

						printf("Incorrect input. please enter q,t, or x\n");
						isCready == true;
					}
					if (strlen(option) > 1) {
						printf("Incorrect input. Enter only one character q,t or x \n");
						isCready == true;

					}
					else if (option[0] == 't') {
						write(pipefd_ptoc[1], option, (strlen(option) + 1));
						bzero(option, 2);

					}

					else if (option[0] == 'x') {
						write(pipefd_ptoc[1], option, (strlen(option) + 1));
						bzero(option, 2);
						fflush(stdout);
						sleep(5);


					}
					else if (option[0] == 'q') {
						isCready = false;
						int done = 0;
						bzero(option, 2);
						kill(pid, SIGRTMIN);

						while (done == 0) {
							int bytesr = read(pipefd_ctop[0], ctopbuffer, 101);
							//printf("bytes read %d\n", bytesr);
							if (ctopbuffer[0] == 'f')
							{
								done = 1;

							}
							else if (bytesr > 0)
							{
								printf("%s\n", ctopbuffer);
							}
							else {
								printf("error\n");
							}
						}
						sleep(5);
						isCready = true;
					}
				}
				else {
					read(pipefd_ctop[0], &isCready, sizeof(isCready) + 1);

				}

			}

			close(pipefd_ptoc[1]);//close write end 
			close(pipefd_ctop[0]);//close read end
			waitpid(pid, NULL, 0);

		}





		else if (pid == 0)//child process 
		{


			close(pipefd_ptoc[1]);//close write end; this pipe is to read from parent
			close(pipefd_ctop[0]);//close read end; this pipe is to write to parent
			isCready = false;
			printf("Computer: ret-pid=%d; PID=%d;\n", pid, getpid());

			/*...........................this is from server.cpp*/
			int sockfd, newsockfd, portno, clilen, client_id, sum;

			struct sockaddr_in serv_addr, cli_addr;
			int ret;

			FILE *inputFile;
			int first_num;
			int tosum[256];
			signal(SIGRTMIN, q_execution);
			if (argc < 3)
			{
				fprintf(stderr, "ERROR, no port provided. Usage: .exe portno sleepvalue\n");
				exit(1);
			}

			portno = atoi(argv[1]);
			sleepval = atoi(argv[2]);


			/*.......Creation of server socket.........*/

			sockfd = socket(AF_INET, SOCK_STREAM, 0);
			if (sockfd < 0) {
				error("ERROR opening socket");
			}
			bzero((char *)&serv_addr, sizeof(serv_addr));
			portno = atoi(argv[1]);
			serv_addr.sin_family = AF_INET;
			serv_addr.sin_addr.s_addr = INADDR_ANY;
			serv_addr.sin_port = htons(portno);
			if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
				error("ERROR binding");
			printf("Computer process server socket ready.\n");


			printf("Returned before while loop\n");

			listen(sockfd, 5);

			while (1) {

				printf("accepting clients\n");
				for (int i = 0; i < 3; i++) {
					char file_name[256];
					clilen = sizeof(cli_addr);
					newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
					if (newsockfd < 0)
						error("ERROR accepting");
					else {

						printf("Accepted client socket %d, %d\n",
							newsockfd, (int)cli_addr.sin_port);
					}
					bzero(buffer, 256);
					ret = read(newsockfd, buffer, 255);

					if (ret < 0)
						error("ERROR reading from socket from server side");
					sscanf(buffer, "%d", &client_id);
					bzero(buffer, 256);
					printf("client id %d\n", client_id);


					ret = read(newsockfd, buffer, 255);

					if (ret < 0)
						error("ERROR reading from socket");

					strcpy(file_name, buffer);
					bzero(buffer, 256);


					printf("%d %s\n", client_id, file_name);


					ReadyQueue.push_back(QueueNode(client_id, strdup(file_name), newsockfd, (int)cli_addr.sin_port));
					bzero(file_name, 256);
				}/*................................................*/

				isCready = true;
				write(pipefd_ctop[1], &isCready, sizeof(isCready) + 1);

				while (1) {
					signal(SIGRTMIN, q_execution);


					if (read(pipefd_ptoc[0], buffer, sizeof(buffer)) != 0) {

						strcpy(option, buffer);
						printf("Received char: %s\n", buffer);
						bzero(buffer, 256);

						if (option[0] == 't') {
							ReadyQueue.clear();
							kill(0, SIGTERM);
						}
						else if (option[0] == 'x') {
							summation(sleepval);
							isCready = true;
							write(pipefd_ctop[1], &isCready, sizeof(isCready) + 1);
							bzero(option, 2);
							printf("finished summation\n");
							fflush(stdout);
							break;
						}
						//sleep(5);
					}
					else {
						printf("Nothing in parent to child buffer\n");
						break;
					}

				}

			}

			close(pipefd_ctop[1]);
			close(pipefd_ptoc[0]);



		}

	}

	else
	{
		printf("fork failed");
		//exit(EXIT_FAILURE);
	}

	//exit(EXIT_SUCCESS);
}