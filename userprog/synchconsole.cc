//----------------------------------------------------------------------------------------
// synchconsole.cc
// Implementacion de la interfaz para proveer un acceso sincronizado sobre una consola.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------


#include "synchconsole.h"


//----------------------------------------------------------------------------------------
// Metodos estaticos para registrar como funciones de callback sobre la consola.
//----------------------------------------------------------------------------------------

static void OnReadDoneCb(void* arg)
{
	((SynchConsole *) arg)->OnReadDone();
}

static void OnWriteDoneCb(void* arg)
{
	((SynchConsole *) arg)->OnWriteDone();
}

//----------------------------------------------------------------------------------------
// SynchConsole::SynchConsole
//----------------------------------------------------------------------------------------

SynchConsole::SynchConsole(const char *readFile, const char *writeFile)
{
	// Creamos una consola.

	console = new Console(readFile, writeFile, OnReadDoneCb, OnWriteDoneCb, this);

	// Creamos los objetos que utilizaremos para sincronizacion.

	readSem = new Semaphore("consoleReadSem", 0);
	writeSem = new Semaphore("consoleWriteSem", 0);
	readLock = new Lock("consoleReadLock");
	writeLock = new Lock("consoleWriteLock");
}

//----------------------------------------------------------------------------------------
// SynchConsole::~SynchConsole
//----------------------------------------------------------------------------------------

SynchConsole::~SynchConsole()
{
	delete readSem;
	delete writeSem;
	delete readLock;
	delete writeLock;
	delete console;
}

//----------------------------------------------------------------------------------------
// SynchConsole::OnReadDone
//----------------------------------------------------------------------------------------

void SynchConsole::OnReadDone()
{
	readSem->V();
}

//----------------------------------------------------------------------------------------
// SynchConsole::OnWriteDone
//----------------------------------------------------------------------------------------

void SynchConsole::OnWriteDone()
{
	writeSem->V();
}

//----------------------------------------------------------------------------------------
// SynchConsole::GetChar
//----------------------------------------------------------------------------------------

char SynchConsole::GetChar()
{
	char c;

	// Tratamos de adquirir el lock de lectura.

	readLock->Acquire();

	// Esperamos hasta que haya un caracter disponible para leer.

	readSem->P();

	// Obtenemos el caracter.

	c = console->GetChar();

	// Liberamos el lock previamente adquirido y retornamos el caracter.

	readLock->Release();
	return c;
}

//----------------------------------------------------------------------------------------
// SynchConsole::PutChar
//----------------------------------------------------------------------------------------

void SynchConsole::PutChar(const char c)
{
	// Tratamos de adquirir el lock de escritura.

	writeLock->Acquire();

	// Insertamos el caracter en la consola y esperamos la notificacion de que la
	// operacion finalizo.

	console->PutChar(c);
	writeSem->P();

	// Liberamos el lock previamente adquirido.

	writeLock->Release();
}

//----------------------------------------------------------------------------------------
// SynchConsole::GetBuffer
//----------------------------------------------------------------------------------------

void SynchConsole::GetBuffer(char *outBuff, int numBytes)
{
	// Tratamos de adquirir el lock de lectura.

	readLock->Acquire();

	// Leemos <numBytes> caracteres desde la consola.

	for (int i = 0; i < numBytes; i++) {
		readSem->P();
		outBuff[i] = console->GetChar();
	}

	// Liberamos el lock previamente adquirido.

	readLock->Release();
}

//----------------------------------------------------------------------------------------
// SynchConsole::PutBuffer
//----------------------------------------------------------------------------------------

void SynchConsole::PutBuffer(const char *buff, int numBytes)
{
	// Tratamos de adquirir el lock de escritura.

	writeLock->Acquire();

	// Insertamos los caracteres en la consola, esperando la notificacion de que la
	// operacion finalizo en cada iteracion.

	for (int i = 0; i < numBytes; i++) {
		console->PutChar(buff[i]);
		writeSem->P();
	}

	// Liberamos el lock previamente adquirido.

	writeLock->Release();
}
