# Variables make the file easier to update
CC = g++
CFLAGS = -Wall -fopenmp

# The first target is the default one run by 'make'
all: vec_exe

# Rule to create the executable 'main' from 'main.c'
vec_exe: main.cpp
	$(CC) $(CFLAGS) -o vec_exe main.cpp

# Rule to remove the built files
clean:
	rm -f vec_exe

#gcc -fopenmp array_op.c -o exe-arr
