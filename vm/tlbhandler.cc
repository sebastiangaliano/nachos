//----------------------------------------------------------------------------------------
// tlbhandler.cc
// Estructura de datos para implementar un manejador sobre la tabla TLB.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------


#include "tlbhandler.h"
#include "machine.h"
#include "system.h"


//----------------------------------------------------------------------------------------
// TlbHandler::TlbHandler
//----------------------------------------------------------------------------------------

TlbHandler::TlbHandler()
{
}

//----------------------------------------------------------------------------------------
// TlbHandler::~TlbHandler
//----------------------------------------------------------------------------------------

TlbHandler::~TlbHandler()
{
}

//----------------------------------------------------------------------------------------
// TlbHandler::ChoiceEntryToReplace
// Encuentra una entrada, candidata a ser reemplazada, en la TLB. Este metodo
// sera llamado en el proceso de actualizacion de la tabla TLB.
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

	int entryToReplace = ChoiceEntryToReplace();
	int vpToReplace = machine->tlb[entryToReplace].virtualPage;

	DEBUG('v',"[TLB]: Changing TLB entry %d (vp %d) from process %s to entry %d.\n",
          entryToReplace, vpToReplace, currentThread->getName(), virtualPage);

	// Actualizamos la tabla TLB.

	TranslationEntry *entry = currentThread->space->GetPage(virtualPage);
	machine->tlb[entryToReplace].virtualPage = entry->virtualPage;
	machine->tlb[entryToReplace].physicalPage = entry->physicalPage;
	machine->tlb[entryToReplace].valid = true;
	machine->tlb[entryToReplace].readOnly = entry->readOnly;
	machine->tlb[entryToReplace].use = entry->use;
	machine->tlb[entryToReplace].dirty = entry->dirty;
}
