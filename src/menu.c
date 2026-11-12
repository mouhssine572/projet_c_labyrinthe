#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "menu.h"
#include "jouer.h"

/* Vide le reste de la ligne courante sur l'entrée standard. */
static void vider_ligne(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/* Termine proprement le programme si l'entrée standard est fermée (Ctrl+D / Ctrl+Z). */
static void verifier_fin_entree(int resultat_scanf) {
    if (resultat_scanf == EOF) {
        printf("\nFin de l'entrée, le jeu se termine.\n");
        exit(EXIT_SUCCESS);
    }
}

/* Indique si le mot saisi demande un retour au menu. */
static int est_retour(const char *mot) {
    return strcmp(mot, MOT_RETOUR) == 0;
}

int lire_entier(void) {
    int valeur;
    int res;

    while (1) {
        res = scanf("%d", &valeur);
        verifier_fin_entree(res);
        vider_ligne();
        if (res == 1) {
            return valeur;
        }
        printf("Saisie invalide, entrez un nombre : ");
    }
}

void lire_mot(char *destination) {
    int res = scanf("%" STR(TAILLE_NOM_MAX) "s", destination);
    verifier_fin_entree(res);
    vider_ligne();
}

int get_action_initial(){
    int choix;

    printf("\n Choississez une action parmi ce qui suit : \n");
    printf("\n 1 - Créer un labyrinthe\n");
    printf("\n 2 - Charger un labyrinthe\n");
    printf("\n 3 - Jouer\n");
    printf("\n 4 - Consulter les scores d'un labyrinthe\n");
    printf("\n 5 - Consulter les règles du jeu\n");
    printf("\n 6 - Quitter\n");

    choix = lire_entier();
    while (choix < 1 || choix > 6) {
        printf("Choix invalide, entrez un nombre entre 1 et 6 : ");
        choix = lire_entier();
    }

    return choix;
}

int labyrinthe_disponible(void) {
    DIR *d = opendir("labyrinthe");
    int trouve = 0;

    if (d) {
        struct dirent *dir;

        while (!trouve && (dir = readdir(d)) != NULL) {
            if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0) {
                trouve = 1;
            }
        }
        closedir(d);
    }

    return trouve;
}

/* Ajoute un nom à la liste s'il n'y est pas déjà. Renvoie le nouveau nombre de noms. */
static int ajouter_nom(char noms[NBR_JOUEURS_MAX][TAILLE_NOM], int nbr_noms, const char *nom) {
    if (nom == NULL || *nom == '\0' || nbr_noms >= NBR_JOUEURS_MAX) return nbr_noms;
    for (int i = 0; i < nbr_noms; i++) {
        if (strcmp(noms[i], nom) == 0) return nbr_noms;
    }
    snprintf(noms[nbr_noms], TAILLE_NOM, "%s", nom);
    return nbr_noms + 1;
}

int charger_noms_joueurs(char noms[NBR_JOUEURS_MAX][TAILLE_NOM]) {
    int nbr_noms = 0;
    char nom_lu[TAILLE_NOM];

    // 1) Les noms déjà utilisés, du plus récent au plus ancien
    FILE *f = fopen(FICHIER_JOUEURS, "r");
    if (f) {
        while (fscanf(f, "%" STR(TAILLE_NOM_MAX) "s", nom_lu) == 1) {
            nbr_noms = ajouter_nom(noms, nbr_noms, nom_lu);
        }
        fclose(f);
    }

    // 2) Les joueurs présents dans les classements des labyrinthes (anciennes sessions)
    DIR *d = opendir("labyrinthe");
    if (d) {
        struct dirent *dir;
        while ((dir = readdir(d)) != NULL) {
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) continue;
            Grille grille;
            if (!charger_labyrinthe(&grille, dir->d_name)) continue;
            for (int i = 0; i < NOMBRE_SCORE; i++) {
                nbr_noms = ajouter_nom(noms, nbr_noms, grille.top_scores[i].name);
            }
            liberer_grille(&grille);
        }
        closedir(d);
    }

    return nbr_noms;
}

