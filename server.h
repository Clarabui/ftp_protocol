
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
extern int errno;
unsigned long port;
FILE * logFile;

/*
 * Purpose: claims all zombie process; if any
 * */
void claim_children();
/*
 * Purpose: trims input data
 *
 * This method adds '\0' when '\n' is found
 * */
void trim(char input[]);
/*
 * Purpose: Creates a daemon process to be run as a server listening to connections
 *
 * Invokes a child process and exits parent process
 * sets child process to session leader and assign file mode creation mask
 * sets sigaction struct with appropriate values
 * */
int daemon_init();
/*
 * Purpose: to list process current directory
 *
 * */
void process_pwd(int sd);
/*
 * Purpose: to list all contents in current directory
 * */
void process_dir(int sd);
/*
 * Purpose: to change directory with given path
 * */
void process_chdir(char * path, int sd);
/*
 * Purpose:
 * */
int convert_to_NBO(int n, int nn);
/*
 * Purpose:
 *  Download file given from path, from the process
 *  current directory in server and save it in current
 *  directory in client...
 * */
void process_get(char * file_name, int sd);



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
