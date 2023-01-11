#!/bin/bash

Sintaxis(){
	
	# Debe de mostrar la sintaxis de uso del programa, con la explicación de los parametros.
	echo "-------------------------------------------------------------------------------------------"
	echo "prepare.sh <fichero_grafo>"
	echo ""
	echo "El parametro es obligatorios:"
	echo "fichero_grafo     : Es el nombre del fichero con los datos del grafo. Ejemplo grafo01"
	echo "-------------------------------------------------------------------------------------------"

}

CheckArguments(){

    FILE=$1;

	# Comprobamos que el número de parametros es 1
	if [ $# -ne 1 ]; then
		echo "Error número de parametros incorrecto"
		Sintaxis
		exit 1
	fi

    # Comprobamos que el fichero pasado existe
    if [ ! -f "$FILE" ]; then
        echo "El fichero [$FILE] no existe."
		Sintaxis
		exit 1
    fi

}

###############################################
# MAIN
###############################################

	# Comprobar los argumentos llamando a CheckArguments apropiadamente
	CheckArguments $*

	# Definimos la variable FILE con el nombre del fichero recibido como parametro
	FILE=$1;

	# Definimos la variable DEPFILE con el nombre del fichero recibido como parametro concatenandole '_dep'
	DEPFILE=$1_dep

	# Inicializamos el contador de lineas leidas
	linenum=0;

    while read LINE; 
    do
		#Incrementamos en 1 el numero de linea
		linenum=$(( $linenum + 1 ))

		# Si es la primera linea la variable C almacena la carga de los procesos
		if [ $linenum -eq 1 ]; then
	    	STR_CARGAS=$LINE 
		else
			STR_ARISTAS[$linenum-2]=$LINE
		fi
    done < $FILE


	# Creamos el fichero de dependencias (utilizado para los waits en la parte C)
	echo -n "" > $DEPFILE

	# Creamos un array con las cargas leidas utilizando la tokenización de un string
	CARGAS=( $STR_CARGAS )

	# Obtenemos el número de procesos (numero de elementos del array CARGAS)
	num_procesos=${#CARGAS[*]}

	# Obtenemos el número de aristas en el fichero (numero de elementos del array STR_ARISTAS)
	num_aristas=${#STR_ARISTAS[*]}


	# Iteramos por los procesos
	for ((p=0; p<$num_procesos; p++))
	do
		# Inicializamos a cadena vacia las dependencias del proceso en curso y a cero el numero de waits y numero de signals
		waitfor=""
		num_waits=0;
		num_signals=0;
	
		# Iteramos por todas las aristas
		for ((e=0; e<$num_aristas; e++))
		do
			# Obetenmos un array TOKENS con los tokens que componen la arista $e 
			TOKENS=( ${STR_ARISTAS[$e]} )
			
			# Definimos las variables $origen y $destino del array de aristas_tokens, elementos 0 y 1 respectivamente
			origen=${TOKENS[0]}
			destino=${TOKENS[1]}

			# Si el origen coincide con el proceso en curso añadimos 1 al numero de signals 
			if [ $origen -eq $p ]; then
				num_signals=$(($num_signals+1))
			fi

			# Si el destino coincide con el proceso en curso concatenamos a waitfor el indice del origen e incrementamos el numero de waits
			if [ $destino -eq $p ]; then
				waitfor="$waitfor $origen"
				num_waits=$(($num_waits+1))
			fi
		done

		# Añadimos al fichero de dependencias una linea para el proceso con: 
		# La carga del proceso, el numero de signals, el numero de waits y la cadena waitfor
		echo ${CARGAS[$p]} $num_signals $num_waits $waitfor >> $DEPFILE
	done

	# Mostramos el fichero de definicion del grafo
	echo -----------$FILE---------------
	cat $FILE
	# Mostramos el fichero de dependencias generado
	echo -----------$DEPFILE---------------
	cat $DEPFILE

	# Ejecutamos la parte C (programa procesar.c debidamente compilado)
	# Le pasamos el nombre del fichero de dependencias
	./procesar $DEPFILE 


