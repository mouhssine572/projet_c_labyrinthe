# Jeu de Labyrinthe en C

## 📋 Description

Ce projet est un jeu de labyrinthe interactif développé en C. Le programme permet de **créer**, **charger** et **jouer** dans des labyrinthes générés aléatoirement. Le jeu inclut un système de scores persistant et diverses mécaniques de gameplay.

## 🎮 Fonctionnalités

### 1. **Génération de Labyrinthe**
- Création automatique de labyrinthes avec dimensions personnalisables
- Algorithme de construction basé sur la fusion de cellules
- Labyrinthes sauvegardés pour rejouer ultérieurement

### 2. **Gameplay**
- Déplacement du joueur avec les touches `z` (haut), `q` (gauche), `d` (droite), `s` (bas)
- Système de scoring basé sur le nombre de mouvements
- Objectif : récupérer la clé ($) et atteindre la sortie (-)

### 3. **Motifs et Pièges**
- **Clé ($)** : À récupérer pour pouvoir sortir du labyrinthe
- **Ogre (G)** : Se déplace d'une case au hasard à chaque tour (sans traverser les murs) ; s'il attrape le joueur, la partie est perdue
- **Fantôme (F)** : Se déplace d'une case au hasard à chaque tour et traverse les murs ; s'il attrape le joueur, la partie continue mais le score est doublé, puis le fantôme réapparaît ailleurs
- **Pièges (@)** : Ajoutent 6 points de pénalité au score
- **Bonus (&)** : Réduisent la pénalité en retirant 6 points (minimum 0)

### 4. **Gestion des Scores**
- Top 10 des meilleurs scores enregistrés par labyrinthe
- Persistent : les scores sont sauvegardés dans les fichiers de labyrinthe
- Affichage du top des scores

## 📁 Structure du Projet

```
projet_c/
├── Makefile              # Configuration de compilation
├── README.md             # Ce fichier
├── include/
│   ├── labyrinthe.h      # Interface du système de labyrinthe
│   ├── jouer.h           # Interface du système de joueur
│   ├── menu.h            # Interface du menu
│   └── minunit.h         # Framework de tests unitaires
├── src/
│   ├── main.c            # Point d'entrée du programme
│   ├── labyrinthe.c      # Implémentation du labyrinthe
│   ├── jouer.c           # Implémentation du joueur et gameplay
│   ├── menu.c            # Implémentation du menu
│   └── tests/
│       ├── test_labyrinthe.c  # Tests du système de labyrinthe
│       └── test_jouer.c       # Tests du système de joueur
├── build/                # Fichiers objets et dépendances (.o, .d)
├── bin/                  # Exécutables compilés
└── labyrinthe/           # Dossier de sauvegarde des labyrinthes
```

## 🛠️ Compilation

### Prérequis
- GCC (compilateur C)
- Make

### Commandes de Compilation

**Compiler le programme principal :**
```bash
make
```

**Compiler et exécuter tous les tests :**
```bash
make test
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

1. **Créer un labyrinthe** : Générez un nouveau labyrinthe avec vos dimensions
2. **Charger un labyrinthe** : Chargez un labyrinthe existant pour le visualiser
3. **Jouer** : Jouez dans un labyrinthe existant
4. **Consulter les scores** : Affichez le top 10 d'un labyrinthe
5. **Consulter les règles du jeu** : Affiche le but du jeu et la signification de chaque caractère (`#`, `o`, `-`, `*`, `$`, `@`, `&`, `G`, `F`)
6. **Quitter** : Quittez le programme

À chaque question (mode, dimensions, nom du joueur, nom du labyrinthe), tapez `0` pour revenir au menu principal. Pendant une partie, la touche `0` abandonne la partie et ramène au menu.

## 🎮 Comment Jouer

1. Sélectionnez l'option **3 - Jouer** du menu principal
2. Entrez votre nom de joueur
3. Choisissez le labyrinthe dans lequel vous voulez jouer
4. Utilisez les touches de déplacement :
   - `z` : Avancer (haut)
   - `q` : Aller à gauche
   - `d` : Aller à droite
   - `s` : Reculer (bas)
   - `0` : Abandonner et revenir au menu
5. Ramassez la clé ($) et atteignez la sortie (-) pour gagner
6. Votre score s'affiche après chaque mouvement

## 📊 Système de Scoring

- **Score initial** : 0
- **Chaque mouvement** : +1 point
- **Piège (@)** : +6 points de pénalité
- **Bonus (&)** : -6 points (minimum 0)
- **Fantôme (F)** : score doublé
- **Objectif** : Terminer avec le score le plus bas possible

## 🧪 Tests

Le projet inclut une suite de tests unitaires complète utilisant le framework **minunit**.

### Tests Disponibles

**Test du labyrinthe** (`test_labyrinthe.c`) :
- Initialisation de la grille et des cellules
- Création de chemins et fusion de cellules
- Vérification de l'état complet du labyrinthe
- Sauvegarde et chargement de labyrinthe

**Test du joueur** (`test_jouer.c`) :
- Positionnement du joueur
- Validation des mouvements
- Gestion des motifs (clé, pièges, bonus)

### Exécuter les Tests

```bash
make test
```

Cela compilera et exécutera les deux suites de tests.

## 📝 Notes Importantes

- Les dimensions du labyrinthe (hauteur et largeur) doivent être impaires, entre 3 et 201 ; le programme redemande tant que la saisie est invalide
- Une touche invalide pendant la partie ne déplace pas le joueur et ne compte pas de point
- Fermer l'entrée (`Ctrl+D` sous Linux, `Ctrl+Z` puis Entrée sous Windows) abandonne la partie / quitte le programme
- Chaque fichier de labyrinthe stocke la grille et le top 10 des scores
- Les labyrinthes sont sauvegardés dans le dossier `labyrinthe/`
- Le mode 2 (Difficile) est actuellement en cours de développement

## 👨‍💻 Auteur

Projet de jeu de labyrinthe en C - Mouhssine572

## 📄 Licence

Ce projet est fourni à titre éducatif.
