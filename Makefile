CC = gcc
CFLAGS = -Wall -Wextra -I/opt/homebrew/opt/libpq/include
LDFLAGS = -L/opt/homebrew/opt/libpq/lib -lpq

all: main

main: main.c db.c
	$(CC) $(CFLAGS) -o main main.c db.c $(LDFLAGS)

clean:
	rm -f main