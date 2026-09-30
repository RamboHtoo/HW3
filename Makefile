all: shape

shape: main.cpp
	g++ -Wall -O3 -g -std=c++17 main.cpp -o shape

clean:
	rm -f shape