
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
extern int errno;


/* */
void trim(char input[]);
/* */
void process_lpwd();
/* */
void process_ldir();
/* */
void process_lcd(char * path);
/* */
void process_get(char * filename, int sd);

#endif //FTP_PROTOCOL_CLIENT_H
