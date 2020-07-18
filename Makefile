# Makefile for this assignment
server: stream.o token.o server.o
	gcc stream.o token.o server.o -o myftpd

stream.o: stream.h stream.c
	gcc -c stream.c

token.o: token.h token.c
	gcc -c token.c

server.o: server.c server.h
	gcc -c server.c

client: stream.o token.o client.o
	gcc stream.o token.o client.o -o myftp

client.o: client.c
	gcc -c client.c

clean:
	rm *.o myftpd myftp