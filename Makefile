CC=gcc
CPPFLAGS=-Wall  -Werror  -O3

# Uncomment below if you want to use debug flags
#
CPPFLAGS= -ggdb3

SRC=puzzle.o
TARGET=15puzzle

all: $(SRC)
	$(CC) -o $(TARGET) $(SRC) $(CPPFLAGS) -lm

clean:
	rm -f $(TARGET) *.o
