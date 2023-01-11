#include <stdio.h>
#include <stdlib.h>
#include <signal.h>   
#include <unistd.h>   
#include <string.h>
#include <sys/types.h>
#include <stdbool.h>
#include <sys/time.h>
#include "func.h"

//***************************************************
//SINTAXIS : Función que vuelca la sintaxis correcta de la llamada
//***************************************************
void Sintaxis(){

	printf("-------------------------------------------------\n");
	printf("Sintaxis del programa:\n");
	printf("procesar <fichero_dependencias>\n");
	printf("	fichero_dependencias: Nombre del fichero de dependencias generado con prepare.sh para un grafo\n");
	printf("-------------------------------------------------\n");

}

//***************************************************
//CREAR PROCESOS : Función que crea un array dinámico de estructuras HIJO
//              Devuelve el numero de hijos si se crea correctamente el array
//              o -1 en caso de error
//***************************************************
int NewArrayProcesos(int numProcesos){
	
	int i; 
	procesos = (PROCESO *) malloc (numProcesos * sizeof(PROCESO));
	if (procesos == NULL) {
		printf("Error allocating memory!\n"); 
		return -1; 
	}   
	// Inicializamos las estructuras de hijos
	for(i = 0; i < numProcesos; i ++){ 
	  procesos[i].pid = 0;
	  procesos[i].carga = 0;
	  procesos[i].signals = 0;
	  procesos[i].waits = 0;

	}

	return numProcesos;
	
}


//***************************************************
//CREAR PIPES : Función que crea los pipe para los procesos en el array global de pipes
//              Devuelve el numero de pipes creados si se crean correctamente el array
//              o -1 en caso de error
//***************************************************
int CrearPipes(int numProcesos){
	
	int i; 
	int retVal = numProcesos;
	// Creación de los pipes de comunicación con los procesos.
	for(i = 0; i < numProcesos; i ++){ // El padre itera creando los pipes
		if(pipe(procesos[i].p) == -1){
			retVal = -1;
			break; 
        }
	}	
	return retVal; 

}

//***************************************************
//DoJOb : Función que simula la ejecución de la tarea del proceso
//        Recibe como parametro la carga del proceso (segundos de procesamiento)
//***************************************************
void DoJob(int carga){
	
	sleep(carga);

}


//***************************************************
//LECTURA DE UN ENTERO DE UN PIPE 
//      fdPipe     : descriptor del pipe del que leer
// retorna
//      dato       : entero leido del pipe
//***************************************************
int LeerPipe(int fdPipe){
	
	int dato, n;
    n = read(fdPipe, &dato, sizeof(int));
    return dato; 

}

//***************************************************
//ESCRITURA DE UN ENTERO EN UN PIPE 
// fdPipe     : descriptor del pipe en el que escribir
// dato       : entero a ecribirlos en el pipe
//***************************************************
void EscribirPipe(int fdPipe, int dato){
   
    write(fdPipe, &dato, sizeof(int));

}

 
//***************************************************
//CARGAR DEPENDENCIAS : Función que lee el fichero de dependencias y carga el array de PROCESOS
//              Devuelve el numero de procesos si se crea correctamente el array
//              o -1 en caso de error
//***************************************************
void CargarDependencias(char *depFilename){
	
	int carga; 
	int numSignals, numWaits, ixWait;
	int p = 0;

	FILE *f;
    f = fopen(depFilename, "r");

	while( fscanf(f, "%i", &carga) == 1){
		
		// Leemos la carga del proceso
		procesos[p].carga = carga;

		// Leemos el número de signals
		fscanf(f, "%i", &numSignals);
		procesos[p].signals = numSignals;

		// Leemos el número de waits
		fscanf(f, "%i", &numWaits);
		procesos[p].waits = numWaits;

		// Leemos los indices de los procesos por los que esperar
        for(int i = 0; i < numWaits; i ++){
            fscanf(f, "%i", &ixWait);
			procesos[p].waitfor[i]=ixWait;
        }
		// Incrementamos el indice del proceso.
		p ++;
    }

    fclose(f);	

}


//***************************************************
//PRINT PROCESO : Funcion que imprime un proceso del array de procesos
//***************************************************
void PrintProceso(int p){
	
	int ixWait; 
	printf("--------------------------\n");
	printf("pid[%d]\n",procesos[p].pid);
	printf("pipes[%d-%d]\n",procesos[p].p[0],procesos[p].p[1]);
	printf("carga[%d]\n",procesos[p].carga);
	printf("signals[%d]\n",procesos[p].signals);
	printf("waits[%d]\n",procesos[p].waits);
	printf("waitfor[");
	for (int i = 0; i < procesos[p].waits; i ++){
		ixWait = procesos[p].waitfor[i];
		if (i == (procesos[p].waits-1))
			printf("%d",ixWait);
		else
			printf("%d,",ixWait);
	}
	printf("]\n");
	printf("--------------------------\n");

}

//***************************************************
//PRINT PROCESOS : Funcion que imprime uno tras otro los proceos del array de procesos
//***************************************************
void PrintProcesos(int numProcs){
	
	for (int i = 0; i < numProcs; i ++){
		PrintProceso(i);
	}

}

//***************************************************
//NUM PROCESOS : Función que devuelve el número de procesos
//               leyendo el numero de líneas del fichero de dependencias.
//***************************************************
int NumProcesos(char *depFilename){
	
	int status;
	FILE *fp;
	char command[100]="wc -l < ";
	char output[100];

	strcat(command,depFilename);

	fp = popen(command, "r");
	 if (fp == NULL){
		printf("Error\n");
		exit(0);
	 }

	 while (fgets(output, 100, fp) != NULL)

	 status = pclose(fp);	
	 return atoi(output);

}