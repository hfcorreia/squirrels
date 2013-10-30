#!/bin/bash
if [ "$#" -ne 5 ]; then
    echo "wrong number"
    exit 1
fi
echo "### Running Serial"
./wolves-squirrels-serial $1 $2 $3 $4 $5 > serial.out 
echo "### Running Parallel"
./wolves-squirrels-omp $1 $2 $3 $4 $5 > parallel.out
echo "### Diff"
diff serial.out parallel.out
