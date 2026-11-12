#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "jouer.h"
#include "labyrinthe.h"

void initialiser_joueur(Joueur *joueur){
    joueur -> x = 0;
    joueur -> y = 1;
    joueur -> have_key = 0;
    joueur -> score = 0;
}

void positionner_joueur(Grille *grille, Joueur *joueur){
    int position_x_joueur = joueur -> x;
    int position_y_joueur = joueur -> y;
    int score_joueur = joueur -> score;
    grille -> cases[position_x_joueur][position_y_joueur] = '*';
    joueur -> score = score_joueur + 1;
}

int mouvement_valide(Grille *grille, int x, int y, Joueur *joueur){

    int n =  joueur -> score;
    if(x >= grille -> hauteur || x < 0) return 0;
    if(y >= grille -> largeur || y < 0) return 0;

    if(grille -> cases[x][y] == '#') return 0;
    if((grille -> cases[x][y] == '-') && joueur -> have_key == 0) return 0;
    if(grille -> cases[x][y] == MOTIF_KEY) joueur -> have_key = 1;
    if(grille -> cases[x][y] == MOTIF_TRAP) joueur -> score = n + 6;
    if(grille -> cases[x][y] == MOTIF_BOOST){
        if (n < 6){
            n = 6;
        }
        joueur -> score = n - 6;
    }
    return 1;
}

int mouvement_joueur(Grille *grille, Joueur *joueur){
    char mvt;

    int x = joueur->x;
    int y = joueur->y;

    if (scanf(" %c", &mvt) != 1) return MVT_FIN; // Fin de l'entrée standard

    switch(mvt){
        case 'z':
            x--;
            break;
        case 'q':
            y--;
            break;
        case 'd':
            y++;
            break;
        case 's':
            x++;
            break;
        case MVT_TOUCHE_RETOUR:
            return MVT_RETOUR;
        default:
            // Le joueur reste affiché à sa place, aucun point n'est compté
            printf("Touche invalide (utilisez z, q, s, d, ou %c pour revenir au menu).\n", MVT_TOUCHE_RETOUR);
            return MVT_INVALIDE;
    }

    grille -> cases[joueur -> x][joueur -> y] = ' ';

    if(mouvement_valide(grille, x, y, joueur)){
        joueur -> x = x;
        joueur -> y = y;
    }

    positionner_joueur(grille, joueur);
    printf("\n Votre score est : %d \n", get_score_joueur(joueur));
    return MVT_JOUE;
}

/* Un monstre reste toujours à l'intérieur du labyrinthe (jamais sur l'entrée ni la sortie).
 * L'ogre ne traverse pas les murs, le fantôme si. */
static int case_accessible_monstre(Grille *grille, Monstre *monstre, int x, int y){
    if (x < 1 || x > grille->hauteur - 2) return 0;
    if (y < 1 || y > grille->largeur - 2) return 0;
    return monstre->traverse_murs || grille->cases[x][y] != '#';
}

static int distance(int x1, int y1, int x2, int y2){
    return abs(x1 - x2) + abs(y1 - y2);
}

void initialiser_monstre(Monstre *monstre, char symbole, int traverse_murs){
    monstre->x = 0;
    monstre->y = 0;
    monstre->symbole = symbole;
    monstre->traverse_murs = traverse_murs;
    monstre->actif = 0;
}

void placer_monstre(Grille *grille, Monstre *monstre, Joueur *joueur){
    int distance_min = (grille->hauteur + grille->largeur) / 3;
    int meilleure_distance = -1;

    monstre->actif = 0;

    // Tirage aléatoire d'une case libre suffisamment éloignée du joueur
    if (grille->hauteur > 2 && grille->largeur > 2) {
        for (int essai = 0; essai < 1000; essai++) {
            int x = 1 + rand() % (grille->hauteur - 2);
            int y = 1 + rand() % (grille->largeur - 2);
            if (grille->cases[x][y] == ' ' && distance(x, y, joueur->x, joueur->y) >= distance_min) {
                monstre->x = x;
                monstre->y = y;
                monstre->actif = 1;
                return;
            }
        }
    }

    // Sinon (petit labyrinthe), on prend la case libre la plus éloignée du joueur
    for (int x = 1; x < grille->hauteur - 1; x++) {
        for (int y = 1; y < grille->largeur - 1; y++) {
            int d = distance(x, y, joueur->x, joueur->y);
            if (grille->cases[x][y] == ' ' && d > meilleure_distance) {
                meilleure_distance = d;
                monstre->x = x;
                monstre->y = y;
                monstre->actif = 1;
            }
        }
    }
}

