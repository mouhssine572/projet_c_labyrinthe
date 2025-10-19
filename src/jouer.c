#include <stdio.h>
#include <stdlib.h>
#include "jouer.h"
#include "labyrinthe.h"

void positionner_joueur(Grille *grille, Joueur *joueur){
    int position_x_joueur = joueur -> x;
    int position_y_joueur = joueur -> y;
    grille->cases[position_x_joueur][position_y_joueur] = '*';
}

int mouvement_valide(Grille *grille, int x, int y){
    if(x > grille -> hauteur || x < 0) return 0;
    if(y > grille -> largeur || y < 0) return 0;

    if(grille -> cases[x][y] == '#') return 0;
    
    return 1;
}

void mouvement_joueur(Grille *grille, Joueur *joueur){
    char mvt;

    int x = joueur->x; 
    int y = joueur->y;

    if (scanf(" %c", &mvt) != 1) return;

    grille -> cases[joueur -> x][joueur -> y] = ' ';

    switch(mvt){
        case 'z':
            x--;
            break;
        case 'q':
            y--;
            break;
        case 'd':
            y++;
            break;
        case 's':
            x++;
            break;
        default:
            printf("Touche invalide.\n");
            return;
    }
    
    if(mouvement_valide(grille, x, y)){
            joueur->x = x;
            joueur->y = y;
    }

    positionner_joueur(grille, joueur);
}

void jouer_labyrinthe(Grille *grille, Joueur *joueur){

    while(!((joueur -> x == grille->hauteur - 1) && (joueur -> y == grille->largeur - 2))){
        afficher_labyrinthe(grille);
        mouvement_joueur(grille, joueur);                 
    }
}