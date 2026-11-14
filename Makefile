# ==============================================================================
# Makefile racine pour le Labyrinthe
# Usage :
#   make                # construit l'exécutable principal
#   make test           # construit et lance les tests
#   make memcheck       # lance les tests sous Valgrind (fuites et erreurs mémoire)
#   make clean          # nettoie les fichiers objets (.o, .d)
#   make distclean      # nettoie les binaires et les dossiers de construction
#   BUILD=debug make    # ajoute -g -O0 pour le débogage
# ==============================================================================

# Outils
CC      = gcc
RM      = rm -f
MKDIR_P = mkdir -p

# Dossiers
SRC_DIR   = ./src
TEST_DIR  = $(SRC_DIR)/tests
BUILD_DIR = ./build
BIN_DIR   = ./bin
INC_DIR   = ./include

# Options de compilation/édition de liens
CPPFLAGS = -I$(INC_DIR) -MMD -MP # -MMD/-MP pour la gestion automatique des dépendances
CFLAGS   = -Wall -Wextra

# Mode debug
ifeq ($(BUILD),debug)
  CFLAGS += -g -O0
else
  CFLAGS += -O2
endif

# Fichiers sources et objets du projet principal
MAIN_SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/labyrinthe.c $(SRC_DIR)/menu.c $(SRC_DIR)/jouer.c $(SRC_DIR)/terminal.c
MAIN_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(MAIN_SRCS))

# Fichiers sources et objets des tests
TEST_JOUER_SRC     = $(TEST_DIR)/test_jouer.c
TEST_LABY_SRC      = $(TEST_DIR)/test_labyrinthe.c
TEST_MENU_SRC      = $(TEST_DIR)/test_menu.c

# Le test du joueur dépend de jouer.o, labyrinthe.o et terminal.o
TEST_JOUER_OBJS    = $(BUILD_DIR)/test_jouer.o $(BUILD_DIR)/jouer.o $(BUILD_DIR)/labyrinthe.o $(BUILD_DIR)/terminal.o
# Le test du labyrinthe dépend de labyrinthe.o et terminal.o (affichage en couleur)
TEST_LABY_OBJS     = $(BUILD_DIR)/test_labyrinthe.o $(BUILD_DIR)/labyrinthe.o $(BUILD_DIR)/terminal.o
# Le test du menu dépend de menu.o et de ce qu'il utilise (labyrinthe, jouer, terminal)
TEST_MENU_OBJS     = $(BUILD_DIR)/test_menu.o $(BUILD_DIR)/menu.o $(BUILD_DIR)/jouer.o $(BUILD_DIR)/labyrinthe.o $(BUILD_DIR)/terminal.o

# Exécutables
EXEC      = $(BIN_DIR)/main
TEST_JOUER_EXEC = $(BIN_DIR)/test_jouer
TEST_LABY_EXEC  = $(BIN_DIR)/test_labyrinthe
TEST_MENU_EXEC  = $(BIN_DIR)/test_menu
ALL_TEST_EXECS  = $(TEST_LABY_EXEC) $(TEST_JOUER_EXEC) $(TEST_MENU_EXEC)

# Fichiers de dépendances générés par -MMD
DEPS = $(MAIN_OBJS:.o=.d) \
       $(BUILD_DIR)/test_jouer.d \
       $(BUILD_DIR)/test_labyrinthe.d \n       $(BUILD_DIR)/test_menu.d

# Cibles phony et cible par défaut
.PHONY: all test memcheck clean distclean dirs

# Cible par défaut
all: dirs $(EXEC)

# Cible pour construire et exécuter tous les tests
test: dirs $(ALL_TEST_EXECS)
	@echo "--- Exécution des tests du labyrinthe ---"
	./$(TEST_LABY_EXEC)
	@echo "--- Exécution des tests du joueur ---"
	./$(TEST_JOUER_EXEC)
	@echo "--- Exécution des tests du menu ---"
	./$(TEST_MENU_EXEC)
	@$(RM) temp_input.txt # Nettoie le fichier temporaire de test_jouer.c

# Lance les tests sous Valgrind : échoue en cas de fuite ou d'accès mémoire invalide
# (installation : sudo apt install valgrind). Pour le jeu lui-même : valgrind --leak-check=full ./bin/main
VALGRIND = valgrind --leak-check=full --show-leak-kinds=definite --errors-for-leak-kinds=definite --error-exitcode=1 -q
memcheck: dirs $(ALL_TEST_EXECS)
	$(VALGRIND) ./$(TEST_LABY_EXEC)
	$(VALGRIND) ./$(TEST_JOUER_EXEC)
	$(VALGRIND) ./$(TEST_MENU_EXEC)
	@$(RM) temp_input.txt

# -------------------------- Règles de construction ----------------------------

# Création des dossiers de sortie
dirs:
	@$(MKDIR_P) $(BIN_DIR) $(BUILD_DIR)

# Règle générale de compilation des sources (fichiers main, jouer, labyrinthe, menu)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@$(MKDIR_P) $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Règle pour l'exécutable principal
$(EXEC): $(MAIN_OBJS)
	$(CC) $^ -o $@

# Règle spécifique pour la compilation de test_jouer.o
$(BUILD_DIR)/test_jouer.o: $(TEST_DIR)/test_jouer.c
	@$(MKDIR_P) $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Règle spécifique pour la compilation de test_labyrinthe.o
$(BUILD_DIR)/test_labyrinthe.o: $(TEST_DIR)/test_labyrinthe.c
	@$(MKDIR_P) $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Règle spécifique pour la compilation de test_menu.o
$(BUILD_DIR)/test_menu.o: $(TEST_DIR)/test_menu.c
	@$(MKDIR_P) $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Règle pour l'exécutable de test du joueur
$(TEST_JOUER_EXEC): $(TEST_JOUER_OBJS)
	$(CC) $^ -o $@

# Règle pour l'exécutable de test du labyrinthe
$(TEST_LABY_EXEC): $(TEST_LABY_OBJS)
	$(CC) $^ -o $@

# Règle pour l'exécutable de test du menu
$(TEST_MENU_EXEC): $(TEST_MENU_OBJS)
	$(CC) $^ -o $@

# ----------------------------- Règles de nettoyage ----------------------------

# Nettoyage des fichiers générés (.o, .d et fichier temporaire des tests)
clean:
	@$(RM) $(BUILD_DIR)/*.o $(BUILD_DIR)/*.d temp_input.txt

# Nettoyage complet (supprime exécutables et dossiers)
distclean: clean
	@$(RM) $(EXEC)
	@$(RM) $(ALL_TEST_EXECS)
	@$(RM) -r $(BUILD_DIR)
	@$(RM) -r $(BIN_DIR)

# Inclusion des dépendances générées automatiquement
-include $(DEPS)