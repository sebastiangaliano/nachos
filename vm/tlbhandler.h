//----------------------------------------------------------------------------------------
// tlbhandler.h
// Estructura de datos que implementa un manejador sobre la tabla TLB.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------


#ifndef TLBHANDLER_H
#define TLBHANDLER_H

//#include "bitmap.h"


class TlbHandler {

public:

	// Constructor y destructor.

	TlbHandler(int nEntries);
	~TlbHandler();

	// Permite actualizar la tabla TLB para que aparezca la entrada asociada a la pagina
	// virtual <virtualPage>.

	void UpdateTLB(int virtualPage);

	//BitMap *bitMap;

private:

	// Encuentra una entrada candidata de la tabla TLB para ser reemplazada.

	int ChoiceEntryToReplace();

	int numEntries;		// Numero de entradas en la TLB.
};


#endif //TLBHANDLER_H
