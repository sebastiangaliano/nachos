//----------------------------------------------------------------------------------------
// La siguiente clase define un "puerto". Un puerto es un mecanismo mas de sincronizacion
// que tambien permite comunicar datos. A grandes rasgos, un thread que llama al metodo
// Send(msg) de un puerto quedara bloqueado hasta que otro thread llame al metodo
// Receive(*msg) de ese mismo puerto, y viceversa. De esta manera el mensaje <msg> es
// enviado desde el thread que invoca a Send() hacia el thread que invoca a Receive().
//
// NOTA: Utilizaremos la bandera 'p' para los mensajes de debug asociados a puertos.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------

#include "synch.h"


class Port {

public:

	Port(const char* debugName);
	~Port();
	void Send(int msg);
	void Receive(int* msg);
	const char* getName() { return name; }

private:

	const char* name;        // Nombre del puerto, util para debugging.
	Lock* pLock;             // Lock asociado al puerto.
	Condition* sndCond;      // Variable de condicion para emisores.
	Condition* rcvCond;      // Variable de condicion para receptores.
	List<int>* buffer;       // Buffer de mensajes del puerto.
	int sndNum;              // Numero de emisores activos.
	int rcvNum;              // Numero de receptores activos.

	char* pLockName;         // Nombre del lock.
	char* sndCondName;       // Nombre de la variable de condicion para emisores.
	char* rcvCondName;       // Nombre de la variable de condicion para receptores.
};
