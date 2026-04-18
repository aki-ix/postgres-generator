CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I/opt/homebrew/opt/libpq/include
LDFLAGS = -L/opt/homebrew/opt/libpq/lib -lpq

all: main

main: main.cpp db.cpp
	$(CXX) $(CXXFLAGS) -o main main.cpp db.cpp $(LDFLAGS)

clean:
	rm -f main