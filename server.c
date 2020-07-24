/**
 * @authors Tram, Navin
 * @file server.c
 * @details Server application source file
 *
 * */


#include  <stdlib.h>     /* strlen(), strcmp() etc */
#include  <stdio.h>      /* printf()  */
#include  <string.h>     /* strlen(), strcmp() etc */
#include  <errno.h>      /* extern int errno, EINTR, perror() */
#include  <signal.h>     /* SIGCHLD, sigaction() */
#include  <syslog.h>
#include  <sys/types.h>  /* pid_t, u_long, u_short */
#include  <sys/socket.h> /* struct sockaddr, socket(), etc */
#include  <sys/wait.h>   /* waitpid(), WNOHAND */
#include  <netinet/in.h> /* struct sockaddr_in, htons(), htonl(), */
/* and INADDR_ANY */
#include  <unistd.h>
#include  <sys/stat.h>
#include  <fcntl.h>
#include  <errno.h>
#include  "stream.h"     /* MAX_BLOCK_SIZE, readn(), writen() */
#include  "token.h"
#include  "nbyte_converter.h"
#include  "file_protocol_error_code.h"
#include "server.h"

#define   SERV_TCP_PORT   40005   /* default server listening port */

void claim_children()
{
    pid_t pid=1;
    while (pid>0) { /* claim as many zombies as we can */
        pid = waitpid(0, (int *)0, WNOHANG);
    }
}

int daemon_init()
{
    pid_t   pid;
    struct sigaction act;

    if ( (pid = fork()) < 0) {
        printf("fork failed\n");
        exit(1);
    } else if (pid > 0) {
        printf("Daemon PID =  %d\n", pid);
        exit(0); // parent bye-bye
    }

    /* child continues */
    setsid(); /* become session leader */
    umask(0); /* clear file mode creation mask */

    /* catch SIGCHLD to remove zombies from system */
    act.sa_handler = claim_children; /* use reliable signal */
    sigemptyset(&act.sa_mask);       /* not to block other signals */
    act.sa_flags= SA_NOCLDSTOP;   /* not catch stopped children */
    sigaction(SIGCHLD,(struct sigaction *)&act,(struct sigaction *)0);


    return(0);
}

void trim(char str[]) {
    size_t len = strlen(str);
    if((len > 0) && (str[len-1] == '\n')){
        str[--len] = '\0';
    }
}


void process_pwd(int sd){

    /*create child process to execute pwd received
     *redirect standard output, standard error to socket
     */
    int pid;

    if((pid = fork()) == 0){

        dup2(sd, STDOUT_FILENO);
        dup2(sd, STDERR_FILENO);

        char arg[256] = "/bin/";
        strcat(arg, server_command);

        execl(arg, server_command, (char *)0);
        printf("the execl call failed.\n");

        exit(1);
    }else if ( pid < 0){
        printf("fork failed\n");
        exit(1);
    }
}


void process_dir(int sd){

    /*create child process to execute dir command received
     *redirect standard output, standard error to socket
     */
    int pid;

    if((pid = fork()) == 0){

        dup2(sd, STDOUT_FILENO);
        dup2(sd, STDERR_FILENO);

        char arg[256] = "/bin/";
        strcat(arg, server_command);

        execl(arg, server_command, (char *)0);
        printf("the execl call failed\n");

        exit(1);
    }else if ( pid < 0){
        printf("Fork failed\n");
        exit(1);
    }
}

void process_cd(char * path, int sd) {
    char * msg;
    if( chdir(path) !=0){
        msg = "Unsuccessfully change directory";
    }else{
        msg = "Successfully change directory";
    }
    write(sd, msg, strlen(msg)); //send message to client if change directory succeeds or not succeeds
}

void process_get(char * file_name, int sd){
    int fd, nbytes, file_size, error_code = 0;
    struct stat f_info;
    char msg2[MAX_BLOCK_SIZE];
    char buf[MAX_BLOCK_SIZE];

    if ( lstat(file_name, &f_info) < 0 ) {
        error_code = -1; //file does not exist
    } else if(!(f_info.st_mode & S_IRUSR) || !(f_info.st_mode & S_IRGRP) || !(f_info.st_mode & S_IROTH)){
        error_code = -2; //no read permissions
    }

    /* Convert file size OR error code  to Network Byte Order and send to client*/

    if (error_code < 0){
        nbytes = convert_to_NBO(error_code);
        write(sd, &nbytes, sizeof(error_code));
        display_error(error_code);
        return;
    } else {
        file_size = f_info.st_size;
        nbytes = convert_to_NBO(file_size);
        write(sd, &nbytes, sizeof(nbytes));
        if (file_size == 0){
            printf("Empty file\n");
            return;
        }
    }

    /* Read client confirmation to download file (Y/N)*/
    read(sd, msg2, sizeof(msg2));
    printf("Client confirm to download file? '%s'", msg2);

    /* Client confirm to download file with Y or y
     * Read file from open file descriptor fd to buf and write data from buf to socket
     * Close fd after finish
     */
    if( strcmp(msg2, "Y") == 0 || strcmp(msg2, "y") == 0){
        printf("\nStart sending file\n");
        int nr, nw;
        fd = open(file_name, O_RDONLY);

        while (1){
            if ((nr = read(fd, buf, sizeof(buf))) <= 0 ){
                printf("Reach EOF\n");
                break;
            }

            if ((nw = writen(sd, buf, nr)) < nr){
                printf("send error\n");
                exit(1);
            }
        }

        close(fd);
    } else {
        printf("\nDont send file\n");
    }
}

