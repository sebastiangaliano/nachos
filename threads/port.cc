//----------------------------------------------------------------------------------------
// Implementacion de la clase Port (Puertos).
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------

#include "port.h"
#include "system.h"


//----------------------------------------------------------------------------------------
// Port::Port
// Constructor: Inicializa un puerto.
//----------------------------------------------------------------------------------------

Port::Port(const char* debugName)
{
	name = debugName;

	// Generamos el lock asociado al puerto. Este lock debera obtenerse antes de
	// ejecutar algun metodo de las variables de condicion definidas a continuacion.

	int nameSize = strlen(name);
	pLockName = new char[nameSize + 10];
	strcpy(pLockName, name);
	strcat(pLockName, ".lock");

	pLock = new Lock(pLockName);

	// Generamos la variable de condicion para los emisores (senders).

	sndCondName = new char[nameSize + 10];
	strcpy(sndCondName, name);
	strcat(sndCondName, ".sndCond");

	sndCond = new Condition(sndCondName, pLock);

	// Generamos la variable de condicion para los receptores (receivers).

	rcvCondName = new char[nameSize + 10];
	strcpy(rcvCondName, name);
	strcat(rcvCondName, ".rcvCond");

	rcvCond = new Condition(rcvCondName, pLock);

	// Inicializamos otros campos.

	buffer = new List<int>;
	sndNum = 0;
	rcvNum = 0;
}

//----------------------------------------------------------------------------------------
// Port::~Port
// Destructor: Libera la memoria utilizada por el puerto.
//----------------------------------------------------------------------------------------

Port::~Port()
{
	delete sndCond; delete sndCondName;
	delete rcvCond; delete rcvCondName;
	delete pLock; delete pLockName;
	delete buffer;
}

//----------------------------------------------------------------------------------------
// Port::Send
// Envia el mensaje <msg> por el puerto. Este metodo bloquea al thread llamante hasta que
// alguien invoque a Receive().
//----------------------------------------------------------------------------------------

void Port::Send(int msg)
{
	// Intentamos adquirir el lock asociado al puerto.

	pLock->Acquire();

	// Si el lock fue adquirido, agregamos el mensaje al buffer interno.

	sndNum++;
	buffer->Append(msg);

	// Mientras no haya receptores, bloqueamos al thread llamante.

	while (rcvNum <= 0)
		sndCond->Wait();

	DEBUG('p',"[PORT]: Thread %s sended msg %d on port %s.\n",
          currentThread->getName(), msg, getName());

	// Despierto a un receptor que estuviera esperando (si es que hay alguno).

	rcvNum--;
	rcvCond->Signal();

	// Liberamos el lock del puerto.

	pLock->Release();
}

//----------------------------------------------------------------------------------------
// Port::Receive
// Recibe el mensaje <msg> por el puerto. Este metodo bloquea al thread llamante hasta que
// alguien llame a Send().
//----------------------------------------------------------------------------------------

void Port::Receive(int *msg)
{
	// Intentamos adquirir el lock asociado al puerto.

	pLock->Acquire();

	// Si el lock fue adquirido, incrementamos el nro. de receptores activos.

	rcvNum++;

	// Mientras no haya emisores, bloqueamos al thread llamante.

	while (sndNum <= 0)
		rcvCond->Wait();

	// Recibimos el mensaje.

	*msg = buffer->Remove();

	DEBUG('p',"[PORT]: Thread %s received msg %d on port %s.\n",
          currentThread->getName(), *msg, getName());

	// Despierto a un emisor que estuviera esperando (si es que hay alguno).

	sndNum--;
	sndCond->Signal();

	// Liberamos el lock del puerto.

	pLock->Release();
}
