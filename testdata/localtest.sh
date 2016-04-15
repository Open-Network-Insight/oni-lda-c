#!/bin/bash

source /etc/oni.conf
export DPATH
export HPATH
export LUSER
export TOL
export KRB_AUTH

rm -Rf testout
mkdir testout

mpiexec.hydra -n 16 ../lda est 2.5 20 ../settings.txt model.dat random testout

