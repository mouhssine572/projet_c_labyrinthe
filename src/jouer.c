#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jouer.h"
#include "labyrinthe.h"
#include "terminal.h"

void initialiser_joueur(Joueur *joueur){
    joueur -> x = 0;
    joueur -> y = 1;
    joueur -> have_key = 0;
    joueur -> score = 0;
    joueur -> case_dessous = MOTIF_VIDE;
    joueur -> message[0] = '\0';
}

void positionner_joueur(Grille *grille, Joueur *joueur){
    int position_x_joueur = joueur -> x;
    int position_y_joueur = joueur -> y;
    int score_joueur = joueur -> score;
    grille -> cases[position_x_joueur][position_y_joueur] = MOTIF_JOUEUR;
    joueur -> score = score_joueur + 1;
}

int mouvement_valide(Grille *grille, int x, int y, Joueur *joueur){

    int n =  joueur -> score;
    if(x >= grille -> hauteur || x < 0) return 0;
    if(y >= grille -> largeur || y < 0) return 0;

    char cible = grille -> cases[x][y];

    if(cible == MOTIF_MUR) return 0;

    if(cible == MOTIF_SORTIE && joueur -> have_key == 0){
        snprintf(joueur -> message, TAILLE_MESSAGE, "La sortie est fermée : allez d'abord chercher la clé (%c).", MOTIF_KEY);
        return 0;
    }

    switch (cible) {
        case MOTIF_KEY:
            joueur -> have_key = 1;
            snprintf(joueur -> message, TAILLE_MESSAGE, "Clé ramassée ! Rejoignez la sortie (%c).", MOTIF_SORTIE);
            break;
        case MOTIF_TRAP:
            joueur -> score = n + PENALITE_PIEGE;
            snprintf(joueur -> message, TAILLE_MESSAGE, "Aïe, un piège : +%d points.", PENALITE_PIEGE);
            break;
        case MOTIF_BOOST:
            joueur -> score = (n < GAIN_BONUS) ? 0 : n - GAIN_BONUS;
            snprintf(joueur -> message, TAILLE_MESSAGE, "Bonus : -%d points !", GAIN_BONUS);
            break;
        default:
            break;
    }
    return 1;
}

/* Ce qui reste sur la case quand le joueur la quitte :
 * les pièges, l'entrée et la sortie restent ; la clé et les bonus ont été ramassés. */
static char case_apres_passage(char c){
    switch (c) {
        case MOTIF_TRAP:
        case MOTIF_ENTREE:
        case MOTIF_SORTIE:
            return c;
        default:
            return MOTIF_VIDE;
    }
}

int mouvement_joueur(Grille *grille, Joueur *joueur){
    int x = joueur->x;
    int y = joueur->y;

    int touche = lire_touche();
    if (touche == TOUCHE_FIN) return MVT_FIN; // Fin de l'entrée standard

    switch(touche){
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
        case TOUCHE_ECHAP:
            return MVT_RETOUR;
        default:
            // Le joueur reste affiché à sa place, aucun point n'est compté
            snprintf(joueur->message, TAILLE_MESSAGE,
                     "Touche invalide (utilisez z, q, s, d ou les flèches, %c ou Échap pour revenir au menu).",
                     MVT_TOUCHE_RETOUR);
            return MVT_INVALIDE;
    }

    // On remet ce qu'il y avait sous le joueur avant qu'il ne parte
    grille -> cases[joueur -> x][joueur -> y] = joueur -> case_dessous;

    if(mouvement_valide(grille, x, y, joueur)){
        joueur -> x = x;
        joueur -> y = y;
    }

    joueur -> case_dessous = case_apres_passage(grille -> cases[joueur -> x][joueur -> y]);
    positionner_joueur(grille, joueur);
    return MVT_JOUE;
}

/* Un monstre reste toujours à l'intérieur du labyrinthe (jamais sur l'entrée ni la sortie).
 * L'ogre ne traverse pas les murs, le fantôme si. */
