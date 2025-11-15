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
    Top_Player top_scores[NOMBRE_SCORE];
} Grille;



void allouer_cellules(Cellule *cellules, int nbr_cellules_verticales, int nbr_cellules_horizontales);

void liberer_cellules(Cellule *cellules);

void allouer_grille(Grille *grille, int hauteur_labyrinthe, int largeur_labyrinthe);

void liberer_grille(Grille *grille);

void initialiser_cellules(Cellule *cellules);

void initialiser_grille(Grille *grille, Cellule *cellules);

void afficher_labyrinthe(Grille *grille);

void construire_labyrinthe(Cellule *cellules, Grille *grille);

int labyrinthe_complet(Cellule *cellules, Grille *grille);

void sauvegarder_labyrinthe(Grille *grille, char *nom_labyrinthe);

void sauvegarder_labyrinthe_score(Grille *grille, Grille *grille_jeu, char *nom_labyrinthe);

void charger_labyrinthe(Grille *grille, char *nom_labyrinthe);

void drop_motif(Grille *grille, char motif);

void consulter_score_labyrinthe(Grille *grille, char *nom_labyrinthe);

#endif