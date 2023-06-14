#!/bin/bash
# Bash script para arrancar el SERVIDOR del simulador de TORCS.

NUMSERVER=$1
OBJSVALUESFILE=$2
TRACK_NAME=$3

RALLY_SIZE=1

BASE_PATH=$(pwd)

# Crear archivo de configuración para la instancia
CONFIG_FILE=${BASE_PATH}/config/quickrace_rallyserver_${NUMSERVER}.xml

./templates/create_quickrace_file_reuse_cars.sh ${CONFIG_FILE} ${NUMSERVER} ${RALLY_SIZE} ${TRACK_NAME}


printf "\n\n----> SIMULATION STARTED <------\n"

SHIFT=$NUMSERVER


# Esta línea es solamente para ver en la consola cómo se ejecutará el comando y sus parámetros.
printf "torcs -rx ${SHIFT} -x rally_${NUMSERVER} -j ${BASE_PATH}/${OBJSVALUESFILE} -t 19000 -r ${CONFIG_FILE}\n"


# Arrancar instancia de servidor TORCS en puerto port
torcs -rx ${SHIFT} -x rally_${NUMSERVER} -j ${BASE_PATH}/${OBJSVALUESFILE} -t 19000 -r ${CONFIG_FILE}

printf "\n----> Terminó el script que arranca la simulación <------\n"
