#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "terminal.h"
#include "labyrinthe.h"

#ifdef _WIN32
    #include <windows.h>
    #include <conio.h>
    #include <io.h>
    #define ENTREE_EST_TERMINAL() _isatty(_fileno(stdin))
    #ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
        #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
    #endif
#else
    #include <termios.h>
    #include <unistd.h>
    #include <signal.h>
    #define ENTREE_EST_TERMINAL() isatty(fileno(stdin))
#endif

/* Codes de contrôle qui signifient « fin de l'entrée » en mode brut. */
#define CTRL_C 3
#define CTRL_D 4
#define CTRL_Z 26

#ifndef _WIN32
static struct termios terminal_origine;
static int terminal_brut = 0;

static void restaurer_terminal(void) {
    if (terminal_brut) {
        tcsetattr(STDIN_FILENO, TCSANOW, &terminal_origine);
        terminal_brut = 0;
    }
}

/* Si le programme est interrompu pendant la lecture d'une touche, on rend le terminal normal. */
static void gestion_signal(int signal_recu) {
    restaurer_terminal();
    _exit(128 + signal_recu);
}

static void passer_en_mode_brut(void) {
    struct termios brut;
    if (tcgetattr(STDIN_FILENO, &terminal_origine) != 0) return;
    brut = terminal_origine;
    brut.c_lflag &= ~(ICANON | ECHO); // Pas de tampon de ligne, pas d'écho
    brut.c_cc[VMIN] = 1;
    brut.c_cc[VTIME] = 0;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &brut) == 0) {
        terminal_brut = 1;
    }
}
#endif

void initialiser_terminal(void) {
#ifdef _WIN32
    HANDLE sortie = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    if (sortie != INVALID_HANDLE_VALUE && GetConsoleMode(sortie, &mode)) {
        SetConsoleMode(sortie, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
    SetConsoleOutputCP(CP_UTF8); // Accents correctement affichés
#else
    signal(SIGINT, gestion_signal);
    signal(SIGTERM, gestion_signal);
    atexit(restaurer_terminal);
#endif
}

/* Lecture classique (entrée redirigée) : on saute les blancs comme scanf(" %c"). */
static int lire_touche_tamponnee(void) {
    int c;
    do {
        c = getchar();
    } while (c != EOF && isspace(c));
    return (c == EOF) ? TOUCHE_FIN : c;
}

int lire_touche(void) {
    int c;

    fflush(stdout); // L'affichage doit être visible avant d'attendre la touche

    if (!ENTREE_EST_TERMINAL()) {
        return lire_touche_tamponnee();
    }

#ifdef _WIN32
    c = _getch();
    if (c == 0 || c == 0xE0) { // Touche spéciale : le code suit
        switch (_getch()) {
            case 72: return 'z'; // Flèche haut
            case 80: return 's'; // Flèche bas
            case 75: return 'q'; // Flèche gauche
            case 77: return 'd'; // Flèche droite
            default: return '?';
        }
    }
#else
    passer_en_mode_brut();
    c = getchar();
    if (c == 27) { // Séquence d'échappement des flèches : ESC [ A/B/C/D
        int c2 = getchar();
        int c3 = (c2 == '[' || c2 == 'O') ? getchar() : 0;
        restaurer_terminal();
        switch (c3) {
            case 'A': return 'z';
            case 'B': return 's';
            case 'D': return 'q';
            case 'C': return 'd';
            default: return '?';
        }
    }
    restaurer_terminal();
    if (c == EOF) return TOUCHE_FIN;
#endif

    if (c == CTRL_C || c == CTRL_D || c == CTRL_Z) return TOUCHE_FIN;
    return tolower(c);
}

void effacer_ecran(void) {
    printf("\033[H\033[2J");
}

const char *couleur_case(char c) {
    switch (c) {
        case '#':           return COULEUR_SOMBRE;
        case MOTIF_JOUEUR:  return COULEUR_BLANC;
        case MOTIF_OGRE:    return COULEUR_ROUGE;
        case MOTIF_FANTOME: return COULEUR_CYAN;
        case MOTIF_KEY:     return COULEUR_JAUNE;
        case MOTIF_TRAP:    return COULEUR_MAGENTA;
        case MOTIF_BOOST:   return COULEUR_VERT;
        case MOTIF_SORTIE:  return COULEUR_VERT;
        case MOTIF_ENTREE:  return COULEUR_GRAS;
        default:            return "";
    }
}
