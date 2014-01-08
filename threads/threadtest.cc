// threadtest.cc
// Simple test case for the threads assignment.
//
// Create several threads, and have them context switch back and forth between themselves
// by calling Thread::Yield, to illustrate the inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation of liability
// and disclaimer of warranty provisions.
//
// Parts from Copyright (c) 2007-2009 Universidad de Las Palmas de Gran Canaria.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------


#include "copyright.h"
#include "system.h"
#include "synch.h"
#include "port.h"
#include "stdlib.h"
#include "time.h"
#include "thread.h"


//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
// THREAD TEST ---------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// SimpleThread.
// Loop 10 times, yielding the CPU to another ready thread each iteration. The argument
// "name" points to a string with a thread name, just for debugging purposes.
//----------------------------------------------------------------------------------------

void SimpleThread(void* name)
{
    // Reinterpret arg "name" as a string.

    char* threadName = (char*) name;

    // If the lines dealing with interrupts are commented, the code will behave
	// incorrectly, because printf execution may cause race conditions.

    for (int num = 0; num < 10; num++) {
		//IntStatus oldLevel = interrupt->SetLevel(IntOff);
		printf("*** Thread %s looped %d times\n", threadName, num);
		//interrupt->SetLevel(oldLevel);
        currentThread->Yield();
    }

    //IntStatus oldLevel = interrupt->SetLevel(IntOff);
    printf(">>> Thread %s has finished\n", threadName);
	//interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------------------------
// ThreadTest.
// Set up a ping-pong between several threads, by launching ten (now five) threads which
// call SimpleThread, and finally calling SimpleThread ourselves.
//----------------------------------------------------------------------------------------

void ThreadTest()
{
	DEBUG('t', "Entering SimpleTest...\n");

	for (int k=1; k<=5; k++) {
		char* threadname = new char[100];
		sprintf(threadname, "Hilo %d", k);
		Thread* newThread = new Thread(threadname);
		newThread->Fork(SimpleThread, (void*) threadname);
    }

    SimpleThread((void*) "Hilo 0");
}


//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
// SYNCH TEST ----------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------


typedef struct {
	char* name;
	Condition* cVar;
	Lock* lock;
} _synchST;

//----------------------------------------------------------------------------------------
// threadRoutine.
//----------------------------------------------------------------------------------------

void threadRoutine(void* data)
{
    // Reinterpret arg <data> as a <_synchST> struct.

	_synchST* synchST_ptr = (_synchST*) data;

	(synchST_ptr->lock)->Acquire();
	(synchST_ptr->cVar)->Wait();
	(synchST_ptr->lock)->Release();

    // If the lines dealing with interrupts are commented, the code will behave
	// incorrectly, because printf execution may cause race conditions.

    for (int num = 0; num < 10; num++) {
		IntStatus oldLevel = interrupt->SetLevel(IntOff);
		printf("*** Thread %s looped %d times\n", synchST_ptr->name, num);
		interrupt->SetLevel(oldLevel);
        //currentThread->Yield();
    }

    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    printf(">>> Thread %s has finished\n", synchST_ptr->name);
	interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------------------------
// SynchTest.
//----------------------------------------------------------------------------------------

void SynchTest()
{
	printf(">>> Entering SynchTest...\n");

	Lock* testLock = new Lock("testLock");
	Condition* testCV = new Condition("testCondVar", testLock);

	for (int k=1; k<=5; k++) {
		char* threadName = new char[100];
		sprintf(threadName, "Hilo%d", k);
		Thread* newThread = new Thread(threadName);

		_synchST* synchST_ptr = new _synchST;
		synchST_ptr->name = threadName;
		synchST_ptr->cVar = testCV;
		synchST_ptr->lock = testLock;

		newThread->Fork(threadRoutine, (void*) synchST_ptr);
    }

	currentThread->Yield();
	testLock->Acquire();
	testCV->Broadcast();
	testLock->Release();
}


//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
// PORT TEST -----------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------


typedef struct {
	char* name;
	Port* port;
	int id;
} _portST;

//----------------------------------------------------------------------------------------
// SenderRoutine.
//----------------------------------------------------------------------------------------

void SenderRoutine(void* data)
{
    // Reinterpret arg <data> as a <_portST> struct.

	_portST* portST_ptr = (_portST*) data;

	// Enviamos un mensaje por el puerto.

    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    printf(">>> Thread %s sending msg %d...\n", portST_ptr->name, portST_ptr->id);
	(portST_ptr->port)->Send(portST_ptr->id);
	interrupt->SetLevel(oldLevel);

	// Luego de enviar el mensaje, el thread finaliza.

    printf(">>> Thread %s has finished.\n", portST_ptr->name);
}

//----------------------------------------------------------------------------------------
// PortTest.
//----------------------------------------------------------------------------------------

void PortTest()
{
	printf(">>> Entering Port Test...\n");

	Port* testPort = new Port("testPort");

	for (int k=1; k<=5; k++) {
		char* threadName = new char[100];
		sprintf(threadName, "Hilo%d", k);
		Thread* newThread = new Thread(threadName);

		_portST* portST_ptr = new _portST;
		portST_ptr->name = threadName;
		portST_ptr->port = testPort;
		portST_ptr->id = k;

		newThread->Fork(SenderRoutine, (void*) portST_ptr);
    }

	int message;

	for (int k=1; k<=5; k++) {
		currentThread->Yield();
		testPort->Receive(&message);
		printf(">>> Main received message %d!.\n", message);
	}
}


//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
// JOIN TEST -----------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------
// ChildRoutine.
//----------------------------------------------------------------------------------------

void ChildRoutine(void* name)
{
	// Reinterpret arg "name" as a string.

	char* threadName = (char*) name;

	for (int num = 0; num < 20; num++) {
		printf("*** Thread %s looped %d times\n", threadName, num);
		//currentThread->Yield();
	}

	printf(">>> Thread %s finishing...\n", threadName);
}

//----------------------------------------------------------------------------------------
// JoinTest.
//----------------------------------------------------------------------------------------

void JoinTest()
{
	printf(">>> Entering JoinTest...\n");

	// Creamos un thread "joinable".

	char* threadName = (char*) "CHILD";
	Thread* childThread = new Thread(threadName, true);
	childThread->Fork(ChildRoutine, (void*) threadName);

	// Hacemos JOIN sobre el thread child esperando su finalizacion.

	//currentThread->Yield();
	printf(">>> Thread %s joining thread %s...\n", currentThread->getName(), threadName);
	childThread->Join();
	printf(">>> Thread %s finishing...\n", currentThread->getName());
}


//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
// PRIORITY TEST -------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------


typedef struct {
	char* name;
	Semaphore* sem;
	Lock* lock;
} _priorST;

//----------------------------------------------------------------------------------------
// SimpleThread_1().
//----------------------------------------------------------------------------------------

void SimpleThread_1(void* data)
{
	// Reinterpret arg <data> as a <_synchST> struct.

	_priorST* priorST_ptr = (_priorST*) data;

    // If the lines dealing with interrupts are commented, the code will behave
	// incorrectly, because printf execution may cause race conditions.

	(priorST_ptr->sem)->P();

	int num = 0;

	while (1) {
		printf("*** Thread %s looped %d times\n", priorST_ptr->name, num);
		num++;
		if (num % 10 == 0)
			currentThread->Yield();
	}

    //IntStatus oldLevel = interrupt->SetLevel(IntOff);
    printf(">>> Thread %s has finished\n", priorST_ptr->name);
	//interrupt->SetLevel(oldLevel);
}


//----------------------------------------------------------------------------------------
// SimpleThread_2().
//----------------------------------------------------------------------------------------

void SimpleThread_2(void* data)
{
	// Reinterpret arg <data> as a <_synchST> struct.

	_priorST* priorST_ptr = (_priorST*) data;

    // If the lines dealing with interrupts are commented, the code will behave
	// incorrectly, because printf execution may cause race conditions.

    for (int num = 0; num < 10; num++) {

		//IntStatus oldLevel = interrupt->SetLevel(IntOff);

		if ((num == 0) && (!strcmp(priorST_ptr->name, "Hilo 0"))) {
			(priorST_ptr->lock)->Acquire();
			(priorST_ptr->sem)->V();
			(priorST_ptr->sem)->V();
			(priorST_ptr->sem)->V();
			(priorST_ptr->sem)->V();
			(priorST_ptr->sem)->V();
			//currentThread->Yield();
		}
		else if ((num == 9) && (!strcmp(priorST_ptr->name, "Hilo 0")))
			(priorST_ptr->lock)->Release();
		else if ((!strcmp(priorST_ptr->name, "Hilo 0")))
			currentThread->Yield();
		else if ((num == 5 ) && (!strcmp(priorST_ptr->name, "Hilo 5"))) {
			(priorST_ptr->sem)->P();
			(priorST_ptr->lock)->Acquire();
		}
		else if ((num == 9) && (!strcmp(priorST_ptr->name, "Hilo 5")))
			(priorST_ptr->lock)->Release();

		printf("*** Thread %s looped %d times\n", priorST_ptr->name, num);
		//interrupt->SetLevel(oldLevel);
		//currentThread->Yield();

	}

    //IntStatus oldLevel = interrupt->SetLevel(IntOff);
    printf(">>> Thread %s has finished\n", priorST_ptr->name);
	//interrupt->SetLevel(oldLevel);
}

//----------------------------------------------------------------------------------------
// PriorityTest.
//----------------------------------------------------------------------------------------

void PriorityTest()
{
	printf(">>> Entering Priority Test...\n");
	//srand(time(NULL));
	Lock* testLock = new Lock("testLock");
	Semaphore* testSem = new Semaphore("testSem", 0);

	for (int k=0; k<=5; k++)
	{
		char* threadname = new char[100];
		sprintf(threadname, "Hilo %d", k);
		//Thread* newThread = new Thread(threadname, false, (rand() % _MAX_PRIORITY + 1));
		Thread* newThread = new Thread(threadname, false, k);
		printf(">>> Se creo el thread %s.\n", newThread->getName());

		_priorST* priorST_ptr = new _priorST;
		priorST_ptr->name = threadname;
		priorST_ptr->sem = testSem;
		priorST_ptr->lock = testLock;

		if (k == 0 || k == 5)
			newThread->Fork(SimpleThread_2, (void*) priorST_ptr);
		else
			newThread->Fork(SimpleThread_1, (void*) priorST_ptr);

		scheduler->Print();
	}

}


//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
// MAIN TEST -----------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------


void Test(const char* testCase)
{
	if (!strcmp(testCase, "thread"))
		ThreadTest();
	else if (!strcmp(testCase, "synch"))
		SynchTest();
	else if (!strcmp(testCase, "port"))
		PortTest();
	else if (!strcmp(testCase, "join"))
		JoinTest();
	else if (!strcmp(testCase, "prior"))
		PriorityTest();
}
