// ***********************************************************************
// Example of use of pthreads library.
// ***********************************************************************
#include <cmath>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>
#include <stdint.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <signal.h>
#include <sstream>
#include <cstdint>
// Global constants
#pragma GCC diagnostic ignored "-Wwrite-strings"
#define MAX 512

using namespace std;
// Global variables

int numthreads,thread_counter,file_counter,num_of_stages, print_counter, oflag = 0;  // variable to be incremented by each thread
sem_t semaphore;  
sem_t semaphore1; //mutex
sem_t semaphore2; 
float sem_values[3];
char* file_name;
char* mode;
pthread_t tid[MAX];
vector<int> file_contents;
vector<int> numbers;

void error(char *msg)
{
	perror(msg);
	exit(1);
}

void *swap(void *arg)
{
	int id = (intptr_t)arg;
	int num_of_phases = num_of_stages;//this is log2(N) where N is the array size
	for (int s = 1; s <= num_of_stages; s++) {//for each stage
	for (int p = 1; p <= num_of_phases; p++)//for each phase
	{
		{//swap code starts here**************************************************
			int num_groups = std::pow(2, p - 1);
			int group_size = numbers.size() / num_groups;
			int gindex = id / (group_size / 2);
			int mindex = id % (group_size / 2); 
			int group_start = gindex * group_size;
			int group_end = (gindex + 1) * group_size - 1;
			int data1 = group_start + mindex;
			int data2 = group_end - mindex;
			//compare - exchange the array items data1 and data2;
			if (numbers[data1] > numbers[data2]) {
				int tmp = numbers[data1];
				numbers[data1] = numbers[data2];
				numbers[data2] = tmp;
			}
			//loop for (data1 + data2) iterations doing nothing(just to introduce different computation times);
			for (int i = 0; i < data1 + data2; i++) {
				;//do nothing
			}
		}//swap code ends here************************************************
		sem_wait(&semaphore1);//mutex
		if (thread_counter < numthreads) {
			thread_counter++;
		}
		if (oflag == 1) {
			cout << "Thread " << id << " " << "has finished stage " << s << " phase " << p << endl;
		}
		if (thread_counter == numthreads) {
			for (int i = 0; i < numthreads; i++) {
				sem_post(&semaphore);
			}
			if (oflag == 1) {
				print_counter = 0;
				for (int i : numbers) {//print out the list after the sort
					cout << i << " ";
					print_counter++;
					if (print_counter == 8) {
						print_counter = 0;
						cout << endl;
					}
				}
				cout << endl;
			}
		}
		sem_post(&semaphore1);//mutex ends
		sem_wait(&semaphore); //barrier
		sem_wait(&semaphore1);//mutex
		thread_counter--;
		if (thread_counter == 0) {
			for (int i = 0; i < numthreads; i++) {
				sem_post(&semaphore2);
			}
		}
		sem_post(&semaphore1);//mutex ends
		sem_wait(&semaphore2);
	}
}
}

void set_semaphore_values() {
	FILE* inputFile;
	char file_name[10] = "sema.init";
	inputFile = fopen(file_name, "r");
	if (inputFile == NULL) {
		error("Error could not find sema.init");
		exit(1);
	}
	else {
		int first_num;
		fscanf(inputFile, "%d", &first_num);
		for (int i = 0; i < first_num; i++) {
			fscanf(inputFile, "%f", &sem_values[i]);
			}
		}
		fclose(inputFile);
}

void scan_file() {
	FILE* inputFile;
	inputFile = fopen(file_name, "r");
	if (inputFile == NULL) {
		error("Error could not find input file");
		exit(1);
	}
	else {
		int temp;
		for(;;){
			fscanf(inputFile, "%d", &temp);
			file_contents.push_back(temp);
			if (feof(inputFile))break;
		}
	}
	fclose(inputFile);
}

void ischar() {
	FILE* inputFile;
	inputFile = fopen(file_name, "r");
	if (inputFile == NULL) {
		error("Error could not find input file");
		exit(1);
	}
	int x;
	while ((x = fgetc(inputFile)) != EOF) {//Ascii: 48 to 58 are digits, 10 is newline, 13 is carriage return
		if (x >= 48 && x <= 58) {
		}
		else if (x == 10||x==13||45) {
		}
		else {
			cout << "invalid character in file. Please only enter integers"<< endl;
			exit(1);
		}
	}
	fclose(inputFile);

}
void make_numbers() {
	numbers.clear();
	int firstnum = file_contents[0];
	if (firstnum == 0) {
		numbers.clear();
		file_contents.clear();
		exit(0);
		return;
	}
	else {
		file_contents.erase(file_contents.begin());//deletes the first number N

		for (int i = 0; i < firstnum; i++) {
			if (!file_contents.empty()) {
				numbers.push_back(file_contents[0]);
				file_contents.erase(file_contents.begin());
			}
			else {
				cout << "Invalid contents in file. Ensure there is an adequate number of integers to be sorted" << endl;
				i = firstnum;
				exit(1);
			}
		}
	}
}
// ***********************************************************************
// The main program:
// ***********************************************************************
int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		fprintf(stderr, "ERROR, no file name provided. Usage: .exe filename -mode");
		exit(1);
	}
	file_name = argv[1];
	mode = argv[2];
	if (strcmp(mode,"-o")==0) {
		oflag = 1;
	}
	else if (strcmp(mode, "-r") == 0) {
		oflag = 0;
	}
	else {
		cout << "Error, please specify mode with '-o' or '-r'" << endl;
		exit(1);
	}
	ischar();
	scan_file();
	set_semaphore_values();

	while(!file_contents.empty()){
	make_numbers();
	numthreads = numbers.size() / 2; 
	num_of_stages = log2(numbers.size());

	if ((numthreads <= 0) || (numthreads > MAX))
	{
		printf("Invalid thread count. Ensure the numbers to be sorted for one list does not exceed 1024\n");
		exit(1);
	}
	sem_init(&semaphore, 0, sem_values[0]);
	sem_init(&semaphore1, 0, sem_values[1]);
	sem_init(&semaphore2, 0, sem_values[2]);
	//print out the list before the sort
	print_counter = 0;
	cout << "Original array: ";
	for (int i : numbers) {
		cout << i << " ";
		print_counter++;
		if (print_counter == 8) {
			print_counter = 0;
			cout << endl;
		}
	}
	cout << endl;
	//Initialize threads
	for (int i = 0; i < numthreads; i++) {
		pthread_create(&tid[i], NULL, swap, (void *)(intptr_t)i);
	}
	// Wait for all the threads to exit using pthread_join
	// 2nd parameter: to obtain the return value from the thread (void **)
	for (int i = 0; i < numthreads; i++)
		pthread_join(tid[i], NULL);
	//print out the list after the sort
	print_counter = 0;
	cout << "Sorted array: ";
	for (int i : numbers) {
		cout << i << " ";
		print_counter++;
		if (print_counter == 8) {
			print_counter = 0;
			cout << endl;
		}
	}
	cout << endl;
	cout << "------------------------------------------------------------------" << endl;
}
	return 0;
}
