CFLAGS= -g -Wall -DRSX
MAINOBJS=fgmain.o fgload.o fgkbd.o fgscreen.o fggrx.o fgslew.o \
	fgtimer.o fgini.o fggeom.o fgevent.o fgpause.o \
	fgres.res fgrand.o fgpalet.o fgbars.o fgclock.o fgfont.o
RSXLIBDIR=-Le:/emx/rsxwdk/lib
RSXEXE=e:\emx\rsxwdk\rsxw32.exe
WIN16RC=c:/usr/program/c/bin/rc.exe

fgmain.exe: $(MAINOBJS)
	gcc $(CFLAGS) -o fgmain.w32 $(MAINOBJS) $(RSXLIBDIR) -lwinio -lemx -lbsd 
	copy /b $(RSXEXE) fgmain.exe
	$(WIN16RC) -DRSX -x -Ic:\usr\include -t fgres.res fgmain.exe

fgres.res:  fgres.rc fgres.h
	$(WIN16RC) -x -Ic:\usr\include -DRSX -r fgres.rc
