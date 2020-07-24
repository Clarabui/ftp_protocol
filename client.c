/**
 * @authors Tram, Navin
 * @file client.c
 * @details Client application source file
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
#include <netdb.h>
/* and INADDR_ANY */
#include  <unistd.h>
#include  <sys/stat.h>
#include  <fcntl.h>
#include  <errno.h>
#include  "stream.h"     /* MAX_BLOCK_SIZE, readn(), writen() */
#include  "token.h"
#include  "nbyte_converter.h"
#include  "file_protocol_error_code.h"
#include  "client.h"

#define   SERV_TCP_PORT  40005 /* default server listening port */

void process_lpwd(){
    int pid;

    if((pid =fork()) == 0){

        execl("/bin/pwd", "pwd", (char *)0);
        printf("the execl call failed\n");
        exit(1);

    }else if (pid < 0){
        printf("Fork failed\n");
        exit(1);
    }
}

void process_ldir(){
    int pid;

    if((pid = fork()) == 0){

        execl("/bin/ls", "ls", (char *)0);
        printf("the execl call failed\n");
        exit(1);

    }else if (pid < 0){
        printf("Fork failed\n");
        exit(1);
    }
}

void process_lcd(char * path){
    // if chdir() doesn't return 0
    // process exits with error
    if (chdir(path) != 0){
        printf("Change directory unsuccessfully\n");
    } else {
        printf("Change directory successfully\n");
    }
}

void process_get(char * filename, int sd){

    /* Receive Network Byte Order int from server and convert it to file size */
    int nbytes, file_size_or_error_code, file_size, error_code, nr2;
    char buf2[20];

    read(sd, &nbytes, sizeof(nbytes));
    file_size_or_error_code = convert_from_NBO(nbytes);

    if (file_size_or_error_code < 0) {
        error_code = file_size_or_error_code;
        display_error(error_code);
        return;
    } else if (file_size_or_error_code == 0){
        printf("Empty file\n");
        return;
    } else {
        file_size = file_size_or_error_code;
    }

    /* Check if nbytes < 0, print error code. Else, print file size*/
    printf("File size is %d\n", file_size);

    printf("Do you want to download file? Enter Y/N: ");
    fgets(buf2, sizeof(buf2), stdin);
    nr2 = strlen(buf2);
    trim(buf2);

    write(sd, buf2, nr2);

    if(strcmp(buf2, "Y") == 0 || strcmp(buf2, "y") == 0){
        /* Downloading file
         * Create file if it does not exist
         */
        int fd, nr3, nw3, total_bytes;

        char f_download[100];       /*construct download filename with prefix get_filenam*/
        strcpy(f_download, "get_");
        strcat(f_download, filename);

        fd = open(f_download, O_WRONLY|O_CREAT, 0766); //open file if exist, create file with permission 766 if not exist
        char buf3[MAX_BLOCK_SIZE];
        total_bytes = 0;

        while(1){
            /* Read data from socket
             * If total bytes read equals to file size, exit while loop
             */
            if ((nr3 = readn(sd, buf3, sizeof(buf3))) <= 0){
                printf("client: read error\n");
                exit(1);
            }
            total_bytes += nr3;
            if(total_bytes == file_size){
                printf("Finish downloading\n");
                write(fd, buf3, nr3);
                file_size = 0;
                break;
            }
            /*Write data to open file*/
            if (( nw3 = write(fd, buf3, nr3)) < nr3){
                printf("Failed to write to file\n");
                close(fd); //close file if cannot write to file
                exit(1);
            }
        }
        close(fd);
    }
}

void process_put(char * filename, int sd){
    int fd, nbytes, file_size, error_code = 0;
    struct stat f_info;
    char buf[MAX_BLOCK_SIZE], msg[200];

    lstat(filename, &f_info);
    file_size = f_info.st_size;

    nbytes = convert_to_NBO(file_size);
    write(sd, &nbytes, sizeof(nbytes)); //send file size to server

    if (file_size == 0){
        printf("Empty file\n");
        return;
    }

    read(sd, &nbytes, sizeof(nbytes));
    error_code = convert_from_NBO(nbytes);
    if (error_code < 0){
        display_error(error_code);
        return;
    } else{ //error_code received = 0, no error
        printf("Server said: ready for uploading\n");
    }

    printf("\nStart uploading file\n");
    int nr, nw;
    fd = open(filename, O_RDONLY);

    while (1){
        if ((nr = read(fd, buf, sizeof(buf))) <= 0 ){
            printf("Reach EOF\n");
            break;
        }

        if ((nw = writen(sd, buf, nr)) < nr){
            printf("Failed to write to socket\n");
            exit(1);
        }
    }

    read(sd, msg, sizeof(msg));
    printf("%s\n", msg);
}

