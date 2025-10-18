SRC_DIR = ./src
BIN_DIR = ./bin
BUILD_DIR = ./build
INCLUDE_DIR = ./include
TEST_DIR = $(SRC_DIR)/tests

# Options du compilateur
CC = gcc
CFLAGS = -Wall -Wextra
CPPFLAGS = -I$(INCLUDE_DIR)

OBJS = $(BUILD_DIR)/main.o $(BUILD_DIR)/labyrinthe.o $(BUILD_DIR)/menu.o
TEST_OBJS = $(BUILD_DIR)/test_labyrinthe.o

EXEC = $(BIN_DIR)/main
TEST_EXEC = $(BIN_DIR)/test_labyrinthe

#Mode debug
ifeq ($(MODE), debug)
	CFLAGS += -g -O0
endif

#Gestion automatique des dependances
DEPS = $(OBJS:.o=.d) $(TEST_OBJS:.o=.d)

all: $(EXEC)

# Construire l’exécutable
$(EXEC): $(OBJS)
	mkdir -p $(BIN_DIR); $(CC) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(BUILD_DIR); $(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

test: $(TEST_EXEC)

$(TEST_EXEC): $(TEST_OBJS)
	mkdir -p $(BIN_DIR); $(CC) $^ -o $@

$(BUILD_DIR)/test_labyrinthe.o: $(TEST_DIR)/test_labyrinthe.c
	mkdir -p $(BUILD_DIR); $(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

.PHONY: clean

# Nettoyage des fichiers générés
clean:
	rm -f $(OBJS) $(EXEC) $(TEST_OBJS) $(TEST_EXEC)

-include $(DEPS)
