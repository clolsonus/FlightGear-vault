# Generated automatically from Makefile.in by configure.
# Generated automatically from Makefile.in by configure.

      srcdir = .
       SHELL = /bin/sh
          CC = gcc
     INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644
STD_INCLUDES = -I. -I$(srcdir)
          AR = ar cq
      RANLIB = ranlib
     LIBOBJS =  # strdup.o
    PROGRAMS = libV.a
      TOPDIR = ../..
  MAKEDEPEND = makedepend

SRCS	= \
	  VOpenVp.c \
	  VResizeVp.c \
	  VExposeVp.c \
	  VCloseVp.c \
	  VGetEyeSpace.c \
	  VGetPlanes.c \
	  VCmpClipN.c \
	  VReadObject.c \
	  VWriteObject.c \
	  VCmpObjExt.c \
	  VCopyObj.c \
	  VOrder.c \
	  VAllocColor.c \
	  VBindColors.c \
	  VExpBuffer.c \
	  VCrPoly.c \
	  VCrPoints.c \
	  VCopyPoly.c \
	  VDrawArc.c \
	  VDrawPoly.c \
	  VDrawSeg.c \
	  VDrawStroke.c \
	  VFillPoly.c \
	  VDepthCue.c \
	  VDrawString.c \
	  VClipPoly.c \
	  VDotProd.c \
	  VCrossProd.c \
	  VRotate.c \
	  VTransPoly.c \
	  VWorldToScr.c \
	  VEyeToScr.c \
	  VTrans.c \
	  VIdentMat.c \
	  VMatMult.c \
	  VQuat.c \
	  VPrintPoly.c \
	  Vmalloc.c \
	  Vdefs.c \
	  frameBuff.c \
	  $(EXTRASRCS) \
	  strdup.c

OBJS	= \
	  VOpenVp.o \
	  VResizeVp.o \
	  VExposeVp.o \
	  VCloseVp.o \
	  VGetEyeSpace.o \
	  VGetPlanes.o \
	  VCmpClipN.o \
	  VReadObject.o \
	  VWriteObject.o \
	  VCmpObjExt.o\
	  VCopyObj.o \
	  VOrder.o \
	  VAllocColor.o \
	  VBindColors.o \
	  VExpBuffer.o \
	  VCrPoly.o \
	  VCrPoints.o \
	  VCopyPoly.o \
	  VDrawArc.o \
	  VDrawPoly.o \
	  VDrawSeg.o \
	  VDrawStroke.o \
	  VFillPoly.o \
	  VDepthCue.o \
	  VDrawString.o \
	  VClipPoly.o \
	  VDotProd.o \
	  VCrossProd.o \
	  VRotate.o \
	  VTransPoly.o \
	  VWorldToScr.o \
	  VEyeToScr.o \
	  VTrans.o \
	  VIdentMat.o \
	  VMatMult.o \
	  VQuat.o \
	  VPrintPoly.o \
	  Vmalloc.o \
	  InitAWin.o \
	  CloseAWin.o \
	  FillPoly.o \
	  FillRect.o \
	  DrawLine.o \
	  DrawPoint.o \
	  FWinRedraw.o \
	  FrameCmpl.o \
	  ClipLine.o \
	  ETToSL.o \
	  MakeET.o \
	  SLDiff.o \
	  DrawSL.o \
	  Vdefs.o \
	  frameBuff.o \
	  $(EXTRAOBJS) \
	  $(LIBOBJS)

       DEFS = $(DEFINES)  -DSTDC_HEADERS=1 -DHAVE_STDLIB_H=1 -DHAVE_MALLOC_H=1 -DHAVE_UNISTD_H=1 -DSVR4=1 -DRETSIGTYPE=void -DHAVE_DIS=1 
     CFLAGS = $(DEFS) -g -O -I/usr/X11R6/include -DNETAUDIO -DHAVE_RAND $(STD_INCLUDES)

# Programs to make tags files.
  ETAGS = etags -tw
  CTAGS = ctags -tw
tagsrcs = $(SRCS)

all:	$(PROGRAMS)

libV.a:	$(OBJS)
	rm -f $@
	$(AR) $@ $(OBJS)
	$(RANLIB) $@

clean:
	rm -f $(PROGRAMS) *.o core *~ TAGS tags *.bak

mostlyclean:
	rm -f *.o core *~

TAGS:	$(tagsrcs)
	$(ETAGS) $(tagsrcs)

tags:   $(tagsrcs)
	$(CTAGS) $(tagsrcs)

distclean: clean
	rm -f Makefile

realclean:	distclean
	rm -f TAGS

install uninstall:

depend:
	$(MAKEDEPEND) $(DEFINES)  -DSTDC_HEADERS=1 -DHAVE_STDLIB_H=1 -DHAVE_MALLOC_H=1 -DHAVE_UNISTD_H=1 -DSVR4=1 -DRETSIGTYPE=void -DHAVE_DIS=1  $(STD_INCLUDES) $(SRCS)

test check:

# DO NOT DELETE THIS LINE -- make depend depends on it.
