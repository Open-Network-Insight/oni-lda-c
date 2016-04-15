#!/bin/bash
#
# Local Smoke Test 
# for oni-lda-c
#
# checks to see if lda runs locally without fault or hang with 2 worker processes,
# and about 86 kb of sample data.
 
source /etc/oni.conf
export DPATH
export HPATH
export LUSER
export TOL
export KRB_AUTH

rm -Rf testout
mkdir testout

mpiexec.hydra -n 2 ../lda est 2.5 20 ../settings.txt model.dat random testout

