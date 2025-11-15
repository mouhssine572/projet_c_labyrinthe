#ifndef MENU_H
#define MENU_H

/**
 * @brief Affiche le menu principal et récupère le choix initial de l'utilisateur.
 * @return int Le numéro de l'action choisie (1 à 5).
 */
int get_action_initial();

/**
 * @brief Liste les fichiers de labyrinthe existants et demande à l'utilisateur de choisir un nom de fichier.
 * *Tente d'ouvrir le répertoire "labyrinthe" et affiche tous les fichiers trouvés.
 * @param nom_labyrinthe Pointeur vers un tableau de caractères où stocker le nom du labyrinthe saisi par l'utilisateur.
 */
void get_nom_labyrinthe(char *nom_labyrinthe);

/**
 * @brief Demande à l'utilisateur de choisir un mode de jeu et s'assure que l'entrée est valide (1 ou 2).
 * @return int Le mode de jeu choisi (1 pour Facile, 2 pour Difficile).
 */
int get_mode_jeu();

/**
 * @brief Demande et récupère la hauteur, la largeur et le nom du labyrinthe.
 * * S'assure que la largeur saisie est un entier impair et gère les erreurs de saisie numérique.
 * @param hauteur Pointeur vers l'entier qui stockera la hauteur.
 * @param largeur Pointeur vers l'entier qui stockera la largeur (doit être impaire).
 * @param nom Pointeur vers le tableau de caractères qui stockera le nom du labyrinthe.
 */
void get_parametres_labyrinthe(int *hauteur, int *largeur, char *nom);

#endif