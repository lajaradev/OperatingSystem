#!/bin/bash

#-------------------------------------------------------------------------------------------
# Definición de Constantes
#-------------------------------------------------------------------------------------------
## En esta sección definiremos constantes que vamos a usar en el código, como por ejemplo
## el número máximo de jugadores, o el número máximo de sorteos, así como cualquier código de
## error que vayamos a utilizar.

# Consantes
MAX_SORTEOS=15
MAX_JUGADORES=10

# Errores
ERR_NUM_ARGS=1
ERR_NUM_SORTEOS=2
ERR_NUM_JUGADORES=3

# Códigos de retorno
RET_OK=0

#----------------------------------------------------------------------------------------------
# Funciones
#----------------------------------------------------------------------------------------------
## En esta sección definiremos todas las funciones que vamos a utilizar en la sección prinpipal

# Esta función se encarga de mostrar la ayuda de cómo utilizar el script y termina su ejecución
# Recibe como parametro el código de terminación
Sintaxis(){

	if [ $1 -eq $RET_OK ]
	then
		echo "El formato de los parámetros de entrada es:"
		echo "Primer parametro -> numero de sorteos"
		echo "Segundo parametro -> numero de jugadores"
	fi

	if [ $1 -eq $ERR_NUM_ARGS ]
	then
		echo "El formato de los parámetros de entrada es:"
		echo "Primer parametro -> numero de sorteos"
		echo "Segundo parametro -> numero de jugadores"
	fi

	if [ $1 -eq $ERR_NUM_SORTEOS ]
	then
		echo "El numero de sorteos no puede ser superior a 15"
	fi

	if [ $1 -eq $ERR_NUM_JUGADORES ]
	then
		echo "El numero de jugadores no puede ser superior a 10"
	fi

	exit $1

}

# Esta función se encarga de inicializar el array de premios donde se iran acumulando el total de los premios de cada jugador (segun la posicion del array)
# Recibe como parámetro el número de jugadores que entran en los sorteos que se ejecutan
Inicializar(){
	
	for jugador in $(seq 1 $1); 
	do
	 premios[$jugador]=0
	done
	
}

# Esta función se encarga de controlar que los argumentos están en el rango permitido
# Recibe el conjunto de argumentos que se han pasado al script
# Devuelve un código de error indicando el tipo de error que se ha producido
# Si piden ayuda con -? llama a Sintaxis con el código de retorno 0
ControlArgumentos(){

	# El usuario pide ayuda
	if [ "$1" = "-?" ]
	then
		Sintaxis $RET_OK
	fi

	# Numero de parametros incorrecto
	if [ $# -lt "2" ] || [ $# -gt "2" ]
	then 
		return $ERR_NUM_ARGS 
	fi

	# Numero de sorteos incorrecto
	if [ $1 -gt "$MAX_SORTEOS" ]
	then 
		return $ERR_NUM_SORTEOS 
	fi

	# Numero de jugadores incorrecto
	if [ $2 -gt "$MAX_JUGADORES" ]
	then 
		return $ERR_NUM_JUGADORES 
	fi

}

# Esta función elimina los ficheros de ejecuciones anteriores a partir de un nombre en comun que tendrán dichos ficheros
# Por defecto llamaremos a todos los ficheros de sorteo ResultadoSorteoX
# Por defecto llamaremos al fichero de resultados anterior ResultadoLoteria
LimpiarFicheros(){
	# Con 2>/dev/null redirigimos el error a null para que no se muestre en el caso de que no haya ficheros a borrar
	rm *ResultadoFinal* 2>/dev/null
	rm *ResultadoLoteria* 2>/dev/null
}

# Esta función se encarga de leer uno a uno los ficheros de premios
# Recibe como parámetros el numero de sorteos y el de jugadores en cada sorteo
# Rellenará el array de premios, sumando a cada jugador su importe correspondiente al numero de sorteo
LeerFicherosPremios(){
	
	aux="S"
	aux2="R"

	# $1 es el numero de sorteos y $2 el de jugadores
	for i in $(seq 1 $1);
	do
		j=0
		# Recordemos que los nombres de los ficheros seran SxR
		nom_fich="$aux$i$aux2"
		while IFS= read -r line
		do
			if [ $j -gt 0 ] && [ $j -le $2 ]
			then
				#echo ${premios[j]}
				let premios[$j]=${premios[j]}+$line
			fi
			let j=$j+1
		done <$nom_fich
	done
}

# Esta función se encarga de mostrar los resultados en el informe final.
MostrarResultados(){
	
	# $1 es el numero de sorteos y $2 el de jugadores
	echo "Informe de resultados " >>ResultadoFinal.txt
	echo "Numero de sorteos: $1 Numero de jugadores: $2" >>ResultadoFinal.txt

	for i in $(seq 1 $2);
	do
		echo "Jugador $i: Total premio ${premios[i]} Euros">>ResultadoFinal.txt
	done

	echo "Fichero de resultadores generado con exito"

}

#-------------------------------------------------------------------------------------------------
# PRINCIPAL - CODIGO
#-------------------------------------------------------------------------------------------------

# Llamamos al controld e argumentos pasándole los argumentos recibidos
ControlArgumentos $*
retVal=$?
if [ $retVal -gt "0" ]; 
then
   # Mostramos mensaje y llamamos a Sintaxis
   echo "Se ha producido el error ...."
   Sintaxis $retVal
fi

# Si superamos el control de argumentos asignamos los argumentos a las variables
numSorteos=$1
numJugadores=$2

# Inicializamos el array de premios
Inicializar $numJugadores

# Limpiamos de ejecuciones anteriores
LimpiarFicheros

# Ejecutamos los sorteos en un bucle para el número de sorteos que nos pasan.
# En cada sorteo pasamos como parámetro el número de sorteo y el número de jugadores
# sorteo es el ejecutable en C que realiza el sorteo

for i in $(seq 1 $numSorteos);
do
	./sorteo $i $2
done

# Una vez realizados los sorteos leemos los premios
LeerFicherosPremios $numSorteos $numJugadores

# Y finalmente mostramos los resultados
MostrarResultados $numSorteos $numJugadores

exit 0