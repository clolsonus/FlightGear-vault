#An ANSI C compiler: gnu's "gcc"
CC = gcc

#Depends on some srgp files
#You will have to change this macro to fit your setup
SRGPDIR = ../srgp


# to pass options in to the compilation, say things like 'make CFLAGS=-g'

#The UNIX flag must be defined for UNIX implementations
CPPFLAGS = -DUNIX -I$(SRGPDIR)

#Very important!  The first flag (struct-return) makes it safe to link
#  gcc objects with cc objects.
GCCFLAGS = -fpcc-struct-return


.c.o:
	$(CC) $(GCCFLAGS) $(CFLAGS) $(CPPFLAGS) -c $*.c

HFILES = 			\
sph_draw.h			\
elementType.h			\
sph_errtypes.h			\
sph_face.h			\
falloc.h			\
fallocdefs.h			\
mat3.h				\
mat3defs.h			\
mat3err.h			\
sphigs.h			\
sphigslocal.h			\
$(SRGPDIR)/srgp_sphigs.h	\
$(SRGPDIR)/srgppublic.h

OBJECTS = 		\
MAT3geom.o		\
MAT3mat.o		\
MAT3vec.o		\
MAT3inv.o		\
sph_cull.o		\
sph_objdebug.o		\
sph_attrib.o		\
sph_bitstr.o		\
sph_traverse.o		\
sph_draw.o		\
sph_edit.o		\
sph_element.o		\
sph_error.o		\
sph_filter.o		\
sph_input.o		\
sph_modxform.o		\
sph_object.o		\
sph_poly.o		\
sph_post.o		\
sph_refresh.o		\
sph_state.o		\
sph_utility.o		\
sph_view.o		\
sph_falloc.o		\
sph_clip.o		\
sph_pdc.o		\
sph_canon.o		\
sph_zsort.o		\
sph_intense.o		\
sph_optimize.o


libsphigs.a: $(OBJECTS) 
	ar rv libsphigs.a $(OBJECTS)
	ranlib libsphigs.a

$(OBJECTS): $(HFILES)
