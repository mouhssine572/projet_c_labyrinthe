#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minunit.h" 
#include "jouer.h" 


Grille *g = NULL;
Joueur *j = NULL;
const int H = 3; 
const int L = 5; 

/* Remplace stdin par un fichier temporaire contenant les touches données. */
static void simuler_entree(const char *touches) {
    FILE *temp_in = freopen("temp_input.txt", "w+", stdin);
    fwrite(touches, 1, strlen(touches), temp_in);
    rewind(temp_in);
}

void test_setup(void) {

    g = (Grille*)malloc(sizeof(Grille));
    g->hauteur = H;
    g->largeur = L;
    g->mode = MODE_FACILE;
    g->cases = (char**)malloc(H * sizeof(char*));
    for (int i = 0; i < H; i++) {
        g->cases[i] = (char*)malloc(L * sizeof(char));
        memset(g->cases[i], '#', L);
    }
    
    g->cases[0][1] = ' '; 
    g->cases[1][1] = ' '; 
    g->cases[1][2] = ' ';
    
    j = (Joueur*)malloc(sizeof(Joueur));
    initialiser_joueur(j);
}

void test_teardown(void) {
    if (g) {
        for (int i = 0; i < g->hauteur; i++) {
            free(g->cases[i]);
        }
        free(g->cases);
        free(g);
        g = NULL;
    }
    
    if (j) free(j);
    j = NULL;
}


MU_TEST(test_positionner_joueur) {
    
    positionner_joueur(g, j); 

    mu_assert_int_eq('*', g->cases[0][1]);
    mu_assert_int_eq(1, j->score);
}

MU_TEST(test_mouvement_valide) {

    mu_assert_int_eq(0, mouvement_valide(g, 0, 0, j)); // Mouvement invalide (mur)
    mu_assert_int_eq(1, mouvement_valide(g, 1, 1, j)); // Mouvement valide (case vide)
    
    g->cases[1][0] = MOTIF_KEY;
    j->have_key = 0; 
    mouvement_valide(g, 1, 0, j);

    mu_assert_int_eq(1, j->have_key); // Clé ramassée
    

    int score_initial = 10;
    j->score = score_initial;
    g->cases[1][3] = MOTIF_TRAP; 
    mouvement_valide(g, 1, 3, j);

    mu_assert_int_eq(score_initial + 6, j->score);
}

MU_TEST(test_mouvement_joueurt) {
    
    simuler_entree("s\n");

    mu_assert_int_eq(1, mouvement_joueur(g, j));

    mu_assert_int_eq(' ', g->cases[0][1]); // L'ancienne case (0, 1) doit être vide
    mu_assert_int_eq('*', g->cases[1][1]); // La nouvelle case (1, 1) doit être '*'
    mu_assert_int_eq(1, j->x);             // Coordonnée X du joueur doit être 1


}

MU_TEST(test_touche_invalide) {

    positionner_joueur(g, j);           // Joueur affiché en (0, 1), score = 1
    simuler_entree("x\n");

    mu_assert_int_eq(MVT_INVALIDE, mouvement_joueur(g, j));

    mu_assert_int_eq('*', g->cases[0][1]); // Le joueur reste affiché à sa place
    mu_assert_int_eq(0, j->x);
    mu_assert_int_eq(1, j->y);
    mu_assert_int_eq(1, j->score);         // Aucun point compté

}

MU_TEST(test_touche_retour) {

    simuler_entree("0\n");

    mu_assert_int_eq(MVT_RETOUR, mouvement_joueur(g, j));
    mu_assert_int_eq(0, j->x);   // Le joueur n'a pas bougé
    mu_assert_int_eq(0, j->score);
}

MU_TEST(test_fin_entree) {

    simuler_entree("");

    mu_assert_int_eq(0, mouvement_joueur(g, j)); // Entrée vide : fin de partie signalée

}

MU_TEST(test_cle_sans_bonus) {

    g->cases[1][1] = MOTIF_KEY;
    j->score = 10;
    mouvement_valide(g, 1, 1, j);
    mu_assert_int_eq(1, j->have_key);
    mu_assert_int_eq(10, j->score); // La clé ne modifie pas le score
}

