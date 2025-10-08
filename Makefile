# Nom de l’exécutable
EXEC = bin/labyrinthe

# Dossier contenant le code source
SRC_DIR = src

# Fichiers source .c
SRCS = $(wildcard $(SRC_DIR)/*.c)

# Fichiers objets .o correspondants
OBJS = $(SRCS:.c=.o)

# Options du compilateur
CC = gcc
CFLAGS = -Wall -Wextra -g

# Règle par défaut : construire l’exécutable
all: $(EXEC)

# Comment construire l’exécutable
$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Comment construire les .o à partir des .c
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage des fichiers générés
clean:
	rm -f $(SRC_DIR)/*.o $(EXEC)

# Tout supprimer (clean + fichiers sauvegardés, scores...)
mrproper: clean
	rm -f save/*.cfg scores/*.score

# Forcer la reconstruction
re: clean all
