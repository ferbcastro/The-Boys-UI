CFLAGS = -Wall -Wextra -g # flags de compilacao
LDFLAGS = -lm -lncurses

CC = gcc 
     
all: theboysUI

theboysUI: theboysUI.o conjunto.o  lef.o fila.o movement.o
	$(CC) -o theboysUI theboysUI.o conjunto.o lef.o fila.o movement.o $(LDFLAGS)

conjunto.o: conjunto.c
	$(CC) -c $(CFLAGS) conjunto.c

lef.o: lef.c
	$(CC) -c $(CFLAGS) lef.c

fila.o: fila.c
	$(CC) -c $(CFLAGS) fila.c

movement.o: movement.c
	$(CC) -c $(CFLAGS) movement.c

theboysUI.o: theboysUI.c
	$(CC) -c $(CFLAGS) theboysUI.c

clean:
	rm -f *.o theboysUI 