MU_TEST(test_deplacer_ogre) {
    // Couloir intérieur : (1,1) et (1,2) libres, (1,3) est un mur.
    // L'ogre en (1,2) ne peut aller qu'en (1,1) : il ne traverse pas le mur.
    Monstre ogre;
    initialiser_monstre(&ogre, MOTIF_OGRE, 0);
    ogre.x = 1;
    ogre.y = 2;
    ogre.actif = 1;

    mu_assert_int_eq(0, deplacer_monstre(g, &ogre, j)); // Joueur en (0,1) : pas attrapé
    mu_assert_int_eq(1, ogre.x);
    mu_assert_int_eq(1, ogre.y);
    mu_assert_int_eq(' ', g->cases[1][1]);              // La grille n'est pas modifiée

    // L'ogre revient en (1,2) où se trouve le joueur
    j->x = 1;
    j->y = 2;
    mu_assert_int_eq(1, deplacer_monstre(g, &ogre, j));
    mu_assert_int_eq(1, monstre_attrape_joueur(&ogre, j));
}

MU_TEST(test_deplacer_fantome) {
    // Le fantôme est dans le mur (1,3) : il ne peut aller qu'en (1,2) ((1,4) est le bord).
    Monstre fantome;
    initialiser_monstre(&fantome, MOTIF_FANTOME, 1);
    fantome.x = 1;
    fantome.y = 3;
    fantome.actif = 1;

    mu_assert_int_eq(0, deplacer_monstre(g, &fantome, j));
    mu_assert_int_eq(1, fantome.x);
    mu_assert_int_eq(2, fantome.y);
    mu_assert_int_eq('#', g->cases[1][3]);              // Le mur est intact

    // Depuis (1,2), il peut entrer dans le mur (1,3) ou aller en (1,1) : il ne reste jamais bloqué
    deplacer_monstre(g, &fantome, j);
    mu_assert(fantome.y == 1 || fantome.y == 3, "le fantome doit avoir bouge");
}

MU_TEST(test_fantome_double_score) {
    Monstre fantome;
    initialiser_monstre(&fantome, MOTIF_FANTOME, 1);

    j->score = 7;
    fantome_attrape_joueur(g, &fantome, j);

    mu_assert_int_eq(14, j->score);                     // Score doublé
    mu_assert_int_eq(1, fantome.actif);                 // Le fantôme est replacé...
    mu_assert_int_eq(0, monstre_attrape_joueur(&fantome, j)); // ...ailleurs que sur le joueur
}

MU_TEST(test_placer_monstre) {
    Monstre ogre;
    initialiser_monstre(&ogre, MOTIF_OGRE, 0);

    placer_monstre(g, &ogre, j);
    // Seules (1,1) et (1,2) sont libres : la plus éloignée du joueur (0,1) est (1,2)
    mu_assert_int_eq(1, ogre.actif);
    mu_assert_int_eq(1, ogre.x);
    mu_assert_int_eq(2, ogre.y);
    mu_assert_int_eq(' ', g->cases[1][2]);              // Le monstre n'est pas écrit dans la grille
}

MU_TEST(test_monstres_mode_facile) {
    Monstre ogre, fantome;
    g->mode = MODE_FACILE;

    preparer_monstres(g, &ogre, &fantome, j);
    mu_assert_int_eq(0, ogre.actif);                 // Aucun monstre
    mu_assert_int_eq(0, fantome.actif);

    // Un monstre inactif ne bouge pas et n'attrape personne
    mu_assert_int_eq(0, deplacer_monstre(g, &ogre, j));
    j->x = ogre.x;
    j->y = ogre.y;
    mu_assert_int_eq(0, monstre_attrape_joueur(&ogre, j));
}

MU_TEST(test_monstres_mode_normal) {
    Monstre ogre, fantome;
    g->mode = MODE_NORMAL;

    preparer_monstres(g, &ogre, &fantome, j);
    mu_assert_int_eq(1, ogre.actif);                 // L'ogre est là...
    mu_assert_int_eq(0, ogre.chasseur);              // ...mais il ne chasse pas
    mu_assert_int_eq(0, fantome.actif);              // Pas de fantôme
}

