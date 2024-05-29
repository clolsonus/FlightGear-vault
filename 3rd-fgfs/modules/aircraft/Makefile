#
# Makefile -- modules/aircraft
#

TARGET = flight.so

CC = gcc

OBJS = air.o damage.o gear.o interpolate.o inventory.o modelacm.o \
	planes.o transpose.o

VOBJS = ../Vlib/*.o

CFLAGS = -g -fPIC -I../acm -I../Vlib -I../../kernel -DHAVE_STDLIB_H -DHAVE_UNISTD_H -DFLIGHTDEBUG

LIBS = -lm

$(TARGET): $(OBJS) flight.o
	ld -shared -o $(TARGET) flight.o $(OBJS) $(VOBJS)

testacm: $(OBJS) testacm.o
	$(CC) -o testacm $(CFLAGS) testacm.o $(OBJS) $(VOBJS) $(LIBS)

air.o:  air.c

damage.o:  damage.c damage.h

gear.o:  gear.c

interpolate.o:  interpolate.c interpolate.h

inventory.o:  inventory.c

modelacm.o:  modelacm.c modelacm.h

planes.o:  planes.c

testacm.o:  testacm.c

transpose.o:  transpose.c

clean:
	rm -f $(TARGET) *.o *~
