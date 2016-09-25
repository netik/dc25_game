/* 
 * sample game code / logic for Defcon 25 Badge Game. 
 *
 * Implements one badge, using multicast to emulate the radio
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
#include <math.h>

#include "util.h"
#include "player.h"
#include "game_constants.h"

#define BCAST_PORT 12345

/* 
 * NOTE!! This software uses multicast to mock the radio. IF you are
 * testing, and you have a default route, this traffic will end up on
 * your default route if your default route is set.
 * 
 * For debugging with the Internet up on your machine, do a 
 *   sudo route add 224.0.0/4 -interface lo0 
 * 
 * ... and everything will work.
 **/

#define BCAST_IP "224.0.0.8"

int randomint(int max) {
  return arc4random_uniform(max);
}

void display_player(player *p) { 
  /* TODO - change this into something more useful / screen based */
  printf("\n=======================================\n");
  printf("Player %s (netid: %d), a %s\n\n", p->name, p->netid, player_type_s[p->type]);
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
  strcpy(p->name, player_fake_names[randomint(MAX_FAKE_NAMES)]);
  p->netid = randomint(65535);
  p->type = randomint(2); 
  p->gold = START_GOLD;
  p->hp = START_HP;
  p->xp = START_XP;
}

player *deserialize_player(char *packed) { 
  /* deserialize a string into a player object */
  char buffer[255];
  char *token;

  player *newplayer = (player*)malloc(sizeof(player));

  int field=0;
  while ((token = strsep(&packed, ":"))) {
    printf("%d: %s\n", field, token);
    field++;
  }

  return(newplayer);
}

void mark_player_seen(player *p) {

}

void show_players_seen(void) { 
 if (seen_players == 0) {
   printf("\n\nNo players.\n\n");
 }
 for (int i = 0; i < seen_players; i++) {
         printf("%d. (%d) %s", 
                i,
                players_seen[i].p.netid, 
                players_seen[i].p.name );
 }
}

int send_message(player *p, int fd, struct sockaddr *addr, char * message) { 
  int sinlen = sizeof(struct sockaddr_in);

  char packet[1024];

  /* serialize the player structure */
  /* protocol: command:args:object type:object */
  sprintf(packet, "%s:_:p:%d:%d:%s:%d:%d:%d:%d:%d:%d:%d:%d:%d", message,
          p->netid,
          p->type,
          p->name,
          p->hp,
          p->xp,
          p->gold,
          p->level,
          p->str,
          p->ac,
          p->dex,
          p->won,
          p->lost
          );

  return sendto(fd, &packet, sizeof(packet), 0, addr, sinlen);
}

void display_hello() {
  printf("DC25 Caesar Game - Init\n\n");
}

void display_cmdprompt(int uptime, player *theplayer) {
  printf("[t=%d] id=%d Cmd? ", uptime, theplayer->netid); fflush(stdout);
}

void do_beacon(int send_fd, struct sockaddr *send_addr, player *theplayer) {
  if (send_message(theplayer, send_fd, send_addr, "BEACON") < 0) {
    perror("bootmsg");
  }
}

void handle_radio_message(char *message) { 
  printf("%s\n",message);

  if (strstr(message,"BEACON") == message) { 
    player *p = deserialize_player(message);
    if (p != NULL) { 
      mark_player_seen(p);
    }
  }
}

