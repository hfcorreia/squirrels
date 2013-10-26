CC = gcc
CFLAGS = -g -Wall -Wextra
all:
	$(CC) -o wolves-squirrels-serial wolves-squirrels-serial_alternative.c $(CFLAGS)

clean:
	rm wolves-squirrels-serial
