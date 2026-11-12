#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "minunit.h"
#include "labyrinthe.h"

#define TEST_LABY_FILE "test_laby_temp.txt"

const int H_CELLS = 5; 
const int L_CELLS = 5;
const int H_GRID = 2 * H_CELLS + 1; // 11
const int L_GRID = 2 * L_CELLS + 1; // 11

static Grille *g = NULL;
static Cellule *c = NULL;

void test_setup(void) {

    c = (Cellule*)malloc(sizeof(Cellule));
    allouer_cellules(c, H_CELLS, L_CELLS);
    initialiser_cellules(c);
    
    g = (Grille*)malloc(sizeof(Grille));
    allouer_grille(g, H_GRID, L_GRID);
    initialiser_grille(g, c);
    
    for (int i = 0; i < NOMBRE_SCORE; i++) {
        g->top_scores[i].name = NULL;
        g->top_scores[i].score = 0;
    }
}

void test_teardown(void) {

    if (g) {
        liberer_grille(g);
        free(g);
        g = NULL;
    }
    if (c) {
        liberer_cellules(c);
        free(c);
        c = NULL;
    }
    
    remove("labyrinthe/" TEST_LABY_FILE);
}

MU_TEST(test_initialisation_grille_et_cellules) {
    mu_assert_int_eq(H_GRID, g->hauteur);
    mu_assert_int_eq(L_GRID, g->largeur);
    
    mu_assert_int_eq(' ', g->cases[1][1]);
    mu_assert_int_eq('#', g->cases[2][1]); 
}

MU_TEST(test_creation_de_chemin_et_fusion) {
    int i, j;
    int found_path = 0;
    
    construire_labyrinthe(c, g);
    
    for (i = 1; i < H_GRID - 1; i++) {
        for (j = 1; j < L_GRID - 1; j++) {
            if (((i % 2 == 0 && j % 2 == 1) || (i % 2 == 1 && j % 2 == 0)) && g->cases[i][j] == ' ') {
                found_path = 1;
                break;
            }
        }
        if (found_path) break;
    }
    
    // Si la construction a eu lieu, un chemin a été créé (found_path = 1)
    mu_assert_int_eq(1, found_path);
    
    int groupe_ref = c->cellule_matrix[0][0];
    int groupes_differents = 0;
    for (int r = 0; r < H_CELLS; r++) {
        for (int k = 0; k < L_CELLS; k++) {
            if (c->cellule_matrix[r][k] != groupe_ref) {
                groupes_differents = 1;
                break;
            }
        }
    }
    // Après une seule étape, le labyrinthe ne doit pas être complet (il doit rester des groupes différents)
    mu_check(groupes_differents);
}

MU_TEST(test_etat_complet_et_finalisation) {

    mu_assert_int_eq(0, labyrinthe_complet(c, g));
    
    int ref_group = c->cellule_matrix[0][0];
    for (int i = 0; i < H_CELLS; i++) {
        for (int j = 0; j < L_CELLS; j++) {
            c->cellule_matrix[i][j] = ref_group;
        }
    }

    mu_assert_int_eq(1, labyrinthe_complet(c, g));

    mu_assert_int_eq('o', g->cases[0][1]);
    mu_assert_int_eq('-', g->cases[H_GRID - 1][L_GRID - 2]);
    
    mu_assert_int_eq(0, g->top_scores[0].score);
}

MU_TEST(test_sauvegarde_persistence) {
    
    g->top_scores[0].name = (char*)malloc(sizeof(char) * 50); 
    
    g->top_scores[0].score = 100;
    strncpy(g->top_scores[0].name, "TestUser", 50); 

    sauvegarder_labyrinthe(g, TEST_LABY_FILE);
    
    FILE *f = fopen("labyrinthe/" TEST_LABY_FILE, "r");
    mu_check(f != NULL);
    if (f) fclose(f);
}

