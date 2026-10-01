all: shape

shape: main.cpp
	g++ -Wall -O3 -g -std=c++17 main.cpp -I/opt/X11/include -L/opt/X11/lib -lX11 -o shape

clean:
	rm -f shape