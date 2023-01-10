#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <ucontext.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#include "funciones.h"
#include "sem.h"

// DEFINES Y CONSTANTES GLOBALES

#define SSEED 99

HIJO *arrayHijos;

int main(int argc, char *argv[]){
	
	if(ValidarArgumentos(argc, argv) == 1){ // Comprobamos los valores de entrada.
		return 0;
	}

	// Cogemos los parametros.
	char *numSorteo = argv[1];
	int numJugadores = atoi(argv[2]);
	int *CombinacionGanadora = (int*) malloc (6 * (sizeof(int)));
	int i, j;

	// Declaración e inicialización de los semáforos usados para la sincronización.
	// Un semáforo que actuará como barrera compartida entre padre-hijos.
	int SBarreraPadre;
	SBarreraPadre = sCreate(SSEED, 0); // Inicializamos el semaforo a 0 (funcion de barrera).
	   
	// Creamos el array que contendrá a los hijos.
	arrayHijos = (HIJO*) malloc (numJugadores * sizeof(HIJO)); // Declaramos el vector de estructuras HIJO.
	for(i = 0; i < numJugadores; i ++){ 
		// Inicializamos las estructuras de hijos.
		arrayHijos[i].pid = 0;
		arrayHijos[i].num = 0;
	  	arrayHijos[i].premio = 0;
	}

	// LLamamos a la funcion CrearTuberia para que cree los pipes de los procesos hijos (se crea un pipe por hijo).
	CrearTuberias(numJugadores, arrayHijos);

	// Creamos el bucle principal donde se llama a fork en cada iteración del padre. 
	// Los hijos hacen exit en su código de forma que no iteran.
	pid_t pid;
	int aciertos;

	for(i = 0; i < numJugadores; i ++){

		// Fork crea un nuevo proceso con una copia del código del padre.
		pid = fork();

		if(pid < 0){
			printf("Error, no se ha podido crear el hijo\n");
		}

	   	if(pid == 0){ // CODIGO DEL HIJO

	   		// Creamos un array que pasaremos a la funcion de crear combinación.
	   		int *combinacion = malloc (6 * (sizeof(int)));

	   		// Generamos la apuesta del hijo.
	   		GenerarCombinacion(combinacion);

	   		// Señalizamos la barrera que compartimos con el padre.
	   		sSignal(SBarreraPadre);

	   		// Leemos del Pipe, aquí se bloquea el hijo si todavía no está la combinación ganadora en el. 
	   		// (no es necesario recibir ACK con un semáforo de sincronización).
	  		LeerGanadora(i, CombinacionGanadora);

	  		// Calculamos los aciertos.
	  		aciertos = ComprobarCombinacion(CombinacionGanadora, combinacion);
	  		arrayHijos[i].num_aciertos = aciertos;	
	  	
	  		// Salimos comunicando aciertos (exit permite salir del proceso hijo y que no se ejecute codigo del padre).
	    	exit(aciertos);

	   	}
	   	else{
	   		// Parte del padre (asignamos los datos a sus hijos).
	   		arrayHijos[i].pid = pid;
	   		//printf("Hijo %i cread \n\n", arrayHijos[i].pid);
	   		arrayHijos[i].num = i;
	   	}	  
	  
	}

	/* ... este código sólo lo ejecuta el padre tras todas las iteraciones... */

	// Esperamos a que todos los hijos apuesten.
	for(i = 0; i < numJugadores; i ++){

		// El proceso padre esperara a que todos los hijos acaben y para ello decrementará el valor de su
		// barrera un numero total igual al numero de jugadores.
		// Cada proceso hijo aumentara con signal el valor del semaforo (para indicar que ya ha apostado)
		// Cuando la barrera del padre vuelva a valer 0 significara que los hijos ya han apostado.
		// Funcion de barrera (diapositivas de clase).
		sWait(SBarreraPadre);

	}

	// Borramos la barrera creada para el padre.
	sDestroy(SBarreraPadre);
	
	// Generamos la combinacion ganadora.
	GenerarCombinacion(CombinacionGanadora);

	// Escribimos la combinación ganadora en el/los pipes.
	for(i = 0; i < numJugadores; i ++){
		ComunicarApuesta(i, CombinacionGanadora);
	}

	// Esperamos a que todos los hijos lleguen a su barrera, lo que indica que han apostado.
	// Obenenemos el valor de retorno de cada hijo y calculamos el importe de su premio.
	for(i = 0; i < numJugadores; i ++){
		// El wait recoge el pid de los procesos hijos(preguntar).
		int pid_hijo = wait(&aciertos);
		int aciertos_hijo = aciertos >> 8;
		


		for(j = 0; j < numJugadores; j ++){
			if(pid_hijo == arrayHijos[j].pid){
				arrayHijos[j].num_aciertos = aciertos_hijo;
				int premio = CalcularPremio(arrayHijos[j].num_aciertos);
				arrayHijos[j].premio = arrayHijos[j].premio + premio;
			}
		}
	}

	// Generamos el fichero de resultados del sorteo en curso y salimos.
	FILE *f;
	char nom_fich[100] = "";
	strcat(nom_fich,"S");
	strcat(nom_fich,numSorteo);
	strcat(nom_fich,"R");
	
	f = fopen(nom_fich, "w");

	if(f == NULL){
		printf("ERROR NO SE HA PODIDO CREAR EL FICHERO DE RESULTADOS %s\n", nom_fich);
	}
	else{
		fprintf(f, "#Resultados Sorteo %s\n",numSorteo);

		for(i = 0; i < numJugadores; i ++){
			fprintf(f, "%ld\n",arrayHijos[i].premio);
		}
	}

	printf("Fichero de resultados %s generado con exito \n\n", nom_fich);
	
	return 0;
}