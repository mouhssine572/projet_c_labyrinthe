# Jeu de Labyrinthe en C

## 📋 Description

Ce projet est un jeu de labyrinthe interactif développé en C. Le programme permet de **créer**, **charger** et **jouer** dans des labyrinthes générés aléatoirement. Le jeu inclut un système de scores persistant, trois modes de difficulté et des menus navigables au clavier.

## 🎮 Fonctionnalités

### 1. **Génération de Labyrinthe**
- Création automatique de labyrinthes avec dimensions personnalisables
- Algorithme de construction basé sur la fusion de cellules
- Labyrinthes sauvegardés pour rejouer ultérieurement
- Si le nom choisi existe déjà, le jeu **demande confirmation avant d'écraser** le labyrinthe (et son top 10)

### 2. **Gameplay**
- Déplacement du joueur avec les touches `z` (haut), `q` (gauche), `d` (droite), `s` (bas) ou les flèches, **sans appuyer sur Entrée** (`_getch()` sous Windows, `termios` sous Linux/macOS)
- L'écran est effacé à chaque tour : la carte reste en place, avec le score, la clé, la légende et le dernier événement affichés dessous
- Affichage en couleur (codes ANSI) : `G` rouge, `F` cyan, `$` jaune, `@` magenta, `&` vert
- Système de scoring basé sur le nombre de mouvements
- Objectif : récupérer la clé ($) et atteindre la sortie (-)
- À la fin d'une partie, le jeu propose de **rejouer le même labyrinthe**

### 3. **Motifs et Pièges**
- **Clé ($)** : Une seule par labyrinthe ; à récupérer pour pouvoir sortir
- **Ogre (G)** *(modes Normal et Difficile)* : Ne traverse pas les murs. En Normal, il erre au hasard. En Difficile, il **chasse** le joueur (plus court chemin) dès que celui-ci est à moins de 10 cases de couloir, sinon il erre au hasard. S'il attrape le joueur, la partie est perdue
- **Fantôme (F)** *(mode Difficile uniquement)* : Se déplace d'une case au hasard à chaque tour et traverse les murs ; s'il attrape le joueur, la partie continue mais le score est doublé, puis le fantôme réapparaît ailleurs
- **Pièges (@)** : Ajoutent 6 points de pénalité au score. **Ils restent en place** et pénalisent à chaque passage
- **Bonus (&)** : Retirent 6 points (minimum 0). **Ils disparaissent** une fois ramassés (usage unique)

### 4. **Modes de jeu**
Le mode est choisi à la création et enregistré dans le fichier du labyrinthe.

| Mode | Ogre | Fantôme | Pièges | Bonus |
|------|------|---------|--------|-------|
| **Facile** | non | non | 2 % de la surface | 2 % |
| **Normal** | oui, erre au hasard | non | 3 % | 1,5 % |
| **Difficile** | oui, chasseur | oui | 4 % | 1 % |

### 5. **Gestion des Scores**
- Top 10 des meilleurs scores enregistrés par labyrinthe
- Persistant : les scores sont sauvegardés dans les fichiers de labyrinthe
- Affichage du top des scores
- À la fin d'une partie gagnée, la place obtenue est affichée (« Vous êtes 3e sur ce labyrinthe ! »)

## 🧭 Navigation dans les menus

Tous les menus (menu principal, choix du joueur, du labyrinthe, du mode...) se parcourent au clavier :

- **Flèches haut/bas** (ou `z`/`s`) pour déplacer la sélection, **Entrée** pour valider
- **Taper le numéro** d'une option (1 à 9) la choisit directement
- **Échap** choisit « Revenir au menu » (sauf dans le menu principal, pour ne pas quitter par erreur)
- Les listes de plus de 10 éléments **défilent** : seule une partie est affichée, avec le nombre d'éléments cachés au-dessus et en dessous
- Quand il faut taper un nom ou un nombre, tapez `0` pour revenir au menu principal

Si l'entrée n'est pas un terminal (fichier ou tube), les menus demandent simplement le numéro de l'option.

## 📁 Structure du Projet

```
projet_c/
├── Makefile              # Configuration de compilation
├── README.md             # Ce fichier
├── include/
│   ├── labyrinthe.h      # Interface du système de labyrinthe
│   ├── jouer.h           # Interface du système de joueur
│   ├── menu.h            # Interface du menu
│   ├── terminal.h        # Saisie sans Entrée, effacement de l'écran, couleurs
│   └── minunit.h         # Framework de tests unitaires
├── src/
│   ├── main.c            # Point d'entrée du programme
│   ├── labyrinthe.c      # Implémentation du labyrinthe
│   ├── jouer.c           # Implémentation du joueur et gameplay
│   ├── menu.c            # Implémentation du menu
│   ├── terminal.c        # Gestion du terminal (Windows / Linux / macOS)
│   └── tests/
│       ├── test_labyrinthe.c  # Tests du système de labyrinthe
│       ├── test_jouer.c       # Tests du système de joueur
│       └── test_menu.c        # Tests du menu (noms valides, labyrinthes existants)
├── build/                # Fichiers objets et dépendances (.o, .d)
├── bin/                  # Exécutables compilés
├── labyrinthe/           # Dossier de sauvegarde des labyrinthes (créé automatiquement)
└── joueurs.txt           # Noms des joueurs connus (créé automatiquement)
```

