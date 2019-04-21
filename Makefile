#Starting makefile for ush

CC = gcc
CFLAGS = -g -Wall
objects = ush.o expand.o builtin.o
depends = ush.c expand.c builtin.c

ush: $(objects)
	$(CC) $(CFLAGS) -o ush $(objects)

depends: defn.h

clean:
	rm -f ush $(objects)
