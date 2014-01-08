//----------------------------------------------------------------------------------------
// fdtable.cc
// Estructura de datos que implementa una tabla para descriptores de archivos (FDT) y
// provee de una interfaz para utilizarla.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------


#include "fdtable.h"
#include "system.h"


//----------------------------------------------------------------------------------------
// FDTable::FDTable()
//----------------------------------------------------------------------------------------

FDTable::FDTable()
{
	fdt = new OpenFile*[FDT_SIZE];
	threadOwner = new Thread*[FDT_SIZE];

	for (int i = 0; i < FDT_SIZE; i++) {
		fdt[i] = NULL;
		threadOwner[i] = NULL;
	}
}

//----------------------------------------------------------------------------------------
// FDTable::~FDTable()
//----------------------------------------------------------------------------------------

FDTable::~FDTable()
{
	delete fdt;
	delete threadOwner;
}

//----------------------------------------------------------------------------------------
// FDTable::getFile()
//----------------------------------------------------------------------------------------

OpenFile* FDTable::getFile(OpenFileId id)
{
	DEBUG('y', "[FDT]: Getting file for descriptor: %d.\n", id);

	// Controlamos que el <id> este en el rango correcto y que el thread que lo quiere
	// obtener sea el dueño.

	if (id < 2 || id >= FDT_SIZE)
	{
		DEBUG('y', "[FDT]: Can't get file, incorrect descriptor!\n");
		return NULL;
	}
	else if (currentThread != threadOwner[id])
	{
		DEBUG('y', "[FDT]: Can't get file, thread is not the owner!\n");
		return NULL;
	}
	else if (fdt[id] == NULL)
	{
		DEBUG('y', "[FDT]: Can't get file, descriptor is free!\n");
		return NULL;
	}

	// Si los controles se pasaron, retornamos el archivo asociado al descriptor.

	return fdt[id];
}

//----------------------------------------------------------------------------------------
// FDTable::attachFile()
//----------------------------------------------------------------------------------------

OpenFileId FDTable::attachFile(OpenFile *openfile)
{
	// Agregamos el archivo en el primer descriptor disponible de la FDT. Recordar que
	// los descriptores 1 y 0 estan reservados para STDIN y STDOUT.

	int i = 2;

	while (fdt[i] != NULL && i < FDT_SIZE)
		i++;

	if (i < FDT_SIZE)
	{
		DEBUG('y',"[FDT]: File attached in position %d.\n", i);
		fdt[i] = openfile;
		threadOwner[i] = currentThread;
		return i;
	}

	DEBUG('y',"[FDT]: Can't attach the file, no descriptor available!\n");
	return -1;
}

//----------------------------------------------------------------------------------------
// FDTable::detachFile()
//----------------------------------------------------------------------------------------

bool FDTable::detachFile(OpenFileId id)
{
	DEBUG('y', "[FDT]: Detaching file from descriptor: %d.\n", id);

	// Controlamos que el <id> este en el rango correcto y que el thread que lo quiere
	// liberar sea el dueño.

	if (id < 2 || id >= FDT_SIZE)
	{
		DEBUG('y', "[FDT]: Can't detach file, incorrect descriptor!\n");
		return false;
	}
	else if (currentThread != threadOwner[id])
	{
		DEBUG('y', "[FDT]: Can't detach file, thread is not the owner!\n");
		return false;
	}
	else if (fdt[id] == NULL)
	{
		DEBUG('y', "[FDT]: Can't detach file, descriptor is already free!\n");
		return false;
	}

	// Si los controles se pasaron, liberamos el descriptor.

	fdt[id] = NULL;
	threadOwner[id] = NULL;
	return true;
}
