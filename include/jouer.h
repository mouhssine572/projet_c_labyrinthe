#ifndef JOUER_H
#define JOUER_H
#include "labyrinthe.h"


#define MOTIF_KEY '$'
#define MOTIF_TRAP '@'
#define MOTIF_BOOST '&'

typedef struct {
    int x;
    int y;
    int have_key;
    int score;
} Joueur;

/**
 * @brief Initialise les attributs du joueur à leurs valeurs de départ.
 * @param joueur Pointeur vers la structure Joueur à initialiser.
 */
void initialiser_joueur(Joueur *joueur);

/**
 * @brief Marque la position actuelle du joueur ('*') dans la grille et incrémente le score.
 * @param grille Pointeur vers la Grille où afficher le joueur.
 * @param joueur Pointeur vers la structure Joueur.
 */
void positionner_joueur(Grille *grille, Joueur *joueur);

/**
 * @brief Vérifie si un mouvement vers une nouvelle coordonnée est valide et gère les interactions (murs, clé, pièges, trésors).
 * @param grille Pointeur vers la Grille de jeu.
 * @param x Coordonnée verticale (ligne) cible.
 * @param y Coordonnée horizontale (colonne) cible.
 * @param joueur Pointeur vers la structure Joueur dont les attributs peuvent être modifiés.
 * @return int 1 si le mouvement est valide et a été traité, 0 sinon.
 */
int mouvement_valide(Grille *grille, int x, int y, Joueur *joueur);

/**
 * @brief Lit l'entrée utilisateur et déplace le joueur.
 * * Fait appel à la fonction **mouvement_valide** pour verifier si le mouvement est valide.
 * * Fait appel à la fonction **positionner_joueur** pour mettre à jour la position du joueur
 * @param grille Pointeur vers la Grille de jeu.
 * @param joueur Pointeur vers la structure Joueur à déplacer.
 */
void mouvement_joueur(Grille *grille, Joueur *joueur);

/**
 * @brief Boucle principale du jeu, gérant l'affichage et les mouvements jusqu'à la fin du jeu.
 * * Fait appel à la fonction **joueur_gagne** pour verifier que le joueur a la clé est arrivé à la sortie du labyrinthe.
 * @param grille Pointeur vers la Grille de jeu.
 * @param joueur Pointeur vers la structure Joueur.
 */
void jouer_labyrinthe(Grille *grille, Joueur *joueur);

/**
 * @brief Vérifie si le joueur a gagné la partie (possède la clé et est sur la sortie).
 * @param grille Pointeur vers la Grille de jeu.
 * @param joueur Pointeur vers la structure Joueur.
 * @return int 1 si le joueur a gagné, 0 sinon.
 */
int joueur_gagne(Grille *grille, Joueur *joueur);

/**
 * @brief Récupère le score actuel du joueur.
 * @param joueur Pointeur vers la structure Joueur.
 * @return int Le score actuel du joueur.
 */
int get_score_joueur(Joueur *joueur);

/**
 * @brief Vérifie si le score du joueur est un nouveau Top Score et met à jour le classement de la grille.
 * @param grille Pointeur vers la Grille dont le classement doit être mis à jour.
 * @param joueur Pointeur vers le joueur qui vient de terminer la partie.
 * @param nom_labyrinthe Nom du joueur à insérer dans le classement.
 * @return int 1 si le classement a été mis à jour, 0 sinon.
 */
int set_score_joueur(Grille *grille, Joueur *joueur, char *nom_labyrinthe);

/**
 * @brief Sauvegarde le classement des scores contenu dans la grille_de_jeu dans le fichier du labyrinthe.
 * @param grille_de_jeu Pointeur vers la Grille qui contient le classement mis à jour.
 * @param nom_labyrinthe Nom du fichier de sauvegarde du labyrinthe.
 */
void sauvegarder_score(Grille *grille_de_jeu, char *nom_labyrinthe);

#endif