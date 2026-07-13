# Variables make the file easier to update
CC = g++
CFLAGS = -Wall -fopenmp

# The first target is the default one run by 'make'
all: exe-grid

# Rule to create the executable 'main' from 'main.c'
exe-grid: multiple-grid-objects.cpp
	$(CC) $(CFLAGS) -o exe-grid multiple-grid-objects.cpp

# Rule to remove the built files
clean:
	rm -f exe-grid

#gcc -fopenmp array_op.c -o exe-arr
