#!/bin/bash

# THIS TEST RUNS IN LOCAL MODE.

# NUMBER OF PROCESSES MUST MATCH THE COMPILED-IN VALUES in lda-estimate.c OTHERWISE LDA CAN HANG DURING WRITEBACK!

NUMBER_OF_PROCESSES=$1
NUMBER_OF_TOPICS=$2
ERROR_THRESHOLD=$3

TEST_DIR=/tmp/oni_ldac_test_one_to_one

rm -Rf $TEST_DIR 
mkdir $TEST_DIR

echo "*** CREATING TEST DATA ***"
python one_to_one_generate_data.py -o $TEST_DIR/one_to_one.dat -t $NUMBER_OF_TOPICS

echo "*** RUNNING LDA IN LOCAL MODE ***"
mpiexec.hydra -n $NUMBER_OF_PROCESSES ../lda est 2.5 $NUMBER_OF_TOPICS ../settings.txt $TEST_DIR/one_to_one.dat random $TEST_DIR

echo "*** VALIDATING RESULTS ***"
python one_to_one_validate.py -i $TEST_DIR -t $NUMBER_OF_TOPICS -e $ERROR_THRESHOLD

echo "results in " $TEST_DIR
