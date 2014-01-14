//----------------------------------------------------------------------------------------
// tlbhandler.cc
// Estructura de datos que implementa un manejador sobre la tabla TLB.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------


#include "tlbhandler.h"
#include "machine.h"
#include "system.h"


//----------------------------------------------------------------------------------------
// TlbHandler::TlbHandler
//----------------------------------------------------------------------------------------

TlbHandler::TlbHandler(int nEntries)
{
	numEntries = nEntries;
	//bitMap = new BitMap(numBits);
}

//----------------------------------------------------------------------------------------
// TlbHandler::~TlbHandler
//----------------------------------------------------------------------------------------

TlbHandler::~TlbHandler()
{
	//delete bitMap;
}

//----------------------------------------------------------------------------------------
// TlbHandler::ChoiceEntryToReplace
// Encuentra una entrada en la TLB que es candidata a ser reemplazada.
//----------------------------------------------------------------------------------------

int TlbHandler::ChoiceEntryToReplace()
{
	int i;

	// En primer lugar, tratamos de buscar alguna entrada no valida.

	for (i = 0; i < TLBSize && (machine->tlb[i].valid); i++)
		;

	// Si todas las entradas son validas, elegimos una al azar.

	if (i == TLBSize)
		i = rand() % TLBSize;

	// Retornamos la entrada candidata a reemplazar.

	return i;
}

//----------------------------------------------------------------------------------------
// TlbHandler::UpdateTLB
// Actualiza la tabla TLB para incluir la entrada asociada a la pagina virtual
// <virtualPage> del thread que se esta ejecutando actualmente.
//----------------------------------------------------------------------------------------

void TlbHandler::UpdateTLB(int virtualPage)
{
	// Obtenemos la entrada a ser reemplazada en la tabla TLB.

	int index = ChoiceEntryToReplace();

	DEBUG('v',"[TLB]: in page %d from process %s, out tlb entry %d.\n",
          virtualPage, currentThread->getName(), index);

	// Actualizamos la tabla TLB.

	TranslationEntry *entry = currentThread->space->GetPage(virtualPage);
	machine->tlb[index].virtualPage = entry->virtualPage;
	machine->tlb[index].physicalPage = entry->physicalPage;
	machine->tlb[index].valid = true;
	machine->tlb[index].readOnly = entry->readOnly;
	machine->tlb[index].use = entry->use;
	machine->tlb[index].dirty = entry->dirty;
}