static int case_accessible_monstre(Grille *grille, Monstre *monstre, int x, int y){
    if (x < 1 || x > grille->hauteur - 2) return 0;
    if (y < 1 || y > grille->largeur - 2) return 0;
    return monstre->traverse_murs || grille->cases[x][y] != MOTIF_MUR;
}

static int distance(int x1, int y1, int x2, int y2){
    return abs(x1 - x2) + abs(y1 - y2);
}

void initialiser_monstre(Monstre *monstre, char symbole, int traverse_murs){
    monstre->x = 0;
    monstre->y = 0;
    monstre->symbole = symbole;
    monstre->traverse_murs = traverse_murs;
    monstre->chasseur = 0;
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
            if (grille->cases[x][y] == MOTIF_VIDE && distance(x, y, joueur->x, joueur->y) >= distance_min) {
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
            if (grille->cases[x][y] == MOTIF_VIDE && d > meilleure_distance) {
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

static const int DX[4] = {-1, 1, 0, 0};
static const int DY[4] = {0, 0, -1, 1};

/* Parcours en largeur depuis le joueur, limité à RAYON_CHASSE cases.
 * Renvoie la direction (0 à 3) qui rapproche le monstre du joueur, ou -1 si le joueur est trop loin. */
static int direction_chasse(Grille *grille, Monstre *monstre, Joueur *joueur){
    int h = grille->hauteur;
    int l = grille->largeur;
    int direction = -1;

    int *dist = malloc(h * l * sizeof(int));
    int *file = malloc(h * l * sizeof(int));
    if (dist == NULL || file == NULL) {
        free(dist);
        free(file);
        return -1; // Faute de mémoire, le monstre se déplace simplement au hasard
    }

    for (int k = 0; k < h * l; k++) dist[k] = -1;

    int debut = 0, fin = 0;
    dist[joueur->x * l + joueur->y] = 0;
    file[fin++] = joueur->x * l + joueur->y;

    while (debut < fin) {
        int courant = file[debut++];
        int cx = courant / l;
        int cy = courant % l;
        if (dist[courant] >= RAYON_CHASSE) continue;

        for (int d = 0; d < 4; d++) {
            int nx = cx + DX[d];
            int ny = cy + DY[d];
            if (!case_accessible_monstre(grille, monstre, nx, ny)) continue;
            if (dist[nx * l + ny] != -1) continue;
            dist[nx * l + ny] = dist[courant] + 1;
            file[fin++] = nx * l + ny;
        }
    }

    int distance_monstre = dist[monstre->x * l + monstre->y];
    if (distance_monstre > 0) {
        // Une case voisine plus proche du joueur (la case du joueur elle-même vaut 0)
        for (int d = 0; d < 4 && direction < 0; d++) {
            int nx = monstre->x + DX[d];
            int ny = monstre->y + DY[d];
            if (nx < 0 || nx >= h || ny < 0 || ny >= l) continue;
            if (dist[nx * l + ny] == distance_monstre - 1 && case_accessible_monstre(grille, monstre, nx, ny)) {
                direction = d;
            }
        }
    }

    free(dist);
    free(file);
    return direction;
}

int deplacer_monstre(Grille *grille, Monstre *monstre, Joueur *joueur){
    int directions_possibles[4];
    int nbr_directions = 0;

    if (!monstre->actif) return 0;

    int d = monstre->chasseur ? direction_chasse(grille, monstre, joueur) : -1;

    if (d < 0) {
        for (int k = 0; k < 4; k++) {
            if (case_accessible_monstre(grille, monstre, monstre->x + DX[k], monstre->y + DY[k])) {
                directions_possibles[nbr_directions++] = k;
            }
        }

        if (nbr_directions == 0) return 0; // Bloqué : le monstre reste sur place

        d = directions_possibles[rand() % nbr_directions];
    }

    monstre->x += DX[d];
    monstre->y += DY[d];

    return monstre_attrape_joueur(monstre, joueur);
}

/* Affiche toute la grille, avec les monstres dessinés par-dessus. */
static void afficher_plateau(Grille *grille, Monstre *monstres[], int nbr_monstres){
    char cases_dessous[NBR_MONSTRES_MAX];

    // Les monstres ne sont pas stockés dans la grille : on les dessine seulement pour l'affichage
    for (int i = 0; i < nbr_monstres; i++) {
        Monstre *m = monstres[i];
        if (m->actif) {
            cases_dessous[i] = grille->cases[m->x][m->y];
            grille->cases[m->x][m->y] = m->symbole;
        }
    }

    for (int i = 0; i < grille->hauteur; i++) {
        for (int j = 0; j < grille->largeur; j++) {
            afficher_case(grille->cases[i][j]);
        }
        putchar('\n');
    }

    // Restauration dans l'ordre inverse (correct même si deux monstres sont sur la même case)
    for (int i = nbr_monstres - 1; i >= 0; i--) {
        Monstre *m = monstres[i];
        if (m->actif) {
            grille->cases[m->x][m->y] = cases_dessous[i];
        }
    }
}

void afficher_jeu(Grille *grille, Monstre *monstres[], int nbr_monstres){
    afficher_plateau(grille, monstres, nbr_monstres);
}

/* Affiche une entrée de la légende : le symbole en couleur suivi de sa signification. */
static void legende(char symbole, const char *texte){
    afficher_case(symbole);
    printf(" %s   ", texte);
}

/* Informations affichées sous la carte : score, clé, légende et dernier message. */
static void afficher_informations(Grille *grille, Joueur *joueur){
    printf("\n Score : %s%d%s   Clé : %s   Mode : %s\n",
           COULEUR_GRAS, get_score_joueur(joueur), COULEUR_RESET,
           joueur->have_key ? "oui" : "non",
           nom_mode(grille->mode));

    printf(" ");
    legende(MOTIF_JOUEUR, "vous");
    legende(MOTIF_KEY, "clé");
    legende(MOTIF_SORTIE, "sortie");
    legende(MOTIF_TRAP, "piège");
    legende(MOTIF_BOOST, "bonus");
    if (grille->mode != MODE_FACILE) { // Pas de monstres en mode Facile
        printf("\n ");
        legende(MOTIF_OGRE, "ogre");
        if (grille->mode == MODE_DIFFICILE) legende(MOTIF_FANTOME, "fantôme");
    }
    printf("\n\n Déplacements : z q s d ou flèches   |   %c ou Échap : revenir au menu\n", MVT_TOUCHE_RETOUR);

    if (joueur->message[0] != '\0') {
        printf("\n %s>> %s%s\n", COULEUR_JAUNE, joueur->message, COULEUR_RESET);
    }
}

/* Efface l'écran puis affiche la carte et les informations de la partie. */
static void afficher_tour(Grille *grille, Joueur *joueur, Monstre *monstres[]){
    effacer_ecran();
    afficher_plateau(grille, monstres, NBR_MONSTRES_MAX);
    afficher_informations(grille, joueur);
}

void fantome_attrape_joueur(Grille *grille, Monstre *fantome, Joueur *joueur){
    joueur->score *= 2;
    snprintf(joueur->message, TAILLE_MESSAGE,
             "Le fantôme (%c) vous a attrapé : votre score est doublé ! Nouveau score : %d",
             fantome->symbole, get_score_joueur(joueur));

    // Le fantôme réapparaît ailleurs pour ne pas attaquer à chaque tour
    placer_monstre(grille, fantome, joueur);
}

int jouer_labyrinthe(Grille *grille, Joueur *joueur){
    Monstre ogre;
    Monstre fantome;
    Monstre *monstres[NBR_MONSTRES_MAX] = {&fantome, &ogre}; // L'ogre est dessiné par-dessus le fantôme
    int difficile = (grille->mode == MODE_DIFFICILE);
    int normal = (grille->mode == MODE_NORMAL);

    initialiser_monstre(&ogre, MOTIF_OGRE, 0);
    initialiser_monstre(&fantome, MOTIF_FANTOME, 1);
    ogre.chasseur = difficile;

    // Les monstres non placés restent inactifs (ni affichés, ni déplacés, ils ne peuvent pas
    // attraper le joueur) : Facile n'a aucun monstre, Normal seulement un ogre qui erre au hasard,
    // Difficile un ogre chasseur et le fantôme.
    if (difficile || normal) {
        placer_monstre(grille, &ogre, joueur);
    }
    if (difficile) {
        placer_monstre(grille, &fantome, joueur);
    }

    // Le joueur est affiché dès le départ, sur l'entrée (sans compter de point)
    joueur->case_dessous = grille->cases[joueur->x][joueur->y];
    grille->cases[joueur->x][joueur->y] = MOTIF_JOUEUR;
    const char *avertissement = difficile ? " Attention : ogre chasseur et fantôme !"
                              : normal    ? " Attention à l'ogre !"
                              : "";
    snprintf(joueur->message, TAILLE_MESSAGE, "Ramassez la clé (%c) puis rejoignez la sortie (%c).%s",
             MOTIF_KEY, MOTIF_SORTIE, avertissement);

    while (1) {
        afficher_tour(grille, joueur, monstres);
        joueur->message[0] = '\0';

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
            snprintf(joueur->message, TAILLE_MESSAGE,
                     "Bravo, vous avez trouvé la sortie avec un score de %d !", get_score_joueur(joueur));
            afficher_tour(grille, joueur, monstres);
            return 1;
        }

        // Tour des monstres
        if (deplacer_monstre(grille, &ogre, joueur)) break;
        if (deplacer_monstre(grille, &fantome, joueur)) fantome_attrape_joueur(grille, &fantome, joueur);
    }

    snprintf(joueur->message, TAILLE_MESSAGE, "L'ogre (%c) vous a attrapé ! Partie terminée.", MOTIF_OGRE);
    afficher_tour(grille, joueur, monstres);
    return 0;
}

int joueur_gagne(Grille *grille, Joueur *joueur){
    if(joueur -> have_key == 0) return 0;
    // La sortie est toujours sur la dernière ligne, avant-dernière colonne
    return joueur->x == grille->hauteur - 1 && joueur->y == grille->largeur - 2;
}

int get_score_joueur(Joueur *joueur){
    return joueur -> score;
}

int set_score_joueur(Grille *grille, Joueur *joueur, char *nom_joueur) {

    int score_joueur = get_score_joueur(joueur);
    int index_insertion = -1;

    for (int i = 0; i < NOMBRE_SCORE; i++) {
        int score_actuel = grille->top_scores[i].score;
        if (score_actuel == 0 || score_joueur < score_actuel) {
            index_insertion = i;
            break;
        }
    }

    if (index_insertion == -1) {
        return 0; // Le score n'entre pas dans le classement
    }

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

    return index_insertion + 1; // Place dans le classement (1 = meilleur score)
}

void sauvegarder_score(Grille *grille_de_jeu, char *nom_labyrinthe) {

    Grille grille_sauvegarde;

    // On recharge la grille d'origine (sans la trace du joueur) avant de réécrire le fichier
    if (!charger_labyrinthe(&grille_sauvegarde, nom_labyrinthe)) {
        fprintf(stderr, "Impossible de sauvegarder le score.\n");
        return;
    }

    // On prête temporairement le classement de la partie à la grille d'origine,
    // puis on remet ses propres scores pour que liberer_grille ne libère que les siens.
    Top_Player scores_origine[NOMBRE_SCORE];
    memcpy(scores_origine, grille_sauvegarde.top_scores, sizeof scores_origine);
    memcpy(grille_sauvegarde.top_scores, grille_de_jeu->top_scores, sizeof scores_origine);

    sauvegarder_labyrinthe(&grille_sauvegarde, nom_labyrinthe);

    memcpy(grille_sauvegarde.top_scores, scores_origine, sizeof scores_origine);
    liberer_grille(&grille_sauvegarde);
}
