#include "menu.h"
#include "jouer.h"
#include <stdio.h>
#include <stdlib.h>

int main(){

    int choix_du_joueur = 0;
    int hauteur_labyrinthe;
    int largeur_labyrinthe;
    char nom_labyrinthe[50];
    Cellule cellules;
    Grille grille;
    Joueur joueur_1;
    

    while (choix_du_joueur == 0) {
        choix_du_joueur =  get_action_initial();

        switch (choix_du_joueur) {
            case 1:
                printf("Vous voulez créer un labyrinthe\n");
                
                printf("\n Hauteur du labyrinthe : \n");
                scanf("%d", &hauteur_labyrinthe);

                printf("\n Largeur du labyrinthe (Impaire) : \n");
                scanf("%d", &largeur_labyrinthe);

                while ((largeur_labyrinthe % 2) != 1){
                    printf("\n La largeur du labyrinthe doit etre un entier impaire, réssayez : \n");
                    scanf("%d", &largeur_labyrinthe);
                }

                printf("\n Nom du labyrinthe : \n");
                scanf("%s", nom_labyrinthe);
                
                int nbr_cellules_verticales = (hauteur_labyrinthe - 1)/ 2;
                int nbr_cellules_horizontales = (largeur_labyrinthe - 1) / 2;
                
                allouer_cellules(&cellules, nbr_cellules_verticales, nbr_cellules_horizontales);
                allouer_grille(&grille, hauteur_labyrinthe, largeur_labyrinthe);
                initialiser_cellules(&cellules);
                initialiser_grille(&grille, &cellules);
                
                while (!labyrinthe_complet(&cellules, &grille)) {
                    construire_labyrinthe(&cellules, &grille);  
                }

                sauvegarder_labyrinthe(&grille, nom_labyrinthe);
                afficher_labyrinthe(&grille);

                liberer_grille(&grille);
                liberer_cellules(&cellules);

                choix_du_joueur =  0;
                break;

            case 2:
                printf("Vous voulez charger un labyrinthe\n");
                get_nom_labyrinthe(nom_labyrinthe);
                
                charger_labyrinthe(&grille, nom_labyrinthe);
                afficher_labyrinthe(&grille);

                liberer_grille(&grille);

                choix_du_joueur = 0;
                break;

            case 3:
                get_nom_labyrinthe(nom_labyrinthe);
                charger_labyrinthe(&grille, nom_labyrinthe);
                joueur_1.x = 0;
                joueur_1.y = 1;

                jouer_labyrinthe(&grille, &joueur_1);

                liberer_grille(&grille);
                choix_du_joueur = 0;
                break;

            case 4:
                printf("Vous quittez le jeu\n");
                break;
        }
    }

   
    

    return 0;
}

