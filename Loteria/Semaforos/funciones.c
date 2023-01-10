#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <time.h>
#include "funciones.h"

////////////  DECLARACION DE VARIABLES  ////////////

extern HIJO *arrayHijos;

////////////  FUNCIONES SOLO PADRE  ////////////

int ValidarArgumentos(int argc, char** argv){
	
	// Los parámetros que recibe son los que recibe el propio main.
	// El valor de retorno puede ser un código de OK u Error (devolvemos 1 si los parámetros son incorrectos y 0 si son correctos)
	int NumSorteos = atoi(argv[1]);
	int NumJugadores = atoi(argv[2]);

	if(argc != 3){
		printf("\nNumero de parámetros incorrecto\n\n");
		return 1;
	}
	else if(NumSorteos < 0 || NumSorteos > 15){
		printf("\nEl numero de sorteos debe estar comprendido entre 1 y 15\n");
		return 1;
	}
	else if(NumJugadores < 0 || NumJugadores > 10){		
		printf("\nEl numero de jugadores debe estar comprendido entre 1 y 10\n");
		return 1;
	}

	return 0;

}


void CrearTuberias(int numHijos, HIJO *arrayHijos){

	// Esta función sirve para que el padre, antes de crear los hijos y sabiendo cuántos son cree todas las tuberias.
	// Una forma es crear un array de tuberías global (que heredarán los hijos)
	int i;
	for (i = 0; i < numHijos; i ++){
		// Creamos el pipe y lo asignamos al elemento correspondiente de la estructura de Hijo
		pipe(arrayHijos[i].pipehijo);
	}

}


void ComunicarApuesta(int numHijo, unsigned int *ganadora){

	// Esta función servirá al padre para enviar la combinación ganadora por la tubería correspondiente al hijo, 
	// siendo numHijo el índice del hijo en el array de tuberías.

	close(arrayHijos[numHijo].pipehijo[0]);
	write(arrayHijos[numHijo].pipehijo[1], ganadora, 6 * sizeof(int));
	// El primer parametro el el lado del pipe por el que se escribe (siempre 1), el segundo parametro es lo que se envia y el tercero el tamaño de lo que se envia
	// Al ser 6 enteros, se envia 6 por el tamaño de un entero
	close (arrayHijos[numHijo].pipehijo[1]);

}


////////////  FUNCIONES SOLO HIJO  ////////////

int ComprobarCombinacion(unsigned int *ganadora, unsigned int *apuesta){
	
	// Esta función la usarán los hijos para comprobar el número de aciertos de su combinación
	// La función devuelve el número de aciertos que tendrá el jugador 

	int i, j;
	int aciertos = 0;

	for(i = 0; i < 6; i ++){
		for(j = 0; j < 6; j ++){
			if(apuesta[j] == ganadora[i]){
				aciertos ++;
			}
		}
	}

	return aciertos;
	
}

int CalcularPremio(int numAciertos){

	// Esta función servirá para calcular el importe del premio en función de la cantidad de números acertados
	// Devuelve el premio en euros

	if(numAciertos >= 6){
		return 10000;
	}
	else if(numAciertos == 5){
		return 500;
	}
	else if(numAciertos == 4){
		return 50;
	}
	else if(numAciertos == 3){
		return 10;
	}
	else{
		return 0;
	}

}

void LeerGanadora(int numHijo, unsigned int *ganadora){

	// Esta función servirá al hijo para leer la combinación ganadora de su tubería, que vendrá indicada
	// por el número de hijo numHijo que es el índice que el hijo tiene en el array de tuberías.
	// La combinación ganadora se depositará en el array ganadora.
	close (arrayHijos[numHijo].pipehijo[1]);
	read (arrayHijos[numHijo].pipehijo[0], ganadora, 6 * sizeof(int));
	close (arrayHijos[numHijo].pipehijo[0]);

}

////////////  FUNCIONES COMUNES PADRE E HIJO  ////////////

int ordenarCombinacion(int *combinacion){

	// Ordenamos la combinacion de menor a mayor
    int aux, i, j;

    for(i = 0; i < 6; i ++){
        for(j = 0; j < 5; j ++){
             if(combinacion[j] > combinacion[j + 1]){
                aux = combinacion[j + 1];
                combinacion[j + 1] = combinacion[j];
                combinacion[j] = aux;
            }
        }
    }

    return 1;

}


void GenerarCombinacion(unsigned int *combinacion){
	
	// Esta función la utilizarán el padre y el hijo para generar la combinación
	// El padre generará la combinación ganadora y el hijo su apuesta.
	// Hay que inicializar el generador de números aleatorios con valores distintos para el padre y para cada hijo
	// Lo ideal es que la combinación se devuelva ordenada o bien hacer uso de otra función para ordenarla como qsort 

	int i, num;
	srand(time(NULL) + getpid());

	// Un numero de loteria tiene 5 numeros (del 0 al 9)
	for(i = 0; i < 6; i ++){
		num = rand()%10;
		combinacion[i] = num;
	}

	ordenarCombinacion(combinacion);

} 