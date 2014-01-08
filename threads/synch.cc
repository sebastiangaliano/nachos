// synch.cc
// Routines for synchronizing threads. Three kinds of synchronization routines are
// defined here: semaphores, locks and condition variables (the implementation of the
// last two are left to the reader).
//
// Any implementation of a synchronization routine needs some primitive atomic operation.
// We assume Nachos is running on a uniprocessor, and thus atomicity can be provided by
// turning off interrupts. While interrupts are disabled, no context switch can occur,
// and thus the current thread is guaranteed to hold the CPU throughout, until interrupts
// are reenabled.
//
// Because some of these routines might be called with interrupts already disabled
// (Semaphore::V for one), instead of turning on interrupts at the end of the atomic
// operation, we always simply re-set the interrupt state back to its original value
// (whether that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved. See copyright.h for copyright notice and limitation of liability
// and disclaimer of warranty provisions.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------

#include "copyright.h"
#include "synch.h"
#include "system.h"


//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
// SEMAPHORES IMPLEMENTATION -------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// Semaphore::Semaphore
// Initialize a semaphore, so that it can be used for synchronization.
//
// "debugName" is an arbitrary name, useful for debugging.
// "initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------------------------

Semaphore::Semaphore(const char* debugName, int initialValue)
{
	name = debugName;
	value = initialValue;
	queue = new List<Thread*>;
	DEBUG('s', "[SEM]: Sem %s created.\n", name);
}

//----------------------------------------------------------------------------------------
// Semaphore::~Semaphore
// De-allocate semaphore, when no longer needed. Assume no one is still waiting on the
// semaphore!.
//----------------------------------------------------------------------------------------

Semaphore::~Semaphore()
{
	delete queue;
	DEBUG('s', "[SEM]: Sem %s destroyed.\n", name);
}

//----------------------------------------------------------------------------------------
// Semaphore::P
// Wait until semaphore value > 0, then decrement. Checking the value and decrementing
// must be done atomically, so we need to disable interrupts before checking the value.
// Note that Thread::Sleep assumes that interrupts are disabled when it is called.
//----------------------------------------------------------------------------------------

