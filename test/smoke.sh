#!/bin/bash
#
# Local Smoke Test 
# for oni-lda-c
#
# checks to see if lda runs locally without fault or hang with 2 worker processes,
# and about 86 kb of sample data.
 
TEST_DIR=/tmp/oni_ldac_test_smoke

rm -Rf $TEST_DIR
mkdir $TEST_DIR

mpiexec.hydra -n 6 ../lda est 2.5 20 ../settings.txt smoke.dat random $TEST_DIR

echo "SMOKE TEST: PASSED."
echo "results in " $TEST_DIR
