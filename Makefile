CC = gcc
CFLAGS = -g -Wall -Wextra
FILE = wolves-squirrels-serial.c
PFILE = wolves-squirrels-omp.c
OMPFLAG = -fopenmp
all: parallel
	$(CC) -o wolves-squirrels-serial $(FILE) $(CFLAGS)

parallel:
	$(CC) -o wolves-squirrels-omp $(PFILE) $(CFLAGS) $(OMPFLAG)

clean:
	rm wolves-squirrels-serial
	rm wolves-squirrels-omp
