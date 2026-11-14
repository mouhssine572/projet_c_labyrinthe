#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "labyrinthe.h"
#include "terminal.h"

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
    if (cellules == NULL || cellules->cellule_matrix == NULL) {
        return; 
    }
    
    for (int i = 0; i < cellules->nbr_cellules_verticales; i++) {
        if (cellules->cellule_matrix[i] != NULL) {
            free(cellules->cellule_matrix[i]);
            cellules->cellule_matrix[i] = NULL;
        }
    }

    free(cellules->cellule_matrix);
    cellules->cellule_matrix = NULL;
}

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

    grille->mode = MODE_FACILE;

    // Classement vide : aucun nom alloué
    for (int i = 0; i < NOMBRE_SCORE; i++) {
        grille->top_scores[i].name = NULL;
        grille->top_scores[i].score = 0;
    }
}

void liberer_grille(Grille *grille){
    if (grille == NULL || grille->cases == NULL) {
        return;
    }

    for (int i = 0; i < NOMBRE_SCORE; i++) {
        if (grille->top_scores[i].name != NULL) {
            free(grille->top_scores[i].name);
            grille->top_scores[i].name = NULL;
        }
    }
    
    for (int i = 0; i < grille->hauteur; i++) {
        if (grille->cases[i] != NULL) {
            free(grille->cases[i]);
            grille->cases[i] = NULL;
        }
    }
    free(grille->cases);
    grille->cases = NULL;
}

void initialiser_cellules(Cellule *cellules){
    for (int i = 0 ; i < cellules->nbr_cellules_verticales ; i ++){
        for (int j = 0 ; j < cellules->nbr_cellules_horizontales ; j ++){
            cellules->cellule_matrix[i][j] = i * cellules->nbr_cellules_horizontales + j;
        }
    }
}

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

void afficher_case(char c){
    const char *couleur = couleur_case(c);
    if (*couleur != '\0') {
        printf("%s%c%s", couleur, c, COULEUR_RESET);
    } else {
        putchar(c);
    }
}

void afficher_labyrinthe(Grille *grille){
    for (int i = 0; i < grille->hauteur; i++) {
        for (int j = 0; j < grille->largeur; j++) {
            afficher_case(grille->cases[i][j]);
        }
        printf("\n");
    }
}

