#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include "menu.h"
#include "jouer.h"
#include "terminal.h"

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

int nom_valide(const char *nom) {
    if (*nom == '\0') return 0;
    for (; *nom; nom++) {
        if (!isalnum((unsigned char) *nom) && *nom != '_' && *nom != '-') return 0;
    }
    return 1;
}

/* Affiche l'invite et lit un nom, en redemandant tant qu'il contient des caractères interdits.
 * Empêche par exemple un nom comme "../fichier" qui ferait écrire hors du dossier labyrinthe/. */
static void lire_nom(const char *invite, char *nom) {
    while (1) {
        printf("%s", invite);
        lire_mot(nom);
        if (nom_valide(nom)) return;
        printf("Nom invalide : utilisez seulement des lettres (sans accent), des chiffres, _ ou -.\n");
    }
}

/* Nombre de lignes occupées par les options (fenêtre + indicateurs de défilement éventuels). */
static int lignes_options(int nbr_options) {
    return (nbr_options <= TAILLE_FENETRE_MENU) ? nbr_options : TAILLE_FENETRE_MENU + 2;
}

/* Affiche les lignes du menu, l'option sélectionnée étant mise en évidence.
 * Si les options sont trop nombreuses, seules TAILLE_FENETRE_MENU d'entre elles sont affichées,
 * à partir de l'indice debut, encadrées par le nombre d'options cachées au-dessus et en dessous. */
static void afficher_options(const char *options[], int nbr_options, int selection, int debut,
                             const char *aide) {
    int defile = nbr_options > TAILLE_FENETRE_MENU;
    int fin = defile ? debut + TAILLE_FENETRE_MENU : nbr_options;

    if (defile) {
        printf("\r\033[2K");
        if (debut > 0) printf(COULEUR_SOMBRE "      ... %d de plus au-dessus" COULEUR_RESET, debut);
        printf("\n");
    }
    for (int i = debut; i < fin; i++) {
        printf("\r\033[2K"); // Efface la ligne avant de la réécrire
        if (i == selection) {
            printf("  " COULEUR_JAUNE "> %d - %s" COULEUR_RESET "\n", i + 1, options[i]);
        } else {
            printf("    %d - %s\n", i + 1, options[i]);
        }
    }
    if (defile) {
        printf("\r\033[2K");
        if (fin < nbr_options) printf(COULEUR_SOMBRE "      ... %d de plus en dessous" COULEUR_RESET, nbr_options - fin);
        printf("\n");
    }
    printf("\r\033[2K   %s\n", aide);
}

/* Remonte le curseur au début du menu et le redessine sur place. */
static void redessiner_options(const char *options[], int nbr_options, int selection, int debut,
                               const char *aide) {
    printf("\033[%dA", lignes_options(nbr_options) + 1);
    afficher_options(options, nbr_options, selection, debut, aide);
}

/* Décale la fenêtre visible pour que l'option sélectionnée y soit. Renvoie le nouveau début. */
static int ajuster_fenetre(int selection, int debut) {
    if (selection < debut) return selection;
    if (selection >= debut + TAILLE_FENETRE_MENU) return selection - TAILLE_FENETRE_MENU + 1;
    return debut;
}

/* Menu aux flèches. Si echap_autorise, Échap choisit la dernière option (« Revenir au menu »). */
static int choisir_option_echap(const char *titre, const char *options[], int nbr_options, int echap_autorise) {
    int selection = 0;
    int debut = 0; // Première option visible dans la fenêtre
    const char *aide = echap_autorise
        ? "(flèches haut/bas puis Entrée, ou tapez le numéro ; Échap pour revenir)"
        : "(flèches haut/bas puis Entrée, ou tapez le numéro)";

    printf("\n %s\n\n", titre);

    // Entrée redirigée (fichier, tube) : on garde la saisie classique d'un numéro
    if (!entree_est_terminal()) {
        for (int i = 0; i < nbr_options; i++) {
            printf("    %d - %s\n", i + 1, options[i]);
        }
        int choix = lire_entier();
        while (choix < 1 || choix > nbr_options) {
            printf("Choix invalide, entrez un nombre entre 1 et %d : ", nbr_options);
            choix = lire_entier();
        }
        return choix - 1;
    }

    afficher_options(options, nbr_options, selection, debut, aide);

    while (1) {
        int touche = lire_touche();

        if (touche == TOUCHE_FIN) {
            printf("\nFin de l'entrée, le jeu se termine.\n");
            exit(EXIT_SUCCESS);
        } else if (touche == '\r' || touche == '\n' || touche == ' ') {
            return selection;
        } else if (touche == TOUCHE_ECHAP && echap_autorise) {
            return nbr_options - 1;
        } else if (touche >= '1' && touche <= '9' && touche - '1' < nbr_options) {
            // Raccourci : le numéro sélectionne et valide directement l'option
            selection = touche - '1';
            debut = ajuster_fenetre(selection, debut);
            redessiner_options(options, nbr_options, selection, debut, aide);
            return selection;
        } else if (touche == 'z') { // Flèche haut (ou z)
            selection = (selection + nbr_options - 1) % nbr_options;
            debut = ajuster_fenetre(selection, debut);
            redessiner_options(options, nbr_options, selection, debut, aide);
        } else if (touche == 's') { // Flèche bas (ou s)
            selection = (selection + 1) % nbr_options;
            debut = ajuster_fenetre(selection, debut);
            redessiner_options(options, nbr_options, selection, debut, aide);
        }
    }
}

