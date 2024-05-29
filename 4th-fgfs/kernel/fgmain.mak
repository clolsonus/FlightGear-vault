#
# Borland C++ IDE generated makefile
#
.AUTODEPEND


#
# Borland C++ tools
#
IMPLIB  = Implib
BCC     = Bcc +BccW16.cfg 
TLINK   = TLink
TLIB    = TLib
BRC     = Brc
TASM    = Tasm
#
# IDE macros
#


#
# Options
#
IDE_LFLAGS =  -LC:\USR\LIB
IDE_RFLAGS =
LLATW16_fgmaindexe =  -d -Twe -c -C -v
RLATW16_fgmaindexe =  -l9 -31
BLATW16_fgmaindexe =
CNIEAT_fgmaindexe = -IC:\USR\INCLUDE -D_RTLDLL;WIN32;__BCC__
LNIEAT_fgmaindexe = -x
LEAT_fgmaindexe = $(LLATW16_fgmaindexe)
REAT_fgmaindexe = $(RLATW16_fgmaindexe)
BEAT_fgmaindexe = $(BLATW16_fgmaindexe)

#
# Dependency List
#
Dep_fgmain = \
	fgmain.exe

fgmain : BccW16.cfg $(Dep_fgmain)
  echo MakeNode

Dep_fgmaindexe = \
	fgtimer.obj\
	fgslew.obj\
	fgres.res\
	fgscreen.obj\
	fgrand.obj\
	fgpause.obj\
	fgpalet.obj\
	fgmain.obj\
	fgload.obj\
	fgkbd.obj\
	fgini.obj\
	fggrx.obj\
	fggeom.obj\
	fgfont.obj\
	fgfixed.obj\
	fgevent.obj\
	fgclock.obj\
	fgbars.obj

fgmain.exe : $(Dep_fgmaindexe)
  $(TLINK)   @&&|
 /v $(IDE_LFLAGS) $(LEAT_fgmaindexe) $(LNIEAT_fgmaindexe) +
C:\USR\LIB\c0wl.obj+
fgtimer.obj+
fgslew.obj+
fgscreen.obj+
fgrand.obj+
fgpause.obj+
fgpalet.obj+
fgmain.obj+
fgload.obj+
fgkbd.obj+
fgini.obj+
fggrx.obj+
fggeom.obj+
fgfont.obj+
fgfixed.obj+
fgevent.obj+
fgclock.obj+
fgbars.obj
$<,$*
C:\USR\LIB\bwcc.lib+
C:\USR\LIB\import.lib+
C:\USR\LIB\crtldll.lib

|
	$(BRC) fgres.res $<

Dep_fgtimerdobj = \
	fgdriver.h\
	fgevent.h\
	fgdefs.h\
	fgtypes.h\
	fgvars.h

fgtimer.obj : $(Dep_fgtimerdobj)
  $(BCC)   -P- -c @&&|
 $(CEAT_fgmaindexe) $(CNIEAT_fgmaindexe) -o$@ fgtimer.c
|

fgslew.obj :  fgslew.c
  $(BCC)   -P- -c @&&|
 $(CEAT_fgmaindexe) $(CNIEAT_fgmaindexe) -o$@ fgslew.c
|

fgres.res :  fgres.rc
  $(BRC) $(IDE_RFLAGS) $(REAT_fgmaindexe) $(CNIEAT_fgmaindexe) -R -FO$@ fgres.rc

Dep_fgscreendobj = \
	fgscreen.h

fgscreen.obj : $(Dep_fgscreendobj)
  $(BCC)   -P- -c @&&|
 $(CEAT_fgmaindexe) $(CNIEAT_fgmaindexe) -o$@ fgscreen.c
|

fgrand.obj :  fgrand.c
  $(BCC)   -P- -c @&&|
 $(CEAT_fgmaindexe) $(CNIEAT_fgmaindexe) -o$@ fgrand.c
|

fgpause.obj :  fgpause.c
  $(BCC)   -P- -c @&&|
 $(CEAT_fgmaindexe) $(CNIEAT_fgmaindexe) -o$@ fgpause.c
|

fgpalet.obj :  fgpalet.c
  $(BCC)   -P- -c @&&|
 $(CEAT_fgmaindexe) $(CNIEAT_fgmaindexe) -o$@ fgpalet.c
|

fgmain.obj :  fgmain.c
  $(BCC)   -P- -c @&&|
 $(CEAT_fgmaindexe) $(CNIEAT_fgmaindexe) -o$@ fgmain.c
|

fgload.obj :  fgload.c
  $(BCC)   -P- -c @&&|
 $(CEAT_fgmaindexe) $(CNIEAT_fgmaindexe) -o$@ fgload.c
|

Dep_fgkbddobj = \
	fgkbd.h

fgkbd.obj : $(Dep_fgkbddobj)
  $(BCC)   -P- -c @&&|
 $(CEAT_fgmaindexe) $(CNIEAT_fgmaindexe) -o$@ fgkbd.c
|

fgini.obj :  fgini.c
  $(BCC)   -P- -c @&&|
 $(CEAT_fgmaindexe) $(CNIEAT_fgmaindexe) -o$@ fgini.c
|

fggrx.obj :  fggrx.c
  $(BCC)   -P- -c @&&|
 $(CEAT_fgmaindexe) $(CNIEAT_fgmaindexe) -o$@ fggrx.c
|

fggeom.obj :  fggeom.c
  $(BCC)   -P- -c @&&|
 $(CEAT_fgmaindexe) $(CNIEAT_fgmaindexe) -o$@ fggeom.c
|

fgfont.obj :  fgfont.c
  $(BCC)   -P- -c @&&|
 $(CEAT_fgmaindexe) $(CNIEAT_fgmaindexe) -o$@ fgfont.c
|

fgfixed.obj :  fgfixed.c
  $(BCC)   -P- -c @&&|
 $(CEAT_fgmaindexe) $(CNIEAT_fgmaindexe) -o$@ fgfixed.c
|

fgevent.obj :  fgevent.c
  $(BCC)   -P- -c @&&|
 $(CEAT_fgmaindexe) $(CNIEAT_fgmaindexe) -o$@ fgevent.c
|

fgclock.obj :  fgclock.c
  $(BCC)   -P- -c @&&|
 $(CEAT_fgmaindexe) $(CNIEAT_fgmaindexe) -o$@ fgclock.c
|

fgbars.obj :  fgbars.c
  $(BCC)   -P- -c @&&|
 $(CEAT_fgmaindexe) $(CNIEAT_fgmaindexe) -o$@ fgbars.c
|

# Compiler configuration file
BccW16.cfg :
   Copy &&|
-R
-v
-vi
-H
-H=fgmain.csm
-A
-4
-Om
-Op
-Oi
-Ov
-a
-A-
-wpin
-wstv
-d
-Ff
-Ff=4096
-Vf-
-H-
-ml
-WS
-dc
-h
-v
-R
-k
| $@


