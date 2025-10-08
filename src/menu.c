#include <stdio.h>
#include "menu.h"

int get_action_initial(){
    int choix;

    printf("\n Choississez une action parmi ce qui suit : \n");
    printf("\n 1 - Créer un labyrinthe\n");
    printf("\n 2 - Charger un labyrinthe\n");
    printf("\n 3 - Jouer\n");
    printf("\n 4 - Quitter\n");

    scanf("%d", &choix);
    
    return choix;
};

void get_taille_labyrinthe(int * hauteur_labyrinthe, int * largeur_labyrinthe){

    printf("\n Hauteur du labyrinthe : \n");
    scanf("%d", hauteur_labyrinthe);

    printf("\n Largeur du labyrinthe (Impaire) : \n");
    scanf("%d", largeur_labyrinthe);

    while((*largeur_labyrinthe % 2) != 1){
        printf("\n La largeur du labyrinthe doit etre un entier impaire, réssayez : \n");
        scanf("%d", largeur_labyrinthe);
    }
}

