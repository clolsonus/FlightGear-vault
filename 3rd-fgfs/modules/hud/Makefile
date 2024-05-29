#
# Makefile -- modules/aircraft
#

TARGET = hud.o

CC = gcc

OBJS = hud.o

# VOBJS = ../Vlib/*.o

# CFLAGS = -g -fPIC -I../acm -I../Vlib -I../../kernel -DHAVE_STDLIB_H -DHAVE_UNISTD_H -DFLIGHTDEBUG -DDEBUG
CFLAGS = -g -fPIC -I../acm -I../aircraft -I../Vlib -DHAVE_VIEWER

# LIBS = -lm

#$(TARGET): $(OBJS) flight.o
#	ld -shared -o $(TARGET) flight.o $(OBJS) $(VOBJS)

hud.o:  hud.c

clean:
	rm -f $(TARGET) *.o *~
