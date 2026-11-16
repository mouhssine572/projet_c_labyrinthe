#ifndef SONS_H
#define SONS_H

/* Effets sonores de la version 2D. Ils sont synthétisés au démarrage (aucun fichier audio
 * n'est nécessaire) et joués avec SDL2_mixer. */
typedef enum {
    SON_PAS,       /* Un pas du joueur */
    SON_MUR,       /* Le joueur se cogne contre un mur */
    SON_PORTE,     /* Le joueur essaie de sortir sans la clé */
    SON_CLE,       /* Clé ramassée */
    SON_PIEGE,     /* Piège : +points */
    SON_BONUS,     /* Bonus : -points */
    SON_FANTOME,   /* Le fantôme attrape le joueur (score doublé) */
    SON_OGRE,      /* L'ogre attrape le joueur (partie perdue) */
    SON_VICTOIRE,  /* Sortie atteinte avec la clé */
    SON_MENU,      /* Déplacement de la sélection dans un menu */
    SON_VALIDER,   /* Choix validé dans un menu */
    NBR_SONS
} Son;

/**
 * @brief Ouvre la sortie audio et fabrique tous les sons.
 * @return int 1 si le son fonctionne, 0 sinon (le jeu continue alors en silence).
 */
int sons_initialiser(void);

/**
 * @brief Joue un son (sans effet si le son est coupé ou indisponible).
 * @param son Le son à jouer.
 */
void sons_jouer(Son son);

/**
 * @brief Coupe ou remet le son.
 */
void sons_basculer_muet(void);

/**
 * @brief Indique si le son est coupé.
 * @return int 1 si le son est coupé, 0 sinon.
 */
int sons_muet(void);

/**
 * @brief Libère les sons et ferme la sortie audio.
 */
void sons_liberer(void);

#endif
