#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>

int main(int argc, char*argv[])
{
  const char *msg_str = "hello, world";
  const char *bcast_addr = argv[1];
  int port = atoi(argv[2]);
  struct sockaddr_in sock_in;
  int yes = 1;
  int sinlen = sizeof(struct sockaddr_in);
  memset(&sock_in, 0, sinlen);
  int sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if ( sock < 0 ) {
    printf("socket: %d %s\n", errno, strerror(errno));
    exit(-1);
  }

  sock_in.sin_addr.s_addr=inet_addr(bcast_addr);  
  sock_in.sin_port = htons(port);
  sock_in.sin_family = PF_INET;

  if ( bind(sock, (struct sockaddr *)&sock_in, sinlen) < 0 ) { 
    printf("bind: %s %d %s\n", bcast_addr, errno, strerror(errno));
    exit(-1);
  }

  if ( setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(int) ) < 0 ) { 
    printf("setsockopt: %d %s\n", errno, strerror(errno));
    exit(-1);
  }

  if  ( sendto(sock, msg_str, strlen(msg_str), 0, (struct sockaddr *)&sock_in, sinlen) < 0 ) {
    printf("sendto: %d %s str='%s', sinlen=%d\n", errno, strerror(errno), msg_str, sinlen);
    exit(-1);
  }
  printf("message sent!\n");
}
