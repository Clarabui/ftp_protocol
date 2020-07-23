# Makefile for this assignment
server: stream.o token.o server.o nbyte_converter.o file_protocol_error_code.o
	gcc stream.o token.o server.o nbyte_converter.o file_protocol_error_code.o -o myftpd

server.o: server.c server.h
	gcc -c server.c

stream.o: stream.h stream.c
	gcc -c stream.c

token.o: token.h token.c
	gcc -c token.c

nbyte_converter.o: nbyte_converter.h nbyte_converter.c
	gcc -c nbyte_converter.c

file_protocol_error_code.o: file_protocol_error_code.h file_protocol_error_code.c
	gcc -c file_protocol_error_code.c

client: stream.o token.o client.o nbyte_converter.o file_protocol_error_code.o 
	gcc stream.o token.o client.o nbyte_converter.o file_protocol_error_code.o -o myftp

client.o: client.c client.h
	gcc -c client.c

clean:
	rm *.o myftpd myftp