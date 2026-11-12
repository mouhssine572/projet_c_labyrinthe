#ifndef MENU_H
#define MENU_H

/* Taille des buffers de noms (joueur, labyrinthe), '\0' compris. */
#define TAILLE_NOM 50
#define TAILLE_NOM_MAX 49

/* Dimensions autorisées pour un labyrinthe (valeurs impaires). */
#define DIMENSION_MIN 3
#define DIMENSION_MAX 201

/* Transforme une macro numérique en chaîne (utile pour les formats scanf). */
#define STR_(x) #x
#define STR(x) STR_(x)

/**
 * @brief Lit un entier sur l'entrée standard, en redemandant tant que la saisie n'est pas un nombre.
 * * Le reste de la ligne est vidé. Le programme se termine proprement si l'entrée est fermée.
 * @return int L'entier saisi.
 */
int lire_entier(void);

/**
 * @brief Lit un mot (sans espace) d'au plus TAILLE_NOM_MAX caractères.
 * @param destination Buffer d'au moins TAILLE_NOM octets.
 */
void lire_mot(char *destination);

/**
 * @brief Affiche le menu principal et récupère le choix initial de l'utilisateur.
 * * Redemande tant que le choix n'est pas compris entre 1 et 5.
 * @return int Le numéro de l'action choisie (1 à 5).
 */
int get_action_initial();

/**
 * @brief Liste les fichiers de labyrinthe existants et demande à l'utilisateur de choisir un nom de fichier.
 * *Tente d'ouvrir le répertoire "labyrinthe" et affiche tous les fichiers trouvés.
 * @param nom_labyrinthe Buffer d'au moins TAILLE_NOM octets où stocker le nom saisi.
 */
void get_nom_labyrinthe(char *nom_labyrinthe);

/**
 * @brief Demande à l'utilisateur de choisir un mode de jeu et s'assure que l'entrée est valide (1 ou 2).
 * @return int Le mode de jeu choisi (1 pour Facile, 2 pour Difficile).
 */
int get_mode_jeu();

/**
 * @brief Demande et récupère la hauteur, la largeur et le nom du labyrinthe.
 * * Hauteur et largeur doivent être des entiers impairs entre DIMENSION_MIN et DIMENSION_MAX.
 * @param hauteur Pointeur vers l'entier qui stockera la hauteur.
 * @param largeur Pointeur vers l'entier qui stockera la largeur.
 * @param nom Buffer d'au moins TAILLE_NOM octets qui stockera le nom du labyrinthe.
 */
void get_parametres_labyrinthe(int *hauteur, int *largeur, char *nom);

#endif
