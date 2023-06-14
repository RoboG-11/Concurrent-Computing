#!/bin/bash

IFILE=$1
NINPUTS=$2
NOUTPUTS=$3
NHIDDEN=$4
NCAR=$5

rm car_${NCAR}.out

echo $NINPUTS >> car_${NCAR}.out
echo $NOUTPUTS >> car_${NCAR}.out
echo $NHIDDEN >> car_${NCAR}.out

sed ${NCAR}'q;d' ${IFILE} >> car_${NCAR}.out
