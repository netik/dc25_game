/* player.h */

/* should match # of sold badges, maybe. */
#define MAX_SEEN 300

typedef enum {GUARD, SENATOR, CAESAR} player_type;

/* defaults */
typedef struct player_struct {
  player_type type;
  char name[20];
  int netid;
  /* stats, dunno if we will use */
  int hp;
  int xp;
  int gold;
  int level;
  int str;
  int ac;
  int dex;
  int won;
  int lost;
} player;

typedef struct player_history {
    int    last_seen;
    player p;  
} player_hist_rec;

player_hist_rec players_seen[MAX_SEEN];
int seen_players = 0;

const char *player_type_s[] = { "Guard",
                                "Senator",
                                "Caesar" };

#define MAX_FAKE_NAMES 8
const char *player_fake_names[] = { "Julius",
                                    "Augustus",
                                    "Dalia",
                                    "Mark",
                                    "Anthony",
                                    "Cleopatra",
                                    "Pullo",
                                    "Titus" };




                                    

