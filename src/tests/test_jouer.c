#include "minunit.h"
#include "jouer.h"

void test_setup(void) {
}

void test_teardown(void) {
}

MU_TEST(test_positionner_joueur) {
}

MU_TEST(test_mouvement_valide) {
}

MU_TEST(test_mouvement_joueurt) {
}

MU_TEST_SUITE(test_suite) {
    MU_SUITE_CONFIGURE(&test_setup, &test_teardown);
    MU_RUN_TEST(test_positionner_joueur);
    MU_RUN_TEST(test_mouvement_valide);
    MU_RUN_TEST(test_mouvement_joueurt); 
}

int main(int argc, char *argv[]) {
    MU_RUN_SUITE(test_suite);
    MU_REPORT();
    return MU_EXIT_CODE;
}