void process_remote_command(char full_command[], int sd) {
    int nw, nr, full_command_len = strlen(full_command);
    char output[MAX_BLOCK_SIZE];

    printf("Sending to socket: %s\n", full_command);

    if ((nw = write(sd, full_command, full_command_len)) < full_command_len) {
        printf("Client: send error\n");
        exit(1);
    }

    if ((nr = read(sd, output, sizeof(output))) <= 0) {
        printf("Client: receive error\n");
        exit(1);
    }

    output[nr] = '\0';
    printf("---------Server Output-----------\n%s\n", output);
}

void trim(char str[]) {
    size_t len = strlen(str);
    if((len > 0) && (str[len-1] == '\n')){
        str[--len] = '\0';
    }
}


int main(int argc, char *argv[]) {
    int sd, n, nr, nw, error_code = 0, i = 0;
    char buf[MAX_BLOCK_SIZE],
    input[MAX_BLOCK_SIZE],
    buf1[MAX_BLOCK_SIZE],
    buf2[MAX_BLOCK_SIZE],
    host[60];

    /* get server host name and port number */
    if (argc==1) {  /* assume server running on the local host and on default port */
        gethostname(host, sizeof(host));

        port = SERV_TCP_PORT;
    } else if (argc == 2) { /* use the given host name */
        strcpy(host, argv[1]);
        port = SERV_TCP_PORT;
    } else if (argc == 3) { // use given host and port for server
        strcpy(host, argv[1]);
        int n = atoi(argv[2]);
        if (n >= 1024 && n < 65536)
            port = n;
        else {
            printf("Error: server port number must be between 1024 and 65535\n");
            exit(1);
        }
    } else {
        printf("Usage: %s [ <server host name> [ <server listening port> ] ]\n", argv[0]);
        exit(1);
    }

    /* get host address, & build a server socket address */
    bzero((char *) &ser_addr, sizeof(ser_addr));
    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(port);

    if ((hp = gethostbyname(host)) == NULL){
        printf("host not found; ERROR\n");
        exit(1);
    }

    ser_addr.sin_addr.s_addr = * (u_long *) hp->h_addr;

    /* create TCP socket & connect socket to server address */
    sd = socket(PF_INET, SOCK_STREAM, 0);
    if (connect(sd, (struct sockaddr *) &ser_addr, sizeof(ser_addr))<0) {
        printf("client failed to connect");
        exit(1);
    }

    printf("VERSION 1.1 CLIENT\n");

    while (1) {

        printf("\nClient Input Command: ");
        fgets(buf, sizeof(buf), stdin);
        nr = strlen(buf);
        memcpy(input, buf, sizeof(buf));
        trim(buf);

        /* parse command line into command array*/
        tokenise(input, command_array);
        char * client_command;
        char * argument;
        client_command = command_array[0];
        argument = command_array[1];
        printf("COMMAND: %s\n", client_command);
        printf("ARGUMENT: %s\n", argument);

        if (strcmp(client_command, "quit")==0) {
            printf("Bye from client\n");
            exit(0);
        }

        /* process client command given*/
        if (strcmp(client_command, "lpwd") == 0) {
            process_lpwd();
        } else if(strcmp(client_command, "ldir") == 0) {
            process_ldir();
        } else if(strcmp(client_command, "lcd") == 0) {
            process_lcd(argument);
        } else if (strcmp(client_command, "dir") == 0 ||
                strcmp(client_command, "pwd") == 0 ||
                strcmp(client_command, "cd") == 0) {
            process_remote_command(buf, sd);
        } else if(strcmp(client_command, "get") == 0) {
            printf("Sending to socket: %s\n", buf);

            if ((nw=write(sd, buf, nr)) < nr) {
                printf("Client: send error\n");
                exit(1);
            }

            process_get(argument, sd);
        } else if (strcmp(client_command, "put") == 0) {
            char * filename = argument;
            struct stat f_info;

            if (lstat(filename, &f_info) < 0) {
                error_code = -1;
            } else if (!(f_info.st_mode & S_IRUSR) || !(f_info.st_mode & S_IRGRP) || !(f_info.st_mode & S_IROTH)){
                error_code = -2;
            }

            /* Check file exist & file read permission
             * Display errors if any
             * Else, send user command line to socket
             */

            if (error_code < 0) {
                display_error(error_code);
                printf("Cannot upload file\n");
                error_code =0; //reset value of error code to 0 for the next read
            } else {
                printf("Sending to socket: %s\n", buf);

                if ((nw=write(sd, buf, nr)) < nr) {
                    printf("Client: send error\n");
                    exit(1);
                }

                process_put(argument, sd);
            }
        } else {
            printf("Undefined command\n");
        }

        wait(NULL); //wait for child process to terminate. Get input from client again
    }
}
