// main.cc 
// Bootstrap code to initialize the operating system kernel.
//
// Allows direct calls into internal operating system functions, to simplify debugging
// and testing. In practice, the bootstrap code would just initialize data structures,
// and start a user program to print the login prompt.
//
// Most of this file is not needed until later assignments.
//
// USAGE: nachos -d <debugflags> -rs <random seed #>
//               -s -x <nachos file> -c <consoleIn> <consoleOut>
//               -f -cp <unix file> <nachos file>
//               -p <nachos file> -r <nachos file> -l -D -t
//               -n <network reliability> -m <machine id>
//               -o <other machine id>
//               -z
//
// GENERAL OPTIONS:
//    -d causes certain debugging messages to be printed (cf. utility.h).
//    -rs causes Yield to occur at random (but repeatable) spots.
//    -z prints the copyright message.
//
// USER_PROGRAM OPTIONS:
//    -s causes user programs to be executed in single-step mode.
//    -x runs a user program.
//    -c tests the console.
//
// FILESYS OPTIONS:
//    -f causes the physical disk to be formatted.
//    -cp copies a file from UNIX to Nachos.
//    -p prints a Nachos file to stdout.
//    -r removes a Nachos file from the file system.
//    -l lists the contents of the Nachos directory.
//    -D prints the contents of the entire file system.
//    -t tests the performance of the Nachos file system.
//
// NETWORK OPTIONS:
//    -n sets the network reliability.
//    -m sets this machine's host id (needed for the network).
//    -o runs a simple test of the Nachos network software.
//
// TESTING OPTIONS (NEW):
//    -test <test_name> call for a particular test.
//
// NOTE: flags are ignored until the relevant assignment. Some of the flags are
// interpreted here; some in system.cc.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation of liability
// and disclaimer of warranty provisions.
//----------------------------------------------------------------------------------------
// Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
//----------------------------------------------------------------------------------------
// Nota: Se agrego la opcion -test <test_name> que facilitara el testeo de ciertas
// funcionalidades de Nachos, como por ejemplo los mecanismos de sincronizacion para
// Threads.
//----------------------------------------------------------------------------------------

#define MAIN
#include "copyright.h"
#undef MAIN

#include "utility.h"
#include "system.h"

// External functions used by this file.

//void ThreadTest();
void Test(const char* testCase);
void Copy(const char *unixFile, const char *nachosFile);
void Print(const char *file);
void PerformanceTest(void);
void StartProcess(const char *file);
void ConsoleTest(const char *in, const char *out);
void MailTest(int networkID);

//----------------------------------------------------------------------------------------
// main
// Bootstrap the operating system kernel.
//	- Check command line arguments.
// 	- Initialize data structures.
// 	- (optionally) Call test procedure.
//
// "argc" is the number of command line arguments (including the name of the command).
// Example: "nachos -d +" -> argc = 3
//
// "argv" is an array of strings, one for each command line argument.
// Example: "nachos -d +" -> argv = {"nachos", "-d", "+"}
//----------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	int argCount;	// The number of arguments for a particular command.

	DEBUG('t', "Entering main...");
    (void) Initialize(argc, argv);

	for (argc--, argv++; argc > 0; argc -= argCount, argv += argCount) {

		argCount = 1;

		// Print copyright.
		if (!strcmp(*argv, "-z"))
			printf ("%s", copyright);

#ifdef THREADS

		// Execute a particular test case.
		if (!strcmp(*argv, "-test")) {
			ASSERT(argc > 1);
			Test(*(argv + 1));
			argCount = 2;
		}

#endif // THREADS.

#ifdef USER_PROGRAM

		// Run a user program.
		if (!strcmp(*argv, "-x")) {
			ASSERT(argc > 1);
			StartProcess(*(argv + 1));
			argCount = 2;
		}
		// Test the console.
		else if (!strcmp(*argv, "-c")) {
			if (argc == 1)
				ConsoleTest(NULL, NULL);
	    	else {
				ASSERT(argc > 2);
	        	ConsoleTest(*(argv + 1), *(argv + 2));
	        	argCount = 3;
			}

			// Once we start the console, then Nachos will loop forever
			// waiting for console input.

			interrupt->Halt();
		}

#endif // USER_PROGRAM.

#ifdef FILESYS

		// Copy from UNIX to Nachos.
		if (!strcmp(*argv, "-cp")) {
			ASSERT(argc > 2);
			Copy(*(argv + 1), *(argv + 2));
			argCount = 3;
		}
		// Print a Nachos file.
		else if (!strcmp(*argv, "-p")) {
			ASSERT(argc > 1);
			Print(*(argv + 1));
			argCount = 2;
		}
		// Remove Nachos file.
		else if (!strcmp(*argv, "-r")) {
			ASSERT(argc > 1);
			fileSystem->Remove(*(argv + 1));
			argCount = 2;
		}
		// List Nachos directory.
		else if (!strcmp(*argv, "-l")) {
			fileSystem->List();
		}
		// Print entire filesystem.
		else if (!strcmp(*argv, "-D")) {
			fileSystem->Print();
		}
		// Performance test.
		else if (!strcmp(*argv, "-t")) {
			PerformanceTest();
		}

#endif // FILESYS

#ifdef NETWORK

		if (!strcmp(*argv, "-o")) {
			ASSERT(argc > 1);
			// Delay for 2 seconds to give the user time to start up another nachos.
			Delay(2);
			MailTest(atoi(*(argv + 1)));
			argCount = 2;
		}

#endif // NETWORK

	} // END OF FOR LOOP.

	// NOTE: if the procedure "main" returns, then the program "nachos" will exit (as any
	// other normal program would). But there may be other threads on the ready list. We
	// switch to those threads by saying that the "main" thread is finished, preventing
	// it from returning.

	currentThread->Finish();

	// The return is not reached...

	return(0);
}
