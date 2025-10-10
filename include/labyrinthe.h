#ifndef LABYRINTHE_H
#define LABYRINTHE_H

typedef struct {
    int groupe_cellules;
    int x,y;    //Position de la cellule (par rappport aux autres cellules)
} Cellule;

//extern int * hauteur_labyrinthe;
//extern int * largeur_labyrinthe;

void allouer_cellules(int nbr_cellules_verticales, int nbr_cellules_horizontales);

void liberer_cellules(int nbr_cellules_verticales);

void allouer_grille(int nbr_lignes, int nbr_colonnes);

void liberer_grille(int nbr_lignes);

void initialiser_cellules(int nbr_cellules_verticales, int nbr_cellules_horizontales);

void initialiser_grille(int nbr_lignes, int nbr_colonnes, int nbr_cellules_verticales, int nbr_cellules_horizontales);

void afficher_labyrinthe(int nbr_lignes, int nbr_colonnes);

void construire_labyrinthe(int nbr_lignes, int nbr_colonnes, int nbr_cellules_verticales, int nbr_cellules_horizontales);

int labyrinthe_complet(int nbr_lignes, int nbr_colonnes, int nbr_cellules_verticales, int nbr_cellules_horizontales);


#endif