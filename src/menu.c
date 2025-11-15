#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include "menu.h"

int get_action_initial(){
    int choix;

    printf("\n Choississez une action parmi ce qui suit : \n");
    printf("\n 1 - Créer un labyrinthe\n");
    printf("\n 2 - Charger un labyrinthe\n");
    printf("\n 3 - Jouer\n");
    printf("\n 4 - Consulter les scores d'un labyrinthe\n");
    printf("\n 5 - Quitter\n");

    scanf("%d", &choix);
    
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

    if (!d || !fichiers_trouves) {
        printf("Le labyrinthe choisie est introuvable ou vide.\n");
    }

    scanf("%s", nom_labyrinthe); 
}

int get_mode_jeu() {
    int mode;

    printf("Choisissez un mode de jeu :\n");
    printf("Mode 1 : Facile\n");
    printf("Mode 2 : Difficile\n");

    do {
        scanf("%d", &mode);
    } while (mode != 1 && mode != 2); 

    printf("Mode de jeu choisi : %d\n", mode);
    return mode;
}

void get_parametres_labyrinthe(int *hauteur, int *largeur, char *nom){
    
    printf("\nHauteur du labyrinthe : ");
    if (scanf("%d", hauteur) != 1) { 
        printf("Erreur de saisie pour la hauteur.\n");
        while (getchar() != '\n');
        *hauteur = 0; 
        return; 
    }
    
    do {
        printf("\nLargeur du labyrinthe (Impaire) : ");
        
        if (scanf("%d", largeur) != 1) { 
            printf("\nErreur: La saisie n'est pas un nombre. Réessayez...\n");
            while (getchar() != '\n');
            *largeur = 2; 
        } 


        if ((*largeur % 2) != 1) {
            printf("\nLa largeur du labyrinthe doit etre un entier impaire, réessayez.\n");
        }
        
    } while ((*largeur % 2) != 1);


    printf("\nNom du labyrinthe : ");
    if (scanf("%s", nom) != 1) { 
        printf("Erreur de saisie pour le nom.\n");
    }
}