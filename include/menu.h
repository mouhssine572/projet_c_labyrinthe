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

/* Saisie permettant de revenir au menu principal depuis n'importe quelle question. */
#define CHOIX_RETOUR 0    /* Pour les questions numériques */
#define MOT_RETOUR "0"    /* Pour les questions de nom */

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
 * * Redemande tant que le choix n'est pas compris entre 1 et 6.
 * @return int Le numéro de l'action choisie (1 à 6).
 */
int get_action_initial();

/**
 * @brief Indique si au moins un labyrinthe est disponible dans le répertoire "labyrinthe".
 * @return int 1 si au moins un fichier de labyrinthe existe, 0 sinon.
 */
int labyrinthe_disponible(void);

/**
 * @brief Liste les fichiers de labyrinthe existants et demande à l'utilisateur de choisir un nom de fichier.
 * *Tente d'ouvrir le répertoire "labyrinthe" et affiche tous les fichiers trouvés.
 * @param nom_labyrinthe Buffer d'au moins TAILLE_NOM octets où stocker le nom saisi.
 * @return int 0 si l'utilisateur a demandé à revenir au menu (MOT_RETOUR), 1 sinon.
 */
int get_nom_labyrinthe(char *nom_labyrinthe);

/**
 * @brief Demande le nom du joueur.
 * @param nom_joueur Buffer d'au moins TAILLE_NOM octets où stocker le nom saisi.
 * @return int 0 si l'utilisateur a demandé à revenir au menu (MOT_RETOUR), 1 sinon.
 */
int get_nom_joueur(char *nom_joueur);

/**
 * @brief Demande à l'utilisateur de choisir un mode de jeu et s'assure que l'entrée est valide (1, 2 ou CHOIX_RETOUR).
 * @return int Le mode de jeu choisi (1 pour Facile, 2 pour Difficile), CHOIX_RETOUR pour revenir au menu.
 */
int get_mode_jeu();

/**
 * @brief Demande et récupère la hauteur, la largeur et le nom du labyrinthe.
 * * Hauteur et largeur doivent être des entiers impairs entre DIMENSION_MIN et DIMENSION_MAX.
 * @param hauteur Pointeur vers l'entier qui stockera la hauteur.
 * @param largeur Pointeur vers l'entier qui stockera la largeur.
 * @param nom Buffer d'au moins TAILLE_NOM octets qui stockera le nom du labyrinthe.
 * @return int 0 si l'utilisateur a demandé à revenir au menu à l'une des questions, 1 sinon.
 */
int get_parametres_labyrinthe(int *hauteur, int *largeur, char *nom);

/**
 * @brief Affiche les règles du jeu et la signification de chaque caractère, puis attend Entrée.
 */
void afficher_regles(void);

#endif
