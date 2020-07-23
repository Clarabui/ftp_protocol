
#ifndef FTP_PROTOCOL_CLIENT_H
#define FTP_PROTOCOL_CLIENT_H

/**
 * @authors Tram, Navin
 * @file client.h
 * @details Client application header file
 *
 * */

#define MAX_BLOCK_SIZE (1024*5)

struct sockaddr_in ser_addr;
struct hostent *hp;
unsigned long port;
char *command_array[MAX_NUM_TOKENS];

/*
 * Purpose: trims input data
 *
 * This method adds '\0' when '\n' is found
 * */
void trim(char str[]);
/*
 * Purpose: process local command lpwd, display current directory on client
 * */
void process_lpwd();
/*
 * Purpose: process local command ldir, display file listings under current client directory
 * */
void process_ldir();
/*
 * Purpose: process local command lcd, change current directory to given path
 * */
void process_lcd(char * path);
/*
 * Purpose: process remote command get, request for download file from server
 *
 * This method sends filename to socket, read data send from server and write to open file with given name
 * */
void process_get(char * filename, int sd);
/*
 * Purpose: process remote command put, request for upload file to server
 *
 * This method sends filename to socket. Upon server acceptance, send data from open file to server
 * */
void process_put(char * filename, int sd);
/*
 * Purpose: process remote commands including pwd, dir, cd
 *
 * This method sends user command line to socket and display returned output from server
 * */
void process_remote_command(char full_command[], int sd);

#endif //FTP_PROTOCOL_CLIENT_H
