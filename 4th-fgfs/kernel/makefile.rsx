.SUFFIXES: .exe .lib .a .dll .so .sa .o .obj .c .s .cc .res .rc .def
RM=del
CP=copy
CC=gcc
FGDEFS=-DRSX
FGLIBS=-lm -lemx -lbsd 
LIBDIRS=-L/emx/rsxwdk/lib
SYS_DEFINES=-D__EMX__ -D__GNUC__
CFLAGS= -g -Wall $(FGDEFS) 
OBJ_SUFFIX=o
EXECUTABLE=fgmain.w32
EXTRA_OBJS=
RESOURCE=fgres.res
RSXEXE=e:\emx\rsxwdk\rsxw32.exe
WIN16RC=c:/usr/program/c/bin/rc.exe
LINK.RESOURCE=copy /b $(RSXEXE) fgmain.exe ; $(WIN16RC) -x -t fgres.res fgmain.exe
ICON=fgwin.ico

$(RESOURCE): fgres.rc $(ICON)
	$(WIN16RC) -x $(WIN16INC) -r fgres.rc

include makefile.inc

