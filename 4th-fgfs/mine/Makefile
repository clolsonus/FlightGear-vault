#
# Makefile for xarmsim
#
#------------------------------------------------------------------------------
# $Id: Imakefile,v 1.7 1994/03/03 00:12:52 curt Exp $
# (Log is kept at end of this file)

CC =	gcc
CXX =	g++
RM =	/bin/rm
CFLAGS =	$(INCLUDES) -Wall -g
CXXFLAGS =	$(INCLUDES) -Wall -g
LDPATH =	
LDFLAGS =	-lm -lvgagl -lvga

OBJS =	Display.o Trans.o main.o mat3.o
SRCS =	Display.cc Trans.cc main.cc mat3.cc

PROGRAM =	main

$(PROGRAM):  $(OBJS)
	$(CC) -o $(PROGRAM) $(OBJS) $(LDPATH) $(LDFLAGS)

Display.o:	Display.cc Display.h

Trans.o:	Trans.cc Trans.h mat3.h

main.o:		Display.h Trans.h

mat3.o:		mat3.cc mat3.h

clean:
	$(RM) $(PROGRAM) *.o *~


# ----------------------------------------------------------------------------
# $Log: Imakefile,v $
