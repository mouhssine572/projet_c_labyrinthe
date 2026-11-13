#include "menu.h"
#include "jouer.h"
#include "terminal.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Numéros des actions du menu principal (voir get_action_initial). */
#define ACTION_CREER    1
#define ACTION_CHARGER  2
#define ACTION_JOUER    3
#define ACTION_SCORES   4
#define ACTION_REGLES   5
#define ACTION_QUITTER  6

/* Indique si au moins un labyrinthe existe ; sinon affiche un message explicatif. */
static int verifier_labyrinthe_disponible(void) {
    if (labyrinthe_disponible()) return 1;
    printf("\nAucun labyrinthe disponible. Créez d'abord un labyrinthe (choix %d).\n", ACTION_CREER);
    return 0;
}

static void option_creer(void) {
    int hauteur, largeur;
    char nom_labyrinthe[TAILLE_NOM];
    Cellule cellules;
    Grille grille;

    printf("Vous voulez créer un labyrinthe\n");

    int mode_jeu = get_mode_jeu();
    if (mode_jeu == CHOIX_RETOUR) return;
    if (!get_parametres_labyrinthe(&hauteur, &largeur, nom_labyrinthe)) return;

    allouer_cellules(&cellules, (hauteur - 1) / 2, (largeur - 1) / 2);
    allouer_grille(&grille, hauteur, largeur);
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
}

static void option_charger(void) {
    char nom_labyrinthe[TAILLE_NOM];
    Grille grille;

    if (!verifier_labyrinthe_disponible()) return;
    printf("Vous voulez charger un labyrinthe\n");
    if (!get_nom_labyrinthe(nom_labyrinthe)) return;
    if (!charger_labyrinthe(&grille, nom_labyrinthe)) return;

    afficher_labyrinthe(&grille);
    printf("\nMode %s.\n", grille.mode == MODE_DIFFICILE ? "Difficile" : "Facile");
    liberer_grille(&grille);
}

static void option_jouer(void) {
    char nom_joueur[TAILLE_NOM];
    char nom_labyrinthe[TAILLE_NOM];
    Grille grille;
    Joueur joueur;

    if (!verifier_labyrinthe_disponible()) return;
    if (!get_nom_joueur(nom_joueur)) return;
    if (!get_nom_labyrinthe(nom_labyrinthe)) return;
    if (!charger_labyrinthe(&grille, nom_labyrinthe)) return;

    initialiser_joueur(&joueur);

    if (jouer_labyrinthe(&grille, &joueur)) {
        int place = set_score_joueur(&grille, &joueur, nom_joueur);
        if (place > 0) {
            sauvegarder_score(&grille, nom_labyrinthe);
            printf("\nVous êtes %d%s sur ce labyrinthe !\n", place, place == 1 ? "er" : "e");
        } else {
            printf("\nVotre score n'entre pas dans le top %d de ce labyrinthe.\n", NOMBRE_SCORE);
        }
    }

    liberer_grille(&grille);
}

static void option_scores(void) {
    char nom_labyrinthe[TAILLE_NOM];
    Grille grille;

    if (!verifier_labyrinthe_disponible()) return;
    if (!get_nom_labyrinthe(nom_labyrinthe)) return;
    if (!charger_labyrinthe(&grille, nom_labyrinthe)) return;

    consulter_score_labyrinthe(&grille, nom_labyrinthe);
    liberer_grille(&grille);
}

int main(void){
    int choix;

    initialiser_terminal();
    srand((unsigned) time(NULL)); // Le seul appel à srand du programme

    do {
        choix = get_action_initial();

        switch (choix) {
            case ACTION_CREER:   option_creer();   break;
            case ACTION_CHARGER: option_charger(); break;
            case ACTION_JOUER:   option_jouer();   break;
            case ACTION_SCORES:  option_scores();  break;
            case ACTION_REGLES:  afficher_regles(); break;
            case ACTION_QUITTER: printf("Vous quittez le jeu\n"); break;
        }
    } while (choix != ACTION_QUITTER);

    return 0;
}
