# ==============================================================================
# Makefile racine pour le Labyrinthe
# Usage :
#   make                # construit le jeu en 2D (bin/labyrinthe) et la version terminal (bin/main)
#   make jeu            # construit seulement le jeu en 2D (nécessite SDL2 et SDL2_ttf)
#   make console        # construit seulement la version terminal
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

# Bibliothèques de la version 2D
# (Ubuntu/WSL : sudo apt install libsdl2-dev libsdl2-ttf-dev libsdl2-mixer-dev)
SDL_CFLAGS := $(shell pkg-config --cflags sdl2 SDL2_ttf SDL2_mixer 2>/dev/null || sdl2-config --cflags 2>/dev/null)
SDL_LIBS   := $(shell pkg-config --libs sdl2 SDL2_ttf SDL2_mixer 2>/dev/null || echo "$$(sdl2-config --libs 2>/dev/null) -lSDL2_ttf -lSDL2_mixer")

# Fichiers sources et objets du projet principal
MAIN_SRCS = $(SRC_DIR)/main.c $(SRC_DIR)/labyrinthe.c $(SRC_DIR)/menu.c $(SRC_DIR)/jouer.c $(SRC_DIR)/terminal.c
MAIN_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(MAIN_SRCS))

# Version 2D : la fenêtre (graphique.c) + la logique du jeu, sans main.c
GUI_SRCS = $(SRC_DIR)/graphique.c $(SRC_DIR)/sons.c $(SRC_DIR)/labyrinthe.c $(SRC_DIR)/menu.c $(SRC_DIR)/jouer.c $(SRC_DIR)/terminal.c
GUI_OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(GUI_SRCS))

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
EXEC_GUI  = $(BIN_DIR)/labyrinthe
TEST_JOUER_EXEC = $(BIN_DIR)/test_jouer
TEST_LABY_EXEC  = $(BIN_DIR)/test_labyrinthe
TEST_MENU_EXEC  = $(BIN_DIR)/test_menu
ALL_TEST_EXECS  = $(TEST_LABY_EXEC) $(TEST_JOUER_EXEC) $(TEST_MENU_EXEC)

# Fichiers de dépendances générés par -MMD
DEPS = $(MAIN_OBJS:.o=.d) \
       $(BUILD_DIR)/graphique.d \
       $(BUILD_DIR)/sons.d \
       $(BUILD_DIR)/test_jouer.d \
       $(BUILD_DIR)/test_labyrinthe.d \
       $(BUILD_DIR)/test_menu.d

# Cibles phony et cible par défaut
.PHONY: all jeu console windows test memcheck clean distclean dirs

# Cible par défaut
all: dirs $(EXEC_GUI) $(EXEC)

# Le jeu en 2D seul, ou la version terminal seule
jeu: dirs $(EXEC_GUI)

console: dirs $(EXEC)

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

# La fenêtre 2D a besoin des en-têtes de SDL2
$(BUILD_DIR)/graphique.o: $(SRC_DIR)/graphique.c
	@$(MKDIR_P) $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SDL_CFLAGS) -c $< -o $@

# Les effets sonores ont besoin des en-têtes de SDL2_mixer
$(BUILD_DIR)/sons.o: $(SRC_DIR)/sons.c
	@$(MKDIR_P) $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(SDL_CFLAGS) -c $< -o $@

# Règle pour le jeu en 2D
$(EXEC_GUI): $(GUI_OBJS)
	$(CC) $^ -o $@ $(SDL_LIBS) -lm

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

# ------------------------- Version Windows (.exe) -----------------------------
# « make windows » compile le jeu 2D pour Windows depuis Linux/WSL (compilateur MinGW :
# sudo apt install gcc-mingw-w64-x86-64 zip). Le résultat est un dossier à donner tel quel :
#   bin/Labyrinthe-Windows/      Labyrinthe.exe, les DLL de SDL2, la police et un mode d'emploi
#   bin/Labyrinthe-Windows.zip   le même dossier, compressé
# Les bibliothèques SDL2 pour Windows sont téléchargées la première fois (outils/sdl_windows.sh).

