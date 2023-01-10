#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <ucontext.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "funciones.h"
#include "sem.h"

#define SSEED 99
#define SHMSEED 35
#define SHMPERMISOS 0644

// DEFINES Y CONSTANTES GLOBALES 
HIJO *arrayHijos;

int main(int argc, char *argv[]){

	// Comprobamos los valores de entrada.
	if(ValidarArgumentos(argc, argv) == 1){
		return 0;
	}

	// Cogemos los parametros.
	char *numSorteo = argv[1];
	int numJugadores = atoi(argv[2]);
	int *CombinacionGanadora = (int*) malloc (6 * (sizeof(int)));
	int i, j;

	// Declaracion e inicializacion de los semáforos usados para la sincronizacion.
	// Un semaforo que actuara como barrera compartida entre padres-hijos.
	int SBarreraPadre;
	SBarreraPadre = sCreate(SSEED,0); // Inicializamos el semaforo a 0 (funcion de barrera).

	// Declaramos el array de semáforos para que los hijos se cuelguen a la espera de que el padre genere la combinacion ganadora. 
	// Lo inicializamos a cero para que el wait de los hijos los bloquee.
	int sHIJOS[numJugadores];
	for(i = 0; i < numJugadores; i ++){
		sHIJOS[i] = sCreate(SSEED + i + 1, 0);
	}
	
	// Creamos el array que contendrá a los hijos.
	arrayHijos = (HIJO*) malloc (numJugadores * sizeof(HIJO)); // Declaramos el vector de estructuras HIJO.
	for(i = 0; i < numJugadores; i ++){ 
		// Inicializamos las estructuras de hijos.
		arrayHijos[i].pid = 0;
		arrayHijos[i].num = 0;
	  	arrayHijos[i].premio = 0;
	}

	/****** Creamos el segmento de memoria compartida, un array de 6 enteros ******/

	// Primero obtenemos el que seria el tamaño del segmento.
	int shmSize = 6 * sizeof(int);

	// Creamos el segmento y asignamos correctamente los punteros.
	// C̉on shmget obtenemos un identificador con el que podemos realizar futuras llamadas al sistema para controlar una zona de memoria compartida.
	int shmId = shmget(ftok("/bin/ls", SHMSEED), shmSize, IPC_CREAT | SHMPERMISOS); 
	
	if(shmId == -1){
		// Si el identificador toma el valor de -1, significa que ha habido un problema en la creación de la memoria compartida.
		printf("Error en la creación de memoria compartida\n");
		return 0;
	}

	// Esta funcion permite al sistema vincular la zona de memoria creada al direccionamiento lógico del proceso (una variable que "apunta" a esa zona de memoria).
	// Este será el array que se comparta.

	int *shmArray = shmat(shmId, 0, 0);

	memset(shmArray, 0, shmSize);

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

	   		// Creamos un array que pasaremos a la funcion de crear combinación 
	   		int* combinacion = malloc (6*(sizeof(int)));

	   		// Generamos la apuesta del hijo
	   		GenerarCombinacion(combinacion);

	   		// Señalizamos la barrera que compartimos con el padre
	   		sSignal(SBarreraPadre);

	   		// Hacemos wait sobre el semaforo que nos corresponde del array para esperar a poder leer de memoria compartida
	   		sWait(sHIJOS[i]);

	   		// Leemos de la memoria compartida
	   		for(j = 0; j < 6; j ++){
	   			// Usamos la variable de CombinacionGanadora que hemos creado para el padre
	   			// pero al ser el proceso hijo se encuentra vacia.
	   			CombinacionGanadora[j] = shmArray[j];
	   			printf("%i", CombinacionGanadora[j]); 
	   		}

	  		// Calculamos los aciertos.
	  		aciertos = ComprobarCombinacion(CombinacionGanadora,combinacion);
	  		arrayHijos[i].num_aciertos = aciertos;	

	  		// Nos desvinculamos de la memoria compartida (opcional al terminar el S.O lo desvinculará).
	   		shmdt(shmArray); // Esta funcion permite desvincular la zona de memoria.

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

	// Esperamos a que todos los hijos apuesten. Esperamos a que lleguen a su barrera y que indiquen que han apostado.
	for(i = 0; i < numJugadores; i ++){
		// El proceso padre esperara a que todos los hijos acaben y para ello decrementará el valor de su
		// barrera un numero total igual al numero de jugadores.
		// Cada proceso hijo aumentara con signal el valor del semaforo (para indicar que ya ha apostado).
		// Cuando la barrera del padre vuelva a valer 0 significara que los hijos ya han apostado.
		// Funcion de barrera (diapositivas de clase).
		sWait(SBarreraPadre);
		
	}

	// Borramos la barrera creada para el padre
	sDestroy(SBarreraPadre);
	
	// Generamos la combinacion ganadora
	GenerarCombinacion(CombinacionGanadora);

	// Escribimos la combinacion ganadora en la memoria compartida.
	for(i = 0; i < 6; i ++){
		shmArray[i] = CombinacionGanadora[i];
	}

	// Señalizamos (signal) en el array de semaforos para desbloquear a los hijos y que lean de la memoria.
	for(i = 0; i < numJugadores; i ++){
		sSignal(sHIJOS[i]);
	}

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
	strcat(nom_fich, "S");
	strcat(nom_fich,numSorteo);
	strcat(nom_fich, "R");
	
	f = fopen(nom_fich, "w");

	if(f == NULL){
		printf ("ERROR NO SE HA PODIDO CREAR EL FICHERO DE RESULTADOS %s\n", nom_fich);
	}
	else{
		fprintf(f, "#Resultados Sorteo %s\n", numSorteo);

		for(i = 0; i < numJugadores; i ++){
			fprintf(f, "%ld\n",arrayHijos[i].premio);
		}
	}

	printf("Fichero de resultados %s generado con exito \n\n", nom_fich);
	
	return 0;
}