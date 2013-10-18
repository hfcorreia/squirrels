CC = gcc
CFLAGS = -g -Wall
all:
	$(CC) -o wolves-squirrels-serial wolves-squirrels-serial.c $(CFLAGS)

clean:
	rm wolves-squirrels-serial
