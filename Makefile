MAKEFLAGS += -s

all:
	make board.o
	make consts.o
	make engine.o
	make globals.o
	make main.o
	make uci.o
	make utils.o
	g++ -std=c++11 -g -O3 board.o consts.o engine.o globals.o main.o uci.o utils.o  -o main.out
	make run

run:
	./main.out

run_test:
	./tests.out

test: tests.cpp
	make board.o
	make consts.o
	g++ -std=c++11 -O3 -w -c tests.cpp -o tests.o
	g++ -std=c++11 -O3 board.o consts.o tests.o -o tests.out

clean:
	rm *.o ||:
	rm *.out ||:

board.o: board.cpp board.h
	g++ -std=c++11 -O3 -w -c board.cpp -o board.o

consts.o: consts.cpp consts.h
	g++ -std=c++11 -O3 -w -c consts.cpp -o consts.o

engine.o: engine.cpp engine.h
	g++ -std=c++11 -O3 -w -c engine.cpp -o engine.o

globals.o: globals.cpp globals.h
	g++ -std=c++11 -O3 -w -c globals.cpp -o globals.o

main.o: main.cpp *.h
	g++ -std=c++11 -O3 -w -c main.cpp -o main.o

uci.o: uci.cpp uci.h
	g++ -std=c++11 -O3 -w -c uci.cpp -o uci.o

utils.o: utils.cpp utils.h
	g++ -std=c++11 -O3 -w -c utils.cpp -o utils.o
