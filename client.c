/* 
 * sample game code / logic for Defcon 25 Badge Game. 
 *
 * Implements one badge, using multicast to emulate the radio
 *
 * J. Adams <9/22/2016>
 *
 * NOTE!! This software uses multicast to mock the radio. IF you are
 * testing, and you have a default route, this traffic will end up on
 * your default route if your default route is set.
 * 
 * For debugging with the Internet up on your machine, do a 
 *   sudo route add 224.0.0/4 -interface lo0 
 * 
 * to force multicast to localhost. Test from there. 
 **/

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
#define BCAST_IP "224.0.0.8"

/* globals, ew.. */
int uptime = 0;
player_hist_rec players_seen[MAX_SEEN];
int players_seen_total = 0;  /* no players == -1 */

/* making these global as this is for testing only.*/
int send_fd, recv_fd;
struct sockaddr_in recv_addr;
struct sockaddr_in send_addr;

int randomint(int max) {
  return arc4random_uniform(max);
}

int four_d_six(void) {
 /* this seems stupid, for a computer. */
 int a[5];
 int tot = 0;
 int lowest = 4;
 a[4] = 6;

 for (int i=0; i<4; i++)  {
   a[i] = randomint(6);
   if (a[i] < a[lowest]) { lowest = i; };
 }

 for (int i=0; i<4; i++)  {
   if (i != lowest) { tot = tot + a[i]; } 
 }
 
 return tot;

}

int xp_for_next_level(player *p) { 
  /* n = n*(n-1)*500 */
  return ( p->level+1 ) * ((p->level+1)-1) * 500;
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
  printf("\nXP for next level: %d ", xp_for_next_level(p)); 
  printf("\n=======================================\n");
}

void init_player(player *p) {
  strcpy(p->name, player_fake_names[randomint(MAX_FAKE_NAMES)]);

  p->netid = randomint(65535);
  p->type = randomint(2); 
  p->gold = START_GOLD;
  p->level = 1;
  p->ac = 1;

  p->str = four_d_six();
  p->dex = four_d_six();

  p->won = 0;
  p->lost =0;

  p->hp = (p->level * LEVEL_HP_MULT);
  p->xp = START_XP;
}