int monstre_attrape_joueur(Monstre *monstre, Joueur *joueur){
    return monstre->actif && monstre->x == joueur->x && monstre->y == joueur->y;
}

int deplacer_monstre(Grille *grille, Monstre *monstre, Joueur *joueur){
    const int dx[4] = {-1, 1, 0, 0};
    const int dy[4] = {0, 0, -1, 1};
    int directions_possibles[4];
    int nbr_directions = 0;

    if (!monstre->actif) return 0;

    for (int d = 0; d < 4; d++) {
        if (case_accessible_monstre(grille, monstre, monstre->x + dx[d], monstre->y + dy[d])) {
            directions_possibles[nbr_directions++] = d;
        }
    }

    if (nbr_directions == 0) return 0; // Bloqué : le monstre reste sur place

    int d = directions_possibles[rand() % nbr_directions];
    monstre->x += dx[d];
    monstre->y += dy[d];

    return monstre_attrape_joueur(monstre, joueur);
}

void afficher_jeu(Grille *grille, Monstre *monstres[], int nbr_monstres){
    char cases_dessous[NBR_MONSTRES_MAX];

    // Les monstres ne sont pas stockés dans la grille : on les dessine seulement pour l'affichage
    for (int i = 0; i < nbr_monstres; i++) {
        Monstre *m = monstres[i];
        if (m->actif) {
            cases_dessous[i] = grille->cases[m->x][m->y];
            grille->cases[m->x][m->y] = m->symbole;
        }
    }

    afficher_labyrinthe(grille);

    // Restauration dans l'ordre inverse (correct même si deux monstres sont sur la même case)
    for (int i = nbr_monstres - 1; i >= 0; i--) {
        Monstre *m = monstres[i];
        if (m->actif) {
            grille->cases[m->x][m->y] = cases_dessous[i];
        }
    }
}

void fantome_attrape_joueur(Grille *grille, Monstre *fantome, Joueur *joueur){
    joueur->score *= 2;
    printf("\nLe fantôme (%c) vous a attrapé : votre score est doublé ! Nouveau score : %d\n",
           fantome->symbole, get_score_joueur(joueur));

    // Le fantôme réapparaît ailleurs pour ne pas doubler le score à chaque tour
    placer_monstre(grille, fantome, joueur);
}

int jouer_labyrinthe(Grille *grille, Joueur *joueur){
    Monstre ogre;
    Monstre fantome;
    Monstre *monstres[NBR_MONSTRES_MAX] = {&fantome, &ogre}; // L'ogre est dessiné par-dessus le fantôme

    srand((unsigned) time(NULL));
    initialiser_monstre(&ogre, MOTIF_OGRE, 0);
    initialiser_monstre(&fantome, MOTIF_FANTOME, 1);
    placer_monstre(grille, &ogre, joueur);
    placer_monstre(grille, &fantome, joueur);

    printf("\nDéplacez-vous avec z, q, s, d (%c pour revenir au menu).\n", MVT_TOUCHE_RETOUR);

    while (1) {
        afficher_jeu(grille, monstres, NBR_MONSTRES_MAX);

        int resultat = mouvement_joueur(grille, joueur);
        if (resultat == MVT_FIN) {
            printf("\nPartie abandonnée.\n");
            return 0;
        }
        if (resultat == MVT_RETOUR) {
            printf("\nPartie abandonnée, retour au menu.\n");
            return 0;
        }
        if (resultat == MVT_INVALIDE) continue; // Pas de tour joué : les monstres ne bougent pas

        // Le joueur s'est jeté sur un monstre
        if (monstre_attrape_joueur(&ogre, joueur)) break;
        if (monstre_attrape_joueur(&fantome, joueur)) fantome_attrape_joueur(grille, &fantome, joueur);

        if (joueur_gagne(grille, joueur)) {
            afficher_jeu(grille, monstres, NBR_MONSTRES_MAX);
            printf("\nBravo, vous avez trouvé la sortie avec un score de %d !\n", get_score_joueur(joueur));
            return 1;
        }

        // Tour des monstres
        if (deplacer_monstre(grille, &ogre, joueur)) break;
        if (deplacer_monstre(grille, &fantome, joueur)) fantome_attrape_joueur(grille, &fantome, joueur);
    }

    afficher_jeu(grille, monstres, NBR_MONSTRES_MAX);
    printf("\nL'ogre (%c) vous a attrapé ! Partie terminée.\n", MOTIF_OGRE);
    return 0;
}