WIN_CC     = x86_64-w64-mingw32-gcc
WIN_BUILD  = $(BUILD_DIR)/windows
WIN_SDL    = $(WIN_BUILD)/sdl
WIN_DIST   = $(BIN_DIR)/Labyrinthe-Windows
WIN_EXE    = $(WIN_DIST)/Labyrinthe.exe
WIN_CFLAGS = -Wall -Wextra -O2 -I$(INC_DIR) -I$(WIN_SDL)/include/SDL2
# -mwindows : pas de fenêtre de terminal ; -static-libgcc : une DLL de moins à livrer
WIN_LIBS   = -L$(WIN_SDL)/lib -lmingw32 -lSDL2main -lSDL2_ttf -lSDL2_mixer -lSDL2 -lm -mwindows -static-libgcc
WIN_OBJS   = $(patsubst $(SRC_DIR)/%.c, $(WIN_BUILD)/%.o, $(GUI_SRCS))
# Police livrée avec le jeu (DejaVu, libre de redistribution) pour avoir le même rendu partout
POLICE_DIR = /usr/share/fonts/truetype/dejavu

windows: $(WIN_EXE)
	@# Données de départ : uniquement le labyrinthe et le joueur fournis (pas les parties de test)
	rm -rf $(WIN_DIST)/labyrinthe $(WIN_DIST)/joueurs.txt
	@$(MKDIR_P) $(WIN_DIST)/labyrinthe
	cp labyrinthe/labyrinthe_1 $(WIN_DIST)/labyrinthe/
	cp joueurs.txt $(WIN_DIST)/
	@$(RM) $(BIN_DIR)/Labyrinthe-Windows.zip
	@if command -v zip >/dev/null; then \
	    cd $(BIN_DIR) && zip -qr Labyrinthe-Windows.zip Labyrinthe-Windows && \
	    echo "Prêt : $(BIN_DIR)/Labyrinthe-Windows.zip"; \
	else \
	    echo "Prêt : $(WIN_DIST)/ (installez zip pour obtenir aussi un .zip)"; \
	fi

# Téléchargement des en-têtes, bibliothèques et DLL de SDL2 pour Windows (une seule fois)
$(WIN_SDL)/include/SDL2/SDL.h:
	sh outils/sdl_windows.sh $(WIN_SDL)

$(WIN_BUILD)/%.o: $(SRC_DIR)/%.c $(WIN_SDL)/include/SDL2/SDL.h
	@$(MKDIR_P) $(WIN_BUILD)
	$(WIN_CC) $(WIN_CFLAGS) -c $< -o $@

$(WIN_EXE): $(WIN_OBJS)
	@$(MKDIR_P) $(WIN_DIST)/assets
	$(WIN_CC) $^ -o $@ $(WIN_LIBS)
	cp $(WIN_SDL)/bin/*.dll $(WIN_DIST)/
	cp $(POLICE_DIR)/DejaVuSans.ttf $(WIN_DIST)/assets/police.ttf
	cp $(POLICE_DIR)/DejaVuSans-Bold.ttf $(WIN_DIST)/assets/police-gras.ttf
	cp outils/LICENCE-DejaVu.txt $(WIN_DIST)/assets/
	cp outils/LISEZMOI-Windows.txt $(WIN_DIST)/LISEZMOI.txt

# ----------------------------- Règles de nettoyage ----------------------------

# Nettoyage des fichiers générés (.o, .d et fichier temporaire des tests)
clean:
	@$(RM) $(BUILD_DIR)/*.o $(BUILD_DIR)/*.d temp_input.txt

# Nettoyage complet (supprime exécutables et dossiers)
distclean: clean
	@$(RM) $(EXEC) $(EXEC_GUI)
	@$(RM) $(ALL_TEST_EXECS)
	@$(RM) -r $(BUILD_DIR)
	@$(RM) -r $(BIN_DIR)

# Inclusion des dépendances générées automatiquement
-include $(DEPS)