char *serialize_player(player *p) { 
  static char data[255];

  sprintf(data,"%d:%d:%s:%d:%d:%d:%d:%d:%d:%d:%d:%d", 
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
  return(data);
};

player *deserialize_player(char *packed) { 
  /* deserialize inbound radio string into a player object */
  /* allocates one player object, it is up to the caller to free. */
  char buffer[255];
  char *token;

  player *newplayer = (player*)malloc(sizeof(player));
  bzero(newplayer,sizeof(player));

  int field=0;
  while ((token = strsep(&packed, ":"))) {

    switch (field) { 
      /* 0=command, 1=subcommand, 2=datatype 
       * here we assume datatype is 'p' for player data.
       */
    case 1:
      if (strstr(token, "_") == (char*)NULL) {
        /* the second field is used to store the target of this operation */
        /* note there can be only one player/target pair per op */
        newplayer->target = atoi(token);
      }
      break;
    case 2:
      newplayer->netid = atoi(token);
      break;
    case 3:
      newplayer->type = atoi(token);
      break;
    case 4:
      strcpy(newplayer->name,token);
      break;
    case 5:
      newplayer->hp = atoi(token);
      break;
    case 6:
      newplayer->xp = atoi(token);
      break;
    case 7:
      newplayer->gold =  atoi(token);
      break;
    case 8:
      newplayer->level = atoi(token);
      break;
    case 9:
      newplayer->str = atoi(token);
      break;
    case 10:
      newplayer->ac = atoi(token);
      break;
    case 11:
      newplayer->dex = atoi(token);
      break;
    case 12:
      newplayer->won = atoi(token);
      break;
    case 13:
      newplayer->lost = atoi(token);
      break;
    case 14:
      newplayer->lost = atoi(token);
      break;
    case 15:
      newplayer->damage = atoi(token);
      break;
    case 16:
      newplayer->is_crit = atoi(token);
      break;
    };
    field++;
  }

  return(newplayer);
}

void mark_player_seen(int uptime, player *p) {
  int seen_id = -1;
  for (int i=0; i < players_seen_total; i++) {
    if (players_seen[i].p.netid == p->netid) {
      players_seen[i].last_seen = uptime;
      seen_id = i;
    }
  }
  
  if (seen_id == -1) { 
    /* new dood */
    players_seen[players_seen_total].last_seen = uptime;
    memcpy(&players_seen[players_seen_total].p, p, sizeof(struct player_struct));
    players_seen_total++;
  }
}

void show_players_seen(void) { 
 if (players_seen_total == 0) {
   printf("\n\nNo players.\n\n");
 }
 for (int i = 0; i < players_seen_total; i++) {
         printf("%d. (%d) %s, Level %d %s, %d HP, %d XP (%d ago)\n", 
                i+1,
                players_seen[i].p.netid, 
                players_seen[i].p.name,
                players_seen[i].p.level,
                player_type_s[players_seen[i].p.type],
                players_seen[i].p.hp,
                players_seen[i].p.xp,
                uptime - players_seen[i].last_seen);
 }
}

int send_message(player *p, char *command, char *append) { 
  int sinlen = sizeof(struct sockaddr_in);

  char packet[1024];

  /* serialize the player structure */
  /* protocol: command:args:object type:object */
  sprintf(packet, "%s:%s", command, serialize_player(p));
  if (append) {
    strcat(packet, append);
  }
  return sendto(send_fd, &packet, sizeof(packet), 0, (struct sockaddr *) &send_addr, sinlen);

}

void display_hello() {
  printf("DC25 Caesar Game - Init\n\n");
}

void display_cmdprompt(int uptime, player *theplayer) {
  printf("[%d] %s %d> ", uptime, theplayer->name, theplayer->netid); 
  fflush(stdout);
}

/* timed events */
void do_beacon(player *theplayer) {
        if (send_message(theplayer, "BEACON:_", NULL) < 0) {
    perror("bootmsg");
  }
}

void do_autoheal(player *theplayer) { 
  int maxhp = (theplayer->level * LEVEL_HP_MULT);
  int healhp = (maxhp / 15);

  if (theplayer->hp < maxhp) { 
          theplayer->hp += healhp;
          if (theplayer->hp > maxhp) theplayer->hp = maxhp;
          
          printf("auto heal: +%d hp\n",healhp ); fflush(stdout); 
  }
}

/* requested events */
void do_attack(player *theplayer, player *victim) { 
  char attackmsg[40];
  sprintf(attackmsg,"ATTACK:%d", victim->netid);

  if (send_message(theplayer, attackmsg, NULL) < 0) {
    perror("attackmsg");
  }
}

void handle_ack(player *local, player *attacker) { 
  char ackmsg[40];
  if (attacker->target != local->netid) {
    /* not us !*/
    return;
  }
  printf("\n\nYou %s (%s%d) for %d damage. (%d hp remaining)\n", 
         attacker->is_crit ? "crit" : "hit", 
         attacker->name,
         attacker->netid, 
         attacker->damage,
         attacker->hp);
}

void handle_kill(player *local, player *attacker) { 
  char ackmsg[40];
  int gpgain = (attacker->level * 500);
  int xpgain = (attacker->level * 200);
  if (attacker->target != local->netid) {
    /* not us !*/
    return;
  }

  printf("\n\nYou kill %s%d!, gaining %d XP and %d gold.\n", 
         attacker->name,
         attacker->netid,
         xpgain,
         gpgain);

  local->gold += gpgain;
  local->xp += xpgain;
}

void handle_hit(player *local, player *attacker) { 
  char ackmsg[40];
  char dmgmsg[40];

  if (attacker->target != local->netid) {
    /* not for us */
    return;
  }

  printf("\n\nPlayer %s%d (%s) Attacks you!\n", 
         attacker->name, 
         attacker->netid, 
         player_type_s[attacker->type]);
    
    /* take damage */
    int roll = randomint(20);
    if (roll < local->ac) { 
      printf("%s%d Misses.", 
             attacker->name,
             attacker->netid);
      return;
    }
    
    int damage = (roll + attacker->str);
    int iscrit = 0;

    if (roll == 20) {
      damage *= 2;
    }
    
    local->hp = local->hp - damage;

    /* display it. */
    if (roll >= 16) {
      iscrit = 1;
      printf("%s%d crits you for %d\n", 
           attacker->name,
             attacker->netid, damage);
    } else {
      printf("%s%d hits you for %d\n", 
             attacker->name,
             attacker->netid, damage);
    }
    
    /* ack attack */
    sprintf(ackmsg,"AACK:%d", attacker->netid);
    sprintf(dmgmsg, ":%d:%d", iscrit, damage);
    if (send_message(local, ackmsg, dmgmsg) < 0) {
      perror("aack");
    }

    if (local->hp <= 0) {
            int xploss = (local->level * 200);
            int gploss = (local->level * 500);
            printf("You die. (-%d xp, -%d gold).\n", xploss, gploss);
            local->hp = 0;
            // TODO: adjust this
            local->xp = local->xp - xploss;
            local->gold = local->gold - gploss;
            sprintf(ackmsg,"KILL:%d", attacker->netid);
            if (send_message(local, ackmsg, NULL) < 0) {
                    perror("killack");
            }
    }

    fflush(stdout);
}

void handle_radio_message(char *message,player *localplayer) { 
//  printf("MSG: %s (%ld bytes)\n",message, strlen(message));

  if (strstr(message,"BEACON") == message) { 
    player *p = deserialize_player(message);
    if (p != NULL) { 
      if (p->netid != localplayer->netid) { 
        mark_player_seen(uptime, p);
      }
      free(p);
    }
  }

  if (strstr(message,"ATTACK") == message) { 
    player *attacker = deserialize_player(message);
    handle_hit(localplayer,attacker);
    free(attacker);
  }

  if (strstr(message,"AACK") == message) { 
    player *attacked = deserialize_player(message);
    handle_ack(localplayer,attacked);
    free(attacked);
  }

  if (strstr(message,"KILL") == message) { 
    player *attacked = deserialize_player(message);
    handle_kill(localplayer,attacked);
    free(attacked);
  }

}

int main(int argc, char *argv[])
{
  struct sockaddr_in peer_addr;

  int cnt, sent;
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
  socklen_t len;
  int attackidx;
  
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
  do_beacon(&theplayer);
  
  do {
    memcpy(&tempset, &readset, sizeof(tempset));
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    result = select(maxfd + 1, &tempset, NULL, NULL, &tv);

    if (result == 0) {
      uptime++;
      if ((uptime % BEACON_INTERVAL) == 0) {
        do_beacon(&theplayer);
      }

      if ((uptime % HEAL_INTERVAL) == 0) {
        do_autoheal(&theplayer);
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
          do_beacon(&theplayer);
          break;
        case 'a':
          attackidx = atoi(buffer + 2);
          if (attackidx == 0 || attackidx > players_seen_total) {
            printf("attack who?\n");
          } else { 
            do_attack(&theplayer, &players_seen[attackidx-1].p);
          }
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
                printf("\nHelp:      p       send a beacon now\n");
                printf("           c       make me caesar\n"); 
                printf("           i       show me/inventory\n"); 
                printf("           s       show players seen\n");
                printf("           a #     attack player #\n");
                printf("           e or q  quit\n\n");
        }
        
        FD_CLR(STDIN, &tempset);
        display_cmdprompt(uptime, &theplayer);
      }
      
      if (FD_ISSET(recv_fd, &tempset)) {
        socklen_t addr_len=sizeof(recv_addr);
        ssize_t count=recvfrom(recv_fd,buffer,sizeof(buffer),0,(struct sockaddr*)&recv_addr,&addr_len);
        buffer[count] = '\0';
        handle_radio_message(buffer, &theplayer);
        FD_CLR(recv_fd, &tempset);
      }
    }
  } while (1);
  
}