void Semaphore::P()
{
	// Disable interrupts.

	IntStatus oldLevel = interrupt->SetLevel(IntOff);

	// When semaphore not available, go to sleep.

	DEBUG('s', "[SEM]: Thread %s checking Sem %s...\n", currentThread->getName(), name);

	while (value == 0) {
		DEBUG('s', "[SEM]: Thread %s blocked.\n", currentThread->getName());
		queue->Append(currentThread);
		currentThread->Sleep();
	}

	// When semaphore available, consume its value.

	value--;
	DEBUG('s', "[SEM]: Thread %s consumed Sem %s.\n", currentThread->getName(), name);

	// Re-enable interrupts.

	interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------------------------
// Semaphore::V
// Increment semaphore value, waking up a waiter if necessary. As with P(), this
// operation must be atomic, so we need to disable interrupts.
// Scheduler::ReadyToRun() assumes that threads are disabled when it is called.
//----------------------------------------------------------------------------------------

void Semaphore::V()
{
	Thread* thread;

	// Disable interrupts.

	IntStatus oldLevel = interrupt->SetLevel(IntOff);

	// Obtain a thread from queue and make it ready, consuming the V immediately.

	thread = queue->Remove();

	if (thread != NULL) {
		scheduler->ReadyToRun(thread);
		DEBUG('s', "[SEM]: Thread %s awakened and READY TO RUN.\n", thread->getName());
	}

	value++;

	// Re-enable interrupts.

	interrupt->SetLevel(oldLevel);
}


//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
// LOCKS IMPLEMENTATION ------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// Lock::Lock
// Inicializa un lock, para luego utilizarlo como mecanismo de sincronizacion.
//
// "debugName" es el nombre asignado al lock, util para debugging.
//----------------------------------------------------------------------------------------

Lock::Lock(const char* debugName)
{
	name = debugName;
	lockOwner = NULL;

	int nameSize = strlen(name);
	semName = new char[nameSize + 10];
	strcpy(semName, name);
	strcat(semName, ".sem");

	lockSem = new Semaphore(semName, 1);
	DEBUG('s', "[LOCK]: Lock %s created.\n", name);
}

//----------------------------------------------------------------------------------------
// Lock::~Lock
// Libera el espacio asignado para el lock cuando ya no sea necesario. Asume que no hay
// nadie esperando en el lock.
//----------------------------------------------------------------------------------------

Lock::~Lock()
{
	delete lockSem;
	delete semName;
	DEBUG('s', "[LOCK]: Lock %s destroyed.\n", name);
}

//----------------------------------------------------------------------------------------
// Lock::isHeldByCurrentThread
// Devuelve <true> solo si el thread actual es el que posee el lock.
//----------------------------------------------------------------------------------------

bool Lock::isHeldByCurrentThread()
{
	return (lockOwner == currentThread);
}

//----------------------------------------------------------------------------------------
// Lock::Acquire
// Intenta adquirir el lock, si el lock no se encuentra disponible, el thread llamante
// queda bloqueado.
//----------------------------------------------------------------------------------------

void Lock::Acquire()
{
	// Comprobamos que el Acquire no lo quiera hacer el thread que ya posee el lock.

	ASSERT(not isHeldByCurrentThread());

	// Se comprueba si es necesario resolver el problema de inversion de prioridades.

	if (lockOwner !=NULL) {

		int ownerPr = lockOwner->getPriority();

		if (ownerPr < currentThread->getPriority()) {

			lockOwner->setPriority(currentThread->getPriority());

			if (lockOwner->getStatus() == READY) {

				Thread* auxTh = scheduler->RemoveFromList(ownerPr);

				while ( auxTh != lockOwner) {
	                scheduler->ReadyToRun(auxTh);
					auxTh = scheduler->RemoveFromList(ownerPr);
				}

                scheduler->ReadyToRun(auxTh);

            }

        }

    }

	// Tratamos de consumir el semaforo asociado al lock. Las interrupciones se
	// deshabilitan para que el mensaje de DEBUG se muestre en el instante en que
	// ocurre la accion.

	IntStatus oldLevel = interrupt->SetLevel(IntOff);
	DEBUG('s', "[LOCK]: Thread %s checking Lock %s...\n", currentThread->getName(), name);
	lockSem->P();

	// Seteamos el nuevo lockOwner.

	lockOwner = currentThread;
	DEBUG('s', "[LOCK]: Thread %s acquired Lock %s.\n", currentThread->getName(), name);
	interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------------------------
// Lock::Release
// Permite liberar el lock, despertando a alguno de los threads que estaban esperando
// para adquirir el mismo.
//----------------------------------------------------------------------------------------

void Lock::Release()
{
	// Comprobamos que el Release lo haga el thread que posee el lock (lockOwner).

	ASSERT(isHeldByCurrentThread());

	// Liberamos el semaforo asociado al lock.

	lockOwner = NULL;

	IntStatus oldLevel = interrupt->SetLevel(IntOff);
	DEBUG('s', "[LOCK]: Thread %s released Lock %s.\n", currentThread->getName(), name);
	lockSem->V();
	interrupt->SetLevel(oldLevel);

	// Restablecemos la prioridad que tenia inicialmente el lockOwner, por las
	// dudas que haya sido modificada.

	currentThread->setPriority(currentThread->getInitialPriority());
}


//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
// CONDITION VARIABLES IMPLEMENTATION ----------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// Condition::Condition
//----------------------------------------------------------------------------------------

Condition::Condition(const char* debugName, Lock* conditionLock)
{
	name = debugName;
	relatedLock = conditionLock;
	waitingList = new List<Semaphore*>;
	DEBUG('s', "[CV]: CondVar %s created.\n", name);
}

//----------------------------------------------------------------------------------------
// Condition::~Condition
//----------------------------------------------------------------------------------------

Condition::~Condition()
{
	delete waitingList;
	DEBUG('s', "[CV]: CondVar %s destroyed.\n", name);
}

//----------------------------------------------------------------------------------------
// Condition::Wait
// Note: Without a correct implementation of Condition::Wait(), the test case in the
// network assignment won't work!.
//----------------------------------------------------------------------------------------

void Condition::Wait()
{
	// Comprobamos que el thread que llama a Wait tenga adquirido el lock.

	ASSERT(relatedLock->isHeldByCurrentThread());

	// Hacemos Wait sobre la variable de condicion.

	int nameSize = strlen(name) + strlen(currentThread->getName());
	char* semName = new char[nameSize + 10];
	strcpy(semName, name);
	strcat(semName, ".sem.");
	strcat(semName, currentThread->getName());

	Semaphore* threadSem = new Semaphore(semName, 0);
	waitingList->Append(threadSem);

	// Liberamos el Lock asociado a la variable de condicion y enviamos el thread
	// llamante a dormir.

	IntStatus oldLevel = interrupt->SetLevel(IntOff);
	DEBUG('s', "[CV]: Thread %s waiting on CondVar %s.\n", currentThread->getName(), name);
	relatedLock->Release();
	threadSem->P();
	interrupt->SetLevel(oldLevel);

	// Cuando el thread despierte debe tomar nuevamente el lock.

	relatedLock->Acquire();

	// Liberamos la memoria alocada para el semaforo.

	delete threadSem;
	delete semName;
}

//----------------------------------------------------------------------------------------
// Condition::Signal
//----------------------------------------------------------------------------------------

void Condition::Signal()
{
	// Comprobamos que el thread que llama a Signal tenga adquirido el lock.

	ASSERT(relatedLock->isHeldByCurrentThread());

	// Despertamos a uno de los threads que esperaban en la variable de condicion.

	IntStatus oldLevel = interrupt->SetLevel(IntOff);
	DEBUG('s', "[CV]: Thread %s signaled CondVar %s.\n", currentThread->getName(), name);

	if (not waitingList->IsEmpty())
		(waitingList->Remove())->V();

	interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------------------------
// Condition::Broadcast
//----------------------------------------------------------------------------------------

void Condition::Broadcast()
{
	// Comprobamos que el thread que llama a Broadcast tenga adquirido el lock.

	ASSERT(relatedLock->isHeldByCurrentThread());

	// Despertamos a todos los threads que esperaban sobre la variable de condicion.

	IntStatus oldLevel = interrupt->SetLevel(IntOff);
	DEBUG('s', "[CV]: Thread %s broadcasted CondVar %s.\n", currentThread->getName(), name);

	while (not waitingList->IsEmpty())
		(waitingList->Remove())->V();

	interrupt->SetLevel(oldLevel);
}
