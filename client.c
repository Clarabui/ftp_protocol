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
#include  "stream.h"           /* MAX_BLOCK_SIZE, readn(), writen() */

#define   SERV_TCP_PORT  40005 /* default server listening port */

int main(int argc, char *argv[])
{
  int sd, n, nr, nw, i=0;
  char buf[MAX_BLOCK_SIZE], host[60];
  unsigned short port;
  struct sockaddr_in ser_addr;
  struct hostent *hp;

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

  while (1) {

    printf("\nClient Input Command: ");
    fgets(buf, sizeof(buf), stdin);
    nr = strlen(buf);

    if (buf[nr-1] == '\n'){
      buf[nr-1] = '\0';
      --nr;
    }


    if (strcmp(buf, "quit")==0) {
      printf("Bye from client\n");
      exit(0);
    }

    if (nr > 0) {
      if ((nw=write(sd, buf, nr)) < nr) {
        printf("Client: send error\n");
        exit(1);
      }

      if ((nr=read(sd, buf, sizeof(buf))) <= 0) {
        printf("Client: receive error\n");
        exit(1);
      }
      buf[nr] = '\0';
      printf("Server Output: %s\n", buf);
    }
  }
}
