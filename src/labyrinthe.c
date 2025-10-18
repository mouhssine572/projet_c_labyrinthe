#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "labyrinthe.h"

// A TESTER
void allouer_cellules(Cellule *cellules, int nbr_cellules_verticales, int nbr_cellules_horizontales){

    cellules->nbr_cellules_horizontales = nbr_cellules_horizontales;
    cellules->nbr_cellules_verticales = nbr_cellules_verticales;

    cellules->cellule_matrix = malloc(cellules->nbr_cellules_verticales * sizeof(int*));
    if (cellules->cellule_matrix == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < cellules->nbr_cellules_verticales; i++) {
        cellules->cellule_matrix[i] = malloc(cellules->nbr_cellules_horizontales * sizeof(int));
        if (cellules->cellule_matrix[i] == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
    }
}

void liberer_cellules(Cellule *cellules){
    for (int i = 0; i < cellules->nbr_cellules_verticales; i++) {
        free(cellules->cellule_matrix[i]);
    }
    free(cellules->cellule_matrix);
}

//A TESTER
void allouer_grille(Grille *grille, int hauteur_labyrinthe, int largeur_labyrinthe){
    
    grille->largeur = largeur_labyrinthe;
    grille->hauteur = hauteur_labyrinthe;
    grille->cases = malloc(grille->hauteur * sizeof(char*));
    if (grille->cases == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < grille->hauteur; i++) {
        grille->cases[i] = malloc(grille->largeur * sizeof(char));
        if (grille->cases[i] == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
    }
}

void liberer_grille(Grille *grille){
    for (int i = 0; i <grille->hauteur; i++) {
        free(grille->cases[i]);
    }
    free(grille->cases);
}

// A TESTER
void initialiser_cellules(Cellule *cellules){
    for (int i = 0 ; i < cellules->nbr_cellules_verticales ; i ++){
        for (int j = 0 ; j < cellules->nbr_cellules_horizontales ; j ++){
            cellules->cellule_matrix[i][j] = i * cellules->nbr_cellules_horizontales + j;
        }
    }
}

//A TESTER
void initialiser_grille(Grille *grille, Cellule *cellules) {
    for (int i = 0; i < grille->hauteur; i++) {
        for (int j = 0; j < grille->largeur; j++) {
            grille->cases[i][j] = '#';
        }
    }

    for (int i = 0; i < cellules->nbr_cellules_verticales; i++) {
        for (int j = 0; j < cellules->nbr_cellules_horizontales; j++) {
            grille->cases[2 * i + 1][2 * j + 1] = ' ';
        }
    }
}

void afficher_labyrinthe(Grille *grille){
    for (int i = 0; i < grille->hauteur; i++) {
        for (int j = 0; j < grille->largeur; j++) {
            printf("%c", grille->cases[i][j]);
        }
        printf("\n");
    }
}

void construire_labyrinthe(Cellule *cellules, Grille *grille) {
    static int initialised = 0;
    if (!initialised) {
        srand(time(NULL));
        initialised = 1;
    }

    int i_random, j_random;
    int ci1, cj1, ci2, cj2;
    int nbr_cellules_verticales = cellules->nbr_cellules_verticales;
    int nbr_cellules_horizontales = cellules->nbr_cellules_horizontales;

    // Boucle jusqu'à ce qu'on trouve un mur qui sépare deux groupes différents
    int trouve_mur = 0;
    while (!trouve_mur) {
        i_random = 1 + rand() % (grille->hauteur - 2);
        j_random = 1 + rand() % (grille->largeur - 2);

        // Mur horizontal ou vertical
        if ((i_random % 2 == 0 && j_random % 2 == 1) ||
            (i_random % 2 == 1 && j_random % 2 == 0)) {

            if (i_random % 2 == 0) {
                // Mur horizontal
                ci1 = (i_random - 1)/2;
                cj1 = (j_random - 1)/2;
                ci2 = (i_random + 1)/2;
                cj2 = cj1;
                trouve_mur = 1;
            } else {
                // Mur vertical
                ci1 = (i_random - 1)/2 ;
                cj1 = (j_random - 1)/2;
                ci2 = ci1;
                cj2 = (j_random + 1)/2;
                trouve_mur = 1;
            }

            // Vérification des bornes
            //if (ci2 >= nbr_cellules_verticales) ci2 = nbr_cellules_verticales - 1;
            //if (cj2 >= nbr_cellules_horizontales) cj2 = nbr_cellules_horizontales - 1;
        }
    }

    int groupe_cellule_1 = cellules->cellule_matrix[ci1][cj1];
    int groupe_cellule_2 = cellules->cellule_matrix[ci2][cj2];

    if (groupe_cellule_1 != groupe_cellule_2) {
        // Ouvrir le mur
        grille->cases[i_random][j_random] = ' ';

        // Fusioner les groupes
        for (int i = 0; i < nbr_cellules_verticales; i++) {
            for (int j = 0; j < nbr_cellules_horizontales; j++) {
                if (cellules->cellule_matrix[i][j] == groupe_cellule_2) {
                    cellules->cellule_matrix[i][j] = groupe_cellule_1;
                }
            }
        }
    }
}

int labyrinthe_complet(Cellule *cellules, Grille *grille) {

    int groupe_ref = cellules->cellule_matrix[0][0];

    for (int i = 0; i < cellules->nbr_cellules_verticales; i++) {
        for (int j = 0; j < cellules->nbr_cellules_horizontales; j++) {
            if (cellules->cellule_matrix[i][j] != groupe_ref) {
                return 0; 
            }
        }
    }

    grille->cases[0][1] = 'o';                                       // Entrée
    grille->cases[grille->hauteur - 1][grille->largeur - 2] = '-';   // Sortie

    return 1;
}

void sauvegarder_labyrinthe(Grille *grille, char *nom_labyrinthe) {
    FILE *f = fopen(nom_labyrinthe, "w");
    if (!f) {
        perror("Erreur ouverture fichier");
        return;
    }

    fprintf(f, "Nom du labyrinthe : %s\n", nom_labyrinthe);


    for (int i = 0; i < grille->hauteur; i++) {
        for (int j = 0; j < grille->largeur; j++) {
            fputc(grille->cases[i][j], f);
        }
        fputc('\n', f);
    }
    fclose(f);
}

/*
void charger_labyrinthe(){

}*/
