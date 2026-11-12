#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "menu.h"

/* Vide le reste de la ligne courante sur l'entrée standard. */
static void vider_ligne(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* Termine proprement le programme si l'entrée standard est fermée (Ctrl+D / Ctrl+Z). */
static void verifier_fin_entree(int resultat_scanf) {
    if (resultat_scanf == EOF) {
        printf("\nFin de l'entrée, le jeu se termine.\n");
        exit(EXIT_SUCCESS);
    }
}

int lire_entier(void) {
    int valeur;
    int res;

    while (1) {
        res = scanf("%d", &valeur);
        verifier_fin_entree(res);
        vider_ligne();
        if (res == 1) {
            return valeur;
        }
        printf("Saisie invalide, entrez un nombre : ");
    }
}

void lire_mot(char *destination) {
    int res = scanf("%" STR(TAILLE_NOM_MAX) "s", destination);
    verifier_fin_entree(res);
    vider_ligne();
}

int get_action_initial(){
    int choix;

    printf("\n Choississez une action parmi ce qui suit : \n");
    printf("\n 1 - Créer un labyrinthe\n");
    printf("\n 2 - Charger un labyrinthe\n");
    printf("\n 3 - Jouer\n");
    printf("\n 4 - Consulter les scores d'un labyrinthe\n");
    printf("\n 5 - Quitter\n");

    choix = lire_entier();
    while (choix < 1 || choix > 5) {
        printf("Choix invalide, entrez un nombre entre 1 et 5 : ");
        choix = lire_entier();
    }

    return choix;
}

void get_nom_labyrinthe(char *nom_labyrinthe) {
    printf("\n Choissisez un labyrinthe de la liste (entrez le nom du fichier) : \n");

    DIR *d = opendir("labyrinthe");
    int fichiers_trouves = 0;

    if (d) {
        struct dirent *dir;

        while ((dir = readdir(d)) != NULL) {
            // Ignorer les entrées système "." et ".."
            if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
                printf("- %s\n", dir->d_name);
                fichiers_trouves = 1;
            }
        }
        closedir(d);
    }

    if (!fichiers_trouves) {
        printf("Aucun labyrinthe disponible pour le moment.\n");
    }

    lire_mot(nom_labyrinthe);
}

int get_mode_jeu() {
    int mode;

    printf("Choisissez un mode de jeu :\n");
    printf("Mode 1 : Facile\n");
    printf("Mode 2 : Difficile\n");

    mode = lire_entier();
    while (mode != 1 && mode != 2) {
        printf("Mode invalide, entrez 1 ou 2 : ");
        mode = lire_entier();
    }

    printf("Mode de jeu choisi : %d\n", mode);
    return mode;
}

/* Demande une dimension impaire comprise entre DIMENSION_MIN et DIMENSION_MAX. */
static int lire_dimension(const char *libelle) {
    int valeur;

    while (1) {
        printf("\n%s du labyrinthe (impaire, entre %d et %d) : ", libelle, DIMENSION_MIN, DIMENSION_MAX);
        valeur = lire_entier();

        if (valeur < DIMENSION_MIN || valeur > DIMENSION_MAX) {
            printf("\nLa %s doit être comprise entre %d et %d, réessayez.\n", libelle, DIMENSION_MIN, DIMENSION_MAX);
        } else if (valeur % 2 != 1) {
            printf("\nLa %s du labyrinthe doit être un entier impair, réessayez.\n", libelle);
        } else {
            return valeur;
        }
    }
}

void get_parametres_labyrinthe(int *hauteur, int *largeur, char *nom){

    *hauteur = lire_dimension("Hauteur");
    *largeur = lire_dimension("Largeur");

    printf("\nNom du labyrinthe : ");
    lire_mot(nom);
}
