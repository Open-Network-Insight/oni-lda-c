#!/bin/bash
#
# Local Smoke Test 
# for oni-lda-c
#
# checks to see if lda runs locally without fault or hang
# and about 86 kb of sample data.
#
# usage:
#  ./smoke.sh <# of processes>
# example:
#  ./smoke.sh 6
#
# The number of processes has to match those compiled into lda-estimate.c or there can be a hang


NUM_PROCESSES=$1
TEST_DIR=/tmp/oni_ldac_test_smoke

rm -Rf $TEST_DIR
mkdir $TEST_DIR

mpiexec.hydra -n $NUM_PROCESSES ../lda est 2.5 20 ../settings.txt smoke.dat random $TEST_DIR

echo "SMOKE TEST: PASSED."
echo "results in " $TEST_DIR
