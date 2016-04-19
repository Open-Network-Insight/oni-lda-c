#!/bin/bash
#
# Local topic modelling corner case: one word per topic, one topic per document.
# usage:
#  ./one_to_one.sh <# of processes> <# of topics> <error threshold>
# example:
#  ./one_to_one.sh 6 1000 0.1
#
# The number of processes has to match those compiled into lda-estimate.c or there can be a hang

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
