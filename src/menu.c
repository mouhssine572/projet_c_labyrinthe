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

