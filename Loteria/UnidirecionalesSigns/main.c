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

// DEFINES Y CONSTANTES GLOBALES
int numHijosApuesta = 0; // Variable que medira el numero de hijos que han apostado
HIJO *arrayHijos;

int main(int argc, char *argv[]){
	
	if(ValidarArgumentos(argc,argv) == 1) { // Comprobamos los valores de entrada
		return 0;
	}

	// Cogemos los parametros
	char *numSorteo = argv[1];
	int numJugadores = atoi(argv[2]);
	int* CombinacionGanadora = malloc (6 * (sizeof(int)));
	int i, j;

	/**** HANDLER PADRE ****/
	// Se define la estructura para el manejador de las señales que recibe el padre
	struct sigaction manejador;
	 
	// Se inicializa el contador de mensajes recibidos, que han sido enviados por los hijos al haber realizado su apuesta
	manejador.sa_sigaction = HijoApuesta  ; // Nombre de la funcion manejadora
	manejador.sa_flags = SA_SIGINFO; // Esto lo definimos para enviar informacion adicional junto a la señal
	sigaction(SIGRTMIN, &manejador , NULL); // Establecemos el manejador de la señal a la función manejadora
	/**** FIN HANDLER PADRE ****/
	   
	// Creamos el array que contendrá a los hijos
	arrayHijos = (HIJO*) malloc (numJugadores * sizeof(HIJO)); // Declaramos el vector de estructuras HIJO
	for(i = 0; i < numJugadores; i ++){ 
		// Inicializamos las estructuras de hijos
		arrayHijos[i].pid = 0;
		arrayHijos[i].num = 0;
	  	arrayHijos[i].premio = 0;

	}

	// LLamamos a la funcion CrearTuberia para que cree los pipes de los procesos hijos (se crea un pipe por hijo)
	CrearTuberias(numJugadores, arrayHijos);

	// Creamos el bucle principal donde se llama a fork en cada iteración del padre. 
	// Los hijos hacen exit en su código de forma que no iteran.
	pid_t pid;
	int aciertos;

	for(i = 0; i < numJugadores; i ++){

		// Fork crea un nuevo proceso con una copia del código del padre
		pid = fork();

		if(pid < 0){
			printf("Error, no se ha podido crear el hijo");
		}

	   	if(pid == 0){ // CODIGO DEL HIJO

	   		// Creamos un array que pasaremos a la funcion de crear combinación 
	   		int *combinacion = malloc (6 * (sizeof(int)));

	   		// Generamos la apuesta del hijo
	   		GenerarCombinacion(combinacion);

	   		// Comunicamos al padre que hemos apostado
	   		ApuestaRealizada();

	   		// Leemos del Pipe (la llamada a read es bloqueante por lo que bloqueara al proceso hijo hasta que alguien escriba en el bufferc)
	  		LeerGanadora(i, CombinacionGanadora);

	  		// Calculamos los aciertos
	  		aciertos = ComprobarCombinacion(CombinacionGanadora,combinacion);
	  		arrayHijos[i].num_aciertos = aciertos;	

	  		// Salimos comunicando aciertos (exit permite salir del proceso hijo y que no se ejecute codigo del padre)
	    	exit(aciertos);

	   	}
	   	else{
	   		// Parte del padre (asignamos los datos a sus hijos)
	   		arrayHijos[i].pid = pid;
	   		arrayHijos[i].num = i;

	   	}	  
	  
	}

	/* ... este código sólo lo ejecuta el padre tras todas las iteraciones... */

	// Esperamos a que todos los hijos apuesten
	while (numHijosApuesta < numJugadores){
		pause(); // Ponemos el proceso padre en pausa
	}

	// Generamos la combinacion ganadora
	GenerarCombinacion(CombinacionGanadora);

	// Escribimos la combinación ganadora en el/los pipes
	for(i = 0; i < numJugadores; i ++){
		ComunicarApuesta(i, CombinacionGanadora);
	}

	// Informamos a los hijos que la combinación ya está generada
	// Esperamos a la terminación de todos los hijos en un bucle con pause()
	// Obenenemos el valor de retorno de cada hijo y calculamos el importe de su premio
	for(i = 0; i < numJugadores; i ++){
		// El wait recoge el pid de los procesos hijos(preguntar)
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

	// Generamos el fichero de resultados del sorteo en curso y salimos
	FILE *f;
	char nom_fich[100] = "";
	strcat(nom_fich, "ResultadoLoteria");
	strcat(nom_fich, numSorteo);
	
	f = fopen(nom_fich, "w");

	if(f == NULL){
		printf ("ERROR NO SE HA PODIDO CREAR EL FICHERO DE RESULTADOS %s\n", nom_fich);
	}
	else{
		fprintf(f, "#Resultados Sorteo %s\n", numSorteo);

		for(i = 0; i < numJugadores; i ++){
			fprintf(f, "%d\n",arrayHijos[i].premio);
		}
	}

	printf("Fichero de resultados %s generado con exito \n\n",nom_fich);
	
	return 0;
}