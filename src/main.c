#include "menu.h"
#include "labyrinthe.h"

#include <stdio.h>
#include <stdlib.h>

int main(){

    //Initialisation nécessaire
    int * hauteur_labyrinthe = malloc(sizeof(int));
    int * largeur_labyrinthe = malloc(sizeof(int));

    int choix_du_joueur =  get_action_initial();

    switch (choix_du_joueur ) {
        case 1:
            printf("Vous voulez créer un labyrinthe\n");

            //get_taille_labyrinthe(hauteur_labyrinthe, largeur_labyrinthe);

            printf("\n Hauteur du labyrinthe : \n");
            scanf("%d", hauteur_labyrinthe);

            printf("\n Largeur du labyrinthe (Impaire) : \n");
            scanf("%d", largeur_labyrinthe);

            while((*largeur_labyrinthe % 2) != 1){
                printf("\n La largeur du labyrinthe doit etre un entier impaire, réssayez : \n");
                scanf("%d", largeur_labyrinthe);
            }

            int nbr_colonnes = (*largeur_labyrinthe * 2) + 1;
            int nbr_lignes = (*hauteur_labyrinthe * 2) + 1;

            int nbr_cellules_verticales = ((*hauteur_labyrinthe)- 1)/ 2;
            int nbr_cellules_horizontales = ((*largeur_labyrinthe-1)/2);

            allouer_cellules(nbr_cellules_verticales, nbr_cellules_horizontales);

            allouer_grille(nbr_lignes, nbr_colonnes);

            initialiser_cellules(nbr_cellules_verticales, nbr_cellules_horizontales);
            initialiser_grille(nbr_lignes, nbr_colonnes, nbr_cellules_verticales, nbr_cellules_horizontales);

            while (!labyrinthe_complet(nbr_lignes, nbr_colonnes, nbr_cellules_verticales, nbr_cellules_horizontales)) {
                construire_labyrinthe(nbr_lignes, nbr_colonnes, nbr_cellules_verticales, nbr_cellules_horizontales);
                afficher_labyrinthe(nbr_lignes, nbr_colonnes);  
            }

            liberer_grille(nbr_lignes);
            liberer_cellules(nbr_cellules_verticales);
            break;

        case 2:
            printf("Vous voulez charger un labyrinthe\n");
            break;

        case 3:
            printf("Vous voulez jouer\n");
            break;

        case 4:
            printf("Vous voulez quitter\n");
            break;
    }
    

    return 0;
}
