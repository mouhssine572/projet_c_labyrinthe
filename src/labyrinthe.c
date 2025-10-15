#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "labyrinthe.h"


Cellule **cellules;
char **grille;


// A TESTER
void allouer_cellules(int nbr_cellules_verticales, int nbr_cellules_horizontales){
    cellules = malloc(nbr_cellules_verticales * sizeof(Cellule*));
    if (cellules == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < nbr_cellules_verticales; i++) {
        cellules[i] = malloc(nbr_cellules_horizontales * sizeof(Cellule));
        if (cellules[i] == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
    }
}

void liberer_cellules(int nbr_cellules_verticales){
    for (int i = 0; i < nbr_cellules_verticales; i++) {
        free(cellules[i]);
    }
    free(cellules);
}

//A TESTER
void allouer_grille(int hauteur_labyrinthe, int largeur_labyrinthe){
    grille = malloc(hauteur_labyrinthe * sizeof(char*));
    if (grille == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < hauteur_labyrinthe; i++) {
        grille[i] = malloc(largeur_labyrinthe * sizeof(char));
        if (grille[i] == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
    }
}

void liberer_grille(int nbr_lignes){
    for (int i = 0; i < nbr_lignes; i++) {
        free(grille[i]);
    }
    free(grille);
}

// A TESTER
void initialiser_cellules(int nbr_cellules_verticales, int nbr_cellules_horizontales){
    for (int i = 0 ; i < nbr_cellules_verticales ; i ++){
        for (int j = 0 ; j < nbr_cellules_horizontales ; j ++){
            cellules[i][j].x = i;
            cellules[i][j].y = j;
            cellules[i][j].groupe_cellules = i * nbr_cellules_horizontales + j;
        }
    }
}

//A TESTER
void initialiser_grille(int hauteur_labyrinthe, int largeur_labyrinthe,
                        int nbr_cellules_verticales, int nbr_cellules_horizontales) {
    for (int i = 0; i < hauteur_labyrinthe; i++) {
        for (int j = 0; j < largeur_labyrinthe; j++) {
            grille[i][j] = '#';
        }
    }

    for (int i = 0; i < nbr_cellules_verticales; i++) {
        for (int j = 0; j < nbr_cellules_horizontales; j++) {
            grille[2 * i + 1][2 * j + 1] = ' ';
        }
    }
}
 
void afficher_labyrinthe(int hauteur_labyrinthe, int largeur_labyrinthe){
    for (int i = 0; i < hauteur_labyrinthe; i++) {
        for (int j = 0; j < largeur_labyrinthe; j++) {
            printf("%c", grille[i][j]);
        }
        printf("\n");
    }
}

void construire_labyrinthe(int nbr_lignes, int nbr_colonnes, int nbr_cellules_verticales, int nbr_cellules_horizontales){
    static int initialised = 0;
    if (!initialised) {
        srand(time(NULL));
        initialised = 1;
    }

    int i_random, j_random;

    // choix random du mur
    while (1) {
        i_random = 1 + rand() % (nbr_lignes - 2);
        j_random = 1 + rand() % (nbr_colonnes - 2);

        if ((i_random % 2 == 0 && j_random % 2 == 1) ||
            (i_random % 2 == 1 && j_random % 2 == 0)) {
            break;
        }
    }

    // Coordonnées des deux cellules autour du mur
    int ci1, cj1, ci2, cj2;

    if (i_random % 2 == 0) {
        // mur horizontal
        ci1 = (i_random - 1) / 2;
        cj1 = (j_random - 1) / 2;
        ci2 = (i_random + 1) / 2;
        cj2 = cj1;
    } else {
        // mur vertical
        ci1 = (i_random - 1) / 2;
        cj1 = (j_random - 1) / 2;
        ci2 = ci1;
        cj2 = (j_random + 1) / 2;
    }

    int g1 = cellules[ci1][cj1].groupe_cellules;
    int g2 = cellules[ci2][cj2].groupe_cellules;

    if (g1 != g2) {
        // Ouvrir le mur
        grille[i_random][j_random] = ' ';

        // Fusionner les groupes : remplacer tous les g2 par g1
        for (int i = 0; i < nbr_cellules_verticales; i++) {
            for (int j = 0; j < nbr_cellules_horizontales; j++) {
                if (cellules[i][j].groupe_cellules == g2) {
                    cellules[i][j].groupe_cellules = g1;
                }
            }
        }
    }
}

int labyrinthe_complet(int nbr_lignes, int nbr_colonnes, int nbr_cellules_verticales, int nbr_cellules_horizontales){
    int groupe_ref = cellules[0][0].groupe_cellules;
    for (int i = 0; i < nbr_cellules_verticales; i++) {
        for (int j = 0; j < nbr_cellules_horizontales; j++) {
            if (cellules[i][j].groupe_cellules != groupe_ref) {
                return 0; 
            }
        }
    }

    grille[0][1] = 'o';
    grille[nbr_lignes - 1][nbr_colonnes - 2] = '-';
    return 1;
}