MU_TEST(test_monstres_mode_difficile) {
    Monstre ogre, fantome;
    g->mode = MODE_DIFFICILE;

    preparer_monstres(g, &ogre, &fantome, j);
    mu_assert_int_eq(1, ogre.actif);
    mu_assert_int_eq(1, ogre.chasseur);              // Ogre chasseur
    mu_assert_int_eq(1, fantome.actif);              // Fantôme présent
    mu_assert_int_eq(1, fantome.traverse_murs);
    mu_assert_int_eq(0, ogre.traverse_murs);
}

MU_TEST(test_afficher_jeu_restaure_grille) {
    Monstre fantome;
    Monstre *monstres[NBR_MONSTRES_MAX] = {&fantome};
    initialiser_monstre(&fantome, MOTIF_FANTOME, 1);
    fantome.x = 1;
    fantome.y = 3;
    fantome.actif = 1;

    afficher_jeu(g, monstres, 1);
    mu_assert_int_eq('#', g->cases[1][3]);              // Le mur sous le fantôme est restauré
}

MU_TEST(test_set_score_joueur) {

    for (int i = 0; i < NOMBRE_SCORE; i++) {
        g->top_scores[i].name = NULL;
        g->top_scores[i].score = 0;
    }

    j->score = 20;
    mu_assert_int_eq(1, set_score_joueur(g, j, "Bob"));   // 1er
    j->score = 10;
    mu_assert_int_eq(1, set_score_joueur(g, j, "Alice")); // 1re, Bob passe 2e
    j->score = 15;
    mu_assert_int_eq(2, set_score_joueur(g, j, "Carl"));  // Entre Alice et Bob : 2e

    // Classement trié du meilleur (plus petit) au moins bon
    mu_assert_int_eq(10, g->top_scores[0].score);
    mu_assert_string_eq("Alice", g->top_scores[0].name);
    mu_assert_int_eq(15, g->top_scores[1].score);
    mu_assert_string_eq("Carl", g->top_scores[1].name);
    mu_assert_int_eq(20, g->top_scores[2].score);
    mu_assert_string_eq("Bob", g->top_scores[2].name);

    // Un classement plein refuse un score moins bon que le dernier
    for (int i = 3; i < NOMBRE_SCORE; i++) {
        j->score = 20 + i;
        set_score_joueur(g, j, "Autre");
    }
    j->score = 1000;
    mu_assert_int_eq(0, set_score_joueur(g, j, "Lent"));

    for (int i = 0; i < NOMBRE_SCORE; i++) {
        free(g->top_scores[i].name);
        g->top_scores[i].name = NULL;
    }
}

MU_TEST(test_score_zero_classe) {

    for (int i = 0; i < NOMBRE_SCORE; i++) {
        g->top_scores[i].name = NULL;
        g->top_scores[i].score = 0;
    }

    j->score = 5;
    mu_assert_int_eq(1, set_score_joueur(g, j, "Bob"));
    j->score = 0;
    mu_assert_int_eq(1, set_score_joueur(g, j, "Zero")); // 0 est le meilleur score : 1er
    j->score = 3;
    mu_assert_int_eq(2, set_score_joueur(g, j, "Carl")); // Passe derrière le 0, pas devant

    mu_assert_int_eq(0, g->top_scores[0].score);
    mu_assert_string_eq("Zero", g->top_scores[0].name);
    mu_assert_string_eq("Carl", g->top_scores[1].name);
    mu_assert_string_eq("Bob", g->top_scores[2].name);

    for (int i = 0; i < NOMBRE_SCORE; i++) {
        free(g->top_scores[i].name);
        g->top_scores[i].name = NULL;
    }
}

MU_TEST(test_piege_reste_bonus_disparait) {
    // Joueur en (1,1), piège en (1,2) : il y passe puis repart, le piège reste
    j->x = 1;
    j->y = 1;
    g->cases[1][2] = MOTIF_TRAP;

    simuler_entree("dq");
    mu_assert_int_eq(MVT_JOUE, mouvement_joueur(g, j));
    mu_assert_int_eq(1 + PENALITE_PIEGE, j->score);
    mu_assert_int_eq(MVT_JOUE, mouvement_joueur(g, j));
    mu_assert_int_eq(MOTIF_TRAP, g->cases[1][2]);         // Le piège est toujours là

    // Même chose avec un bonus : il disparaît
    g->cases[1][2] = MOTIF_BOOST;
    j->score = 20;
    simuler_entree("dq");
    mouvement_joueur(g, j);
    mu_assert_int_eq(20 - GAIN_BONUS + 1, j->score);
    mouvement_joueur(g, j);
    mu_assert_int_eq(MOTIF_VIDE, g->cases[1][2]);         // Le bonus a été consommé
}

