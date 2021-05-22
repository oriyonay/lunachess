MAKEFLAGS += -s

all:
	make board.o
	make consts.o
	make engine.o
	make main.o
	g++ -std=c++11 -O3 board.o consts.o engine.o main.o -o main.out
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

engine.o: engine.cpp engine.h
	g++ -std=c++11 -O3 -w -c engine.cpp -o engine.o

main.o: main.cpp *.h
	g++ -std=c++11 -O3 -w -c main.cpp -o main.o
