# Makefile for this assignment
server: stream.o parser.o server.o
	gcc stream.o parser.o server.o -o ser

stream.o: stream.h stream.c
	gcc -c stream.c

parser.o: parser.h parser.c
	gcc -c parser.c

server.o: server.c
	gcc -c server.c

client: stream.o client.o
	gcc stream.o client.o -o cli

client.o: client.c
	gcc -c client.c

clean:
	rm *.o ser cli