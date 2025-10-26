#include <stdio.h>
#include <stdlib.h>
#include "jouer.h"
#include "labyrinthe.h"

void initialiser_joueur(Joueur *joueur){
    joueur -> x = 0;
    joueur -> y = 1;
    joueur -> have_key = 0;
    joueur -> score = 0;
}

void positionner_joueur(Grille *grille, Joueur *joueur){
    int position_x_joueur = joueur -> x;
    int position_y_joueur = joueur -> y;
    int score_joueur = joueur -> score;
    grille -> cases[position_x_joueur][position_y_joueur] = '*';
    joueur -> score = score_joueur + 1;

}

int mouvement_valide(Grille *grille, int x, int y, Joueur *joueur){

    int n =  joueur -> score;
    if(x >= grille -> hauteur || x < 0) return 0;
    if(y >= grille -> largeur || y < 0) return 0;

    if(grille -> cases[x][y] == '#') return 0;
    if((grille -> cases[x][y] == '-') && joueur -> have_key == 0) return 0;
    if(grille -> cases[x][y] == MOTIF_KEY) joueur -> have_key = 1;
    if(grille -> cases[x][y] == MOTIF_TRAP) joueur -> score = n + 6;
    if(grille -> cases[x][y] == MOTIF_BOOST){
        if (n < 6){
            n = 6;
        }
        joueur -> score = n - 6;
    }
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
    printf("\n Votre score est : %d \n", get_score_joueur(joueur));
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

int get_score_joueur(Joueur *joueur){
    return joueur -> score; 
}