void enregistrer_nom_joueur(const char *nom) {
    char anciens[NBR_JOUEURS_MAX][TAILLE_NOM];
    int nbr_anciens = 0;
    char nom_lu[TAILLE_NOM];

    FILE *f = fopen(FICHIER_JOUEURS, "r");
    if (f) {
        while (fscanf(f, "%" STR(TAILLE_NOM_MAX) "s", nom_lu) == 1) {
            if (strcmp(nom_lu, nom) != 0) {
                nbr_anciens = ajouter_nom(anciens, nbr_anciens, nom_lu);
            }
        }
        fclose(f);
    }

    f = fopen(FICHIER_JOUEURS, "w");
    if (!f) return; // Pas grave : le nom ne sera simplement pas proposé la prochaine fois
    fprintf(f, "%s\n", nom);
    for (int i = 0; i < nbr_anciens && i < NBR_JOUEURS_MAX - 1; i++) {
        fprintf(f, "%s\n", anciens[i]);
    }
    fclose(f);
}

/* Indique si le mot ne contient que des chiffres (un numéro de la liste). */
static int est_numero(const char *mot) {
    if (*mot == '\0') return 0;
    for (; *mot; mot++) {
        if (*mot < '0' || *mot > '9') return 0;
    }
    return 1;
}

int get_nom_joueur(char *nom_joueur) {
    char noms[NBR_JOUEURS_MAX][TAILLE_NOM];
    int nbr_noms = charger_noms_joueurs(noms);

    if (nbr_noms > 0) {
        printf("\n Joueurs déjà connus :\n");
        for (int i = 0; i < nbr_noms; i++) {
            printf("   %d - %s\n", i + 1, noms[i]);
        }
        printf("\n Tapez le numéro de votre nom, ou un nouveau nom (" MOT_RETOUR " pour revenir au menu) : \n");
    } else {
        printf("\n Nom du joueur (" MOT_RETOUR " pour revenir au menu) : \n");
    }

    while (1) {
        lire_mot(nom_joueur);
        if (est_retour(nom_joueur)) return 0;

        if (nbr_noms > 0 && est_numero(nom_joueur)) {
            int numero = atoi(nom_joueur);
            if (numero >= 1 && numero <= nbr_noms) {
                snprintf(nom_joueur, TAILLE_NOM, "%s", noms[numero - 1]);
            } else {
                printf("Numéro invalide, entrez un numéro entre 1 et %d ou un nom : ", nbr_noms);
                continue;
            }
        }
        break;
    }

    printf("Bonjour %s !\n", nom_joueur);
    enregistrer_nom_joueur(nom_joueur);
    return 1;
}

int get_nom_labyrinthe(char *nom_labyrinthe) {
    printf("\n Choissisez un labyrinthe de la liste (entrez le nom du fichier, " MOT_RETOUR " pour revenir au menu) : \n");

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

    if (!fichiers_trouves) {
        printf("Aucun labyrinthe disponible pour le moment.\n");
    }

    lire_mot(nom_labyrinthe);
    return !est_retour(nom_labyrinthe);
}

int get_mode_jeu() {
    int mode;

    printf("Choisissez un mode de jeu :\n");
    printf("Mode 1 : Facile    (ogre qui erre au hasard, carte visible)\n");
    printf("Mode 2 : Difficile (brouillard, ogre chasseur, plus de pièges et moins de bonus)\n");
    printf("%d : Revenir au menu\n", CHOIX_RETOUR);

    mode = lire_entier();
    while (mode != 1 && mode != 2 && mode != CHOIX_RETOUR) {
        printf("Mode invalide, entrez 1, 2 ou %d : ", CHOIX_RETOUR);
        mode = lire_entier();
    }

    if (mode != CHOIX_RETOUR) {
        printf("Mode de jeu choisi : %d\n", mode);
    }
    return mode;
}

/* Demande une dimension impaire comprise entre DIMENSION_MIN et DIMENSION_MAX.
 * Renvoie CHOIX_RETOUR si l'utilisateur veut revenir au menu. */
