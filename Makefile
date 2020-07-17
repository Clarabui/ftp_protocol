# Makefile for this assignment
server: stream.o token.o server.o
	gcc stream.o token.o server.o -o ser

stream.o: stream.h stream.c
	gcc -c stream.c

token.o: token.h token.c
	gcc -c token.c

server.o: server.c 
	gcc -c server.c

client: stream.o client.o
	gcc stream.o client.o -o cli

client.o: client.c
	gcc -c client.c

clean:
	rm *.o ser cli