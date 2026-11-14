#ifndef LABYRINTHE_H
#define LABYRINTHE_H

#ifdef _WIN32
    #include <direct.h> // Pour _mkdir() sur Windows
    #define CREATE_DIR(path) _mkdir(path)
#else
    #include <sys/stat.h> // Pour mkdir() sur Linux/macOS
    #define CREATE_DIR(path) mkdir(path, 0777)
#endif

#define NOMBRE_SCORE 10

/* Dossier où sont rangés les fichiers de labyrinthe. */
#define DOSSIER_LABYRINTHES "labyrinthe"
/* Taille des buffers de chemin ("labyrinthe/<nom>"), '\0' compris. */
#define TAILLE_CHEMIN 128

/* Caractères du labyrinthe. */
#define MOTIF_MUR '#'
#define MOTIF_VIDE ' '
#define MOTIF_ENTREE 'o'
#define MOTIF_SORTIE '-'
#define MOTIF_JOUEUR '*'
#define MOTIF_KEY '$'
#define MOTIF_TRAP '@'
#define MOTIF_BOOST '&'
#define MOTIF_OGRE 'G'
#define MOTIF_FANTOME 'F'

/* Modes de jeu (enregistrés dans le fichier du labyrinthe).
 * Les valeurs 1 et 2 sont celles des anciens fichiers : on ne les change pas. */
#define MODE_FACILE 1
#define MODE_DIFFICILE 2
#define MODE_NORMAL 3

typedef struct {
    int **cellule_matrix;
    int nbr_cellules_horizontales;
    int nbr_cellules_verticales;  
} Cellule;

typedef struct {
    char *name;
    int score;
} Top_Player;

typedef struct {
    char **cases;
    int hauteur;
    int largeur;
    int mode;      /* MODE_FACILE, MODE_NORMAL ou MODE_DIFFICILE */
    Top_Player top_scores[NOMBRE_SCORE];
} Grille;


/**
 * @brief Alloue dynamiquement la matrice de cellules pour le labyrinthe.
 * * Cette fonction configure les dimensions logiques et alloue la mémoire pour le tableau de cellules utilisé dans la génération du labyrinthe.
 * @param cellules Pointeur vers la structure Cellule à allouer.
 * @param nbr_cellules_verticales Nombre de lignes de cellules (basé sur la hauteur du labyrinthe).
 * @param nbr_cellules_horizontales Nombre de colonnes de cellules (basé sur la largeur du labyrinthe).
 * @note En cas d'échec d'allocation, le programme se termine (exit(EXIT_FAILURE)).
 */
void allouer_cellules(Cellule *cellules, int nbr_cellules_verticales, int nbr_cellules_horizontales);

/**
 * @brief Libère la mémoire allouée dynamiquement pour la matrice de cellules.
 * @param cellules Pointeur vers la structure Cellule à libérer.
 */
void liberer_cellules(Cellule *cellules);

/**
 * @brief Alloue dynamiquement la grille (matrice de caractères) du labyrinthe.
 * Cette fonction configure les dimensions réelles (cases), alloue la mémoire pour la grille affichable du labyrinthe
 * et initialise un classement vide (noms à NULL, scores à 0) et le mode (facile).
 * @param grille Pointeur vers la structure Grille à allouer.
 * @param hauteur_labyrinthe Hauteur totale de la grille (lignes).
 * @param largeur_labyrinthe Largeur totale de la grille (colonnes).
 * @note En cas d'échec d'allocation, le programme se termine (exit(EXIT_FAILURE)).
 */
void allouer_grille(Grille *grille, int hauteur_labyrinthe, int largeur_labyrinthe);

/**
 * @brief Libère la mémoire allouée dynamiquement pour la grille du labyrinthe.
 * @param grille Pointeur vers la structure Grille à libérer.
 */
void liberer_grille(Grille *grille);

/**
 * @brief Initialise les identifiants de groupe des cellules.
 * * Chaque cellule reçoit un identifiant de groupe unique (index linéaire), essentiel pour l'algorithme de génération de labyrinthe (e.g., Kruskal ou Union-Find).
 * @param cellules Pointeur vers la structure Cellule à initialiser.
 */
void initialiser_cellules(Cellule *cellules);

/**
 * @brief Initialise la grille du labyrinthe avec des murs et des passages.
 * * La grille est remplie de murs ('#'). Les positions correspondant aux centres des cellules sont marquées comme des passages (' ').
 * @param grille Pointeur vers la Grille à initialiser.
 * @param cellules Pointeur vers la structure Cellule (pour déterminer les centres).
 */
void initialiser_grille(Grille *grille, Cellule *cellules);

/**
 * @brief Affiche le labyrinthe case par case sur la console, en couleur.
 * @param grille Pointeur vers la structure Grille à afficher.
 */