MU_TEST(test_sortie_fermee_sans_cle) {
    g->cases[1][2] = MOTIF_SORTIE;

    j->have_key = 0;
    mu_assert_int_eq(0, mouvement_valide(g, 1, 2, j));    // Pas de clé : porte fermée
    j->have_key = 1;
    mu_assert_int_eq(1, mouvement_valide(g, 1, 2, j));    // Avec la clé : la porte s'ouvre
}

MU_TEST(test_joueur_gagne) {
    // La sortie de la grille 3x5 est en (2,3)
    j->x = 1;
    j->y = 1;
    j->have_key = 1;
    mu_assert_int_eq(0, joueur_gagne(g, j));    // Avec la clé mais pas sur la sortie

    j->x = 2;
    j->y = 3;

    j->have_key = 0;
    mu_assert_int_eq(0, joueur_gagne(g, j));
    j->have_key = 1;
    mu_assert_int_eq(1, joueur_gagne(g, j));
}

MU_TEST(test_ogre_chasseur) {
    // Couloir en L : l'ogre en (1,5) doit prendre le chemin qui mène au joueur en (5,1)
    const char *plan[] = {
        "#######",
        "#     #",
        "# ### #",
        "# # # #",
        "# ### #",
        "#     #",
        "#######",
    };
    Grille couloir;
    couloir.hauteur = 7;
    couloir.largeur = 7;
    couloir.cases = malloc(7 * sizeof(char*));
    for (int i = 0; i < 7; i++) {
        couloir.cases[i] = malloc(7);
        memcpy(couloir.cases[i], plan[i], 7);
    }

    Monstre ogre;
    initialiser_monstre(&ogre, MOTIF_OGRE, 0);
    ogre.chasseur = 1;
    ogre.actif = 1;
    ogre.x = 1;
    ogre.y = 2;
    j->x = 3;
    j->y = 1;

    // Le joueur est à 3 cases par la gauche : l'ogre va vers (1,1) à chaque fois
    deplacer_monstre(&couloir, &ogre, j);
    mu_assert_int_eq(1, ogre.x);
    mu_assert_int_eq(1, ogre.y);
    deplacer_monstre(&couloir, &ogre, j);
    mu_assert_int_eq(2, ogre.x);
    mu_assert_int_eq(1, ogre.y);
    mu_assert_int_eq(1, deplacer_monstre(&couloir, &ogre, j)); // Il attrape le joueur

    for (int i = 0; i < 7; i++) free(couloir.cases[i]);
    free(couloir.cases);
}


MU_TEST_SUITE(test_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);

    MU_RUN_TEST(test_positionner_joueur);
    MU_RUN_TEST(test_mouvement_valide);
    MU_RUN_TEST(test_mouvement_joueurt);
    MU_RUN_TEST(test_touche_invalide);
    MU_RUN_TEST(test_touche_retour);
    MU_RUN_TEST(test_fin_entree);
    MU_RUN_TEST(test_cle_sans_bonus);
    MU_RUN_TEST(test_deplacer_ogre);
    MU_RUN_TEST(test_deplacer_fantome);
    MU_RUN_TEST(test_fantome_double_score);
    MU_RUN_TEST(test_placer_monstre);
    MU_RUN_TEST(test_score_zero_classe);
    MU_RUN_TEST(test_monstres_mode_facile);
    MU_RUN_TEST(test_monstres_mode_normal);
    MU_RUN_TEST(test_monstres_mode_difficile);
    MU_RUN_TEST(test_afficher_jeu_restaure_grille);
    MU_RUN_TEST(test_set_score_joueur);
    MU_RUN_TEST(test_piege_reste_bonus_disparait);
    MU_RUN_TEST(test_sortie_fermee_sans_cle);
    MU_RUN_TEST(test_joueur_gagne);
    MU_RUN_TEST(test_ogre_chasseur);
}

int main(void) {
    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    return MU_EXIT_CODE;
}