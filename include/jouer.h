#ifndef JOUER_H
#define JOUER_H
#include "labyrinthe.h"


/* Nombre de monstres dans une partie (l'ogre et le fantôme). */
#define NBR_MONSTRES_MAX 2

/* Valeurs de retour de mouvement_joueur. */
#define MVT_FIN 0       /* Entrée standard fermée */
#define MVT_JOUE 1      /* Un tour a été joué */
#define MVT_INVALIDE 2  /* Touche invalide : aucun tour joué */
#define MVT_RETOUR 3    /* Le joueur abandonne pour revenir au menu */

/* Touche permettant d'abandonner la partie et de revenir au menu. */
#define MVT_TOUCHE_RETOUR '0'

/* Effets des pièges et des bonus sur le score. */
#define PENALITE_PIEGE 6
#define GAIN_BONUS 6

/* Mode Difficile : distance (en cases de couloir) à partir de laquelle l'ogre chasse le joueur. */
#define RAYON_CHASSE 10

/* Taille du message affiché sous le labyrinthe. */
#define TAILLE_MESSAGE 160

typedef struct {
    int x;
    int y;
    int have_key;       /* 1 si le joueur a ramassé la clé */
    int score;
    char case_dessous;  /* Caractère de la grille caché sous le joueur, remis quand il part */
    char message[TAILLE_MESSAGE]; /* Dernier événement, affiché sous le labyrinthe */
} Joueur;

/* Un monstre (ogre ou fantôme). Il n'est pas écrit dans la grille : il est seulement
 * dessiné par-dessus au moment de l'affichage (voir afficher_jeu). */
typedef struct {
    int x;
    int y;
    char symbole;      /* MOTIF_OGRE ou MOTIF_FANTOME */
    int traverse_murs; /* 1 si le monstre peut passer à travers les murs (fantôme) */
    int chasseur;      /* 1 si le monstre se rapproche du joueur quand il est proche (ogre en Difficile) */
    int actif;         /* 0 si aucune case n'a permis de placer le monstre */
} Monstre;

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
 * @brief Vérifie si un mouvement vers une nouvelle coordonnée est valide et gère les interactions
 * (murs, sortie fermée, clé, pièges, bonus).
 * * Les pièges restent en place ; la clé et les bonus disparaissent une fois ramassés.
 * @param grille Pointeur vers la Grille de jeu.
 * @param x Coordonnée verticale (ligne) cible.
 * @param y Coordonnée horizontale (colonne) cible.
 * @param joueur Pointeur vers la structure Joueur dont les attributs peuvent être modifiés.
 * @return int 1 si le mouvement est valide et a été traité, 0 sinon.
 */
int mouvement_valide(Grille *grille, int x, int y, Joueur *joueur);

/**
 * @brief Lit une touche (sans attendre Entrée) et déplace le joueur.
 * * Fait appel à la fonction **mouvement_valide** pour verifier si le mouvement est valide.
 * * Fait appel à la fonction **positionner_joueur** pour mettre à jour la position du joueur.
 * * Une touche invalide ne déplace pas le joueur et ne compte pas de point.
 * @param grille Pointeur vers la Grille de jeu.
 * @param joueur Pointeur vers la structure Joueur à déplacer.
 * @return int MVT_JOUE si un tour a été joué, MVT_INVALIDE si la touche est invalide,
 * MVT_RETOUR si le joueur demande à revenir au menu, MVT_FIN si l'entrée standard est fermée.
 */
int mouvement_joueur(Grille *grille, Joueur *joueur);

/**
 * @brief Initialise un monstre (non placé : actif vaut 0, non chasseur).
 * @param monstre Pointeur vers le Monstre à initialiser.
 * @param symbole Caractère affiché (MOTIF_OGRE ou MOTIF_FANTOME).
 * @param traverse_murs 1 si le monstre peut traverser les murs, 0 sinon.
 */
void initialiser_monstre(Monstre *monstre, char symbole, int traverse_murs);