int choisir_option(const char *titre, const char *options[], int nbr_options) {
    return choisir_option_echap(titre, options, nbr_options, 1);
}

int get_action_initial(void){
    const char *options[] = {
        "Créer un labyrinthe",
        "Charger un labyrinthe",
        "Jouer",
        "Consulter les scores d'un labyrinthe",
        "Consulter les règles du jeu",
        "Quitter"
    };

    // Échap désactivé ici : une fausse manipulation ne doit pas quitter le jeu
    return choisir_option_echap("Choisissez une action parmi ce qui suit :", options, 6, 0) + 1;
}

int labyrinthe_disponible(void) {
    DIR *d = opendir(DOSSIER_LABYRINTHES);
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

int labyrinthe_existe(const char *nom) {
    char chemin[TAILLE_CHEMIN];
    chemin_labyrinthe(nom, chemin);

    FILE *f = fopen(chemin, "r");
    if (f == NULL) return 0;
    fclose(f);
    return 1;
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

    // Cas normal : les noms déjà utilisés, du plus récent au plus ancien
    FILE *f = fopen(FICHIER_JOUEURS, "r");
    if (f) {
        while (fscanf(f, "%" STR(TAILLE_NOM_MAX) "s", nom_lu) == 1) {
            nbr_noms = ajouter_nom(noms, nbr_noms, nom_lu);
        }
        fclose(f);
        return nbr_noms;
    }

    // Premier lancement (pas encore de FICHIER_JOUEURS) : on retrouve une seule fois les joueurs
    // des classements des labyrinthes, puis on les enregistre pour ne plus ouvrir tous les fichiers.
    DIR *d = opendir(DOSSIER_LABYRINTHES);
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

    if (nbr_noms > 0) {
        f = fopen(FICHIER_JOUEURS, "w");
        if (f) {
            for (int i = 0; i < nbr_noms; i++) {
                fprintf(f, "%s\n", noms[i]);
            }
            fclose(f);
        }
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

int get_nom_joueur(char *nom_joueur) {
    char noms[NBR_JOUEURS_MAX][TAILLE_NOM];
    int nbr_noms = charger_noms_joueurs(noms);

    // Menu : les joueurs connus, puis "Nouveau joueur" et "Revenir au menu"
    const char *options[NBR_JOUEURS_MAX + 2];
    for (int i = 0; i < nbr_noms; i++) {
        options[i] = noms[i];
    }
    options[nbr_noms] = "Nouveau joueur";
    options[nbr_noms + 1] = "Revenir au menu";

    int choix = choisir_option("Qui joue ?", options, nbr_noms + 2);

    if (choix == nbr_noms + 1) {
        return 0;
    } else if (choix == nbr_noms) {
        lire_nom("\n Nom du nouveau joueur (lettres, chiffres, _ ou - ; " MOT_RETOUR " pour revenir au menu) : ",
                 nom_joueur);
        if (est_retour(nom_joueur)) return 0;
    } else {
        snprintf(nom_joueur, TAILLE_NOM, "%s", noms[choix]);
    }

    printf("Bonjour %s !\n", nom_joueur);
    enregistrer_nom_joueur(nom_joueur);
    return 1;
}

/* Comparaison alphabétique pour qsort. */
static int comparer_noms(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b);
}

int get_nom_labyrinthe(char *nom_labyrinthe) {
    char fichiers[NBR_LABYRINTHES_MAX][TAILLE_NOM];
    int nbr_fichiers = 0;

    DIR *d = opendir(DOSSIER_LABYRINTHES);
    if (d) {
        struct dirent *dir;

        while ((dir = readdir(d)) != NULL && nbr_fichiers < NBR_LABYRINTHES_MAX) {
            // Ignorer les entrées système "." et ".." ainsi que les noms trop longs
            if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0) continue;
            if (strlen(dir->d_name) >= TAILLE_NOM) continue;
            snprintf(fichiers[nbr_fichiers], TAILLE_NOM, "%s", dir->d_name);
            nbr_fichiers++;
        }
        closedir(d);
    }

    if (nbr_fichiers == 0) {
        printf("Aucun labyrinthe disponible pour le moment.\n");
        return 0;
    }

    qsort(fichiers, nbr_fichiers, TAILLE_NOM, comparer_noms);

    // Menu : les labyrinthes, puis "Revenir au menu"
    const char *options[NBR_LABYRINTHES_MAX + 1];
    for (int i = 0; i < nbr_fichiers; i++) {
        options[i] = fichiers[i];
    }
    options[nbr_fichiers] = "Revenir au menu";

    int choix = choisir_option("Choisissez un labyrinthe :", options, nbr_fichiers + 1);
    if (choix == nbr_fichiers) return 0;

    snprintf(nom_labyrinthe, TAILLE_NOM, "%s", fichiers[choix]);
    return 1;
}

int get_mode_jeu(void) {
    const char *options[] = {
        "Facile    (pas de monstres)",
        "Normal    (un ogre qui erre sans chasser, pièges et bonus intermédiaires)",
        "Difficile (ogre chasseur, fantôme, plus de pièges et moins de bonus)",
        "Revenir au menu"
    };
    const int modes[] = { MODE_FACILE, MODE_NORMAL, MODE_DIFFICILE };
    int choix = choisir_option("Choisissez un mode de jeu :", options, 4);

    if (choix == 3) return CHOIX_RETOUR;

    printf("Mode de jeu choisi : %s\n", nom_mode(modes[choix]));
    return modes[choix];
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

    while (1) {
        lire_nom("\nNom du labyrinthe (lettres, chiffres, _ ou - ; " MOT_RETOUR " pour revenir au menu) : ", nom);
        if (est_retour(nom)) return 0;
        if (!labyrinthe_existe(nom)) return 1;

        // Un labyrinthe porte déjà ce nom : on demande avant d'écraser son fichier et son top 10
        char titre[TAILLE_NOM + 64];
        const char *options[] = {
            "Oui, l'écraser (son top 10 sera perdu)",
            "Non, choisir un autre nom",
            "Revenir au menu"
        };
        snprintf(titre, sizeof titre, "Le labyrinthe \"%s\" existe déjà. L'écraser ?", nom);

        int choix = choisir_option(titre, options, 3);
        if (choix == 0) return 1;
        if (choix == 2) return 0;
    }
}

void afficher_regles(void) {
    printf("\n================ RÈGLES DU JEU ================\n");

    printf("\n But du jeu :\n");
    printf("   Partez de l'entrée, ramassez la clé puis atteignez la sortie\n");
    printf("   avec le score le plus bas possible, sans vous faire attraper par l'ogre.\n");

    printf("\n Déplacements :\n");
    printf("   z : haut    s : bas    q : gauche    d : droite  (ou les flèches du clavier)\n");
    printf("   Pas besoin d'appuyer sur Entrée : le joueur bouge dès que la touche est pressée.\n");
    printf("   %c ou Échap : abandonner la partie et revenir au menu\n", MVT_TOUCHE_RETOUR);
    printf("   Dans les menus, Échap revient aussi au menu principal.\n");

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
    printf("   %c  Ogre (modes Normal et Difficile) : ne traverse pas les murs.\n", MOTIF_OGRE);
    printf("      En Normal, il erre au hasard. En Difficile, il vous chasse dès que vous êtes\n");
    printf("      à moins de %d cases, sinon il erre au hasard.\n", RAYON_CHASSE);
    printf("      S'il vous attrape, la partie est perdue et votre score n'est pas enregistré.\n");
    printf("   %c  Fantôme (mode Difficile uniquement) : se déplace au hasard et traverse les murs.\n", MOTIF_FANTOME);
    printf("      S'il vous attrape, la partie continue mais votre score est doublé ;\n");
    printf("      le fantôme réapparaît alors ailleurs dans le labyrinthe.\n");

    printf("\n Modes de jeu (choisis à la création, enregistrés dans le labyrinthe) :\n");
    printf("   Facile    : ni ogre ni fantôme.\n");
    printf("   Normal    : un ogre qui erre sans chasser, pas de fantôme ; pièges et bonus intermédiaires.\n");
    printf("   Difficile : ogre chasseur et fantôme, deux fois plus de pièges et deux fois moins de bonus.\n");

    printf("\n Score :\n");
    printf("   Chaque déplacement ajoute 1 point (même contre un mur).\n");
    printf("   Une touche invalide ne compte pas et les monstres ne bougent pas.\n");
    printf("   Plus le score est bas, meilleur il est : les 10 meilleurs sont enregistrés.\n");

    printf("\n Retour au menu :\n");
    printf("   Dans les listes, choisissez « Revenir au menu » (flèches puis Entrée).\n");
    printf("   Quand il faut taper un nom ou un nombre, tapez %s pour revenir au menu principal.\n", MOT_RETOUR);

    printf("\n===============================================\n");
    printf("\nAppuyez sur Entrée pour revenir au menu...");
    fflush(stdout);
    vider_ligne();
}
