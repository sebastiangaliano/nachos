# Use normal make for this Makefile
#
# Makefile for building user programs to run on top of Nachos
#
# Several things to be aware of:
#
# Nachos assumes that the location of the program startup routine (the location the
# kernel jumps to when the program initially starts up) is at location 0. This
# means: start.o must be the first .o passed to ld, in order for the routine "Start"
# to be loaded at location 0.
#-----------------------------------------------------------------------------------------
# Edited by: Leonardo Forti, Sebastian Galiano, Diego Smania
#-----------------------------------------------------------------------------------------


# If you are cross-compiling, you need to point to the right executables and change the
# flags to ld and the build procedure for as.

#TODO: GCCDIR should be correctly set.
#GCCDIR = ~/gnu/local/decstation-ultrix/bin/
GCCDIR = /home/gulinha/mips-dec-ultrix42/mips-
LDFLAGS = -T ../script -N
ASFLAGS = -mips1
CPPFLAGS = $(INCDIR)

CC = $(GCCDIR)gcc
AS = $(GCCDIR)as
LD = $(GCCDIR)ld

CPP = /lib/cpp
INCDIR =-I../../userprog -I../../threads
CFLAGS = -G 0 -c $(INCDIR) -mips1


# Regla para generar "start.o".

start.o: ../start.s ../../userprog/syscall.h
	$(CPP) $(CPPFLAGS) ../start.s > strt.s
	$(AS) $(ASFLAGS) -o start.o strt.s
	rm strt.s

# Regla generica para compilar programas simples, que consistan en un único fuente.

%: %.c
	make start.o 
	$(CC) $(CFLAGS) -c $<
	$(LD) $(LDFLAGS) start.o $*.o -o $*.coff
	../../bin/coff2noff $*.coff $@
	rm start.o $*.o $*.coff

