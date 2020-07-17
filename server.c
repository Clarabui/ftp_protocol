/*
 *  ser6.c  - 	(Topic 11, HX 22/5/1995)
 *		An improved version of "ser5.c". This version handles the message boundaries
 *              which are not preserved by the TCP. Each message transmitted between the
 *              client and the server is preceeded by a two byte value which is the length
 *              of the message. The handling of the message length is done in routines readn
 *              and writen.
 *  revised:	22/05/1996
 *  revised:	18/10/2006
 */

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
    exit(0);                  /* parent goes bye-bye */
  }

  /* child continues */
  setsid();                      /* become session leader */
  //chdir(cwd);                  /* attach daemon process to current server directory, comment out the change working directory */
  umask(0);                      /* clear file mode creation mask */

  /* catch SIGCHLD to remove zombies from system */
  act.sa_handler = claim_children; /* use reliable signal */
  sigemptyset(&act.sa_mask);       /* not to block other signals */
  act.sa_flags   = SA_NOCLDSTOP;   /* not catch stopped children */
  sigaction(SIGCHLD,(struct sigaction *)&act,(struct sigaction *)0);
  /* note: a less than perfect method is to use
     signal(SIGCHLD, claim_children);
     */

  return(0);
}

void process_pwd(char * server_command, int sd){

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


void process_dir(char * server_command, int sd){

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
    printf("the execl call failed.\n");

    exit(1);
  }else if ( pid < 0){
    printf("Fork failed\n");
    exit(1);
  }
}

void process_get(char * file_name, int sd){
  int fd;
  struct stat f_info;
  char * msg;

  if( (fd = open(file_name, O_RDONLY)) == -1 ){
    printf("File does not exist\n");
    exit(1);
  }

  if ( lstat(file_name, &f_info) < 0 ){
    printf("lstat failed\n");
    exit(1);
  }
  
  msg = "File does exist and ready to download. Do you want to continue?(Y/N)";
  write(sd, msg, strlen(msg));

}

void serve_a_client(int sd)
{
  int nr, nw;
  char buf[MAX_BLOCK_SIZE];
  char *command_array[MAX_NUM_TOKENS];
  int tk_num;

  while (1){
    /* read data from client */
    if ((nr = read(sd, buf, sizeof(buf))) <= 0){
      printf("Connection is broken\n");
      exit(1);
    }
    buf[nr] = '\0';

    /*parse command line from client into server command array */
      tk_num = tokenise(buf, command_array);

      if(tk_num != -1){
        printf("Number of argument: %d\n", tk_num);
        for(int i = 0; i< tk_num; i++){
          printf("Argument %d: %s\n", i + 1, command_array[i]);
        }
      }else{
        printf("Command array size is too small\n");
      }

      /*map client_command to server_command*/
      char * client_command;
      char * server_command;
      char * file_name;

      client_command = command_array[0];
      file_name = command_array[1];

      if( strcmp(client_command, "pwd") == 0 ){
        server_command = "pwd";
        process_pwd(server_command, sd);
      }else if( strcmp(client_command, "dir") == 0){
        server_command = "ls";
        process_dir(server_command, sd);
      }else if( strcmp(client_command, "cd") == 0){
        //to-do
        exit(0);
      }else if( strcmp(client_command, "get") == 0){
        process_get(file_name, sd);
      }else if( strcmp(client_command, "put") == 0){
        //to-do
        exit(0);
      }else{
        printf("Undefined command\n");
        exit(0);
      }

      printf("Waiting for next command....\n\n");
      wait(NULL); //wait until child process terminated, the parent continue to read next command
    }
  }

  int main(int argc, char *argv[])
  {
    int sd, nsd, n;
    extern int errno;
    pid_t pid;
    FILE * log;
    char * logfilename = "Logfile";

    unsigned short port;   // server listening port
    socklen_t cli_addrlen;
    struct sockaddr_in ser_addr, cli_addr;
    /* get the port number */
    if (argc == 1) {
      port = SERV_TCP_PORT;
    } else if (argc == 2) {
      int n = atoi(argv[1]);
      if (n >= 1024 && n < 65536)
        port = n;
      else {
        printf("Error: port number must be between 1024 and 65535\n");
        exit(1);
      }
    } else {
      printf("Usage: %s [ server listening port ]\n", argv[0]);
      exit(1);
    }
    /* create log file */
    log = fopen(logfilename, "w");
    if (log == NULL){
      printf("ERROR: Can't create a log file\n");
      exit(3);
    }

    /* turn the program into a daemon */
    if (daemon_init() == -1){
      printf("ERROR: Can't become a daemon\n");
      exit(4);
    }

    fprintf(log, "Server pid = %d\n", getpid());
    fflush(log);

    /* set up listening socket sd */
    if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
      fprintf(log, "server:socket");
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
      fprintf(log,"server bind");
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
        fprintf(log,"server:accept"); 
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

      /*refirect output of child prgram output to socket*/
      serve_a_client(nsd);
      exit(0);
    }
  }