## 🛠️ Compilation

### Prérequis
- GCC (compilateur C) et Make : sous Ubuntu / WSL, `sudo apt install build-essential`
- Valgrind (facultatif, pour `make memcheck`) : `sudo apt install valgrind`

### Commandes de Compilation

**Compiler le programme principal :**
```bash
make
```

**Lancer le jeu :**
```bash
./bin/main
```

**Compiler et exécuter tous les tests :**
```bash
make test
```

**Lancer les tests sous Valgrind (fuites et erreurs mémoire) :**
```bash
make memcheck
```

**Nettoyer les fichiers objets et dépendances :**
```bash
make clean
```

**Nettoyer complètement (binaires + dossiers de build) :**
```bash
make distclean
```

**Compiler en mode debug (avec symboles de débogage) :**
```bash
BUILD=debug make
```

## 🎯 Menu Principal

Au lancement du programme, vous avez 6 options :

1. **Créer un labyrinthe** : Choisissez le mode, les dimensions et le nom d'un nouveau labyrinthe
2. **Charger un labyrinthe** : Chargez un labyrinthe existant pour le visualiser
3. **Jouer** : Jouez dans un labyrinthe existant
4. **Consulter les scores** : Affichez le top 10 d'un labyrinthe
5. **Consulter les règles du jeu** : Affiche le but du jeu, les modes et la signification de chaque caractère (`#`, `o`, `-`, `*`, `$`, `@`, `&`, `G`, `F`)
6. **Quitter** : Quittez le programme

## 🎮 Comment Jouer

1. Sélectionnez **Jouer** dans le menu principal
2. Choisissez qui joue : les joueurs déjà connus (y compris ceux des sessions précédentes) sont listés ; choisissez « Nouveau joueur » pour taper un nouveau nom
3. Choisissez le labyrinthe dans lequel vous voulez jouer
4. Utilisez les touches de déplacement (pas besoin d'appuyer sur Entrée) :
   - `z` ou ↑ : Avancer (haut)
   - `q` ou ← : Aller à gauche
   - `d` ou → : Aller à droite
   - `s` ou ↓ : Reculer (bas)
   - `0` ou Échap : Abandonner et revenir au menu
5. Ramassez la clé ($) et atteignez la sortie (-) pour gagner
6. Votre score et la clé s'affichent sous la carte
7. À la fin de la partie, choisissez « Rejouer le même labyrinthe » ou « Revenir au menu »

## 📊 Système de Scoring

- **Score initial** : 0
- **Chaque mouvement** : +1 point
- **Piège (@)** : +6 points de pénalité
- **Bonus (&)** : -6 points (minimum 0)
- **Fantôme (F)** : score doublé
- **Objectif** : Terminer avec le score le plus bas possible

## 🧪 Tests

Le projet inclut une suite de tests unitaires utilisant le framework **minunit**.

### Tests Disponibles

**Test du labyrinthe** (`test_labyrinthe.c`) :
- Initialisation de la grille et des cellules
- Création de chemins et fusion de cellules
- Vérification de l'état complet du labyrinthe
- Sauvegarde et chargement de labyrinthe (avec le mode)
- Placement des motifs selon le mode (Facile, Normal, Difficile)

**Test du joueur** (`test_jouer.c`) :
- Positionnement du joueur
- Validation des mouvements
- Gestion des motifs (clé, pièges qui restent, bonus consommés)
- Ogre chasseur, fantôme, place dans le classement

**Test du menu** (`test_menu.c`) :
- Noms acceptés (`Paris_2`) et refusés (vide, `../x`, `a b`, accents...)
- Construction du chemin d'un labyrinthe, détection d'un labyrinthe existant

### Exécuter les Tests

```bash
make test       # les trois suites de tests
make memcheck   # les mêmes tests sous Valgrind
```

## 📝 Notes Importantes

- Les dimensions du labyrinthe (hauteur et largeur) doivent être impaires, entre 3 et 201 ; le programme redemande tant que la saisie est invalide
- Les noms (joueurs et labyrinthes) ne peuvent contenir que des lettres sans accent, des chiffres, `_` ou `-`
- Une touche invalide pendant la partie ne déplace pas le joueur et ne compte pas de point
- Fermer l'entrée (`Ctrl+D` sous Linux, `Ctrl+Z` puis Entrée sous Windows) abandonne la partie / quitte le programme
- Chaque fichier de labyrinthe stocke la grille, le mode et le top 10 des scores ; la première ligne est `hauteur largeur mode` (1 = Facile, 2 = Difficile, 3 = Normal ; les anciens fichiers sans mode sont chargés en mode Facile)
- Les labyrinthes sont sauvegardés dans le dossier `labyrinthe/`, créé automatiquement s'il n'existe pas
- Les noms des joueurs sont mémorisés dans `joueurs.txt` (le plus récent en premier). Au premier lancement, ce fichier est rempli avec les noms trouvés dans les classements des labyrinthes existants
- Les couleurs nécessitent un terminal compatible ANSI (Windows 10+, Linux, macOS)

## 👨‍💻 Auteur

Projet de jeu de labyrinthe en C - Mouhssine572

## 📄 Licence

Ce projet est fourni à titre éducatif.