MU_TEST(test_chargement_persistence) {
    
    const char *mock_content = 
        "3 5\n"       
        "#####\n"
        "# X #\n"       
        "#####\n"
        "150 Alpha\n"; 
    
    CREATE_DIR("labyrinthe");

    FILE *f_mock = fopen("labyrinthe/" TEST_LABY_FILE, "w");
    if (!f_mock) {
        mu_fail("Impossible de créer le fichier mock pour le chargement");
    }
    fprintf(f_mock, "%s", mock_content);
    fclose(f_mock);

    Grille g_loaded = {0}; 
    
    for (int i = 0; i < NOMBRE_SCORE; i++) {
        g_loaded.top_scores[i].name = NULL;
        g_loaded.top_scores[i].score = 0;
    }
    
    mu_assert_int_eq(1, charger_labyrinthe(&g_loaded, TEST_LABY_FILE));

    mu_assert_int_eq(3, g_loaded.hauteur);
    mu_assert_int_eq(5, g_loaded.largeur);
    mu_assert_int_eq('X', g_loaded.cases[1][2]); 
    mu_assert_int_eq(150, g_loaded.top_scores[0].score);
    mu_assert_int_eq(0, strcmp("Alpha", g_loaded.top_scores[0].name)); 

    mu_check(g_loaded.top_scores[1].name == NULL); // Emplacements vides initialisés

    liberer_grille(&g_loaded);
}

MU_TEST(test_chargement_fichier_absent) {

    Grille g_absent = {0};

    // Un nom inexistant ne doit plus quitter le programme
    mu_assert_int_eq(0, charger_labyrinthe(&g_absent, "fichier_qui_n_existe_pas"));
    mu_check(g_absent.cases == NULL);
}

MU_TEST(test_allouer_grille_initialise_scores) {

    Grille g_neuve;
    memset(&g_neuve, 0xAB, sizeof(g_neuve)); // Simule une variable non initialisée

    allouer_grille(&g_neuve, 3, 3);
    for (int i = 0; i < NOMBRE_SCORE; i++) {
        mu_check(g_neuve.top_scores[i].name == NULL);
        mu_assert_int_eq(0, g_neuve.top_scores[i].score);
    }

    liberer_grille(&g_neuve); // Ne doit pas libérer de pointeur invalide
}

MU_TEST(test_sauvegarde_puis_chargement) {

    CREATE_DIR("labyrinthe");
    g->top_scores[0].name = strdup("Zoe");
    g->top_scores[0].score = 42;
    g->cases[1][1] = '$';

    sauvegarder_labyrinthe(g, TEST_LABY_FILE);

    Grille g_relu = {0};
    mu_assert_int_eq(1, charger_labyrinthe(&g_relu, TEST_LABY_FILE));
    mu_assert_int_eq(H_GRID, g_relu.hauteur);
    mu_assert_int_eq(L_GRID, g_relu.largeur);
    mu_assert_int_eq('$', g_relu.cases[1][1]);
    mu_assert_int_eq('#', g_relu.cases[H_GRID - 1][L_GRID - 1]); // Dernière case bien lue
    mu_assert_int_eq(42, g_relu.top_scores[0].score);
    mu_assert_string_eq("Zoe", g_relu.top_scores[0].name);

    liberer_grille(&g_relu);
}


MU_TEST_SUITE(test_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);
    MU_RUN_TEST(test_initialisation_grille_et_cellules);
    MU_RUN_TEST(test_creation_de_chemin_et_fusion);
    MU_RUN_TEST(test_etat_complet_et_finalisation);
    MU_RUN_TEST(test_sauvegarde_persistence);
    MU_RUN_TEST(test_chargement_persistence);
    MU_RUN_TEST(test_chargement_fichier_absent);
    MU_RUN_TEST(test_allouer_grille_initialise_scores);
    MU_RUN_TEST(test_sauvegarde_puis_chargement);
}

int main(void) {
    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    return MU_EXIT_CODE;
}