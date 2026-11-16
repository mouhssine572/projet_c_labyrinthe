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
#define ACTION_SUPPRIMER 6
#define ACTION_QUITTER  7

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
    printf("\nLabyrinthe \"%s\" créé en mode %s.\n", nom_labyrinthe, nom_mode(mode_jeu));

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
    printf("\nMode %s.\n", nom_mode(grille.mode));
    liberer_grille(&grille);
}

/* Joue une partie sur le labyrinthe donné et enregistre le score en cas de victoire.
 * Renvoie 0 si le labyrinthe n'a pas pu être chargé, 1 sinon. */
static int jouer_une_partie(const char *nom_joueur, char *nom_labyrinthe) {
    Grille grille;
    Joueur joueur;

    // Rechargé à chaque partie : la précédente a modifié la grille (bonus pris, trace du joueur)
    if (!charger_labyrinthe(&grille, nom_labyrinthe)) return 0;

    initialiser_joueur(&joueur);

    if (jouer_labyrinthe(&grille, &joueur)) {
        char message[2 * TAILLE_MESSAGE];
        int place = enregistrer_victoire(&grille, &joueur, nom_joueur, nom_labyrinthe, message, sizeof message);
        printf("\n%s%s%s\n", place == 1 ? COULEUR_JAUNE : "", message, COULEUR_RESET);
    }

    liberer_grille(&grille);
    return 1;
}

/* Propose de rejouer à la fin d'une partie. Renvoie 1 pour rejouer, 0 pour revenir au menu. */
static int proposer_rejouer(const char *nom_labyrinthe) {
    char titre[TAILLE_NOM + 32];
    const char *options[] = {
        "Rejouer le même labyrinthe",
        "Revenir au menu"
    };
    snprintf(titre, sizeof titre, "Partie terminée sur \"%s\".", nom_labyrinthe);
    return choisir_option(titre, options, 2) == 0;
}

static void option_jouer(void) {
    char nom_joueur[TAILLE_NOM];
    char nom_labyrinthe[TAILLE_NOM];

    if (!verifier_labyrinthe_disponible()) return;
    if (!get_nom_joueur(nom_joueur)) return;
    if (!get_nom_labyrinthe(nom_labyrinthe)) return;

    while (jouer_une_partie(nom_joueur, nom_labyrinthe) && proposer_rejouer(nom_labyrinthe)) {
        // On rejoue avec le même joueur sur le même labyrinthe
    }
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

static void option_supprimer(void) {
    char nom_labyrinthe[TAILLE_NOM];
    char titre[TAILLE_NOM + 64];
    const char *options[] = {
        "Oui, le supprimer (son top 10 sera perdu)",
        "Non, revenir au menu"
    };

    if (!verifier_labyrinthe_disponible()) return;
    printf("Vous voulez supprimer un labyrinthe\n");
    if (!get_nom_labyrinthe(nom_labyrinthe)) return;

    // Confirmation avant de supprimer, comme pour l'écrasement d'un labyrinthe existant
    snprintf(titre, sizeof titre, "Supprimer définitivement le labyrinthe \"%s\" ?", nom_labyrinthe);
    if (choisir_option(titre, options, 2) != 0) {
        printf("Suppression annulée.\n");
        return;
    }

    if (supprimer_labyrinthe(nom_labyrinthe)) {
        printf("Labyrinthe \"%s\" supprimé.\n", nom_labyrinthe);
    } else {
        perror("Impossible de supprimer le labyrinthe");
    }
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
            case ACTION_SUPPRIMER: option_supprimer(); break;
            case ACTION_QUITTER: printf("Vous quittez le jeu\n"); break;
        }
    } while (choix != ACTION_QUITTER);

    return 0;
}
