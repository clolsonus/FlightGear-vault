#
# Makefile -- modules/aircraft
#

TARGET = fghud.so

CC = gcc

OBJS = hud.o imath.o scale.o

# VOBJS = ../Vlib/*.o

# CFLAGS = -g -fPIC -I../acm -I../Vlib -I../../kernel -DHAVE_STDLIB_H -DHAVE_UNISTD_H -DFLIGHTDEBUG -DDEBUG
CFLAGS = -g -fPIC -I../../kernel -I../acm -I../aircraft -I../sound -I../Vlib

# LIBS = -lm

$(TARGET): $(OBJS) fghud.o
	ld -shared -o $(TARGET) fghud.o $(OBJS) $(VOBJS)

fghud.o:  fghud.c fghud.h 

hud.o:  hud.c

imath.o:  imath.c imath.h

scale.o:  scale.c scale.h

clean:
	rm -f $(TARGET) *.o *~
