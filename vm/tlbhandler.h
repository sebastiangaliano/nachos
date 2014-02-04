//----------------------------------------------------------------------------------------
// tlbhandler.h
// Estructura de datos para implementar un manejador sobre la tabla TLB.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------


#ifndef TLBHANDLER_H
#define TLBHANDLER_H


class TlbHandler {

public:

	// Constructor y destructor.

	TlbHandler();
	~TlbHandler();

	// Permite actualizar la tabla TLB para que contenga la entrada asociada a la
	// pagina virtual <virtualPage>.

	void UpdateTLB(int virtualPage);

private:

	// Permite buscar una entrada, candidata a ser reemplazada, en la tabla TLB.

	int ChoiceEntryToReplace();
};


#endif //TLBHANDLER_H