void afficher_labyrinthe(Grille *grille);

/**
 * @brief Affiche une case du labyrinthe avec sa couleur.
 * @param c Caractère à afficher.
 */
void afficher_case(char c);

/**
 * @brief Exécute une étape de construction du labyrinthe (algorithme de fusion).
 * * Cette fonction sélectionne un mur aléatoire entre deux cellules. S'il sépare 
 * deux groupes de cellules différents, le mur est retiré et les deux groupes sont fusionnés.
 * * @param cellules Pointeur vers la structure Cellule (pour la gestion des groupes).
 * @param grille Pointeur vers la Grille (pour l'ouverture du mur).
 */
void construire_labyrinthe(Cellule *cellules, Grille *grille);

/**
 * @brief Vérifie si le labyrinthe est entièrement construit (toutes les cellules appartiennent au même groupe).
 * * Si le labyrinthe est complet, cette fonction définit également les points d'entrée ('o') et de sortie ('-'),
 * et initialise les meilleurs scores.
 * * @param cellules Pointeur vers la structure Cellule à vérifier.
 * @param grille Pointeur vers la Grille pour placer l'entrée/sortie et initialiser les scores.
 * @return int 1 si le labyrinthe est complet (toutes les cellules ont le même groupe), 0 sinon.
 */
int labyrinthe_complet(Cellule *cellules, Grille *grille);

/**
 * @brief Construit le chemin du fichier d'un labyrinthe : DOSSIER_LABYRINTHES "/" nom.
 * @param nom Nom du labyrinthe.
 * @param chemin Buffer d'au moins TAILLE_CHEMIN octets qui reçoit le chemin.
 */
void chemin_labyrinthe(const char *nom, char *chemin);

/**
 * @brief Sauvegarde la grille du labyrinthe et les scores associés dans un fichier.
 * * Le format de sauvegarde inclut d'abord les dimensions et le mode, puis la grille de caractères,
 * et enfin la liste des meilleurs scores (score et nom du joueur).
 * * @param grille Pointeur vers la Grille à sauvegarder.
 * @param nom_labyrinthe Nom du fichier de sauvegarde.
 */
void sauvegarder_labyrinthe(Grille *grille, char *nom_labyrinthe);

/**
 * @brief Charge un labyrinthe et ses meilleurs scores à partir d'un fichier.
 * * Cette fonction lit les dimensions et le mode (facile si absent, pour les anciens fichiers),
 * alloue la mémoire pour la grille, charge les caractères de la grille, puis lit la liste des scores.
 * * @param grille Pointeur vers la structure Grille où charger les données.
 * @param nom_labyrinthe Nom du fichier de sauvegarde à charger.
 * @return int 1 si le chargement a réussi, 0 sinon (fichier absent ou invalide ; rien n'est alloué).
 * @note Le programme se termine uniquement en cas d'échec d'allocation mémoire.
 */
int charger_labyrinthe(Grille *grille, char *nom_labyrinthe);

/**
 * @brief Place un motif (clé, piège, boost) aléatoirement dans un passage libre (' ') de la grille.
 * * @param grille Pointeur vers la Grille où placer le motif.
 * @param motif Le caractère représentant le motif à placer (e.g., MOTIF_KEY, MOTIF_TRAP).
 * @return int 1 si le motif a été placé, 0 s'il ne reste aucune case libre.
 */
int drop_motif(Grille *grille, char motif);

/**
 * @brief Compte le nombre de cases de la grille contenant un caractère donné.
 * @param grille Pointeur vers la Grille.
 * @param motif Caractère à compter.
 * @return int Le nombre de cases trouvées.
 */
int compter_motif(Grille *grille, char motif);

/**
 * @brief Place la clé (une seule), les pièges et les bonus dans un labyrinthe construit.
 * * Plus le mode est difficile, plus il y a de pièges et moins il y a de bonus.
 * @param grille Pointeur vers la Grille (son mode est mis à jour).
 * @param mode MODE_FACILE, MODE_NORMAL ou MODE_DIFFICILE (un mode inconnu vaut Facile).
 */
void placer_motifs(Grille *grille, int mode);

/**
 * @brief Renvoie le nom affichable d'un mode de jeu.
 * @param mode MODE_FACILE, MODE_NORMAL ou MODE_DIFFICILE.
 * @return const char* "Facile", "Normal" ou "Difficile".
 */
const char *nom_mode(int mode);

/**
 * @brief Affiche la liste des meilleurs scores enregistrés pour le labyrinthe chargé.
 * * @param grille Pointeur vers la Grille contenant les scores (grille->top_scores).
 * @param nom_labyrinthe Nom du labyrinthe pour l'affichage.
 */
void consulter_score_labyrinthe(Grille *grille, char *nom_labyrinthe);

#endif
