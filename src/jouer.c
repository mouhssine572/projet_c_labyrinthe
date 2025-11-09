#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jouer.h"
#include "labyrinthe.h"

void initialiser_joueur(Joueur *joueur){
    joueur -> x = 0;
    joueur -> y = 1;
    joueur -> have_key = 0;
    joueur -> score = 0;
}

void positionner_joueur(Grille *grille, Joueur *joueur){
    int position_x_joueur = joueur -> x;
    int position_y_joueur = joueur -> y;
    int score_joueur = joueur -> score;
    grille -> cases[position_x_joueur][position_y_joueur] = '*';
    joueur -> score = score_joueur + 1;

}

int mouvement_valide(Grille *grille, int x, int y, Joueur *joueur){

    int n =  joueur -> score;
    if(x >= grille -> hauteur || x < 0) return 0;
    if(y >= grille -> largeur || y < 0) return 0;

    if(grille -> cases[x][y] == '#') return 0;
    if((grille -> cases[x][y] == '-') && joueur -> have_key == 0) return 0;
    if(grille -> cases[x][y] == MOTIF_KEY) joueur -> have_key = 1;
    if(grille -> cases[x][y] == MOTIF_TRAP) joueur -> score = n + 6;
    if(grille -> cases[x][y] == MOTIF_BOOST){
        if (n < 6){
            n = 6;
        }
        joueur -> score = n - 6;
    }
    return 1;
}

void mouvement_joueur(Grille *grille, Joueur *joueur){
    char mvt;

    int x = joueur->x; 
    int y = joueur->y;

    if (scanf(" %c", &mvt) != 1) return;

    grille -> cases[joueur -> x][joueur -> y] = ' ';

    switch(mvt){
        case 'z':
            x--;
            break;
        case 'q':
            y--;
            break;
        case 'd':
            y++;
            break;
        case 's':
            x++;
            break;
        default:
            printf("Touche invalide.\n");
            return;
    }
    
    if(mouvement_valide(grille, x, y, joueur)){
        joueur -> x = x;
        joueur -> y = y;
    }

    positionner_joueur(grille, joueur);
    printf("\n Votre score est : %d \n", get_score_joueur(joueur));
}

void jouer_labyrinthe(Grille *grille, Joueur *joueur){
    
    do {
        afficher_labyrinthe(grille);
        mouvement_joueur(grille, joueur);                 
    } while(joueur_gagne(grille, joueur) == 0);

}

int joueur_gagne(Grille *grille, Joueur *joueur){
    if(joueur -> have_key == 0) return 0;
    if(grille->cases[grille->hauteur - 1][grille->largeur - 2] != '*') return 0;
    return 1;
}

int get_score_joueur(Joueur *joueur){
    return joueur -> score; 
}

int set_score_joueur(Grille *grille, Joueur *joueur) {
    
    int score_joueur = get_score_joueur(joueur);
    int maj_scores = 0;
    int index_insertion = -1;

    for (int i = 0; i < NOMBRE_SCORE; i++) {
        int score_actuel = grille->scores[i];
        if (score_actuel == 0 || score_joueur < score_actuel) {
            index_insertion = i;
            break;
        }
    }

    if (index_insertion != -1) {
        
        for (int i = NOMBRE_SCORE - 1; i > index_insertion; i--) {
            grille->scores[i] = grille->scores[i - 1];
        }
        
        grille->scores[index_insertion] = score_joueur;
        maj_scores = 1;
    }
    
    return maj_scores;
}

void sauvegarder_score(Grille *grille_de_jeu, char *nom_labyrinthe) {

    Grille grille_sauvegarde;
    
    charger_labyrinthe(&grille_sauvegarde, nom_labyrinthe); 

    memcpy(grille_sauvegarde.scores, grille_de_jeu->scores, sizeof(grille_sauvegarde.scores));
    
    CREATE_DIR("labyrinthe");
    char chemin_complet[100];
    snprintf(chemin_complet, 100, "labyrinthe/%s", nom_labyrinthe);
    FILE *f = fopen(chemin_complet, "w");
    if (!f) {
        perror("Erreur ouverture fichier de sauvegarde");
        liberer_grille(&grille_sauvegarde);
        return;
    }

    fprintf(f, "%d %d\n", grille_sauvegarde.hauteur, grille_sauvegarde.largeur);

    for (int i = 0; i < grille_sauvegarde.hauteur; i++) {
        for (int j = 0; j < grille_sauvegarde.largeur; j++) {
            fputc(grille_sauvegarde.cases[i][j], f);
        }
        fputc('\n', f);
    }

    for (int i = 0; i < NOMBRE_SCORE; i++) {
        int score = grille_sauvegarde.scores[i];
        if (score >= 1) { 
                fprintf(f, "%d\n", score);
        }
    }

    fclose(f);
    liberer_grille(&grille_sauvegarde);
}