void construire_labyrinthe(Cellule *cellules, Grille *grille){
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

int labyrinthe_complet(Cellule *cellules, Grille *grille){

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
    
    for(int i = 0; i < NOMBRE_SCORE; i++){
        free(grille->top_scores[i].name);
        grille->top_scores[i].name = NULL;
        grille->top_scores[i].score = 0;
    }

    return 1;
}

void chemin_labyrinthe(const char *nom, char *chemin){
    snprintf(chemin, TAILLE_CHEMIN, DOSSIER_LABYRINTHES "/%s", nom);
}

void sauvegarder_labyrinthe(Grille *grille, char *nom_labyrinthe){

    CREATE_DIR(DOSSIER_LABYRINTHES);
    char chemin_complet[TAILLE_CHEMIN];
    chemin_labyrinthe(nom_labyrinthe, chemin_complet);
    FILE *f = fopen(chemin_complet, "w");
    if (!f) {
        perror("Erreur ouverture fichier de sauvegarde");
        return;
    }

    fprintf(f, "%d %d %d\n", grille->hauteur, grille->largeur, grille->mode);

    for (int i = 0; i < grille->hauteur; i++) {
        for (int j = 0; j < grille->largeur; j++) {
            fputc(grille->cases[i][j], f);
        }
        fputc('\n', f);
    }
        
    for (int i = 0; i < NOMBRE_SCORE; i++) {
        char *name = grille->top_scores[i].name;
        int score = grille->top_scores[i].score;
        if (score > 0 && name != NULL && *name != '\0') {
            fprintf(f, "%d %s\n", score, name);
        }
    }

    fclose(f);
}

int charger_labyrinthe(Grille *grille, char *nom_labyrinthe){

    char chemin_complet[TAILLE_CHEMIN];
    chemin_labyrinthe(nom_labyrinthe, chemin_complet);
    FILE *fichier = fopen(chemin_complet, "r");
    if (fichier == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        return 0;
    }

    // Première ligne : "hauteur largeur [mode]" (le mode est absent des anciens fichiers)
    char entete[100];
    int hauteur, largeur, mode = MODE_FACILE;
    if (fgets(entete, sizeof(entete), fichier) == NULL
        || sscanf(entete, "%d %d %d", &hauteur, &largeur, &mode) < 2
        || hauteur < 1 || largeur < 1) {
        fprintf(stderr, "Erreur: Impossible de lire les dimensions du labyrinthe.\n");
        fclose(fichier);
        return 0;
    }
    if (mode != MODE_DIFFICILE) {
        mode = MODE_FACILE;
    }

    allouer_grille(grille, hauteur, largeur);
    grille->mode = mode;

    // Buffer de ligne : largeur + '\r' + '\n' + '\0'
    char *ligne = malloc(largeur + 3);
    if (ligne == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < grille->hauteur; i++) {
        if (fgets(ligne, largeur + 3, fichier) == NULL) {
            fprintf(stderr, "Erreur: Impossible de lire les données du labyrinthe.\n");
            free(ligne);
            fclose(fichier);
            liberer_grille(grille);
            return 0;
        }
        ligne[strcspn(ligne, "\r\n")] = '\0';

        // Copie dans la grille (complétée par des murs si la ligne est trop courte)
        size_t longueur = strlen(ligne);
        for (int j = 0; j < largeur; j++) {
            grille->cases[i][j] = ((size_t)j < longueur) ? ligne[j] : '#';
        }
    }
    free(ligne);

    int score_lu;
    int scores_trouves = 0;
    char nom_lu[100];

    while (scores_trouves < NOMBRE_SCORE) {
        if (fscanf(fichier, "%d %99s", &score_lu, nom_lu) == 2) {
            grille->top_scores[scores_trouves].score = score_lu;
            grille->top_scores[scores_trouves].name = strdup(nom_lu);
            scores_trouves++;
        } else {
            break;
        }
    }

    fclose(fichier);
    return 1;
}

int compter_motif(Grille *grille, char motif){
    int total = 0;
    for (int i = 0; i < grille->hauteur; i++) {
        for (int j = 0; j < grille->largeur; j++) {
            if (grille->cases[i][j] == motif) total++;
        }
    }
    return total;
}

/* Nombre de cases intérieures libres de la grille. */
static int compter_cases_libres(Grille *grille){
    int total = 0;
    for (int i = 1; i < grille->hauteur - 1; i++) {
        for (int j = 1; j < grille->largeur - 1; j++) {
            if (grille->cases[i][j] == MOTIF_VIDE) total++;
        }
    }
    return total;
}

int drop_motif(Grille *grille, char motif){
    // Sans case libre, la boucle de tirage ne se terminerait jamais
    if (compter_cases_libres(grille) == 0) return 0;

    int i_random;
    int j_random;

    do{
        i_random = 1 + rand() % (grille->hauteur - 2);
        j_random = 1 + rand() % (grille->largeur - 2);
    }while(grille->cases[i_random][j_random] != MOTIF_VIDE);

    grille->cases[i_random][j_random] = motif;
    return 1;
}

void placer_motifs(Grille *grille, int mode){
    int surface = grille->hauteur * grille->largeur;
    int difficile = (mode == MODE_DIFFICILE);

    grille->mode = difficile ? MODE_DIFFICILE : MODE_FACILE;

    // La clé en premier pour qu'elle ait toujours une place
    drop_motif(grille, MOTIF_KEY);

    // Difficile : deux fois plus de pièges, deux fois moins de bonus
    // Pourcentages de la surface (4 % / 2 % de pièges, 1 % / 2 % de bonus), calculés en entiers
    int nbr_pieges = surface * (difficile ? 4 : 2) / 100;
    int nbr_bonus  = surface * (difficile ? 1 : 2) / 100;

    for (int k = 0; k < nbr_pieges; k++) drop_motif(grille, MOTIF_TRAP);
    for (int k = 0; k < nbr_bonus; k++)  drop_motif(grille, MOTIF_BOOST);
}

void consulter_score_labyrinthe(Grille *grille, char *nom_labyrinthe){
    
    printf("\nLe Top 10 des scores du labyrinthe %s est:\n", nom_labyrinthe);
    int scores_affiches = 0;
    for(int i = 0; i < NOMBRE_SCORE; i++){
        if (grille->top_scores[i].name == NULL || grille->top_scores[i].score <= 0) {
            continue;
        }
        printf("Score %d : %d  Joueur : %s \n", i+1, grille->top_scores[i].score, grille->top_scores[i].name);
        scores_affiches++;
    }
    if (scores_affiches == 0) {
        printf("Aucun score enregistré pour ce labyrinthe.\n");
    }
}