/**
 * @brief Place un monstre sur une case libre de l'intérieur du labyrinthe, loin du joueur si possible.
 * @param grille Pointeur vers la Grille de jeu (elle n'est pas modifiée).
 * @param monstre Pointeur vers le Monstre à placer (actif vaut 0 si aucune case libre n'existe).
 * @param joueur Pointeur vers le Joueur (pour éviter de placer le monstre près de lui).
 */
void placer_monstre(Grille *grille, Monstre *monstre, Joueur *joueur);

/**
 * @brief Déplace un monstre d'une case, sans sortir de l'intérieur du labyrinthe.
 * * Un monstre chasseur se rapproche du joueur (par le plus court chemin) si celui-ci est
 *   à moins de RAYON_CHASSE cases de couloir ; sinon le monstre se déplace au hasard.
 * * Les murs sont évités, sauf si le monstre peut les traverser. S'il est bloqué, il reste sur place.
 * @param grille Pointeur vers la Grille de jeu (elle n'est pas modifiée).
 * @param monstre Pointeur vers le Monstre à déplacer.
 * @param joueur Pointeur vers le Joueur.
 * @return int 1 si le monstre arrive sur la case du joueur, 0 sinon.
 */
int deplacer_monstre(Grille *grille, Monstre *monstre, Joueur *joueur);

/**
 * @brief Indique si le monstre et le joueur sont sur la même case.
 * @return int 1 si le joueur est attrapé, 0 sinon.
 */
int monstre_attrape_joueur(Monstre *monstre, Joueur *joueur);

/**
 * @brief Double le score du joueur attrapé par le fantôme, puis fait réapparaître le fantôme ailleurs.
 * @param grille Pointeur vers la Grille de jeu.
 * @param fantome Pointeur vers le fantôme.
 * @param joueur Pointeur vers le Joueur attrapé.
 */
void fantome_attrape_joueur(Grille *grille, Monstre *fantome, Joueur *joueur);

/**
 * @brief Affiche le labyrinthe en couleur avec les monstres dessinés par-dessus
 * (le dernier du tableau est au premier plan).
 * @param grille Pointeur vers la Grille de jeu (restaurée à l'identique après l'affichage).
 * @param monstres Tableau de pointeurs vers les monstres.
 * @param nbr_monstres Nombre de monstres (au plus NBR_MONSTRES_MAX).
 */
void afficher_jeu(Grille *grille, Monstre *monstres[], int nbr_monstres);

/**
 * @brief Boucle principale du jeu : efface l'écran, affiche la carte, la légende et le score,
 * puis gère les mouvements jusqu'à la fin de la partie.
 * * Fait appel à la fonction **joueur_gagne** pour verifier que le joueur a la clé et est arrivé à la sortie.
 * * En mode Difficile seulement, l'ogre ('G', chasseur) et le fantôme ('F') sont présents
 *   et se déplacent après chaque tour. En mode Facile, il n'y a aucun monstre.
 * * Si l'ogre attrape le joueur, la partie est perdue ; si le fantôme l'attrape, le score est doublé.
 * @param grille Pointeur vers la Grille de jeu.
 * @param joueur Pointeur vers la structure Joueur.
 * @return int 1 si le joueur a gagné, 0 si la partie est perdue (ogre) ou abandonnée.
 */
int jouer_labyrinthe(Grille *grille, Joueur *joueur);

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
 * @param nom_joueur Nom du joueur à insérer dans le classement (il est dupliqué).
 * @return int La place obtenue dans le classement (1 à NOMBRE_SCORE), 0 si le score n'y entre pas.
 */
int set_score_joueur(Grille *grille, Joueur *joueur, char *nom_joueur);

/**
 * @brief Sauvegarde le classement des scores contenu dans la grille_de_jeu dans le fichier du labyrinthe.
 * @param grille_de_jeu Pointeur vers la Grille qui contient le classement mis à jour.
 * @param nom_labyrinthe Nom du fichier de sauvegarde du labyrinthe.
 */
void sauvegarder_score(Grille *grille_de_jeu, char *nom_labyrinthe);

#endif