static int lire_dimension(const char *libelle) {
    int valeur;

    while (1) {
        printf("\n%s du labyrinthe (impaire, entre %d et %d ; %d pour revenir au menu) : ",
               libelle, DIMENSION_MIN, DIMENSION_MAX, CHOIX_RETOUR);
        valeur = lire_entier();

        if (valeur == CHOIX_RETOUR) {
            return CHOIX_RETOUR;
        } else if (valeur < DIMENSION_MIN || valeur > DIMENSION_MAX) {
            printf("\nLa %s doit être comprise entre %d et %d, réessayez.\n", libelle, DIMENSION_MIN, DIMENSION_MAX);
        } else if (valeur % 2 != 1) {
            printf("\nLa %s du labyrinthe doit être un entier impair, réessayez.\n", libelle);
        } else {
            return valeur;
        }
    }
}

int get_parametres_labyrinthe(int *hauteur, int *largeur, char *nom){

    *hauteur = lire_dimension("Hauteur");
    if (*hauteur == CHOIX_RETOUR) return 0;

    *largeur = lire_dimension("Largeur");
    if (*largeur == CHOIX_RETOUR) return 0;

    printf("\nNom du labyrinthe (" MOT_RETOUR " pour revenir au menu) : ");
    lire_mot(nom);
    return !est_retour(nom);
}

void afficher_regles(void) {
    printf("\n================ RÈGLES DU JEU ================\n");

    printf("\n But du jeu :\n");
    printf("   Partez de l'entrée, ramassez la clé puis atteignez la sortie\n");
    printf("   avec le score le plus bas possible, sans vous faire attraper par l'ogre.\n");

    printf("\n Déplacements :\n");
    printf("   z : haut    s : bas    q : gauche    d : droite  (ou les flèches du clavier)\n");
    printf("   Pas besoin d'appuyer sur Entrée : le joueur bouge dès que la touche est pressée.\n");
    printf("   %c : abandonner la partie et revenir au menu\n", MVT_TOUCHE_RETOUR);

    printf("\n Les caractères du labyrinthe :\n");
    printf("   #  Mur : infranchissable.\n");
    printf("   o  Entrée : votre point de départ.\n");
    printf("   -  Sortie : ne s'ouvre que si vous avez la clé.\n");
    printf("   *  Vous (le joueur).\n");
    printf("   %c  Clé : permet d'ouvrir la porte de sortie (aucun effet sur le score).\n", MOTIF_KEY);
    printf("   %c  Piège : ajoute %d points au score. Il reste en place : il pique à chaque passage.\n",
           MOTIF_TRAP, PENALITE_PIEGE);
    printf("   %c  Bonus : retire %d points au score (minimum 0). Il disparaît une fois pris.\n",
           MOTIF_BOOST, GAIN_BONUS);
    printf("   %c  Ogre : se déplace d'une case au hasard à chaque tour, sans traverser les murs.\n", MOTIF_OGRE);
    printf("      En mode Difficile, il vous chasse dès que vous êtes à moins de %d cases.\n", RAYON_CHASSE);
    printf("      S'il vous attrape, la partie est perdue et votre score n'est pas enregistré.\n");
    printf("   %c  Fantôme : se déplace d'une case au hasard à chaque tour et traverse les murs.\n", MOTIF_FANTOME);
    printf("      S'il vous attrape, la partie continue mais votre score est doublé ;\n");
    printf("      le fantôme réapparaît alors ailleurs dans le labyrinthe.\n");

    printf("\n Mode Difficile :\n");
    printf("   Brouillard : vous ne voyez que les cases à %d de distance autour de vous.\n", RAYON_BROUILLARD);
    printf("   Ogre chasseur, plus de pièges et moins de bonus.\n");

    printf("\n Score :\n");
    printf("   Chaque déplacement ajoute 1 point (même contre un mur).\n");
    printf("   Une touche invalide ne compte pas et les monstres ne bougent pas.\n");
    printf("   Plus le score est bas, meilleur il est : les 10 meilleurs sont enregistrés.\n");

    printf("\n Retour au menu :\n");
    printf("   À chaque question, tapez %s pour revenir au menu principal.\n", MOT_RETOUR);

    printf("\n===============================================\n");
    printf("\nAppuyez sur Entrée pour revenir au menu...");
    fflush(stdout);
    vider_ligne();
}
