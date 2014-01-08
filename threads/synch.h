// synch.h
// NOTA: este es el unico fichero fuente con los comentarios en espaniol.
// 2000 - Jose Miguel Santos Espino - ULPGC
//
// Estructuras de datos para sincronizar hilos (threads).
//
// Aqui se definen tres mecanismos de sincronizacion: semaforos (semaphores), cerrojos
// (locks) y variables de condicion (condition variables). Solo estan implementados los
// semaforos; de los cerrojos y las variables de condicion solo se proporciona la
// interfaz. Precisamente el primer trabajo incluye realizar estas implementaciones.
//
// Todos los objetos de sincronizacion tienen un parametro "name" en el constructor, su
// unica finalidad es facilitar la depuracion del programa.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation.
// synch.h -- synchronization primitives.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------

#ifndef SYNCH_H
#define SYNCH_H

#include "copyright.h"
#include "thread.h"
#include "list.h"

//----------------------------------------------------------------------------------------
// La siguiente clase define un "semaforo" cuyo valor es un entero positivo. El semaforo
// ofrece solo dos operaciones, P() y V():
//
//	P() -- Espera a que value > 0, luego decrementa value.
//
//	V() -- Incrementa value, despierta un hilo en espera si lo hay.
//
// Observen que esta interfaz NO permite leer directamente el valor del semaforo - aunque
// hubieras podido leer el valor, no te sirve de nada, porque mientras tanto otro hilo
// puede haber modificado el semaforo, si tu has perdido la CPU durante un tiempo.
//----------------------------------------------------------------------------------------

class Semaphore {

public:

	// Constructor: otorga un valor inicial al semaforo.

	Semaphore(const char* debugName, int initialValue);

	// Destructor: libera memoria.

	~Semaphore();

	// Metodo util para depuracion.

	const char* getName() { return name; }

	// Las unicas operaciones publicas sobre el semaforo: ambas deben ser "atomicas".

	void P();
	void V();

private:

	const char* name;        // Nombre del semaforo, util para depuracion.
	int value;               // valor del semaforo, siempre es >= 0.
	List<Thread*>* queue;    // Cola con los hilos que esperan por el semaforo.
};

//----------------------------------------------------------------------------------------
// La siguiente clase define un "cerrojo" (Lock). Un lock puede tener dos estados: libre
// y ocupado. Solo se permiten dos operaciones sobre un lock:
//
//	Acquire() -- Espera a que el lock este disponible y lo marca como ocupado.
//
//	Release() -- Marca el cerrojo como libre, despertando a algun otro hilo que
//               estuviera bloqueado en un Acquire().
//
// Por conveniencia, nadie excepto el hilo que tiene adquirido el cerrojo puede
// liberarlo. No hay ninguna operacion para leer el estado del cerrojo.
//----------------------------------------------------------------------------------------

class Lock {

public:

	// Constructor: inicializa el cerrojo como libre.

	Lock(const char* debugName);

	// Destructor: libera espacio.

	~Lock();

	// Metodo util para depuracion.

	const char* getName() { return name; }

	// Operaciones publicas sobre el cerrojo. Ambas deben ser atomicas.

	void Acquire();
	void Release();

	// El siguiente metodo devuelve 'true' si el hilo actual es quien posee el
	// cerrojo. Este metodo es util para comprobaciones en el Release() y en
	// las variables de condicion.

	bool isHeldByCurrentThread();

private:

	const char* name;      // Nombre del Lock, util para depuracion.
	Thread* lockOwner;     // Referencia al thread que actualmente posee el Lock.
	Semaphore* lockSem;    // Semaforo asociado al Lock.
	char* semName;         // Nombre del semaforo asociado al Lock.
};

//----------------------------------------------------------------------------------------
// La siguiente clase define una "variable de condicion". Una variable de condicion no
// tiene valor asociado. Se utiliza para encolar hilos que esperan (Wait) a que otro hilo
// les avise (Signal). Las variables de condicion estan vinculadas a un cerrojo (Lock).
// Estas son las tres operaciones sobre una variable condicion:
//
//	Wait() -- Libera el cerrojo y expulsa al hilo de la CPU. El hilo espera
//            hasta que alguien le hace un Signal().
//
//  Signal() -- Si hay alguien esperando en la variable, despierta a uno de
//              los hilos. De otra manera, no ocurre nada.
//
//	Broadcast() -- Despierta a todos los hilos que estan esperando.
//
// Todas las operaciones sobre una variable de condicion deben ser realizadas adquiriendo
// previamente el cerrojo. Esto significa que las operaciones han de ejecutarse en
// exclusion mutua.
//
// Las variables de condicion de Nachos deberian funcionar segun el estilo "Mesa". Cuando
// un Signal() o Broadast() despierta a otro hilo, este se coloca en la cola de
// preparados. El hilo despertado es responsable de volver a adquirir el cerrojo. Esto lo
// deben implementar en el cuerpo de la funcion Wait(). En contraste, tambien existe otro
// estilo de variables de condicion, segun el estilo "Hoare", el hilo que hace el Signal()
// pierde el control del cerrojo y entrega la CPU al hilo despertado, quien se ejecuta de
// inmediato y cuando libera el cerrojo, devuelve el control al hilo que efectuo el
// Signal().
//
// El estilo "Mesa" es algo mas facil de implementar, pero no garantiza que el hilo
// despertado recupere de inmediato el control del cerrojo.
//----------------------------------------------------------------------------------------

class Condition {

public:

	// Constructor: se le indica cual es el cerrojo que tiene asociado.

	Condition(const char* debugName, Lock* conditionLock);

	// Destructor: libera el objeto.

	~Condition();

	// Metodo util para depuracion.

	const char* getName() { return (name); }

	// Las tres operaciones sobre variables de condicion. El hilo que invoque a
	// cualquiera de estas operaciones debe tener adquirido el cerrojo, de lo contrario
	// se debe producir un error.

	void Wait();
	void Signal();
	void Broadcast();

private:

	const char* name;                 // Nombre de la variable de condicion.
	Lock* relatedLock;                // Lock asociado a la variable de condicion.
	List<Semaphore*>* waitingList;    // Lista con los semaforos asociados a los hilos
                                      // que esperan sobre la variable de condicion.
};

/*****************************************************************************************
Codigo original de Nachos para las variables de condicion - NO USAR.

class Condition {

public:

	Condition(char* debugName);	// initialize condition to "no one waiting"
	~Condition();				// deallocate the condition
	char* getName() { return (name); }

	// These are the 3 operations on condition variables; releasing the
	// lock and going to sleep are *atomic* in Wait(), conditionLock
	// must be held by the currentThread for all of these operations.

	void Wait(Lock *conditionLock);
	void Signal(Lock *conditionLock);
	void Broadcast(Lock *conditionLock);

private:

	char* name;
	// Plus some other stuff you'll need to define.
};
*****************************************************************************************/

#endif // SYNCH_H
