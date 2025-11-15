#ifndef MENU_H
#define MENU_H

//Fonction pour afficher le menu itnitiale et renvoie le choix du joueur
int get_action_initial();

void get_nom_labyrinthe(char *nom_labyrinthe);

void get_parametres_labyrinthe(int *hauteur, int *largeur, char *nom);

int get_mode_jeu();

#endif