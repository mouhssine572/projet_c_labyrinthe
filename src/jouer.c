#include <stdio.h>
#include <stdlib.h>
#include "jouer.h"
#include "labyrinthe.h"

void initialiser_joueur(Joueur *joueur){
    joueur -> x = 0;
    joueur -> y = 1;
    joueur -> have_key = 0;
}

void positionner_joueur(Grille *grille, Joueur *joueur){
    int position_x_joueur = joueur -> x;
    int position_y_joueur = joueur -> y;
    grille->cases[position_x_joueur][position_y_joueur] = '*';
}

int mouvement_valide(Grille *grille, int x, int y, Joueur *joueur){
    if(x >= grille -> hauteur || x < 0) return 0;
    if(y >= grille -> largeur || y < 0) return 0;

    if(grille -> cases[x][y] == '#') return 0;
    if(grille -> cases[x][y] == '$') joueur -> have_key = 1;
    if((grille -> cases[x][y] == '-') && joueur -> have_key == 0) return 0;

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
    
    if(mouvement_valide(grille, x, y, joueur)){
        joueur -> x = x;
        joueur -> y = y;
    }

    positionner_joueur(grille, joueur);
}

void jouer_labyrinthe(Grille *grille, Joueur *joueur){
    
    do {
        afficher_labyrinthe(grille);
        mouvement_joueur(grille, joueur);                 
    } while(joueur_gagne(grille, joueur) == 0);
}

int joueur_gagne(Grille *grille, Joueur *joueur){
    if(joueur -> have_key == 0) return 0;
    if(grille->cases[grille->hauteur - 1][grille->largeur - 2] != '*') return 0;
    return 1;
}
