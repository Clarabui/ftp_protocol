cmake_minimum_required(VERSION 3.16)
project(ftp_protocol)

set(CMAKE_CXX_STANDARD 14)

add_executable(client.o client.c client.h)
add_executable(stream.o stream.h stream.c)
add_executable(server.o server.c server.h)
add_executable(token.o token.h token.c)