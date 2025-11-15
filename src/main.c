#include "menu.h"
#include "jouer.h"
#include <stdio.h>
#include <stdlib.h>

int main(){

    int choix_du_joueur = 0;
    int hauteur_labyrinthe;
    int largeur_labyrinthe;
    int mode_jeu;

    int maj_score;
    char nom_joueur[50];

    char nom_labyrinthe[50];
    Cellule cellules;
    Grille grille;
    Joueur joueur_1;
    

    while (choix_du_joueur == 0) {
        choix_du_joueur =  get_action_initial();

        switch (choix_du_joueur) {
            case 1:
                printf("Vous voulez créer un labyrinthe\n");
                
                mode_jeu = get_mode_jeu();

                get_parametres_labyrinthe(&hauteur_labyrinthe, &largeur_labyrinthe, nom_labyrinthe);

                int nbr_cellules_verticales = (hauteur_labyrinthe - 1)/ 2;
                int nbr_cellules_horizontales = (largeur_labyrinthe - 1) / 2;
                
                allouer_cellules(&cellules, nbr_cellules_verticales, nbr_cellules_horizontales);
                allouer_grille(&grille, hauteur_labyrinthe, largeur_labyrinthe);
                initialiser_cellules(&cellules);
                initialiser_grille(&grille, &cellules);
                
                while (!labyrinthe_complet(&cellules, &grille)) {
                    construire_labyrinthe(&cellules, &grille);  
                }

                drop_motif(&grille, MOTIF_KEY);
                
                int nbr_motifs_a_placer = (hauteur_labyrinthe * largeur_labyrinthe) * 0.02;
                for (int k = 0; k < nbr_motifs_a_placer; k++) {
                    drop_motif(&grille, MOTIF_TRAP); 
                    drop_motif(&grille, MOTIF_BOOST);
                }

                sauvegarder_labyrinthe(&grille, nom_labyrinthe);
                afficher_labyrinthe(&grille);

                if(mode_jeu == 2){
                    printf("le mode 2 est en cours de developpement\n");
                }

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
                printf("\n Nom du joueur : \n");
                scanf("%s", nom_joueur);
                get_nom_labyrinthe(nom_labyrinthe);
                charger_labyrinthe(&grille, nom_labyrinthe);
                initialiser_joueur(&joueur_1);

                jouer_labyrinthe(&grille, &joueur_1);

                maj_score = set_score_joueur(&grille, &joueur_1, nom_joueur); 
                if(maj_score == 1){
                    sauvegarder_score(&grille, nom_labyrinthe);
                }

                liberer_grille(&grille);
                choix_du_joueur = 0;
                break;
            case 4:
                get_nom_labyrinthe(nom_labyrinthe);
                charger_labyrinthe(&grille, nom_labyrinthe);

                consulter_score_labyrinthe(&grille, nom_labyrinthe);

                liberer_grille(&grille);
                choix_du_joueur = 0;
                break;
            case 5:
                printf("Vous quittez le jeu\n");
                break;
        }
    }

   
    

    return 0;
}

