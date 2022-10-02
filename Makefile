
CC=g++
LIB=mips.cpp assembler.cpp instruction-set.cpp debugger.cpp
APP=main.cpp
# TEST=test-mips.cpp
CFLAGS=-I./ -Wswitch -std=c++11
NAME=mips

all: lib app

run: lib app
	./a.out

lib:
	$(CC) -c $(CFLAGS) $(LIB)

app: lib
	$(CC) $(CFLAGS) $(APP) $(LIB:%.cpp=%.o)

# This is the test
showcase: lib
	$(CC) $(CFLAGS) test-showcase.cpp $(LIB:%.cpp=%.o)
# -lUnitTest++
	./a.out

clean:
	rm -rf runner.cpp
	rm -rf *.o
	rm -f a.out
