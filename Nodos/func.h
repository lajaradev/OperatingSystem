#define PIPE_END -1

// Definimos una unión usada en sigqueue
union sigval value;

// Definimos la estructura PROCESO
typedef struct { 
    int pid ;        // Pid del proceso 
  	int p[2];        // Descriptores el pipe del proceso
  	int carga;       // Carga del proceso 
  	int signals;     // Nuero de signals que tiene que hacer el proceso
	int waits;       // Numero de waits que tiene que hacer el proceso
  	int waitfor[10]; // Array con el indice de los procesos en los que hace wait
} PROCESO ;

int numProcesos; // Numero de procesos
PROCESO *procesos;  // Array de estructuras proceso

void Sintaxis();
int NewArrayProcesos(int numHijos);
int CrearPipes(int numHijos);
void DoJob(int carga);
int GetChildIndex(PROCESO *hijos, int numHijos, int childPid);
int LeerPipe(int fdPipe);
void EscribirPipe(int fdPipe, int dato);
void CargarDependencias(char *depFilename);
void PrintProceso(int ix);
void PrintProcesos(int numProcs);
int NumProcesos(char *depFilename);
