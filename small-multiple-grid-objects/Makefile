# Variables make the file easier to update
CC = g++
CFLAGS = -Wall -fopenmp
TARGET = exe-grid

# The first target is the default one run by 'make'
all: CFLAGS += -O2 -DNDEBUG
all: $(TARGET)

# Debug build target
debug: CFLAGS += -g -O0 -DDEBUG
debug: $(TARGET)

# Rule to create the executable 
exe-grid: multiple-grid-objects.cpp
	$(CC) $(CFLAGS) -o $(TARGET) multiple-grid-objects.cpp -v -I/home/yasodhan/oldhomedir/benchmark/numactl/ -L /home/yasodhan/oldhomedir/benchmark/numactl/.libs/ -lnuma

# Rule to remove the built files
clean:
	rm -f exe-grid

