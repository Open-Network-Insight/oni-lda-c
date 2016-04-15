#!/bin/bash

TEST_DIR=/tmp/oni_ldac_test_trivial3

rm -Rf $TEST_DIR 
mkdir $TEST_DIR

mpiexec.hydra -n 3 ../lda est 2.5 3 ../settings.txt trivial3.dat random $TEST_DIR
 
python validate_trivial3.py -i $TEST_DIR
echo "results in " $TEST_DIR
