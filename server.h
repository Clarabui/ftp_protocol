
#ifndef FTP_PROTOCOL_SERVER_H
#define FTP_PROTOCOL_SERVER_H

/**
 * @authors Tram, Navin
 * @file server.h
 * @details Server application header file
 * */

#define MAX_BLOCK_SIZE (1024*5)


char * cwd_userArg;
char * client_command;
char * server_command;
char * command_array[MAX_NUM_TOKENS];
unsigned long port;
FILE * logFile;

/*
 * Purpose: claims all zombie process; if any
 * */
void claim_children();
/*
 * Purpose: Creates a daemon process to be run as a server listening to connections
 *
 * Invokes a child process and exits parent process
 * sets child process to session leader and assign file mode creation mask
 * sets sigaction struct with appropriate values
 * */
int daemon_init();
/*
 * Purpose: trims input data
 *
 * This method adds '\0' when '\n' is found
 * */
void trim(char str[]);
/*
 * Purpose: to list process current directory
 *
 * */
void process_pwd(int sd);
/*
 * Purpose: to list all file listings in current directory
 * */
void process_dir(int sd);
/*
 * Purpose: to change current process directory to given path
 * */
void process_cd(char * path, int sd);
/*
 * Purpose: process client's request for downloading file with given filename
 *
 * This method reads  data from given filename and send file data to client over socket
 * */
void process_get(char * file_name, int sd);
/*
 * Purpose: process client's request for uploading file with given filename
 *
 * This method read data from socket send by client and write to file
 * */
void process_put(char * file_name, int sd);
/*
 * Purpose: Serve client commands such as cd, dir, pwd and get
 *
 * This method reads command input and splits the tokens using tokenise method,
 * passes these commands to appropriate methods that handles the request.
 * On each appropriate methods;
 * fork() is invoked to create child process
 * to handle request.
 *
 * Parent process waits for child to terminate
 *      therefore waits for next command
 *
 * */
void serve_a_client(int sd);



#endif //FTP_PROTOCOL_SERVER_H
