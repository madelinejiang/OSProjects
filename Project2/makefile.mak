CC=g++
all: sort 

sort: sort.cpp
	$(CC) -o sort.exe sort.cpp

	