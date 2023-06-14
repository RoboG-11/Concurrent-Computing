#!/bin/bash

FILE=$1
NCAR=$2

PORT=3001
   
./get_nth_car.sh $FILE 24 5 0 $NCAR
printf "\nclient port:${PORT} ifile:car_$NCAR.out serverid:0 rsize:1\n"

neuronal port:${PORT} ifile:car_$NCAR.out serverid:0 rsize:1

