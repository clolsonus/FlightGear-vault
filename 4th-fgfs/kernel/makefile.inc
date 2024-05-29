all: $(EXECUTABLE)
fltgear: all

MAINSOURCE=fgmain.c fgload.c fgkbd.c fgscreen.c fggrx.c fgslew.c \
	fgtimer.c fgini.c fggeom.c fgevent.c fgpause.c \
	fgrand.c fgpalet.c fgbars.c fgclock.c fgfont.c

MAININCS=fgmain.h fgload.h fgkbd.h fgscreen.h fggrx.h fgslew.h \
        fgtimer.h fgini.h fggeom.h fgevent.h fgpause.h \
	fgrand.h fgpalet.h fgbars.h fgclock.h fgfont.h \
	fgdefs.h fgtypes.h fgvars.h

MAINOBJS=fgmain.$(OBJ_SUFFIX) \
         fgload.$(OBJ_SUFFIX) \
	 fgkbd.$(OBJ_SUFFIX) \
	 fgscreen.$(OBJ_SUFFIX) \
	 fggrx.$(OBJ_SUFFIX) \
	 fgslew.$(OBJ_SUFFIX) \
	 fgtimer.$(OBJ_SUFFIX) \
	 fgini.$(OBJ_SUFFIX) \
	 fggeom.$(OBJ_SUFFIX) \
	 fgevent.$(OBJ_SUFFIX) \
	 fgpause.$(OBJ_SUFFIX) \
	 fgrand.$(OBJ_SUFFIX) \
	 fgpalet.$(OBJ_SUFFIX) \
	 fgbars.$(OBJ_SUFFIX) \
	 fgclock.$(OBJ_SUFFIX) \
	 fgfont.$(OBJ_SUFFIX) 

depend: 
	makedepend -fmakefile.inc -o.$(OBJ_SUFFIX) $(SYS_DEFINES) $(FGDEFS) \
				     $(MAINSOURCE)

clean:
	$(RM) $(EXECUTABLE) $(RESOURCE) *.$(OBJ_SUFFIX) 



$(EXECUTABLE): $(MAINOBJS) $(RESOURCE) $(EXTRA_OBJS)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(MAINOBJS) $(EXTRA_OBJS) $(LIBDIRS) $(FGLIBS)
	$(LINK.RESOURCE)

fgmain.$(OBJ_SUFFIX): fgdefs.h fgtypes.h fgvars.h fgini.h fgevent.h 
fgmain.$(OBJ_SUFFIX): fgdriver.h fggrx.h fgstatdv.h fgslew.h fgtimer.h fgpause.h
fgmain.$(OBJ_SUFFIX): fgscreen.h fgkbd.h fgrand.h fgbars.h fgclock.h fggeom.h fgload.h
fgmain.$(OBJ_SUFFIX): fgfont.h fgmain.h
fgload.$(OBJ_SUFFIX): fgdefs.h
fgload.$(OBJ_SUFFIX): fgdriver.h fgload.h
fgkbd.$(OBJ_SUFFIX): fgdefs.h fgtypes.h fgdriver.h fgevent.h
fgkbd.$(OBJ_SUFFIX): fgkbd.h fgtimer.h fgscreen.h fgmain.h fgkeydef.h
fgscreen.$(OBJ_SUFFIX): fgscreen.h fgdefs.h fgtypes.h fgdriver.h fgevent.h
fgscreen.$(OBJ_SUFFIX): fggrx.h fgkbd.h fgtimer.h fgmain.h fgpalet.h fgfont.h fgini.h
fgscreen.$(OBJ_SUFFIX): fgpause.h
fggrx.$(OBJ_SUFFIX): fgdefs.h fgtypes.h fggeom.h fgscreen.h
fggrx.$(OBJ_SUFFIX): fgdriver.h fgevent.h fgpalet.h
fgslew.$(OBJ_SUFFIX): fgdefs.h fgtypes.h fgdriver.h fgevent.h
fgslew.$(OBJ_SUFFIX): fggeom.h fgvars.h fgslew.h fgtimer.h
fgtimer.$(OBJ_SUFFIX): fgdefs.h fgtypes.h fgdriver.h fgevent.h
fgtimer.$(OBJ_SUFFIX): fgvars.h fgtimer.h fgmain.h
fgini.$(OBJ_SUFFIX):  fgdefs.h fgtypes.h fgvars.h fgdriver.h
fgini.$(OBJ_SUFFIX): fgevent.h fgini.h fgmain.h
fggeom.$(OBJ_SUFFIX):  fgtypes.h fgvars.h fgdefs.h fggeom.h
fggeom.$(OBJ_SUFFIX): fgdriver.h fgmain.h
fgevent.$(OBJ_SUFFIX):  fgdefs.h fgevent.h fgtypes.h
fgpause.$(OBJ_SUFFIX):  fgdefs.h fgtypes.h fgdriver.h fgevent.h
fgpause.$(OBJ_SUFFIX): fgkbd.h fgpause.h
fgrand.$(OBJ_SUFFIX):  fgdefs.h
fgrand.$(OBJ_SUFFIX): fgtypes.h fgdriver.h fgevent.h fgtimer.h fggrx.h fgrand.h
fgrand.$(OBJ_SUFFIX): fgscreen.h fgvars.h
fgpalet.$(OBJ_SUFFIX):  fgdefs.h fgtypes.h fgpalet.h
fgpalet.$(OBJ_SUFFIX): fgscreen.h fgdriver.h fgevent.h
fgbars.$(OBJ_SUFFIX):  fgdefs.h fgtypes.h fgdriver.h
fgbars.$(OBJ_SUFFIX): fgevent.h fgtimer.h fggrx.h fgscreen.h fgbars.h
fgclock.$(OBJ_SUFFIX):  fgdefs.h fgtypes.h fgvars.h
fgclock.$(OBJ_SUFFIX): fgdriver.h fgevent.h fgtimer.h fggrx.h fgfont.h fgscreen.h
fgclock.$(OBJ_SUFFIX): fgclock.h
fgfont.$(OBJ_SUFFIX):  fgdefs.h
fgfont.$(OBJ_SUFFIX): fgtypes.h fgfont.h fgpalet.h fgdriver.h fgmain.h
