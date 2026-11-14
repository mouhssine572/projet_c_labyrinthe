#ifndef TERMINAL_H
#define TERMINAL_H

/* Codes couleur ANSI utilisés pour l'affichage du labyrinthe. */
#define COULEUR_RESET   "\033[0m"
#define COULEUR_GRAS    "\033[1m"
#define COULEUR_SOMBRE  "\033[90m"
#define COULEUR_ROUGE   "\033[1;31m"
#define COULEUR_VERT    "\033[1;32m"
#define COULEUR_JAUNE   "\033[1;33m"
#define COULEUR_BLEU    "\033[1;34m"
#define COULEUR_MAGENTA "\033[1;35m"
#define COULEUR_CYAN    "\033[1;36m"
#define COULEUR_BLANC   "\033[1;37m"

/* Valeur renvoyée par lire_touche quand l'entrée est fermée (Ctrl+D, Ctrl+Z, Ctrl+C). */
#define TOUCHE_FIN (-1)

/* Valeur renvoyée par lire_touche pour la touche Échap seule. */
#define TOUCHE_ECHAP 27

/**
 * @brief Prépare le terminal : active les codes ANSI et l'UTF-8 sous Windows.
 * * À appeler une fois au début du programme. Sans effet particulier sous Linux/macOS.
 */
void initialiser_terminal(void);

/**
 * @brief Lit une touche sans attendre Entrée (_getch sous Windows, termios sous Linux/macOS).
 * * Les flèches du clavier sont converties en z (haut), s (bas), q (gauche), d (droite).
 * * La touche Échap seule renvoie TOUCHE_ECHAP (sous Linux/macOS, on attend un court instant
 *   pour la distinguer du début d'une séquence de flèche).
 * * Si l'entrée standard n'est pas un terminal (fichier, tube, tests), la lecture se fait
 *   normalement caractère par caractère en ignorant les espaces et retours à la ligne.
 * @return int Le caractère lu, ou TOUCHE_FIN si l'entrée est fermée.
 */
int lire_touche(void);

/**
 * @brief Indique si l'entrée standard est un terminal interactif (clavier).
 * @return int 1 si c'est un terminal, 0 si l'entrée est redirigée (fichier, tube, tests).
 */
int entree_est_terminal(void);

/**
 * @brief Efface l'écran et replace le curseur en haut à gauche.
 */
void effacer_ecran(void);

/**
 * @brief Renvoie le code couleur ANSI associé à un caractère du labyrinthe.
 * @param c Caractère de la grille.
 * @return const char* Code couleur ("" si le caractère reste dans la couleur par défaut).
 */
const char *couleur_case(char c);

#endif
