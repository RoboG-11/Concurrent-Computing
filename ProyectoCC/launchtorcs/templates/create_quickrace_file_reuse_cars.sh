#!/bin/bash

# Name of the configuration XML file to be created.
CONFIG_FILE=$1

# Parallel Rally number
NUMSERVER=$2

# Number of cars in the rally.
RALLY_SIZE=$3

# Track name
NTRACK=$4

SHIFT=0

# Poner el inicio del archivo de configuración, sustituyendo ID_SERVER por el ID del primer auto.
sed 's/#ID_SERVER/'${SHIFT}'/g' ./templates/quickrace_rally_head_template.xml > ${CONFIG_FILE}

TEMPFILE=./templates/quickrace_rally_body_parcial_$NUMSERVER.xml

# Borrar el archivo parcial para que el resultado tenga todavía el tag NAME_SERVER.
rm -f $TEMPFILE


n=1
i=0

# Concatenar configuración del auto i al archivo de configuración.
sed 's/#NAME_SERVER/'${n}'/g' ./templates/quickrace_rally_body_template.xml > $TEMPFILE
   

# Aquí el archivo TEMPFILE ya tiene valor en el tag NAME_SERVER, falta por ponerle valor a ID_SERVER
sed 's/#ID_SERVER/'${i}'/g' $TEMPFILE >> ${CONFIG_FILE}
   

# Concatenarle al archivo de configuración la parte final del archivo xml.
sed 's/#NAME_TRACK/'${NTRACK}'/g' ./templates/quickrace_rally_tail_template.xml >> ${CONFIG_FILE}

rm $TEMPFILE
