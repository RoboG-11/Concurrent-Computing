#!/bin/bash
# Bash script para arrancar el cliente (neurocontrolador) de TORCS
PORT=$1
WEIGHTSFILE=$2
OBJFILE=$3
SERVERID=$4
RALLY_SIZE=1

# Esta línea es solamente para ver en la consola cómo se ejecutará el comando y sus parámetros.
printf "\n neuronal port:${PORT} ifile:${WEIGHTSFILE} ofile:${OBJFILE} serverid:${SERVERID} rsize:${RALLY_SIZE}\n"

# Su programa client debe estar en su directorio $HOME/bin para encontrarlo desde cualquier directorio
neuronal port:${PORT} ifile:${WEIGHTSFILE} ofile:${OBJFILE} serverid:${SERVERID} rsize:${RALLY_SIZE} &
