MAKEFLAGS += -s

all:
	make board.o
	make consts.o
	make main.o
	g++ -std=c++11 -O3 board.o consts.o main.o -o main.out
	make run

run:
	./main.out

clean:
	rm *.o ||:
	rm *.out ||:

board.o: board.cpp board.h
	g++ -std=c++11 -O3 -w -c board.cpp -o board.o

consts.o: consts.cpp consts.h
	g++ -std=c++11 -O3 -w -c consts.cpp -o consts.o

main.o: main.cpp *.h
	g++ -std=c++11 -O3 -w -c main.cpp -o main.o
