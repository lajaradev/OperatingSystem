#!/bin/bash

ERR_NOT_NUMBER=1
ERR_OUT_OF_RANGE=2

MIN_HIJOS=2
MAX_HIJOS=12
MIN_PIEZAS=1
MAX_PIEZAS=10

Number(){
	
	# Salida:
	#    1  : si es numérico
	#    0  : si no es numérico
	if [ "$1" -eq "$1" ] 2>/dev/null; then
		return 0
	else
		return 1
	fi

}

Sintaxis(){
	
	# Debe de mostrar la sintaxis de uso del programa, con la explicación de los parametros.
	echo "-------------------------------------------------------------------------------------------"
	echo "pintar.sh <numHijos> <maxPiezas>"
	echo ""
	echo "Los parametros son obligatorios:"
	echo "numHijos     : Es el número de hijos que se van a lanzar"
	echo "               El valor mínimo para este parámetro es $MIN_HIJOS"
	echo "               El valor máximo para este parámetro es $MAX_HIJOS"
	echo "maxPiezas    : Es el número máximo de piezas que puede asumir un hijo"
	echo "               El valor mínimo para este parámetro es $MIN_PIEZAS"
	echo "               El valor máximo para este parámetro es $MAX_PIEZAS"
	echo "-------------------------------------------------------------------------------------------"

}

StrLen(){
	
	# Devuelve la longitud de la cadena que se pasa por parametro
	str=$1
	echo ${#str}

}

ToNumber(){
	
	# Convierte una cadena de caracteres a numero.
	# No hace compobaciones todos los caracteres deben ser numeros. Se adminte el . decimal
	str=$1
	echo $(bc -l <<<"${str}")

}

GetFileName(){
	
	# Devuelve el nombre del fichero en función del número pasado, teniendo en cuenta de poner un 0
	# delante de los digitos 0 al 9
	child=$1
	if [ $(StrLen $child) -eq 1 ]; then
		# Componemos el nombre del fichero antenponiendo un 0 al valor de $child
		fichero="Child0${child}Data.txt"
	else
		# Componemos el nombre del fichero utilizando simplemente $child
		fichero="Child${child}Data.txt"
	fi
	echo $fichero	

}

MostrarResultados(){

	echo ""
	echo "Informe ---------------------"
	echo "Numero de hijos $NUM_HIJOS"
	echo "Maximo de piezas por hijo $NUM_PIEZAS"

	totDelay=0
	for i in `seq 0 $((NUM_HIJOS-1))`;
	do
		echo "Hijo $i:"
		echo "  NumPiezas: ${piezasHijos[$i]} Delay:${delays[$i]}"
		totDelay=$(echo $totDelay+${delays[$i]} | bc)
	done

	echo "Total Delays: $totDelay"
	echo "-----------------------------"

}

CheckArguments(){

	# Comprobamos que el número de parametros es 2
	if [ $# -ne 2 ]; then
		echo "Error número de parametros incorrecto"
		Sintaxis
		exit 1
	fi

	# Comprobamos que el parámetro numHijos es un número
	if ! Number $1 ; then
	  echo "El parámetro <numHijos> ($1) debe ser un número"
	  Sintaxis
	  exit 1
	fi

	# Comprobamos el rango del parámetro numHijos
	if [ $1 -lt $MIN_HIJOS ] || [ $1 -gt $MAX_HIJOS ] ; then
		echo "El parametro <numHijos> debe estar en el intervalo [$MIN_HIJOS..$MAX_HIJOS]"
		Sintaxis
		exit 1
	fi

	# Comprobamos que el parámetro numPiezas es un número
	if ! Number $2 ; then
	  echo "El parámetro <numPiezas> debe ser un número"
	  Sintaxis
	  exit 1
	fi

	# Comprobamos el rango del parámetro numPiezas
	if [ $2 -lt $MIN_PIEZAS ] || [ $2 -gt $MAX_PIEZAS ] ; then
		echo "El parametro <numHijos> debe estar en el intervalo [$MIN_PIEZAS..$MAX_PIEZAS]"
		Sintaxis
		exit 1
	fi

}

LoadPiezas(){
	
	# Asignamos a la variable numhijos el parametro pasado
	numHijos=$1

	# Iteramos de 0 a numHijos-1
	for i in `seq 0 $((numHijos-1))`; 
	do
		# Obtenemos el nombre del fichero para el hijo $i
		fichero=`GetFileName $i`

		# Leemos el número de piezas del fichero. Primera linea del mismo.
		read nPiezas < $fichero

		# Asignamos el numero de piezas al elemento $i del array piezasHijos
		piezasHijos[$i]=`ToNumber $nPiezas`
	done

}

LoadDelays(){
	
	# Iteramos de 0 a numHijos-1
	for i in `seq 0 $((NUM_HIJOS-1))`;
	do
		# Obtenemos el nombre del fichero usando la funcion GetFileName
		fichero=`GetFileName $i`

		# En la variable delay obtenemos el todal de delay del hijo $i 
		# usando la funcion ReadDelay a la que pasamos el nombre del fichero
		delay=$(ReadDelay $fichero)

		# Almacenamos el delay del hijo en el array delays, en la posición $i
		delays[$i]=$delay
	done

}

ReadDelay(){
	
	# Asignamos el parametro de 
	fichero=$1

	# Definimos un bloque anónimo { ... } al que le redireccionamos el fichero de entrada para que el comando read lea de éste.
	{
		# Inicializamos a cero el contador total de delay
		childDelay=0

		# Leemos la primera linea del fichero que es el numero de piezas en la variable nPiezas
		read nPiezas

		# Iteramos desde 1 hasta nPiezas
		for i in `seq 1 $(($nPiezas))`;
        do
        	# Leemos una linea del fichero
        	read linea 

        	# Obtenemos el tercer campo de la linea (separador es ;), que corresponde al delay
            strDelay=`echo $linea | cut -d';' -f 3`

            # El delay leeido es una cadana de caracteres que respresenta un numero flotante
            # La pasamos a Numérico con la funcíon ToNumber
            numDelay=`ToNumber $strDelay`

            # Incrementamos el valor de childDelay con el delay leido. Tener en cuenta que es un numero flotante (con decimales)
            childDelay=$(echo $childDelay+$numDelay | bc)

        done   
    } < $fichero

    # Devolvemos el delay total del hijo.
	echo "$childDelay"

}

#----------------------------------------------
# MAIN
#----------------------------------------------

echo "Me pasan los siguientes parametros"
echo $*
echo "Que son $# parametros"
	# Comprobar los argumentos llamando a CheckArguments apropiadamente
	CheckArguments $*

echo "Esto lo muestra si CheckArguments no sale con exit"

# Una vez comprobado que todo correcto asignamos los parametros a variables
NUM_HIJOS=$1
NUM_PIEZAS=$2

# Llamamos al ejecutable pintar pasándole el numero de hijos y el numero de piezas por hijo.
./pintar $NUM_HIJOS $NUM_PIEZAS

# Utilizamos la funcion LoadPiezas pasándole el numero de hijos. Esta funcion dejará el array piezasHijo cargado correctamente.
LoadPiezas $NUM_HIJOS

# Llamamos a la funcion LoadDelays que nos va a dejar cargado el array delays correctamente.
LoadDelays

MostrarResultados

#----------------------------------
# FIN
#----------------------------------