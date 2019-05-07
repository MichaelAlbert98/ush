#Starting makefile for ush

CC = gcc
CFLAGS = -g -Wall
objects = ush.o expand.o builtin.o strmode.o

ush: $(objects)
	$(CC) $(CFLAGS) -o $@ $(objects)

ush.o expand.o builtin.o: defn.h globals.h

clean:
	rm -f ush $(objects)
