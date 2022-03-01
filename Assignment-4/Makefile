CC=gcc
CFLAGS=-Wall

user: user1 user2

user1: user1.o librsocket.a
	$(CC) $(CFLAGS) -o user1 user1.o -L. -lrsocket -lpthread

user1.o: user1.c
	$(CC) $(CFLAGS) -c user1.c

user2: user2.o librsocket.a
	$(CC) $(CFLAGS) -o user2 user2.o -L. -lrsocket -lpthread

user2.o: user2.c
	$(CC) $(CFLAGS) -c user2.c

lib: librsocket.a

librsocket.a: rsocket.o
	ar -rcs librsocket.a rsocket.o

rsocket.o: rsocket.c rsocket.h
	$(CC) $(CFLAGS) -c rsocket.c -lpthread

clean:
	rm -f user1 user1.o user2 user2.o librsocket.a rsocket.o