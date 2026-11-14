#ifndef MENU_H
#define MENU_H

/* Taille des buffers de noms (joueur, labyrinthe), '\0' compris. */
#define TAILLE_NOM 50
#define TAILLE_NOM_MAX 49

/* Fichier qui garde les noms des joueurs d'une session à l'autre. */
#define FICHIER_JOUEURS "joueurs.txt"
#define NBR_JOUEURS_MAX 20

/* Nombre maximal de labyrinthes proposés dans la liste de choix. */
#define NBR_LABYRINTHES_MAX 100

/* Nombre maximal d'options affichées à la fois dans un menu (au-delà, la liste défile). */
#define TAILLE_FENETRE_MENU 10

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
 * @brief Affiche un menu navigable avec les flèches haut/bas, validé par Entrée.
 * * On peut aussi taper directement le numéro d'une option (1 à 9).
 * * Échap choisit la dernière option, qui doit donc être « Revenir au menu ».
 * * Au-delà de TAILLE_FENETRE_MENU options, seule une partie est affichée et la liste défile.
 * * Si l'entrée n'est pas un terminal, l'utilisateur saisit simplement le numéro.
 * @param titre Titre affiché au-dessus des options.
 * @param options Libellés des options.
 * @param nbr_options Nombre d'options.
 * @return int L'indice (à partir de 0) de l'option choisie.
 */
int choisir_option(const char *titre, const char *options[], int nbr_options);

/**
 * @brief Affiche le menu principal et récupère le choix initial de l'utilisateur.
 * * Navigation avec les flèches (voir choisir_option).
 * @return int Le numéro de l'action choisie (1 à 7).
 */
int get_action_initial(void);

/**
 * @brief Indique si au moins un labyrinthe est disponible dans le répertoire "labyrinthe".
 * @return int 1 si au moins un fichier de labyrinthe existe, 0 sinon.
 */
int labyrinthe_disponible(void);

/**
 * @brief Indique si un labyrinthe portant ce nom existe déjà dans le répertoire "labyrinthe".
 * @param nom Nom du fichier de labyrinthe.
 * @return int 1 s'il existe, 0 sinon.
 */
int labyrinthe_existe(const char *nom);

/**
 * @brief Vérifie qu'un nom ne contient que des lettres (sans accent), des chiffres, '_' ou '-'.
 * * Empêche les chemins comme "../fichier" qui feraient écrire en dehors du dossier labyrinthe/.
 * @param nom Nom à vérifier.
 * @return int 1 si le nom est valide (et non vide), 0 sinon.
 */
int nom_valide(const char *nom);

/**
 * @brief Indique si un fichier du dossier des labyrinthes peut être un labyrinthe du jeu.
 * * Le nom doit être valide (voir nom_valide) et tenir dans TAILLE_NOM_MAX caractères.
 *   Écarte ".", "..", et les fichiers parasites comme ".DS_Store" ou "PARIS~".
 * @param nom_fichier Nom du fichier (sans le dossier).
 * @return int 1 si c'est un nom de labyrinthe possible, 0 sinon.
 */
int est_nom_labyrinthe(const char *nom_fichier);

/**
 * @brief Liste les labyrinthes du répertoire "labyrinthe" (ordre alphabétique) dans un menu
 *        navigable avec les flèches, suivi de l'option « Revenir au menu ».
 * @param nom_labyrinthe Buffer d'au moins TAILLE_NOM octets où stocker le nom choisi.
 * @return int 0 si l'utilisateur revient au menu (ou si aucun labyrinthe n'existe), 1 sinon.
 */
int get_nom_labyrinthe(char *nom_labyrinthe);

/**
 * @brief Charge les noms des joueurs déjà connus, du plus récent au plus ancien.
 * * Les noms viennent du fichier FICHIER_JOUEURS. S'il n'existe pas encore, ils sont retrouvés
 *   une seule fois dans les classements des labyrinthes existants, puis enregistrés dans
 *   FICHIER_JOUEURS pour les lancements suivants. Sans doublon.
 * @param noms Tableau où ranger les noms.
 * @return int Le nombre de noms trouvés (au plus NBR_JOUEURS_MAX).
 */
int charger_noms_joueurs(char noms[NBR_JOUEURS_MAX][TAILLE_NOM]);

/**
 * @brief Enregistre un nom dans FICHIER_JOUEURS en tête de liste (le plus récent en premier).
 * @param nom Nom du joueur.
 */
void enregistrer_nom_joueur(const char *nom);

/**
 * @brief Demande le nom du joueur.
 * * Menu navigable avec les flèches : les joueurs déjà connus (sessions précédentes),
 *   puis « Nouveau joueur » (le nom est alors saisi au clavier) et « Revenir au menu ».
 * @param nom_joueur Buffer d'au moins TAILLE_NOM octets où stocker le nom choisi.
 * @return int 0 si l'utilisateur a demandé à revenir au menu, 1 sinon.
 */
int get_nom_joueur(char *nom_joueur);

/**
 * @brief Demande à l'utilisateur de choisir un mode de jeu (menu navigable avec les flèches).
 * @return int Le mode choisi (MODE_FACILE, MODE_NORMAL ou MODE_DIFFICILE), CHOIX_RETOUR pour revenir au menu.
 */
int get_mode_jeu(void);

/**
 * @brief Demande et récupère la hauteur, la largeur et le nom du labyrinthe.
 * * Hauteur et largeur doivent être des entiers impairs entre DIMENSION_MIN et DIMENSION_MAX.
 * @param hauteur Pointeur vers l'entier qui stockera la hauteur.
 * @param largeur Pointeur vers l'entier qui stockera la largeur.
 * @param nom Buffer d'au moins TAILLE_NOM octets qui stockera le nom du labyrinthe.
 * * Le nom doit être valide (voir nom_valide). S'il est déjà pris, on demande avant d'écraser.
 * @return int 0 si l'utilisateur a demandé à revenir au menu à l'une des questions, 1 sinon.
 */
int get_parametres_labyrinthe(int *hauteur, int *largeur, char *nom);

/**
 * @brief Affiche les règles du jeu et la signification de chaque caractère, puis attend Entrée.
 */
void afficher_regles(void);

#endif
