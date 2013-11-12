CC = gcc
CFLAGS = -g
SERIAL = wolves-squirrels-serial
PARALLEL = wolves-squirrels-omp
OMPFLAG = -fopenmp

all: parallel serial testgen

parallel: 
	$(CC) $(CFLAGS) -o $(PARALLEL) src/$(PARALLEL).c $(OMPFLAG)

serial: 
	$(CC) $(CFLAGS) -o $(SERIAL) src/$(SERIAL).c

testgen: 
	$(CC) $(CFLAGS) -o testgen src/testgen.c

clean:
	rm $(SERIAL) $(PARALLEL) testgen
	find . -name "*.o" -exec rm -f {} \;
