//----------------------------------------------------------------------------------------
// fdtable.h
// Estructura de datos que implementa una tabla para descriptores de archivos (FDT) y
// provee de una interfaz para utilizarla.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------


#ifndef FDTABLE_H
#define FDTABLE_H

#include "syscall.h"
#include "filesys.h"
#include "thread.h"

#define FDT_SIZE 128


class FDTable {

public:

	// Constructor y destructor.

	FDTable();
	~FDTable();

	// Permite obtener un archivo a partir de su descriptor de archivo.

	OpenFile* getFile(OpenFileId id);

	// Permite agregar un nuevo archivo a la FDT asignandole el menor ID de descriptor
	// disponible en la tabla. Retorna dicho ID o -1 si no se pudo agregar el archivo.

	OpenFileId attachFile(OpenFile *openfile);

	// Permite quitar un archivo de la FDT a partir de su descriptor de archivo,
	// retorna <false> si el descriptor no se encontraba asignado.

	bool detachFile(OpenFileId id);

private:

	OpenFile **fdt;
	Thread **threadOwner;
};


#endif // FDTABLE_H
