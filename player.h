/* player.h */

/* should match # of sold badges, maybe. */
#define MAX_SEEN 300

typedef enum {GUARD, SENATOR, CAESAR} player_type;

/* defaults */
typedef struct player_struct {
  player_type type;
  char name[20];

  int in_combat;  //
  int lastcombat; // how long since combat started

  int target;  // some player operations have a target
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

  /* these fields are only used during attack-response */  
  int damage;
  int is_crit;
} player;

typedef struct player_history {
    int    last_seen;
    player p;  
} player_hist_rec;

const char *player_type_s[] = { "Guard",
                                "Senator",
                                "Caesar" };

#define MAX_FAKE_NAMES 288

const char *player_fake_names[] = {
        "Aelia",
        "Aeliana",
        "Aelianus",
        "Aelius",
        "Aemilia",
        "Aemiliana",
        "Aemilianus",
        "Aemilius",
        "Aetius",
        "Africanus",
        "Agrippa",
        "Agrippina",
        "Ahenobarbus",
        "Alba",
        "Albanus",
        "Albina",
        "Albinus",
        "Albus",
        "Antonia",
        "Antonina",
        "Antoninus",
        "Antonius",
        "Appius",
        "Aquila",
        "Aquilina",
        "Aquilinus",
        "Atilius",
        "Augusta",
        "Augustina",
        "Augustinus",
        "Augustus",
        "Aulus",
        "Aurelia",
        "Aureliana",
        "Aurelianus",
        "Aurelius",
        "Avilius",
        "Avitus",
        "Balbina",
        "Balbinus",
        "Balbus",
        "Blandina",
        "Blandinus",
        "Blandus",
        "Blasius",
        "Brutus",
        "Caecilia",
        "Caecilius",
        "Caelia",
        "Caelina",
        "Caelinus",
        "Caelius",
        "Caius",
        "Camilla",
        "Camillus",
        "Cassia",
        "Cassian",
        "Cassianus",
        "Cassius",
        "Cato",
        "Celsus",
        "Cicero",
        "Claudia",
        "Claudius",
        "Cloelia",
        "Cloelius",
        "Cnaeus",
        "Cornelia",
        "Cornelius",
        "Crispinus",
        "Crispus",
        "Cyprianus",
        "Decima",
        "Decimus",
        "Diocletianus",
        "Domitia",
        "Domitianus",
        "Domitilla",
        "Feminine",
        "Domitius",
        "Drusa",
        "Drusilla",
        "Drusus",
        "Duilius",
        "Egnatius",
        "Ennius",
        "Fabia",
        "Fabiana",
        "Fabianus",
        "Fabiola",
        "Fabius",
        "Fabricia",
        "Fabricius",
        "Fausta",
        "Faustina",
        "Faustinus",
        "Faustus",
        "Felix",
        "Festus",
        "Flavia",
        "Flaviana",
        "Flavianus",
        "Flavius",
        "Floriana",
        "Florianus",
        "Florus",
        "Fulvia",
        "Fulvius",
        "Gaius",
        "Gallus",
        "Germana",
        "Germanus",
        "Glaucia",
        "Gnaeus",
        "Gordianus",
        "Gratiana",
        "Gratianus",
        "Hadriana",
        "Hadrianus",
        "Herminia",
        "Herminius",
        "Hilaria",
        "Hilarius",
        "Horatia",
        "Horatius",
        "Hortensia",
        "Hortensius",
        "Ianuarius",
        "Iovianus",
        "Iovita",
        "Iulia",
        "Iuliana",
        "Iulianus",
        "Iulius",
        "Iunia",
        "Iunius",
        "Iuvenalis",
        "Januarius",
        "Jovian",
        "Julia",
        "Juliana",
        "Julius",
        "Junia",
        "Junius",
        "Laelia",
        "Laelius",
        "Laurentia",
        "Laurentina",
        "Laurentinus",
        "Laurentius",
        "Livia",
        "Liviana",
        "Livianus",
        "Livius",
        "Longina",
        "Longinus",
        "Lucanus",
        "Lucia",
        "Luciana",
        "Lucianus",
        "Lucilia",
        "Lucilius",
        "Lucilla",
        "Lucius",
        "Lucretia",
        "Lucretius",
        "Manius",
        "Manlius",
        "Marcella",
        "Marcellina",
        "Marcellinus",
        "Marcellus",
        "Marcia",
        "Marciana",
        "Marcianus",
        "Marcius",
        "Marcus",
        "Mariana",
        "Marianus",
        "Marina",
        "Marinus",
        "Marius",
        "Martialis",
        "Martina",
        "Martinus",
        "Maxentius",
        "Maxima",
        "Maximianus",
        "Maximiliana",
        "Maximilianus",
        "Maximinus",
        "Maximus",
        "Naevius",
        "Nero",
        "Nerva",
        "Nona",
        "Nonus",
        "Octavia",
        "Octavianus",
        "Octavius",
        "Otho",
        "Ovidius",
        "Paula",
        "Paulina",
        "Paulinus",
        "Paulus",
        "Petronia",
        "Petronius",
        "Plinius",
        "Pompeius",
        "Pompilius",
        "Pomponia",
        "Pomponius",
        "Pontius",
        "Porcia",
        "Porcius",
        "Prisca",
        "Priscilla",
        "Priscus",
        "Publius",
        "Quintilianus",
        "Quintillus",
        "Quintina",
        "Quintinus",
        "Quintus",
        "Regulus",
        "Rufina",
        "Rufinus",
        "Rufus",
        "Sabina",
        "Sabinus",
        "Saturnina",
        "Saturninus",
        "Scaevola",
        "Secundinus",
        "Secundus",
        "Seneca",
        "Septima",
        "Septimius",
        "Septimus",
        "Sergius",
        "Servius",
        "Severianus",
        "Severina",
        "Severinus",
        "Severus",
        "Sextilius",
        "Sextus",
        "Silvanus",
        "Spurius",
        "Tacita",
        "Tacitus",
        "Tarquinius",
        "Tatiana",
        "Tatianus",
        "Tatius",
        "Terentius",
        "Tertius",
        "Thracius",
        "Tiberius",
        "Tiburtius",
        "Titiana",
        "Titianus",
        "Titus",
        "Traianus",
        "Tullia",
        "Tullius",
        "Valens",
        "Valentina",
        "Valentinianus",
        "Valentinus",
        "Valeria",
        "Valeriana",
        "Valerianus",
        "Valerius",
        "Varinia",
        "Varinius",
        "Varius",
        "Vergilius",
        "Verginia",
        "Verginius",
        "Vespasianus",
        "Vibiana",
        "Vibianus",
        "Vibius",
        "Vinicius",
        "Virginia",
        "Vita",
        "Vitus"};
