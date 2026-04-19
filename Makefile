CXX = g++

PG_CONFIG := $(shell which pg_config 2>/dev/null)

ifeq ($(PG_CONFIG),)
$(error pg_config not found. Please install PostgreSQL development tools)
endif

INCLUDE_DIR = $(shell pg_config --includedir)
LIB_DIR = $(shell pg_config --libdir)

CXXFLAGS = -std=c++17 -Wall -Wextra -I$(INCLUDE_DIR)
LDFLAGS = -L$(LIB_DIR) -lpq

all: main

main: main.cpp db.cpp
	$(CXX) $(CXXFLAGS) -o main main.cpp db.cpp $(LDFLAGS)

clean:
	rm -f main