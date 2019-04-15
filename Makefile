#Starting makefile for ush

CC = gcc
CFLAGS = -g -Wall
objects = ush.o ush2.o

ush: $(objects)
	$(CC) $(CFLAGS) -o ush $(objects)

clean:
	rm -f ush $(objects)
