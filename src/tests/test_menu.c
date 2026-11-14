#include <stdio.h>
#include <string.h>
#include "minunit.h"
#include "labyrinthe.h"
#include "menu.h"

#define TEST_MENU_FILE "test_menu_temp.txt"

static void test_teardown(void) {
    char chemin[TAILLE_CHEMIN];
    chemin_labyrinthe(TEST_MENU_FILE, chemin);
    remove(chemin);
}

MU_TEST(test_nom_valide_accepte) {
    mu_assert_int_eq(1, nom_valide("Paris_2"));
    mu_assert_int_eq(1, nom_valide("a"));
    mu_assert_int_eq(1, nom_valide("laby-01"));
}

MU_TEST(test_nom_valide_refuse) {
    mu_assert_int_eq(0, nom_valide(""));          // Vide
    mu_assert_int_eq(0, nom_valide("../x"));      // Sortie du dossier
    mu_assert_int_eq(0, nom_valide("a/b"));       // Sous-dossier
    mu_assert_int_eq(0, nom_valide("a b"));       // Espace
    mu_assert_int_eq(0, nom_valide("nom.txt"));   // Point
    mu_assert_int_eq(0, nom_valide("été"));       // Accents
}

MU_TEST(test_chemin_labyrinthe) {
    char chemin[TAILLE_CHEMIN];
    chemin_labyrinthe("Paris_2", chemin);
    mu_assert_string_eq(DOSSIER_LABYRINTHES "/Paris_2", chemin);
}

MU_TEST(test_labyrinthe_existe) {
    char chemin[TAILLE_CHEMIN];
    chemin_labyrinthe(TEST_MENU_FILE, chemin);

    remove(chemin);
    mu_assert_int_eq(0, labyrinthe_existe(TEST_MENU_FILE));

    CREATE_DIR(DOSSIER_LABYRINTHES);
    FILE *f = fopen(chemin, "w");
    mu_check(f != NULL);
    fclose(f);

    mu_assert_int_eq(1, labyrinthe_existe(TEST_MENU_FILE));
    mu_assert_int_eq(1, labyrinthe_disponible());
}

MU_TEST_SUITE(test_suite) {
    MU_SUITE_CONFIGURE(NULL, &test_teardown);
    MU_RUN_TEST(test_nom_valide_accepte);
    MU_RUN_TEST(test_nom_valide_refuse);
    MU_RUN_TEST(test_chemin_labyrinthe);
    MU_RUN_TEST(test_labyrinthe_existe);
}

int main(void) {
    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    return MU_EXIT_CODE;
}
