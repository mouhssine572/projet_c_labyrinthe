#include "menu.h"
#include "jouer.h"
#include "terminal.h"
#include <stdio.h>
#include <stdlib.h>

int main(){

    int choix_du_joueur = 0;
    int hauteur_labyrinthe;
    int largeur_labyrinthe;
    int mode_jeu;

    int place_classement;
    char nom_joueur[TAILLE_NOM];

    char nom_labyrinthe[TAILLE_NOM];
    Cellule cellules;
    Grille grille;
    Joueur joueur_1;

    initialiser_terminal();

    while (choix_du_joueur == 0) {
        choix_du_joueur =  get_action_initial();

        switch (choix_du_joueur) {
            case 1:
                printf("Vous voulez créer un labyrinthe\n");

                mode_jeu = get_mode_jeu();
                if (mode_jeu == CHOIX_RETOUR) {
                    choix_du_joueur = 0;
                    break;
                }

                if (!get_parametres_labyrinthe(&hauteur_labyrinthe, &largeur_labyrinthe, nom_labyrinthe)) {
                    choix_du_joueur = 0;
                    break;
                }

                int nbr_cellules_verticales = (hauteur_labyrinthe - 1)/ 2;
                int nbr_cellules_horizontales = (largeur_labyrinthe - 1) / 2;

                allouer_cellules(&cellules, nbr_cellules_verticales, nbr_cellules_horizontales);
                allouer_grille(&grille, hauteur_labyrinthe, largeur_labyrinthe);
                initialiser_cellules(&cellules);
                initialiser_grille(&grille, &cellules);

                while (!labyrinthe_complet(&cellules, &grille)) {
                    construire_labyrinthe(&cellules, &grille);
                }

                placer_motifs(&grille, mode_jeu);

                sauvegarder_labyrinthe(&grille, nom_labyrinthe);
                afficher_labyrinthe(&grille);
                printf("\nLabyrinthe \"%s\" créé en mode %s.\n",
                       nom_labyrinthe, mode_jeu == MODE_DIFFICILE ? "Difficile" : "Facile");

                liberer_grille(&grille);
                liberer_cellules(&cellules);

                choix_du_joueur =  0;
                break;

            case 2:
                if (!labyrinthe_disponible()) {
                    printf("\nAucun labyrinthe à charger. Créez d'abord un labyrinthe (choix 1).\n");
                    choix_du_joueur = 0;
                    break;
                }
                printf("Vous voulez charger un labyrinthe\n");
                if (!get_nom_labyrinthe(nom_labyrinthe)) {
                    choix_du_joueur = 0;
                    break;
                }

                if (charger_labyrinthe(&grille, nom_labyrinthe)) {
                    afficher_labyrinthe(&grille);
                    printf("\nMode %s.\n", grille.mode == MODE_DIFFICILE ? "Difficile" : "Facile");
                    liberer_grille(&grille);
                }

                choix_du_joueur = 0;
                break;

            case 3:
                if (!labyrinthe_disponible()) {
                    printf("\nAucun labyrinthe n'a été créé. Créez d'abord un labyrinthe (choix 1).\n");
                    choix_du_joueur = 0;
                    break;
                }
                if (!get_nom_joueur(nom_joueur) || !get_nom_labyrinthe(nom_labyrinthe)) {
                    choix_du_joueur = 0;
                    break;
                }
                if (!charger_labyrinthe(&grille, nom_labyrinthe)) {
                    choix_du_joueur = 0;
                    break;
                }
                initialiser_joueur(&joueur_1);

                if (jouer_labyrinthe(&grille, &joueur_1)) {
                    place_classement = set_score_joueur(&grille, &joueur_1, nom_joueur);
                    if (place_classement > 0) {
                        sauvegarder_score(&grille, nom_labyrinthe);
                        printf("\nVous êtes %d%s sur ce labyrinthe !\n",
                               place_classement, place_classement == 1 ? "er" : "e");
                    } else {
                        printf("\nVotre score n'entre pas dans le top %d de ce labyrinthe.\n", NOMBRE_SCORE);
                    }
                }

                liberer_grille(&grille);
                choix_du_joueur = 0;
                break;

            case 4:
                if (!labyrinthe_disponible()) {
                    printf("\nAucun labyrinthe disponible. Créez d'abord un labyrinthe (choix 1).\n");
                    choix_du_joueur = 0;
                    break;
                }
                if (!get_nom_labyrinthe(nom_labyrinthe)) {
                    choix_du_joueur = 0;
                    break;
                }
                if (charger_labyrinthe(&grille, nom_labyrinthe)) {
                    consulter_score_labyrinthe(&grille, nom_labyrinthe);
                    liberer_grille(&grille);
                }
                choix_du_joueur = 0;
                break;

            case 5:
                afficher_regles();
                choix_du_joueur = 0;
                break;

            case 6:
                printf("Vous quittez le jeu\n");
                break;
        }
    }

    return 0;
}