int joueur_gagne(Grille *grille, Joueur *joueur){
    if(joueur -> have_key == 0) return 0;
    if(grille->cases[grille->hauteur - 1][grille->largeur - 2] != '*') return 0;
    return 1;
}

int get_score_joueur(Joueur *joueur){
    return joueur -> score; 
}

int set_score_joueur(Grille *grille, Joueur *joueur, char *nom_joueur) {
    
    int score_joueur = get_score_joueur(joueur);
    int maj_scores = 0;
    int index_insertion = -1;

    for (int i = 0; i < NOMBRE_SCORE; i++) {
        int score_actuel = grille->top_scores[i].score;
        if (score_actuel == 0 || score_joueur < score_actuel) {
            index_insertion = i;
            break;
        }
    }

    if (index_insertion != -1) {
        // Si la dernière entrée contient un nom alloué, on la libère car elle sera écrasée
        if (grille->top_scores[NOMBRE_SCORE - 1].name != NULL) {
            free(grille->top_scores[NOMBRE_SCORE - 1].name);
            grille->top_scores[NOMBRE_SCORE - 1].name = NULL;
            grille->top_scores[NOMBRE_SCORE - 1].score = 0;
        }

        // Décaler scores et pointeurs de noms vers la fin
        for (int i = NOMBRE_SCORE - 1; i > index_insertion; i--) {
            grille->top_scores[i].score = grille->top_scores[i - 1].score;
            grille->top_scores[i].name  = grille->top_scores[i - 1].name;
        }

        // Insérer la nouvelle entrée (dupliquer le nom pour en prendre possession)
        grille->top_scores[index_insertion].score = score_joueur;
        if (nom_joueur != NULL) {
            grille->top_scores[index_insertion].name = strdup(nom_joueur);
        } else {
            grille->top_scores[index_insertion].name = NULL;
        }
        maj_scores = 1;
    }
    
    return maj_scores;
}

void sauvegarder_score(Grille *grille_de_jeu, char *nom_labyrinthe) {

    Grille grille_sauvegarde;
    
    // On recharge la grille d'origine (sans la trace du joueur) avant de réécrire le fichier
    if (!charger_labyrinthe(&grille_sauvegarde, nom_labyrinthe)) {
        fprintf(stderr, "Impossible de sauvegarder le score.\n");
        return;
    }

    CREATE_DIR("labyrinthe");
    char chemin_complet[100];
    snprintf(chemin_complet, 100, "labyrinthe/%s", nom_labyrinthe);
    FILE *f = fopen(chemin_complet, "w");
    if (!f) {
        perror("Erreur ouverture fichier de sauvegarde");
        liberer_grille(&grille_sauvegarde);
        return;
    }

    fprintf(f, "%d %d\n", grille_sauvegarde.hauteur, grille_sauvegarde.largeur);

    for (int i = 0; i < grille_sauvegarde.hauteur; i++) {
        for (int j = 0; j < grille_sauvegarde.largeur; j++) {
            fputc(grille_sauvegarde.cases[i][j], f);
        }
        fputc('\n', f);
    }

    for (int i = 0; i < NOMBRE_SCORE; i++) {
        char *name = grille_de_jeu->top_scores[i].name;
        int score = grille_de_jeu->top_scores[i].score;
        if (score > 0 && name != NULL && *name != '\0') {
            fprintf(f, "%d %s\n", score, name);
        }
    }

    fclose(f);
    liberer_grille(&grille_sauvegarde);
}
