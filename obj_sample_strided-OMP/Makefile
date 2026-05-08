# Variables make the file easier to update
CC = g++
CFLAGS = -Wall -fopenmp

# The first target is the default one run by 'make'
all: exe-arr

# Rule to create the executable 'main' from 'main.c'
exe-arr: array_op.cpp
	$(CC) $(CFLAGS) -o exe-arr array_op.cpp

# Rule to remove the built files
clean:
	rm -f exe-arr

#gcc -fopenmp array_op.c -o exe-arr
