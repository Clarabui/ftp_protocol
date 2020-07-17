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

#include  "stream.h"     /* MAX_BLOCK_SIZE, readn(), writen() */
#include  "parser.h"


#define   SERV_TCP_PORT   40005   /* default server listening port */

void claim_children()
{
  pid_t pid=1;
  while (pid>0) { /* claim as many zombies as we can */
    pid = waitpid(0, (int *)0, WNOHANG);
  }
}


void daemon_init()
{
  pid_t   pid;
  struct sigaction act;

  if ( (pid = fork()) < 0) {
    printf("fork");
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
}

void process_pwd(char * server_command, int sd){

  /*create child process to execute command received
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


void process_dir(char * server_command, int sd){

  /*create child process to execute command received
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
  exit(0);
}

void serve_a_client(int sd)
{
  int nr, nw;
  int lc =0;
  char buf[MAX_BLOCK_SIZE];
  char *cmd;
  command **cl;


  while (1){
    /* read data from client */
    if ((nr = read(sd, buf, sizeof(buf))) <= 0){
      printf("Connection is broken\n");
      exit(1);
    }
    buf[nr] = '\0';

    /*parse command line from client
     *print out the struct of received command
     */
    cmd = buf;
    cl = process_cmd_line(cmd, 1);

    while (cl[lc] != NULL) {
      dump_structure(cl[lc], lc);
      print_human_readable(cl[lc], lc);
      lc++;
    }
    lc = 0; //reset lc for the next read

    /*map client_command to server_command*/
    char * client_command;
    char * server_command;
    char * file_name;
    client_command = cl[0]->com_name;

    printf("Filename: %c\n", file_name);
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

    clean_up(cl);
    printf("Waiting for next command....\n\n");
    wait(NULL); //wait until child process terminated, the parent continue to read next command
  }
}

int main(int argc, char *argv[])
{
  printf("SERVER VERSION 1.0\n");
  int sd, nsd, n;
  pid_t pid;
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

  /* turn the program into a daemon */
  daemon_init();

  /* set up listening socket sd */
  if ((sd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
    printf("server:socket");
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
    printf("server bind");
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
      printf("server:accept"); exit(1);
    }

    /* create a child process to handle this client */
    if ((pid=fork()) <0) {
      printf("fork");
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
