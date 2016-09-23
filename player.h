/* player.h */
typedef enum {GUARD, SENATOR, CAESAR} player_type;

/* defaults */
typedef struct player_struct {
  player_type type;
  char *name;
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

char *player_type_s[] = { "Guard",
			  "Senator",
			  "Caesar" };