void process_put(char * file_name, int sd){
    char *msg;
    int nbytes, file_size, error_code;
    read(sd, &nbytes, sizeof(nbytes));
    file_size = convert_from_NBO(nbytes);

    if(file_size == 0){
        printf("Empty file\n");
        return;
    }

    printf("File size is %d\n", file_size);


    int fd, nr3, nw3, total_bytes;

    char f_upload[100];       /*construct download filename with prefix put_filenam*/
    strcpy(f_upload, "put_");
    strcat(f_upload, file_name);

    if ((fd = open(f_upload, O_WRONLY|O_CREAT, 0766)) < 0){
        error_code = -3; //cannot create or open given file name
        display_error(error_code);
        nbytes = convert_to_NBO(error_code);
        write(sd, &nbytes, sizeof(nbytes));
        return;
    }else{
        error_code = 0; //return code for successfully create the file (not error actually)
        nbytes = convert_to_NBO(error_code);
        write(sd, &nbytes, sizeof(nbytes));
    }

    char buf3[MAX_BLOCK_SIZE];
    total_bytes = 0;

    while(1){
        /* Read file from socket
         * If total bytes read equals to file size, exit while loop
         */
        if ((nr3 = readn(sd, buf3, sizeof(buf3))) <= 0){
            printf("server: read error\n");
            exit(1);
        }
        total_bytes += nr3;
        if(total_bytes == file_size){
            write(fd, buf3, nr3);
            msg = "Finish uploading";
            printf("%s\n", msg);
            write(sd, msg, strlen(msg)); //send to client to inform uploading finishes
            file_size = 0;
            break;
        }

        if (( nw3 = write(fd, buf3, nr3)) < nr3){
            printf("Failed to write to file\n");
            close(fd); //close file if cannot write to file
            exit(1);
        }
    }
    close(fd);
}

void serve_a_client(int sd)
{
    int nr;
    char buf[MAX_BLOCK_SIZE];
    char *command_array[MAX_NUM_TOKENS];
    int tk_num;

    while (1){
        /* read command line from client */
        if ((nr = read(sd, buf, sizeof(buf))) <= 0){
            printf("Connection is broken\n");
            exit(1);
        }
        buf[nr] = '\0';

        /*parse command line from client into server command array */
        tk_num = tokenise(buf, command_array);

        if(tk_num != -1){
            printf("\nNumber of argument: %d\n", tk_num);
            for(int i = 0; i< tk_num; i++){
                printf("Argument %d: %s\n", i + 1, command_array[i]);
            }
        }else{
            printf("Command array size is too small\n");
        }

        /* map client_command to server_command */
        char * file_name, *path;

        client_command = command_array[0];
        file_name = command_array[1];
        path = command_array[1];

        if( strcmp(client_command, "pwd") == 0 ){
            server_command = "pwd";
            process_pwd(sd);
        }else if( strcmp(client_command, "dir") == 0){
            server_command = "ls";
            process_dir(sd);
        }else if( strcmp(client_command, "cd") == 0){
            process_cd(path, sd);
        }else if( strcmp(client_command, "get") == 0){
            process_get(file_name, sd);
        }else if( strcmp(client_command, "put") == 0){
            process_put(file_name, sd);
        }else{
            printf("Undefined command\n");
            exit(0);
        }

        printf("Waiting for next command....\n\n");
        wait(NULL); //wait until child process terminated, parent process  continue to read next command
    }
}

int main(int argc, char *argv[])
{
    int sd, nsd;
    pid_t pid;
    char * logfilename = "serverLog";

    socklen_t cli_addrlen;
    struct sockaddr_in ser_addr, cli_addr;
    /* get the port number */
    port = SERV_TCP_PORT;
    if (port >= 1024 && port < 65536) {
        printf("Usage: %lu [ server listening port ]\n", port);
    } else {
        printf("Error: port number must be between 1024 and 65535\n");
        exit(1);
    }

    /* create log file */
    logFile = fopen(logfilename, "w");
    if (logFile == NULL){
        printf("ERROR: Can't create a log file\n");
        exit(3);
    }

    if ( argv[1] != NULL ) {
        strcpy(cwd_userArg, argv[1]);
        if (chdir(cwd_userArg) < 0) {
            fprintf(logFile, "Can't set to directory, ERROR\n");
        }
        printf("Arg passed is: %s\n", cwd_userArg);
    }

    /* turn the program into a daemon */
    if (daemon_init() == -1){
        printf("ERROR: Can't become a daemon\n");
        exit(4);
    }

    fprintf(logFile, "Server pid = %d\n", getpid());
    fflush(logFile);

    /* set up listening socket sd */
    if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(logFile, "setup for listening socket ERROR");
        exit(1);
    }

    /* build server Internet socket address */
    bzero((char *)&ser_addr, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(port);
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    /* note: accept client request sent to any one of the
       network interface(s) on this host.
       */

    /* bind server address to socket sd */
    if (bind(sd, (struct sockaddr *) &ser_addr, sizeof(ser_addr))<0){
        fprintf(logFile,"server bind ERROR");
        exit(1);
    }

    /* become a listening socket */
    listen(sd, 5);

    while (1) {
        /* wait to accept a client request for connection */
        cli_addrlen = sizeof(cli_addr);
        nsd = accept(sd, (struct sockaddr *) &cli_addr, &cli_addrlen);
        if (nsd < 0) {
            if (errno == EINTR)   /* if interrupted by SIGCHLD */
                continue;
            fprintf(logFile,"server:accept");
            exit(1);
        }

        /* create a child process to handle this client */
        if ((pid=fork()) <0) {
            printf("fork failed");
            exit(1);
        } else if (pid > 0) {
            close(nsd);
            continue; /* parent to wait for next client */
        }

        /* now in child, serve the current client */
        close(sd);

        /*redirect output of child program output to socket*/
        serve_a_client(nsd);
        exit(0);
    }
}
