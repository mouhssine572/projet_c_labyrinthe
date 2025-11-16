#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "minunit.h" 
#include "jouer.h" 


Grille *g = NULL;
Joueur *j = NULL;
const int H = 3; 
const int L = 5; 

extern void initialiser_joueur(Joueur *joueur);
extern void positionner_joueur(Grille *grille, Joueur *joueur);
extern int mouvement_valide(Grille *grille, int x, int y, Joueur *joueur);
extern void mouvement_joueur(Grille *grille, Joueur *joueur);

void test_setup(void) {

    g = (Grille*)malloc(sizeof(Grille));
    g->hauteur = H;
    g->largeur = L;
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
    
    char buffer[] = "s\n";
    FILE *temp_in = freopen("temp_input.txt", "w+", stdin);
    fwrite(buffer, 1, strlen(buffer), temp_in);
    rewind(temp_in);
    
    mouvement_joueur(g, j);

    mu_assert_int_eq(' ', g->cases[0][1]); // L'ancienne case (0, 1) doit être vide
    mu_assert_int_eq('*', g->cases[1][1]); // La nouvelle case (1, 1) doit être '*'
    mu_assert_int_eq(1, j->x);             // Coordonnée X du joueur doit être 1
    
    fclose(temp_in);

}


MU_TEST_SUITE(test_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);
    
    MU_RUN_TEST(test_positionner_joueur);
    MU_RUN_TEST(test_mouvement_valide);
    MU_RUN_TEST(test_mouvement_joueurt); 
}

int main(int /*argc*/, char * /*argv*/ []) {
    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    return MU_EXIT_CODE;
}