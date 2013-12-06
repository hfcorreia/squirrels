#!/bin/bash
if [ "$#" -ne 5 ]; then
    echo "wrong number"
    exit 1
fi
echo "### Running Serial"
./wolves-squirrels-serial $1 $2 $3 $4 $5 > /tmp/serial.out 
echo "### Running Parallel"
./wolves-squirrels-omp $1 $2 $3 $4 $5 > /tmp/parallel.out
echo "### Diff"
diff /tmp/serial.out /tmp/parallel.out
