/* 
 * sample game code / logic
 * for Defcon 25 Badge Game. Implements one client which uses multicast to emulate the radio
 *
 * J. Adams <9/22/2016>
 *
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "player.h"
#include "game_constants.h"

#define STDIN 0
#define STDOUT 1
#define HELLO_PORT 12345
#define HELLO_GROUP "224.0.0.1"
#define FALSE 0 
#define TRUE 1
#define MAX_BUFFER_SIZE 1024

#if defined(__APPLE__) || defined(__MACH__)
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL SO_NOSIGPIPE
#endif
#endif

int randomint(int max) {
  return arc4random();
}

void display_player(player *p) { 
  /* TODO - change this into something more useful / screen based */
  printf("\n=======================================\n");
  printf("Player %s\n\n", p->name);
  printf("hp: %d  ",p->hp);
  printf("xp: %d  ",p->xp);
  printf("gold: %d  ",p->gold);
  printf("level: %d  ",p->level);
  printf("str: %d  ",p->str);
  printf("ac: %d  ",p->ac);
  printf("dex: %d  ",p->dex);
  printf("won: %d  ",p->won);
  printf("lost: %d  ",p->lost);
   
  printf("\n=======================================\n");
}

void init_player(player *p) {
  p->name = strdup("Hacker");

  p->netid = randomint(65535);
  p->type = randomint(3);
  p->gold = START_GOLD;
  p->hp = START_HP;
  p->xp = START_XP;
}

void send_message(int fd, struct sockaddr * addr, char * message) {
  if (sendto(fd,message,sizeof(message),0, addr, sizeof(addr)) < 0) {
    perror("sendto");
    exit(1);
  }
}

void display_hello() {
  printf("DC25 Caesear Game - Init\n\n");
}

void display_cmdprompt(uptime) {
  printf("[%d] Cmd> ", uptime); fflush(stdout);
}

void do_beacon() {
  printf("\n\n==beacon==\n\n");
}

int main(int argc, char *argv[])
{
  struct sockaddr_in addr;
  int send_fd, recv_fd, cnt, sent;
  struct ip_mreq mreq;

  u_int yes = 1;

  /* create what looks like an ordinary UDP socket */
  if ((send_fd=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
    perror("socket");
    exit(1);
  }

  if ((recv_fd=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
    perror("socket");
    exit(1);
  }

  /* set up sending addr and recv addr */
  memset(&addr,0,sizeof(addr));
  addr.sin_family=AF_INET;
  addr.sin_addr.s_addr=inet_addr(HELLO_GROUP);
  addr.sin_port=htons(HELLO_PORT);

  /* allow multiple sockets to use the same PORT number */
  if (setsockopt(recv_fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0) {
    perror("Reusing ADDR failed");
    exit(1);
  }

  /* bind to receive address */
  if (bind(recv_fd,(struct sockaddr *) &addr,sizeof(addr)) < 0) {
    perror("bind");
    exit(1);
  }

  /* main loop */
  int maxfd, result, peersock, i, j;
  int uptime = 0;
  socklen_t len;
  
  fd_set readset, tempset;
  struct timeval tv;
  char buffer[MAX_BUFFER_SIZE+1];
  
  FD_ZERO(&readset);
  FD_SET(recv_fd, &readset);
  FD_SET(STDIN, &readset);
  
  maxfd = recv_fd;

  player theplayer;

  display_hello();
  init_player(&theplayer);
  display_player(&theplayer);
  // ask_for_name();
  display_cmdprompt(uptime);

  do {
    memcpy(&tempset, &readset, sizeof(tempset));
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    result = select(maxfd + 1, &tempset, NULL, NULL, &tv);

    if (result == 0) {
      uptime++;
      if ((uptime % BEACON_INTERVAL) == 0) {
	do_beacon();
	display_cmdprompt(uptime);
      }
    }
    else if (result < 0 && errno != EINTR) {
      printf("Error in select(): %s\n", strerror(errno));
    }
    else if (result > 0) {
      /* we have data */
      if (FD_ISSET(STDIN, &tempset)){
	int bytes_read = read(STDIN, buffer, MAX_BUFFER_SIZE);
	buffer[bytes_read] = '\0';
	buffer[bytes_read-1] = '\0'; // nuke the \n

	/* quick'n'dirty cmd processing */
	if (buffer[0] == 'p') {
	  printf("PING\n");
	  send_message(send_fd, (struct sockaddr *) &addr, "P");
	}
	if (buffer[0] == 'e' || buffer[0] == 'q') {
	  return(0);
	}
	
	if (buffer[0] == 'h') {
	  printf("     Help: (p)ing (m)sg (e)xit/(q)uit\n");
	}
	
	FD_CLR(STDIN, &tempset);
	display_cmdprompt(uptime);
      }

      if (FD_ISSET(recv_fd, &tempset)) {
	len = sizeof(addr);
	peersock = accept(recv_fd, (struct sockaddr *)&addr, &len);
	if (peersock < 0) {
	  printf("Error in accept(): %s\n", strerror(errno));
	}
	else {
	  FD_SET(peersock, &readset);
	  maxfd = (maxfd < peersock)?peersock:maxfd;
	}
	FD_CLR(recv_fd, &tempset);
      }

      for (j=0; j<maxfd+1; j++) {
	if (FD_ISSET(j, &tempset)) {
	  do {
	    result = recv(j, buffer, MAX_BUFFER_SIZE, 0);
	  } while (result == -1 && errno == EINTR);

	  if (result > 0) {
	    buffer[result] = 0;
	    printf("Echoing: %s\n", buffer);
	    sent = 0;

	    do {
	      result = send(j, buffer+sent, result-sent, MSG_NOSIGNAL);
	      if (result > 0)
		sent += result;
	      else if (result < 0 && errno != EINTR)
		;
	      break;
	    } while (result > sent);

	  }
	  else if (result == 0) {
	    close(j);
	    FD_CLR(j, &readset);
	  }
	  else {
	    printf("Error in recv(): %s\n", strerror(errno));
	  }
	}      // end if (FD_ISSET(j, &tempset))
      }      // end for (j=0;...)
    }      // end else if (result > 0)
  } while (1);
  
}
