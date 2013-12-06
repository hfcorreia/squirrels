CC = gcc
CFLAGS = -g
SERIAL = wolves-squirrels-serial
PARALLEL = wolves-squirrels-omp
MPI = wolves-squirrels-mpi
OMPFLAG = -fopenmp

all: mpi parallel serial testgen

mpi: 
	mpicc -o $(MPI) src/$(MPI).c  -g  -Wall -Wextra 
parallel: 
	$(CC) $(CFLAGS) -o $(PARALLEL) src/$(PARALLEL).c $(OMPFLAG)

serial: 
	$(CC) $(CFLAGS) -o $(SERIAL) src/$(SERIAL).c -pg

testgen: 
	$(CC) $(CFLAGS) -o testgen src/testgen.c

clean:
	rm $(SERIAL) $(PARALLEL) testgen $(MPI)
	find . -name "*.o" -exec rm -f {} \;
