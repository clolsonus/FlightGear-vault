#An ANSI C compiler: gnu's "gcc"
CC = gcc

# to pass options in to the compilation, say things like 'make CFLAGS=-g'

#The following flags must be defined for UNIX/X11 implementations
CPPFLAGS = -DX11 -DUNIX -I/usr/local/X11R5/include

#Very important!  The first flag (struct-return) makes it safe to link
#  gcc objects with cc objects.
GCCFLAGS = -fpcc-struct-return


.c.o:
	$(CC) $(GCCFLAGS) $(CFLAGS) $(CPPFLAGS) -c $*.c


OBJECTS = 		\
srgp_attrib.o		\
srgp_canvas.o		\
srgp_color_X.o		\
srgp_cursor.o		\
srgp_echo_X.o		\
srgp_error.o		\
srgp_font.o		\
srgp_input.o		\
srgp_input_X.o		\
srgp_inquire.o		\
srgp_marker.o		\
srgp_output.o		\
srgp_pattern.o		\
srgp_raster.o		\
srgp_state.o		\
srgp_state_X.o		\
geom.o

libsrgp.a: $(OBJECTS) 
	ar rv libsrgp.a $(OBJECTS)
	ranlib libsrgp.a

$(OBJECTS): srgplocal.h srgp_sphigs.h srgppublic.h srgp_errtypes.h macros.h
geom.o: geom.h
srgp_pattern.o: default_patterns.h
