#
# Makefile for xarmsim
#
#------------------------------------------------------------------------------
# $Id: Imakefile,v 1.7 1994/03/03 00:12:52 curt Exp $
# (Log is kept at end of this file)

CC =	gcc
CXX =	g++
RM =	-rm -f
CFLAGS =	$(INCLUDES) -Wall -g
CXXFLAGS =	$(INCLUDES) -Wall # -g
INCLUDES =	-I/usr/local/X11R5/include # -I../srgp -I../sphigs
LDPATH =	-L/usr/local/X11R5/lib # -L../srgp -L../sphigs
LDFLAGS =	-lm -lX11 # -lsrgp -lsphigs

OBJS =	Graphics.o ArmScreen.o Trans.o main.o mat3.o
SRCS =	Graphics.cc ArmScreen.cc Trans.cc main.cc mat3.cc

PROGRAM =	xarmsim

$(PROGRAM):  $(OBJS)
	$(CXX) -o $(PROGRAM) $(OBJS) $(LDPATH) $(LDFLAGS)

Graphics.o:	Graphics.cc Graphics.h xarmsim.h bitmaps/icon.bmp

ArmScreen.o:	ArmScreen.cc ArmScreen.h Graphics.h

Trans.o:	Trans.cc Trans.h mat3.h

main.o:		ArmScreen.h Trans.h

mat3.o:		mat3.cc mat3.h

clean:
	$(RM) $(PROGRAM) *.o *~
# ----------------------------------------------------------------------------
# $Log: Imakefile,v $
