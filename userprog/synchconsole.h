//----------------------------------------------------------------------------------------
// synchconsole.h
// Estructura de datos para exportar una interfaz que provea acceso sincronizado sobre
// una consola.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------


#ifndef SYNCHCONSOLE_H
#define SYNCHCONSOLE_H

#include "console.h"
#include "synch.h"


class SynchConsole {

public:

	// Constructor, inicializa un objeto de la clase.

	SynchConsole(const char *readFile, const char *writeFile);

	// Destructor, libera la memoria alocada.

	~SynchConsole();

	// Funciones de callbacks para registrar sobre la consola, estas seran
	// llamadas para notificar que una lectura y/o escritura ha finalizado.

	void OnReadDone();
	void OnWriteDone();

	// Interfaz publica de la clase: Provee acceso a la consola pero de
	// manera sincronizada.

	char GetChar();
	void PutChar(const char c);
	void GetBuffer(char *outBuff, int numBytes);
	void PutBuffer(const char *buff, int numBytes);

private:

	Console *console;   	// Consola sobre la cual actuaremos.
	Semaphore *readSem;		// Semaforo para sincronizar el proceso de lectura.
	Semaphore *writeSem;	// Semaforo para sincronizar el proceso de escritura.
	Lock *readLock;			// Lock para sincronizar multiples lecturas.
	Lock *writeLock;		// Lock para sincronizar multiples escrituras.
};

#endif
