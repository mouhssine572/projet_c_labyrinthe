#include "minunit.h"
#include "labyrinthe.h"

/*this is just a test for the git push origin master*/
void test_setup(void) {
}

void test_teardown(void) {
}

MU_TEST(test_initialiser_grille) {
}

MU_TEST(test_construire_labyrinthe) {
}

MU_TEST(test_labyrinthe_complet) {
}

MU_TEST(test_sauvegarder_labyrinthe) {
}

MU_TEST(test_charger_labyrinthe) {
}

MU_TEST_SUITE(test_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);
    MU_RUN_TEST(test_initialiser_grille);
    MU_RUN_TEST(test_construire_labyrinthe);
    MU_RUN_TEST(test_labyrinthe_complet);
    MU_RUN_TEST(test_sauvegarder_labyrinthe);
    MU_RUN_TEST(test_charger_labyrinthe); 
}

int main(int argc, char *argv[]) {
    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    return MU_EXIT_CODE;
}
