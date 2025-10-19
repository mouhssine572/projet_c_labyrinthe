#ifndef JOUER_H
#define JOUER_H
#include "labyrinthe.h"


typedef struct {
    int x;
    int y;
} Joueur;

void positionner_joueur(Grille *grille, Joueur *joueur);

void mouvement_joueur(Grille *grille, Joueur *joueur);

int mouvement_valide(Grille *grille, int x, int y);

void jouer_labyrinthe(Grille *grille, Joueur *joueur);

#endif