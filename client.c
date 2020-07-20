/* cli6.c - 	(Topic 11, HX 22/5/1995)
 *		An improved version of "cli5.c". Since TCP does not preserve the message
 *              boundaries, each message is preceeded by a two byte value which is the
 *              length of the message.
 * Revised;	06/11/2007
 */

#include  <stdlib.h>
#include  <unistd.h>
#include  <stdio.h>
#include  <sys/types.h>
#include  <sys/socket.h>
#include  <netinet/in.h>       /* struct sockaddr_in, htons, htonl */
#include  <netdb.h>            /* struct hostent, gethostbyname() */
#include  <string.h>
#include  <errno.h>
#include  <fcntl.h>
#include  "token.h"
#include  "stream.h"           /* MAX_BLOCK_SIZE, readn(), writen() */

#define   SERV_TCP_PORT  40005 /* default server listening port */

void trim(char input[]) {
  size_t len = strlen(input);
  if((len > 0) && (input[len-1] == '\n')){
    input[--len] = '\0';
  }
}

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
  int pid;

  if((pid = fork()) == 0){
      printf("Whats your path: %s", path);
    if (chdir(path) != 0){
      printf("chdir failed\n");
      exit(1);
    }
  }else if (pid < 0){
    printf("Fork failed\n");
    exit(1);
  }
}

void process_get(char * filename, int sd){

  /* Receive Network Byte Order int from server and convert it to file size */
  int nbytes, file_size, nr2;
  char buf2[20];

  read(sd, &nbytes, sizeof(nbytes));
  if(sizeof(file_size) == 2){
    file_size = ntohs(nbytes);
  }else{
    file_size = ntohl(nbytes);
  }

  printf("File size is %d\n", file_size);

  printf("Do you want to download file? Enter Y/N: ");
  fgets(buf2, sizeof(buf2), stdin);
  nr2 = strlen(buf2);
  trim(buf2);

  write(sd, buf2, nr2);

  if(strcmp(buf2, "Y") == 0){
    /* Downloading file
     * Create file if it does not exist
     */
    int fd, nr3, nw3, total_bytes;
    fd = open("foo", O_WRONLY|O_CREAT, 0766); //replcae = filename later
    char buf3[MAX_BLOCK_SIZE];
    nr3 = 0;

    while(1){
      /* Read file from socket
       * If total bytes read equals to file size, exit while loop
       */
      nr3 = readn(sd, buf3, sizeof(buf3));
      total_bytes += nr3;
      if(total_bytes == file_size){
        printf("Finish downloading\n");
        break;
      }

      printf("Read nr = %d\n", nr3);

      if (( nw3 = write(fd, buf3, nr3)) < 0){
        printf("Failed to write to file\n");
        exit(1);
      }
    }
  }
}

int main(int argc, char *argv[])
{
  int sd, n, nr, nw, i=0;
  extern int errno;
  char buf[MAX_BLOCK_SIZE], buf1[MAX_BLOCK_SIZE], buf2[MAX_BLOCK_SIZE], host[60];
  unsigned short port;
  struct sockaddr_in ser_addr;
  struct hostent *hp;
  char *command_array[MAX_NUM_TOKENS];

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
    printf("host %s not found\n", host);
    exit(1);
  }

  ser_addr.sin_addr.s_addr = * (u_long *) hp->h_addr;

  /* create TCP socket & connect socket to server address */
  sd = socket(PF_INET, SOCK_STREAM, 0);
  if (connect(sd, (struct sockaddr *) &ser_addr, sizeof(ser_addr))<0) {
    printf("client failed to connect");
    exit(1);
  }

  printf("VERSION 1.1\n");
  while (1) {

    printf("\nClient Input Command: ");
    fgets(buf, sizeof(buf), stdin);
    nr = strlen(buf);
    trim(buf);

    /* parse command line into command array*/
    tokenise(buf, command_array);
    char * client_command;
    char * argument;
    client_command = command_array[0];
    argument = command_array[1];
    printf("COMMAND: %s\n", client_command);

    if (strcmp(client_command, "quit")==0) {
      printf("Bye from client\n");
      exit(0);
    }

    /*handle local client command*/
    if (strcmp(client_command, "lpwd") == 0){
      process_lpwd();
    }else if(strcmp(client_command, "ldir") == 0){
      process_ldir();
    }else if(strcmp(client_command, "lcd") == 0) {

      process_lcd(argument); //new path is passed as argument in command line
    }else{

      /* handle command sent to server*/
      if (nr > 0) {
        if ((nw=write(sd, buf, nr)) < nr) {
          printf("Client: send error\n");
          exit(1);
        }

        /* implement get method */
        if(strcmp(client_command, "get") == 0){
          process_get(argument, sd);
        }else if(strcmp(client_command, "put") == 0){
          //TO DO
        }else if (strcmp(client_command, "cd") == 0){
          //TO DO
        }else if (strcmp(client_command, "pwd") == 0 || strcmp(client_command, "dir") == 0){
          if ((nr=read(sd, buf1, sizeof(buf))) <= 0) {
            printf("Client: receive error\n");
            exit(1);
          }
          buf1[nr] = '\0';
          printf("---------Server Output-----------\n%s\n", buf1);
        }else{
          printf("Undefined command\n");
        }
      }
    }
    wait(NULL); //wait for child process to terminate. Get input from client again
  }
}