int main(int argc, char *argv[])
{
  struct sockaddr_in recv_addr;
  struct sockaddr_in send_addr;
  struct sockaddr_in peer_addr;

  int send_fd, recv_fd, cnt, sent;
  u_int yes = 1;
  int sinlen = sizeof(struct sockaddr_in);

  /* create what looks like an ordinary UDP socket */
  if ((send_fd=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
    perror("socket");
    exit(1);
  }

  if ((recv_fd=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
    perror("socket");
    exit(1);
  }

  /* set up recv addr */
  memset(&recv_addr,0,sinlen);
  recv_addr.sin_family=AF_INET;
  recv_addr.sin_addr.s_addr=INADDR_ANY;
  recv_addr.sin_port=htons(BCAST_PORT);
  recv_addr.sin_family=PF_INET;

  /* set up sending addr */
  memset(&send_addr, 0, sinlen);
  send_addr.sin_addr.s_addr=inet_addr(BCAST_IP);
  send_addr.sin_port=htons(BCAST_PORT);
  send_addr.sin_family=PF_INET;



  /* allow multiple sockets to use the same address number */
  if (setsockopt(send_fd,IPPROTO_IP,IP_MULTICAST_LOOP,&yes,sizeof(yes)) < 0) {
    perror("set SO_BROADCAST failed");
    exit(1);
  }

  if (setsockopt(recv_fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes)) < 0) {
    perror("Reusing ADDR failed");
    exit(1);
  }

  /* set up multicast, maybe? */
  /* use setsockopt() to request that the kernel join a multicast group */

#if defined(__APPLE__) || defined(__MACH__)
  if (setsockopt(recv_fd,SOL_SOCKET,SO_REUSEPORT,&yes,sizeof(yes)) < 0) {
    perror("Reusing PORT failed");
    exit(1);
  }
#endif

  /* bind to receive address */
  if (bind(recv_fd,(struct sockaddr *) &recv_addr,sizeof(recv_addr)) < 0) {
    perror("bind");
    exit(1);
  } else { 
    printf("bind ok.");
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr=inet_addr(BCAST_IP);
    mreq.imr_interface.s_addr=inet_addr("127.0.0.1");

    /* jna: does this fix the problem with losing multicast when main interface is up? */
    setsockopt(recv_fd,IPPROTO_IP,IP_MULTICAST_IF,&mreq.imr_interface,sizeof(struct in_addr));

    if (setsockopt(recv_fd,IPPROTO_IP,IP_ADD_MEMBERSHIP,(char*)&mreq,sizeof(mreq)) < 0) {
      perror("setsockopt - multicaat failed ");
      exit(1);
    }
  }

  /* main loop */
  int maxfd = 0;
  int result, peersock, i, j;
  int uptime = 0;
  socklen_t len;
  
  fd_set readset, tempset;
  struct timeval tv;
  char buffer[MAX_BUFFER_SIZE+1];
  
  FD_ZERO(&readset);
  FD_SET(recv_fd, &readset);
  FD_SET(STDIN, &readset);

  maxfd = MAX(maxfd,STDIN);
  maxfd = MAX(recv_fd,STDIN);

  player theplayer;

  display_hello();
  init_player(&theplayer);
  // at this point we'd ask them for their name. Instead, randomize!
  // ask_for_name();
  display_player(&theplayer);
  display_cmdprompt(uptime, &theplayer);

  // we are born. beacon.
  do_beacon(send_fd, (struct sockaddr *) &send_addr, &theplayer);

  do {
    memcpy(&tempset, &readset, sizeof(tempset));
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    result = select(maxfd + 1, &tempset, NULL, NULL, &tv);

    if (result == 0) {
      uptime++;
      if ((uptime % BEACON_INTERVAL) == 0) {
        do_beacon(send_fd, (struct sockaddr *) &send_addr, &theplayer);
      }
    }
    else if (result < 0 && errno != EINTR) {
      printf("Error in select(): %s\n", strerror(errno));
    }
    else if (result > 0) {
      /* we have data */
      if (FD_ISSET(STDIN, &tempset)){
        memset(buffer, 0, MAX_BUFFER_SIZE);
	int bytes_read = read(STDIN, buffer, MAX_BUFFER_SIZE);
	buffer[bytes_read] = '\0';
	buffer[bytes_read-1] = '\0'; // nuke the \n from stdin

	/* quick'n'dirty cmd processing, first letter only. oh well. */
        switch(buffer[0]) { 
        case 'p':
                do_beacon(send_fd, (struct sockaddr *) &send_addr, &theplayer);
                break;
        case 'e':
                return(0);
                break;
        case 's':
                show_players_seen();
                break;
        case 'i':
                display_player(&theplayer);
                break;
        default:
                printf("     Help: (p)ing (m)sg (i)show (s)een (e)xit/(q)uit\n");
        }
	
	FD_CLR(STDIN, &tempset);
	display_cmdprompt(uptime, &theplayer);
      }

      if (FD_ISSET(recv_fd, &tempset)) {
        socklen_t addr_len=sizeof(recv_addr);
        ssize_t count=recvfrom(recv_fd,buffer,sizeof(buffer),0,(struct sockaddr*)&recv_addr,&addr_len);
	buffer[count] = '\0';
        handle_radio_message(buffer);
	FD_CLR(recv_fd, &tempset);
      }
    }
  } while (1);
  
}
