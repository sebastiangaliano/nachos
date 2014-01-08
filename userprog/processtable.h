//----------------------------------------------------------------------------------------
// processtable.h
// Estructura de datos que implementa una tabla de descriptores sobre los procesos que se
// estan ejecutando en el sistema y provee de una interfaz para utilizarla.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------


#ifndef PROCESSTABLE_H
#define PROCESSTABLE_H

#include "syscall.h"
#include "thread.h"


// Definimos el numero maximo de procesos permitidos simultaneamente.

#define MAX_PROCESS 128

// Definimos una enumeracion que contempla los estados de un proceso.

enum ProcessStatus { ALIVE, DEAD };

// Definimos una estructura que define un proceso.

struct Process {
	Thread* thread;
	ProcessStatus status;
	int exitValue;
};

// Definimos la clase para la tabla de procesos.

class ProcessTable {

public:

	// Constructor y destructor.

	ProcessTable();
	~ProcessTable();

	// Permite agregar un nuevo proceso a la tabla asignandole el menor ID de descriptor
	// disponible. Retorna dicho ID o -1 si no se pudo agregar el proceso.

	SpaceId attachProcess(Thread* thread);

	// Permite sacar un proceso de la tabla de procesos a partir de un descriptor.

	bool detachProcess(SpaceId id, int exitValue);

	// Permite obtener el valor de retorno de un proceso a partir de un descriptor.

	bool getExitValue(SpaceId id, int& exitValue);

	// Permite obtener un proceso a partir de su descriptor.

	Thread* getThread(SpaceId id);

	// Permite obtener el descriptor asociado a un proceso.

	SpaceId getSpaceId(Thread* thread);

private:

	Process** processList;

};


#endif // PROCESSTABLE_H
