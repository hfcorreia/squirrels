CC = gcc
CFLAGS = -g -Wall -Wextra
FILE = wolves.c
all:
	$(CC) -o wolves-squirrels-serial $(FILE) $(CFLAGS)

clean:
	rm wolves-squirrels-serial
