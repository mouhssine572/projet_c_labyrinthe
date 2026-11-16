/*
 * graphique.c : version 2D du jeu de labyrinthe, dans une fenêtre (SDL2 + SDL2_ttf).
 *
 * Toute la logique du jeu (génération, déplacements, monstres, scores, fichiers) vient de
 * labyrinthe.c, jouer.c et menu.c : ce fichier ne fait que l'affichage et la lecture du
 * clavier et de la souris. Chaque écran (menu, saisie, partie...) a sa propre petite boucle
 * d'événements, comme les fonctions de menu.c dans la version terminal.
 */
#include <SDL.h>
#include <SDL_ttf.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "jouer.h"
#include "labyrinthe.h"
#include "menu.h"
#include "terminal.h" /* TOUCHE_ECHAP */

/* ------------------------------------------------------------------------------------------
 * Réglages
 * ------------------------------------------------------------------------------------------ */

#define FENETRE_LARGEUR 1100
#define FENETRE_HAUTEUR 800
#define FENETRE_LARGEUR_MIN 800
#define FENETRE_HAUTEUR_MIN 600
#define IMAGES_PAR_SECONDE 60

#define HAUTEUR_BANDEAU 84    /* Bandeau du haut pendant la partie (score, clé, mode...) */
#define HAUTEUR_BARRE 58      /* Barre du bas pendant la partie (message, aide) */
#define MARGE 16
#define TUILE_MAX 48          /* Taille maximale d'une case à l'écran (en pixels) */
#define TUILE_MIN_JEU 18      /* En dessous, la caméra suit le joueur au lieu de tout montrer */
#define DELAI_REPETITION 85   /* Touche maintenue : un pas toutes les DELAI_REPETITION ms */
#define DUREE_FIN 650         /* Animation de fin de partie avant l'écran de résultat (ms) */
#define DUREE_MESSAGE 4500    /* Durée d'affichage d'un message avant qu'il ne s'efface (ms) */
#define DUREE_FLASH 320       /* Durée du flash coloré (piège, bonus, clé, fantôme) */
#define DUREE_CONSTRUCTION 4000 /* Durée maximale de l'animation de construction (ms) */

#define BOUTON_HAUTEUR 46
#define BOUTON_ESPACE 8

/* Polices essayées dans l'ordre : celle du projet, puis celles du système. */
static const char *POLICES[] = {
    "assets/police.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/TTF/DejaVuSans.ttf",
    "/usr/share/fonts/dejavu/DejaVuSans.ttf",
    "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
    "/System/Library/Fonts/Supplemental/Arial.ttf",
    "/Library/Fonts/Arial.ttf",
    "C:/Windows/Fonts/segoeui.ttf",
    "C:/Windows/Fonts/arial.ttf",
};
static const char *POLICES_GRAS[] = {
    "assets/police-gras.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
    "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf",
    "/usr/share/fonts/dejavu/DejaVuSans-Bold.ttf",
    "/usr/share/fonts/truetype/ubuntu/Ubuntu-B.ttf",
    "/usr/share/fonts/truetype/liberation/LiberationSans-Bold.ttf",
    "/System/Library/Fonts/Supplemental/Arial Bold.ttf",
    "/Library/Fonts/Arial Bold.ttf",
    "C:/Windows/Fonts/segoeuib.ttf",
    "C:/Windows/Fonts/arialbd.ttf",
};

/* Couleurs */
static const SDL_Color FOND          = { 14,  16,  26, 255};
static const SDL_Color FOND_MOTIF    = {255, 255, 255,   7};
static const SDL_Color PANNEAU       = { 26,  30,  46, 255};
static const SDL_Color PANNEAU_BORD  = { 58,  66, 102, 255};
static const SDL_Color BOUTON        = { 37,  42,  64, 255};
static const SDL_Color ACCENT        = {255, 196,  60, 255};
static const SDL_Color TEXTE         = {236, 238, 245, 255};
static const SDL_Color TEXTE_SOMBRE  = { 18,  18,  28, 255};
static const SDL_Color TEXTE_DOUX    = {150, 156, 180, 255};
static const SDL_Color ROUGE         = {235,  80,  80, 255};
static const SDL_Color VERT          = { 80, 215, 130, 255};
static const SDL_Color CYAN          = {110, 225, 255, 255};
static const SDL_Color ORANGE        = {245, 150,  60, 255};

static const SDL_Color SOL_1         = { 33,  37,  54, 255};
static const SDL_Color SOL_2         = { 36,  40,  58, 255};
static const SDL_Color MUR_DESSUS    = { 74,  86, 132, 255};
static const SDL_Color MUR_FACE      = { 46,  53,  86, 255};
static const SDL_Color OR            = {255, 205,  55, 255};
static const SDL_Color OR_SOMBRE     = {190, 140,  20, 255};

/* ------------------------------------------------------------------------------------------
 * Fenêtre et dessin de base
 * ------------------------------------------------------------------------------------------ */

typedef struct {
    SDL_Window *fenetre;
    SDL_Renderer *rendu;
    TTF_Font *police_titre;  /* Grande, en gras */
    TTF_Font *police_gras;
    TTF_Font *police;
    TTF_Font *police_petite;
    int largeur;             /* Taille actuelle de la zone de dessin */
    int hauteur;
    int quitter;             /* 1 quand l'utilisateur a fermé la fenêtre */
    Uint32 derniere_image;
} App;

/* Fonction qui dessine ce qu'il y a derrière un menu (par exemple la partie terminée). */
typedef void (*Fond)(App *app, void *donnees);

static float minf(float a, float b) { return a < b ? a : b; }
static float maxf(float a, float b) { return a > b ? a : b; }
static int mini(int a, int b) { return a < b ? a : b; }
static int maxi(int a, int b) { return a > b ? a : b; }
static float temps_secondes(void) { return SDL_GetTicks() / 1000.0f; }

static SDL_Color avec_alpha(SDL_Color c, int alpha) {
    c.a = (Uint8) (alpha < 0 ? 0 : alpha > 255 ? 255 : alpha);
    return c;
}

static void choisir_couleur(App *app, SDL_Color c) {
    SDL_SetRenderDrawColor(app->rendu, c.r, c.g, c.b, c.a);
}

static void rectangle(App *app, float x, float y, float w, float h, SDL_Color c) {
    SDL_FRect r = { x, y, w, h };
    choisir_couleur(app, c);
    SDL_RenderFillRectF(app->rendu, &r);
}

/* Secteur de disque (de l'angle a0 à a1, en radians), dessiné en éventail de triangles. */
static void secteur(App *app, float cx, float cy, float rayon, float a0, float a1, SDL_Color c) {
    enum { SEGMENTS = 24 };
    SDL_Vertex v[SEGMENTS + 2];
    int indices[SEGMENTS * 3];

    v[0].position = (SDL_FPoint) { cx, cy };
    v[0].color = c;
    v[0].tex_coord = (SDL_FPoint) { 0, 0 };
    for (int i = 0; i <= SEGMENTS; i++) {
        float a = a0 + (a1 - a0) * i / SEGMENTS;
        v[i + 1].position = (SDL_FPoint) { cx + cosf(a) * rayon, cy + sinf(a) * rayon };
        v[i + 1].color = c;
        v[i + 1].tex_coord = (SDL_FPoint) { 0, 0 };
    }
    for (int i = 0; i < SEGMENTS; i++) {
        indices[3 * i] = 0;
        indices[3 * i + 1] = i + 1;
        indices[3 * i + 2] = i + 2;
    }
    SDL_RenderGeometry(app->rendu, NULL, v, SEGMENTS + 2, indices, SEGMENTS * 3);
}

static void disque(App *app, float cx, float cy, float rayon, SDL_Color c) {
    secteur(app, cx, cy, rayon, 0, 2 * (float) M_PI, c);
}

/* Polygone convexe (points dans l'ordre). */
static void polygone(App *app, const SDL_FPoint *points, int nbr, SDL_Color c) {
    SDL_Vertex v[8];
    int indices[18];
    if (nbr > 8) nbr = 8;
    for (int i = 0; i < nbr; i++) {
        v[i].position = points[i];
        v[i].color = c;
        v[i].tex_coord = (SDL_FPoint) { 0, 0 };
    }
    for (int i = 0; i < nbr - 2; i++) {
        indices[3 * i] = 0;
        indices[3 * i + 1] = i + 1;
        indices[3 * i + 2] = i + 2;
    }
    SDL_RenderGeometry(app->rendu, NULL, v, nbr, indices, (nbr - 2) * 3);
}

static void triangle(App *app, float x1, float y1, float x2, float y2, float x3, float y3, SDL_Color c) {
    SDL_FPoint p[3] = { {x1, y1}, {x2, y2}, {x3, y3} };
    polygone(app, p, 3, c);
}

/* Rectangle aux coins arrondis (sans recouvrement, pour que la transparence reste uniforme). */
static void rectangle_arrondi(App *app, float x, float y, float w, float h, float r, SDL_Color c) {
    const float PI = (float) M_PI;
    r = minf(r, minf(w, h) / 2);
    rectangle(app, x + r, y, w - 2 * r, h, c);
    rectangle(app, x, y + r, r, h - 2 * r, c);
    rectangle(app, x + w - r, y + r, r, h - 2 * r, c);
    secteur(app, x + r,     y + r,     r, PI,         1.5f * PI, c);
    secteur(app, x + w - r, y + r,     r, 1.5f * PI,  2 * PI,    c);
    secteur(app, x + w - r, y + h - r, r, 0,          0.5f * PI, c);
    secteur(app, x + r,     y + h - r, r, 0.5f * PI,  PI,        c);
}

/* Panneau arrondi avec une bordure. */
static void panneau(App *app, float x, float y, float w, float h, SDL_Color fond, SDL_Color bord) {
    rectangle_arrondi(app, x - 2, y - 2, w + 4, h + 4, 18, bord);
    rectangle_arrondi(app, x, y, w, h, 16, fond);
}

/* --- Texte --- */

#define GAUCHE 0
#define CENTRE 1
#define DROITE 2

static int largeur_texte(TTF_Font *police, const char *texte) {
    int w = 0, h = 0;
    TTF_SizeUTF8(police, texte, &w, &h);
    return w;
}

/* Dessine une ligne de texte. Renvoie sa largeur en pixels. */
static int texte(App *app, TTF_Font *police, const char *chaine, float x, float y, SDL_Color c, int alignement) {
    if (chaine == NULL || chaine[0] == '\0') return 0;
    SDL_Surface *surface = TTF_RenderUTF8_Blended(police, chaine, c);
    if (surface == NULL) return 0;
    SDL_Texture *texture = SDL_CreateTextureFromSurface(app->rendu, surface);
    SDL_Rect r = { (int) x, (int) y, surface->w, surface->h };
    if (alignement == CENTRE) r.x -= surface->w / 2;
    if (alignement == DROITE) r.x -= surface->w;
    if (texture != NULL) {
        SDL_RenderCopy(app->rendu, texture, NULL, &r);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
    return r.w;
}

/* Dessine un paragraphe coupé automatiquement à largeur_max pixels (les \n sont respectés).
 * Si dessiner vaut 0, mesure seulement. Renvoie la hauteur occupée. */
static int paragraphe(App *app, TTF_Font *police, const char *chaine, float x, float y, int largeur_max,
                      SDL_Color c, int centre, int dessiner) {
    if (chaine == NULL || chaine[0] == '\0') return 0;
#if SDL_TTF_VERSION_ATLEAST(2, 20, 0)
    TTF_SetFontWrappedAlign(police, centre ? TTF_WRAPPED_ALIGN_CENTER : TTF_WRAPPED_ALIGN_LEFT);
#endif
    SDL_Surface *surface = TTF_RenderUTF8_Blended_Wrapped(police, chaine, c, (Uint32) largeur_max);
    if (surface == NULL) return 0;
    int hauteur = surface->h;
    if (dessiner) {
        SDL_Texture *texture = SDL_CreateTextureFromSurface(app->rendu, surface);
        SDL_Rect r = { (int) x, (int) y, surface->w, surface->h };
        if (centre) r.x -= surface->w / 2;
        if (texture != NULL) {
            SDL_RenderCopy(app->rendu, texture, NULL, &r);
            SDL_DestroyTexture(texture);
        }
    }
    SDL_FreeSurface(surface);
    return hauteur;
}

/* --- Images --- */

static void debut_image(App *app) {
    SDL_GetRendererOutputSize(app->rendu, &app->largeur, &app->hauteur);
    choisir_couleur(app, FOND);
    SDL_RenderClear(app->rendu);
}

/* Affiche l'image et limite le nombre d'images par seconde (économise le processeur). */
static void fin_image(App *app) {
    SDL_RenderPresent(app->rendu);
    Uint32 duree = 1000 / IMAGES_PAR_SECONDE;
    Uint32 ecoule = SDL_GetTicks() - app->derniere_image;
    if (ecoule < duree) SDL_Delay(duree - ecoule);
    app->derniere_image = SDL_GetTicks();
}

/* Fond décoratif des menus : un motif de murs de labyrinthe qui défile lentement. */
static void fond_decoratif(App *app) {
    const int pas = 44;
    int decalage = (int) (temps_secondes() * 6) % pas;
    for (int i = -1; i <= app->hauteur / pas + 1; i++) {
        for (int j = -1; j <= app->largeur / pas + 1; j++) {
            unsigned h = (unsigned) (i * 73856093) ^ (unsigned) (j * 19349663);
            float x = (float) (j * pas + decalage);
            float y = (float) (i * pas + decalage);
            if (h % 3 == 0) rectangle(app, x, y, pas, 6, FOND_MOTIF);
            if (h % 5 == 1) rectangle(app, x, y, 6, pas, FOND_MOTIF);
        }
    }
}

/* Assombrit tout l'écran (derrière une fenêtre de dialogue). */
static void voile(App *app) {
    rectangle(app, 0, 0, (float) app->largeur, (float) app->hauteur, (SDL_Color) {5, 6, 12, 175});
}

/* ------------------------------------------------------------------------------------------
 * Dessin des cases et des personnages
 * ------------------------------------------------------------------------------------------ */

static void dessiner_cle(App *app, float cx, float cy, float t, SDL_Color c, SDL_Color trou) {
    disque(app, cx - 0.16f * t, cy, 0.16f * t, c);
    disque(app, cx - 0.16f * t, cy, 0.07f * t, trou);
    rectangle(app, cx - 0.03f * t, cy - 0.04f * t, 0.36f * t, 0.08f * t, c);
    rectangle(app, cx + 0.18f * t, cy + 0.03f * t, 0.06f * t, 0.10f * t, c);
    rectangle(app, cx + 0.27f * t, cy + 0.03f * t, 0.06f * t, 0.14f * t, c);
}

/* Dessine une case du labyrinthe (sans les personnages) dans le rectangle (x, y, w, h).
 * mur_dessous : 1 si la case du dessous est un mur (sinon on dessine la face avant du mur).
 * a_cle : 1 si le joueur a la clé (la sortie est alors ouverte). */
static void dessiner_case(App *app, char c, float x, float y, float w, float h, float temps,
                          int a_cle, int mur_dessous, int damier) {
    float t = minf(w, h);
    float cx = x + w / 2, cy = y + h / 2;

    if (c == MOTIF_MUR) {
        if (mur_dessous) {
            rectangle(app, x, y, w, h, MUR_DESSUS);
        } else {
            rectangle(app, x, y, w, h * 0.68f, MUR_DESSUS);
            rectangle(app, x, y + h * 0.68f, w, h - h * 0.68f, MUR_FACE);
        }
        return;
    }

    rectangle(app, x, y, w, h, damier ? SOL_2 : SOL_1);
    if (t < 6) { /* Labyrinthe vu de très loin : de simples taches de couleur */
        SDL_Color couleur;
        switch (c) {
            case MOTIF_KEY:    couleur = OR; break;
            case MOTIF_TRAP:   couleur = ROUGE; break;
            case MOTIF_BOOST:  couleur = VERT; break;
            case MOTIF_ENTREE: couleur = (SDL_Color) {46, 140, 90, 255}; break;
            case MOTIF_SORTIE: couleur = a_cle ? VERT : ORANGE; break;
            default: return;
        }
        rectangle(app, x, y, w, h, couleur);
        return;
    }

    switch (c) {
        case MOTIF_ENTREE:
            rectangle_arrondi(app, x + 0.12f * w, y + 0.12f * h, 0.76f * w, 0.76f * h, 0.15f * t,
                              (SDL_Color) {46, 140, 90, 150});
            triangle(app, cx - 0.2f * t, cy - 0.08f * t, cx + 0.2f * t, cy - 0.08f * t, cx, cy + 0.18f * t,
                     (SDL_Color) {140, 240, 180, 220});
            break;

        case MOTIF_SORTIE:
            if (a_cle) { /* Portail ouvert qui pulse */
                float pulsation = 0.5f + 0.5f * sinf(temps * 5);
                disque(app, cx, cy, (0.36f + 0.08f * pulsation) * t, avec_alpha(VERT, 70));
                disque(app, cx, cy, 0.30f * t, (SDL_Color) {60, 200, 120, 255});
                disque(app, cx, cy, 0.17f * t, (SDL_Color) {190, 255, 215, 255});
            } else {     /* Porte fermée par un cadenas */
                rectangle_arrondi(app, x + 0.14f * w, y + 0.08f * h, 0.72f * w, 0.86f * h, 0.2f * t,
                                  (SDL_Color) {92, 56, 34, 255});
                rectangle(app, x + 0.22f * w, y + 0.16f * h, 0.56f * w, 0.78f * h, (SDL_Color) {128, 80, 46, 255});
                disque(app, cx, cy - 0.06f * t, 0.12f * t, OR_SOMBRE);
                disque(app, cx, cy - 0.06f * t, 0.06f * t, (SDL_Color) {128, 80, 46, 255});
                rectangle(app, cx - 0.15f * t, cy, 0.30f * t, 0.22f * t, OR);
            }
            break;

        case MOTIF_KEY: {
            float halo = 0.5f + 0.5f * sinf(temps * 4);
            disque(app, cx, cy, (0.36f + 0.06f * halo) * t, avec_alpha(OR, 40 + (int) (40 * halo)));
            dessiner_cle(app, cx - 0.04f * t, cy, t, OR, damier ? SOL_2 : SOL_1);
            break;
        }

        case MOTIF_TRAP: {
            rectangle_arrondi(app, x + 0.1f * w, y + 0.1f * h, 0.8f * w, 0.8f * h, 0.12f * t,
                              (SDL_Color) {96, 30, 38, 255});
            float base = y + 0.82f * h;
            for (int k = 0; k < 3; k++) {
                float gauche = x + (0.16f + 0.23f * k) * w;
                triangle(app, gauche, base, gauche + 0.22f * w, base, gauche + 0.11f * w, y + 0.22f * h,
                         (SDL_Color) {205, 208, 220, 255});
            }
            break;
        }

        case MOTIF_BOOST: {
            float flottement = sinf(temps * 3 + x * 0.05f) * 0.05f * t;
            float py = cy + flottement;
            SDL_FPoint gemme[4] = {
                {cx, py - 0.32f * t}, {cx + 0.25f * t, py - 0.04f * t},
                {cx, py + 0.32f * t}, {cx - 0.25f * t, py - 0.04f * t}
            };
            polygone(app, gemme, 4, (SDL_Color) {60, 200, 110, 255});
            triangle(app, cx, py - 0.32f * t, cx - 0.25f * t, py - 0.04f * t, cx, py - 0.04f * t,
                     (SDL_Color) {150, 255, 190, 255});
            disque(app, cx + 0.22f * t, py - 0.28f * t, 0.04f * t,
                   avec_alpha(TEXTE, (int) (127 + 127 * sinf(temps * 6))));
            break;
        }

        default:
            break;
    }
}

/* Joueur : une bille bleue dont les yeux regardent dans la direction du dernier pas. */
static void dessiner_joueur(App *app, float cx, float cy, float t, int dir_ligne, int dir_colonne) {
    disque(app, cx, cy + 0.06f * t, 0.38f * t, (SDL_Color) {0, 0, 0, 70});         /* ombre */
    disque(app, cx, cy, 0.38f * t, (SDL_Color) {30, 80, 160, 255});
    disque(app, cx, cy, 0.32f * t, (SDL_Color) {80, 160, 255, 255});
    disque(app, cx - 0.10f * t, cy - 0.12f * t, 0.08f * t, (SDL_Color) {170, 215, 255, 255});
    float ox = dir_colonne * 0.08f * t, oy = dir_ligne * 0.08f * t;
    for (int k = -1; k <= 1; k += 2) {
        float ex = cx + k * 0.12f * t + ox, ey = cy - 0.02f * t + oy;
        disque(app, ex, ey, 0.085f * t, TEXTE);
        disque(app, ex + ox * 0.4f, ey + oy * 0.4f, 0.045f * t, TEXTE_SOMBRE);
    }
}

/* Ogre : une grosse boule rouge avec des cornes, des sourcils froncés et des crocs. */
static void dessiner_ogre(App *app, float cx, float cy, float t, float temps) {
    float respiration = 1 + 0.04f * sinf(temps * 6);
    float r = 0.40f * t * respiration;
    SDL_Color corne = {235, 225, 195, 255};
    disque(app, cx, cy + 0.07f * t, r, (SDL_Color) {0, 0, 0, 80});
    triangle(app, cx - 0.30f * t, cy - 0.18f * t, cx - 0.14f * t, cy - 0.30f * t, cx - 0.34f * t, cy - 0.48f * t, corne);
    triangle(app, cx + 0.30f * t, cy - 0.18f * t, cx + 0.14f * t, cy - 0.30f * t, cx + 0.34f * t, cy - 0.48f * t, corne);
    disque(app, cx, cy, r, (SDL_Color) {120, 26, 26, 255});
    disque(app, cx, cy, r - 0.05f * t, (SDL_Color) {205, 60, 50, 255});
    for (int k = -1; k <= 1; k += 2) {
        float ex = cx + k * 0.13f * t, ey = cy - 0.08f * t;
        disque(app, ex, ey, 0.08f * t, (SDL_Color) {255, 235, 120, 255});
        disque(app, ex, ey + 0.01f * t, 0.04f * t, TEXTE_SOMBRE);
        triangle(app, ex - 0.11f * t, ey - 0.14f * t - k * 0.04f * t, ex + 0.11f * t, ey - 0.14f * t + k * 0.04f * t,
                 ex, ey - 0.06f * t, (SDL_Color) {80, 10, 10, 255});
    }
    rectangle(app, cx - 0.17f * t, cy + 0.10f * t, 0.34f * t, 0.10f * t, (SDL_Color) {60, 8, 8, 255});
    triangle(app, cx - 0.14f * t, cy + 0.10f * t, cx - 0.06f * t, cy + 0.10f * t, cx - 0.10f * t, cy + 0.19f * t, TEXTE);
    triangle(app, cx + 0.06f * t, cy + 0.10f * t, cx + 0.14f * t, cy + 0.10f * t, cx + 0.10f * t, cy + 0.19f * t, TEXTE);
}

/* Fantôme : un drap bleu translucide qui flotte ; ses yeux suivent le joueur. */
static void dessiner_fantome(App *app, float cx, float cy, float t, float temps, float regard_x, float regard_y) {
    SDL_Color corps = {130, 225, 255, 190};
    cy += sinf(temps * 4) * 0.05f * t;
    float r = 0.33f * t;
    float bas = cy + 0.30f * t;
    secteur(app, cx, cy - 0.05f * t, r, (float) M_PI, 2 * (float) M_PI, corps);
    rectangle(app, cx - r, cy - 0.05f * t, 2 * r, bas - (cy - 0.05f * t), corps);
    float ondulation = sinf(temps * 8) * 0.03f * t;
    for (int k = 0; k < 3; k++) {
        float gauche = cx - r + k * (2 * r / 3);
        triangle(app, gauche, bas, gauche + 2 * r / 3, bas, gauche + r / 3 + ondulation, bas + 0.13f * t, corps);
    }
    float longueur = sqrtf(regard_x * regard_x + regard_y * regard_y);
    if (longueur > 0.01f) { regard_x /= longueur; regard_y /= longueur; }
    for (int k = -1; k <= 1; k += 2) {
        float ex = cx + k * 0.12f * t, ey = cy - 0.06f * t;
        disque(app, ex, ey, 0.085f * t, TEXTE);
        disque(app, ex + regard_x * 0.035f * t, ey + regard_y * 0.035f * t, 0.045f * t, (SDL_Color) {30, 60, 160, 255});
    }
}

/* Une icône de légende : la case seule, avec le personnage par-dessus pour '*', 'G' et 'F'. */
static void dessiner_icone(App *app, char c, float x, float y, float t, int a_cle) {
    float temps = temps_secondes();
    char fond = (c == MOTIF_JOUEUR || c == MOTIF_OGRE || c == MOTIF_FANTOME) ? MOTIF_VIDE : c;
    rectangle_arrondi(app, x - 2, y - 2, t + 4, t + 4, 6, PANNEAU_BORD);
    dessiner_case(app, fond, x, y, t, t, temps, a_cle, 0, 0);
    if (c == MOTIF_JOUEUR)  dessiner_joueur(app, x + t / 2, y + t / 2, t, 1, 0);
    if (c == MOTIF_OGRE)    dessiner_ogre(app, x + t / 2, y + t / 2, t, temps);
    if (c == MOTIF_FANTOME) dessiner_fantome(app, x + t / 2, y + t / 2, t, temps, 1, 0);
}

/* ------------------------------------------------------------------------------------------
 * Partie en cours
 * ------------------------------------------------------------------------------------------ */

typedef struct {
    Grille grille;
    Joueur joueur;
    Monstre ogre;
    Monstre fantome;
    /* Positions affichées (ligne, colonne) : elles glissent vers la vraie position pour animer. */
    float joueur_l, joueur_c;
    float ogre_l, ogre_c;
    float fantome_l, fantome_c;
    int dir_ligne, dir_colonne;    /* Direction du dernier pas du joueur */
    char message[TAILLE_MESSAGE];  /* Dernier événement, affiché en bas */
    Uint32 message_debut;
    Uint32 flash_debut;
    SDL_Color flash_couleur;
    const char *nom_joueur;
    const char *nom_labyrinthe;
} Partie;

/* Dessine le labyrinthe dans la zone donnée. Avec une partie, dessine aussi le joueur et les
 * monstres ; si les cases sont trop petites, la caméra suit le joueur. */
static void dessiner_labyrinthe(App *app, Grille *g, SDL_Rect zone, Partie *p) {
    float temps = temps_secondes();
    float t = minf((float) zone.w / g->largeur, (float) zone.h / g->hauteur);
    if (t > TUILE_MAX) t = TUILE_MAX;
    if (p != NULL && t < TUILE_MIN_JEU) t = TUILE_MIN_JEU;

    float largeur = t * g->largeur, hauteur = t * g->hauteur;
    float ox, oy; /* Position à l'écran du coin haut gauche du labyrinthe */
    if (largeur <= zone.w || p == NULL) {
        ox = zone.x + (zone.w - largeur) / 2;
    } else {
        float centre = p->joueur_c * t + t / 2 - zone.w / 2.0f;
        ox = zone.x - maxf(0, minf(centre, largeur - zone.w));
    }
    if (hauteur <= zone.h || p == NULL) {
        oy = zone.y + (zone.h - hauteur) / 2;
    } else {
        float centre = p->joueur_l * t + t / 2 - zone.h / 2.0f;
        oy = zone.y - maxf(0, minf(centre, hauteur - zone.h));
    }

    SDL_RenderSetClipRect(app->rendu, &zone);

    /* Seules les cases visibles sont dessinées */
    int j0 = maxi(0, (int) floorf((zone.x - ox) / t));
    int j1 = mini(g->largeur - 1, (int) ceilf((zone.x + zone.w - ox) / t));
    int i0 = maxi(0, (int) floorf((zone.y - oy) / t));
    int i1 = mini(g->hauteur - 1, (int) ceilf((zone.y + zone.h - oy) / t));
    int a_cle = (p != NULL && p->joueur.have_key);

    for (int i = i0; i <= i1; i++) {
        float y = floorf(oy + i * t), h = floorf(oy + (i + 1) * t) - y;
        for (int j = j0; j <= j1; j++) {
            float x = floorf(ox + j * t), w = floorf(ox + (j + 1) * t) - x;
            char c = g->cases[i][j];
            if (c == MOTIF_JOUEUR) c = (p != NULL) ? p->joueur.case_dessous : MOTIF_VIDE;
            int mur_dessous = (i + 1 < g->hauteur && g->cases[i + 1][j] == MOTIF_MUR);
            dessiner_case(app, c, x, y, w, h, temps, a_cle, mur_dessous, (i + j) % 2);
        }
    }

    if (p != NULL) {
        float jx = ox + (p->joueur_c + 0.5f) * t, jy = oy + (p->joueur_l + 0.5f) * t;
        dessiner_joueur(app, jx, jy, t, p->dir_ligne, p->dir_colonne);
        if (p->fantome.actif) {
            dessiner_fantome(app, ox + (p->fantome_c + 0.5f) * t, oy + (p->fantome_l + 0.5f) * t, t, temps,
                             p->joueur_c - p->fantome_c, p->joueur_l - p->fantome_l);
        }
        if (p->ogre.actif) {
            dessiner_ogre(app, ox + (p->ogre_c + 0.5f) * t, oy + (p->ogre_l + 0.5f) * t, t, temps);
        }
    }

    SDL_RenderSetClipRect(app->rendu, NULL);
}

static SDL_Color couleur_mode(int mode) {
    switch (mode) {
        case MODE_DIFFICILE: return ROUGE;
        case MODE_NORMAL:    return ORANGE;
        default:             return VERT;
    }
}

/* Petite étiquette arrondie portant le nom du mode. Renvoie sa largeur. */
static int badge_mode(App *app, int mode, float x, float y) {
    const char *nom = nom_mode(mode);
    int w = largeur_texte(app->police_gras, nom) + 24;
    int h = TTF_FontHeight(app->police_gras) + 8;
    rectangle_arrondi(app, x, y, (float) w, (float) h, h / 2.0f, couleur_mode(mode));
    texte(app, app->police_gras, nom, x + w / 2.0f, y + 4, TEXTE_SOMBRE, CENTRE);
    return w;
}

/* Écran complet d'une partie : bandeau, labyrinthe, flash et message. */
static void dessiner_partie(App *app, void *donnees) {
    Partie *p = donnees;
    Uint32 maintenant = SDL_GetTicks();
    char ligne[TAILLE_MESSAGE + TAILLE_NOM];

    /* Labyrinthe */
    SDL_Rect zone = { MARGE, HAUTEUR_BANDEAU + MARGE, app->largeur - 2 * MARGE,
                      app->hauteur - HAUTEUR_BANDEAU - HAUTEUR_BARRE - 2 * MARGE };
    dessiner_labyrinthe(app, &p->grille, zone, p);

    /* Flash coloré après un événement (piège, bonus, clé, fantôme, ogre) */
    Uint32 age_flash = maintenant - p->flash_debut;
    if (p->flash_debut != 0 && age_flash < DUREE_FLASH) {
        int alpha = (int) (110 * (1.0f - (float) age_flash / DUREE_FLASH));
        rectangle(app, (float) zone.x, (float) zone.y, (float) zone.w, (float) zone.h, avec_alpha(p->flash_couleur, alpha));
    }

    /* Bandeau du haut */
    rectangle(app, 0, 0, (float) app->largeur, HAUTEUR_BANDEAU, PANNEAU);
    rectangle(app, 0, HAUTEUR_BANDEAU - 2, (float) app->largeur, 2, PANNEAU_BORD);

    float x = 24;
    texte(app, app->police_petite, "SCORE", x, 12, TEXTE_DOUX, GAUCHE);
    snprintf(ligne, sizeof ligne, "%d", get_score_joueur(&p->joueur));
    texte(app, app->police_titre, ligne, x, 28, TEXTE, GAUCHE);

    x = 150;
    texte(app, app->police_petite, "CLÉ", x, 12, TEXTE_DOUX, GAUCHE);
    rectangle_arrondi(app, x, 34, 40, 38, 8, BOUTON);
    dessiner_cle(app, x + 21, 53, 40, p->joueur.have_key ? OR : (SDL_Color) {80, 86, 110, 255}, BOUTON);
    texte(app, app->police, p->joueur.have_key ? "trouvée !" : "à trouver", x + 52, 42,
          p->joueur.have_key ? OR : TEXTE_DOUX, GAUCHE);

    x = 320;
    texte(app, app->police_petite, "MODE", x, 12, TEXTE_DOUX, GAUCHE);
    badge_mode(app, p->grille.mode, x, 38);

    x = 470;
    texte(app, app->police_petite, "RECORD À BATTRE", x, 12, TEXTE_DOUX, GAUCHE);
    Top_Player *record = &p->grille.top_scores[0];
    if (record->name != NULL) {
        snprintf(ligne, sizeof ligne, "%d par %s", record->score, record->name);
    } else {
        snprintf(ligne, sizeof ligne, "aucun, soyez le premier !");
    }
    texte(app, app->police, ligne, x, 42, TEXTE, GAUCHE);

    texte(app, app->police_gras, p->nom_joueur, app->largeur - 24.0f, 18, ACCENT, DROITE);
    texte(app, app->police_petite, p->nom_labyrinthe, app->largeur - 24.0f, 48, TEXTE_DOUX, DROITE);

    /* Barre du bas : dernier message (qui s'efface peu à peu) et aide */
    float y_barre = (float) (app->hauteur - HAUTEUR_BARRE);
    Uint32 age_message = maintenant - p->message_debut;
    if (p->message[0] != '\0' && age_message < DUREE_MESSAGE + 600) {
        int alpha = age_message < DUREE_MESSAGE ? 255 : 255 - (int) ((age_message - DUREE_MESSAGE) * 255 / 600);
        texte(app, app->police_gras, p->message, app->largeur / 2.0f, y_barre + 2, avec_alpha(ACCENT, alpha), CENTRE);
    }
    texte(app, app->police_petite, "Flèches ou Z Q S D : se déplacer      Échap : abandonner",
          app->largeur / 2.0f, y_barre + 32, TEXTE_DOUX, CENTRE);
}

static void flash(Partie *p, SDL_Color couleur) {
    p->flash_debut = SDL_GetTicks();
    p->flash_couleur = couleur;
}

/* Joue un tour avec la touche donnée ('z', 'q', 's' ou 'd'). Renvoie l'état de la partie. */
static int jouer_coup(Partie *p, int touche) {
    int score_avant = p->joueur.score;
    int cle_avant = p->joueur.have_key;

    p->joueur.message[0] = '\0';
    if (appliquer_touche(&p->grille, &p->joueur, touche) != MVT_JOUE) return PARTIE_EN_COURS;

    p->dir_ligne = (touche == 's') - (touche == 'z');
    p->dir_colonne = (touche == 'd') - (touche == 'q');

    /* Un pas normal ajoute 1 point : plus, c'est un piège ; moins, c'est un bonus */
    int score_apres_pas = p->joueur.score;
    if (score_apres_pas > score_avant + 1) flash(p, ROUGE);
    if (score_apres_pas < score_avant + 1) flash(p, VERT);
    if (!cle_avant && p->joueur.have_key) flash(p, OR);

    int etat = resoudre_tour(&p->grille, &p->joueur, &p->ogre, &p->fantome);

    if (p->joueur.score > score_apres_pas) flash(p, CYAN); /* Score doublé par le fantôme */
    if (etat == PARTIE_PERDUE) flash(p, ROUGE);
    if (etat == PARTIE_GAGNEE) flash(p, VERT);

    if (p->joueur.message[0] != '\0') {
        snprintf(p->message, sizeof p->message, "%s", p->joueur.message);
        p->message_debut = SDL_GetTicks();
    }
    return etat;
}

/* Fait glisser une position affichée vers sa vraie valeur (saut direct si elle est loin). */
static void glisser(float *affiche, int cible, float dt) {
    if (fabsf(cible - *affiche) > 1.5f) {
        *affiche = (float) cible;
    } else {
        *affiche += (cible - *affiche) * minf(1, dt * 18);
    }
}

/* Convertit une touche du clavier en touche du jeu (0 si elle ne sert pas). */
static int touche_jeu(SDL_Keycode k) {
    switch (k) {
        case SDLK_UP:    case SDLK_z: case SDLK_w: return 'z';
        case SDLK_DOWN:  case SDLK_s:              return 's';
        case SDLK_LEFT:  case SDLK_q: case SDLK_a: return 'q';
        case SDLK_RIGHT: case SDLK_d:              return 'd';
        case SDLK_ESCAPE: case SDLK_0: case SDLK_KP_0: return TOUCHE_ECHAP;
        default: return 0;
    }
}

/* ------------------------------------------------------------------------------------------
 * Écrans génériques : menu, saisie, page à lire
 * ------------------------------------------------------------------------------------------ */

typedef struct {
    const char *titre;
    const char *texte;       /* Paragraphe sous le titre (peut être NULL) */
    const char **options;
    int nbr_options;
    int echap;               /* 1 : Échap choisit la dernière option (« Revenir au menu ») */
    Fond fond;               /* Ce qu'il y a derrière le menu (NULL : fond décoratif) */
    void *donnees;           /* Donné à la fonction fond */
    SDL_Color couleur_titre; /* Couleur du titre (blanc si non précisée) */
} Menu;

/* Numéro (1 à 9) correspondant à une touche chiffre, sur la rangée du haut ou le pavé
 * numérique ; 0 sinon. On utilise la position de la touche pour marcher aussi en AZERTY. */
static int chiffre_touche(SDL_Scancode s) {
    if (s >= SDL_SCANCODE_1 && s <= SDL_SCANCODE_9) return s - SDL_SCANCODE_1 + 1;
    if (s >= SDL_SCANCODE_KP_1 && s <= SDL_SCANCODE_KP_9) return s - SDL_SCANCODE_KP_1 + 1;
    return 0;
}

/* Garde la sélection dans la partie visible de la liste. */
static int ajuster_debut(int selection, int debut, int visibles, int nbr) {
    if (selection < debut) debut = selection;
    if (selection >= debut + visibles) debut = selection - visibles + 1;
    return maxi(0, mini(debut, nbr - visibles));
}

static int point_dans(int x, int y, SDL_Rect r) {
    return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
}

/* Affiche un menu dans un panneau : flèches haut/bas ou souris pour choisir, Entrée ou clic
 * pour valider, chiffres 1 à 9 pour choisir directement, molette pour faire défiler.
 * Renvoie l'indice de l'option choisie (la dernière si la fenêtre est fermée). */
static int gui_choisir(App *app, const Menu *m) {
    int n = m->nbr_options;
    int selection = 0, debut = 0;
    int visibles = mini(n, TAILLE_FENETRE_MENU);
    SDL_Rect boutons[TAILLE_FENETRE_MENU];
    int nbr_boutons = 0;
    SDL_Color couleur_titre = m->couleur_titre.a ? m->couleur_titre : TEXTE;

    while (1) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                app->quitter = 1;
                return n - 1;
            } else if (e.type == SDL_KEYDOWN) {
                SDL_Keycode k = e.key.keysym.sym;
                int chiffre = chiffre_touche(e.key.keysym.scancode);
                if (k == SDLK_UP || k == SDLK_z || k == SDLK_w) selection = (selection + n - 1) % n;
                else if (k == SDLK_DOWN || k == SDLK_s) selection = (selection + 1) % n;
                else if (k == SDLK_HOME) selection = 0;
                else if (k == SDLK_END) selection = n - 1;
                else if (k == SDLK_PAGEUP) selection = maxi(0, selection - visibles);
                else if (k == SDLK_PAGEDOWN) selection = mini(n - 1, selection + visibles);
                else if (k == SDLK_RETURN || k == SDLK_KP_ENTER || k == SDLK_SPACE) return selection;
                else if (k == SDLK_ESCAPE && m->echap) return n - 1;
                else if (chiffre >= 1 && chiffre <= n) return chiffre - 1;
                debut = ajuster_debut(selection, debut, visibles, n);
            } else if (e.type == SDL_MOUSEMOTION) {
                for (int k = 0; k < nbr_boutons; k++) {
                    if (point_dans(e.motion.x, e.motion.y, boutons[k])) selection = debut + k;
                }
            } else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                for (int k = 0; k < nbr_boutons; k++) {
                    if (point_dans(e.button.x, e.button.y, boutons[k])) return debut + k;
                }
            } else if (e.type == SDL_MOUSEWHEEL) {
                debut = maxi(0, mini(debut - e.wheel.y, n - visibles));
                selection = maxi(debut, mini(selection, debut + visibles - 1));
            }
        }

        debut_image(app);
        if (m->fond != NULL) {
            m->fond(app, m->donnees);
            voile(app);
        } else {
            fond_decoratif(app);
        }

        /* Mise en page : on réduit le nombre d'options visibles si la fenêtre est trop petite */
        int largeur = mini(640, app->largeur - 2 * MARGE);
        int interieur = largeur - 2 * 32;
        int h_titre = TTF_FontLineSkip(app->police_titre);
        int h_texte = paragraphe(app, app->police, m->texte, 0, 0, interieur, TEXTE_DOUX, 1, 0);
        int hauteur;
        visibles = mini(n, TAILLE_FENETRE_MENU) + 1;
        do {
            visibles--;
            int h_liste = visibles * (BOUTON_HAUTEUR + BOUTON_ESPACE) - BOUTON_ESPACE + (n > visibles ? 2 * 24 : 0);
            hauteur = 28 + h_titre + (h_texte ? 10 + h_texte : 0) + 22 + h_liste + 28;
        } while (visibles > 1 && hauteur > app->hauteur - 2 * MARGE);
        debut = ajuster_debut(selection, debut, visibles, n);

        float x = (app->largeur - largeur) / 2.0f;
        float y = maxf(MARGE, (app->hauteur - hauteur) / 2.0f);
        panneau(app, x, y, (float) largeur, (float) hauteur, PANNEAU, PANNEAU_BORD);

        float cy = y + 28;
        texte(app, app->police_titre, m->titre, app->largeur / 2.0f, cy, couleur_titre, CENTRE);
        cy += h_titre;
        if (h_texte) {
            paragraphe(app, app->police, m->texte, app->largeur / 2.0f, cy + 10, interieur, TEXTE_DOUX, 1, 1);
            cy += 10 + h_texte;
        }
        cy += 22;

        if (n > visibles) {
            char info[32] = "";
            if (debut > 0) snprintf(info, sizeof info, "▲ %d de plus", debut);
            texte(app, app->police_petite, info, app->largeur / 2.0f, cy, TEXTE_DOUX, CENTRE);
            cy += 24;
        }

        nbr_boutons = visibles;
        for (int k = 0; k < visibles; k++) {
            int i = debut + k;
            int choisi = (i == selection);
            SDL_Rect r = { (int) x + 32, (int) cy, interieur, BOUTON_HAUTEUR };
            boutons[k] = r;
            rectangle_arrondi(app, (float) r.x, (float) r.y, (float) r.w, (float) r.h, 10, choisi ? ACCENT : BOUTON);
            float tx = r.x + 18.0f;
            if (i < 9) {
                char numero[4];
                snprintf(numero, sizeof numero, "%d", i + 1);
                texte(app, app->police_petite, numero, tx, r.y + 14.0f, choisi ? TEXTE_SOMBRE : TEXTE_DOUX, GAUCHE);
            }
            texte(app, choisi ? app->police_gras : app->police, m->options[i], tx + 26, r.y + 11.0f,
                  choisi ? TEXTE_SOMBRE : TEXTE, GAUCHE);
            cy += BOUTON_HAUTEUR + BOUTON_ESPACE;
        }

        if (n > visibles) {
            char info[32] = "";
            int restant = n - debut - visibles;
            if (restant > 0) snprintf(info, sizeof info, "▼ %d de plus", restant);
            texte(app, app->police_petite, info, app->largeur / 2.0f, cy, TEXTE_DOUX, CENTRE);
        }

        fin_image(app);
    }
}

/* Message dans un panneau, avec un seul bouton « Continuer ». */
static void gui_message(App *app, const char *titre, const char *message, SDL_Color couleur_titre) {
    const char *options[] = { "Continuer" };
    Menu m = { .titre = titre, .texte = message, .options = options, .nbr_options = 1,
               .echap = 1, .couleur_titre = couleur_titre };
    gui_choisir(app, &m);
}

/* Saisie d'un texte au clavier dans un panneau. chiffres : 1 pour n'accepter que des chiffres,
 * sinon lettres sans accent, chiffres, '_' et '-'. erreur : message affiché en rouge (ou NULL).
 * Renvoie 1 si la saisie est validée par Entrée (non vide), 0 pour revenir (Échap). */
static int gui_saisir(App *app, const char *titre, const char *aide, char *saisie, int taille_max,
                      int chiffres, const char *erreur) {
    SDL_StartTextInput();
    while (1) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            int longueur = (int) strlen(saisie);
            if (e.type == SDL_QUIT) {
                app->quitter = 1;
                SDL_StopTextInput();
                return 0;
            } else if (e.type == SDL_TEXTINPUT) {
                for (const char *c = e.text.text; *c; c++) {
                    unsigned char u = (unsigned char) *c;
                    int accepte = chiffres ? isdigit(u) : (u < 128 && (isalnum(u) || u == '_' || u == '-'));
                    if (accepte && longueur < taille_max) {
                        saisie[longueur++] = *c;
                        saisie[longueur] = '\0';
                    }
                }
            } else if (e.type == SDL_KEYDOWN) {
                SDL_Keycode k = e.key.keysym.sym;
                if (k == SDLK_BACKSPACE && longueur > 0) {
                    saisie[longueur - 1] = '\0';
                } else if ((k == SDLK_RETURN || k == SDLK_KP_ENTER) && longueur > 0) {
                    SDL_StopTextInput();
                    return 1;
                } else if (k == SDLK_ESCAPE) {
                    SDL_StopTextInput();
                    return 0;
                }
            }
        }

        debut_image(app);
        fond_decoratif(app);

        int largeur = mini(640, app->largeur - 2 * MARGE);
        int interieur = largeur - 64;
        int h_titre = TTF_FontLineSkip(app->police_titre);
        int h_aide = paragraphe(app, app->police, aide, 0, 0, interieur, TEXTE_DOUX, 1, 0);
        int h_erreur = paragraphe(app, app->police, erreur, 0, 0, interieur, ROUGE, 1, 0);
        int hauteur = 28 + h_titre + 10 + h_aide + 22 + 56 + (h_erreur ? 14 + h_erreur : 0) + 20 + 20 + 28;
        float x = (app->largeur - largeur) / 2.0f;
        float y = maxf(MARGE, (app->hauteur - hauteur) / 2.0f);
        panneau(app, x, y, (float) largeur, (float) hauteur, PANNEAU, PANNEAU_BORD);

        float cy = y + 28;
        texte(app, app->police_titre, titre, app->largeur / 2.0f, cy, TEXTE, CENTRE);
        cy += h_titre + 10;
        paragraphe(app, app->police, aide, app->largeur / 2.0f, cy, interieur, TEXTE_DOUX, 1, 1);
        cy += h_aide + 22;

        /* Champ de saisie avec un curseur qui clignote */
        rectangle_arrondi(app, x + 30, cy - 2, interieur + 4.0f, 60, 12, ACCENT);
        rectangle_arrondi(app, x + 32, cy, (float) interieur, 56, 10, BOUTON);
        int w = texte(app, app->police_titre, saisie, x + 50, cy + 6, TEXTE, GAUCHE);
        if ((SDL_GetTicks() / 500) % 2 == 0) {
            rectangle(app, x + 52 + w, cy + 12, 3, 32, ACCENT);
        }
        cy += 56;

        if (h_erreur) {
            paragraphe(app, app->police, erreur, app->largeur / 2.0f, cy + 14, interieur, ROUGE, 1, 1);
            cy += 14 + h_erreur;
        }
        texte(app, app->police_petite, "Entrée : valider      Échap : revenir", app->largeur / 2.0f, cy + 20,
              TEXTE_DOUX, CENTRE);

        fin_image(app);
    }
}

/* Fonction qui dessine une page (règles, scores, aperçu) décalée vers le haut de defilement
 * pixels, et renvoie la hauteur totale de son contenu. */
typedef int (*Page)(App *app, void *donnees, int defilement);

/* Affiche une page jusqu'à Entrée, Échap ou un clic. Flèches et molette font défiler. */
static void gui_page(App *app, Page page, void *donnees) {
    int defilement = 0, hauteur_contenu = 0;
    while (1) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                app->quitter = 1;
                return;
            } else if (e.type == SDL_KEYDOWN) {
                SDL_Keycode k = e.key.keysym.sym;
                if (k == SDLK_RETURN || k == SDLK_KP_ENTER || k == SDLK_ESCAPE || k == SDLK_SPACE) return;
                if (k == SDLK_UP || k == SDLK_z) defilement -= 40;
                if (k == SDLK_DOWN || k == SDLK_s) defilement += 40;
                if (k == SDLK_PAGEUP) defilement -= app->hauteur / 2;
                if (k == SDLK_PAGEDOWN) defilement += app->hauteur / 2;
            } else if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                return;
            } else if (e.type == SDL_MOUSEWHEEL) {
                defilement -= e.wheel.y * 50;
            }
        }
        defilement = maxi(0, mini(defilement, hauteur_contenu - (app->hauteur - 50)));

        debut_image(app);
        fond_decoratif(app);
        hauteur_contenu = page(app, donnees, defilement);

        rectangle(app, 0, app->hauteur - 44.0f, (float) app->largeur, 44, PANNEAU);
        texte(app, app->police_petite, hauteur_contenu > app->hauteur - 50
                  ? "Flèches ou molette : faire défiler      Entrée, Échap ou clic : revenir"
                  : "Entrée, Échap ou clic : revenir",
              app->largeur / 2.0f, app->hauteur - 32.0f, TEXTE_DOUX, CENTRE);
        fin_image(app);
    }
}

/* ------------------------------------------------------------------------------------------
 * Pages : aperçu d'un labyrinthe, meilleurs scores, règles
 * ------------------------------------------------------------------------------------------ */

typedef struct {
    Grille *grille;
    const char *titre;
} Apercu;

/* Légende horizontale centrée (icône + texte). */
static void legende(App *app, float y, int mode) {
    struct { char motif; const char *texte; } elements[] = {
        {MOTIF_JOUEUR, "vous"}, {MOTIF_ENTREE, "entrée"}, {MOTIF_SORTIE, "sortie"}, {MOTIF_KEY, "clé"},
        {MOTIF_TRAP, "piège"}, {MOTIF_BOOST, "bonus"}, {MOTIF_OGRE, "ogre"}, {MOTIF_FANTOME, "fantôme"},
    };
    int nbr = 6 + (mode != MODE_FACILE) + (mode == MODE_DIFFICILE);
    const float icone = 26, espace = 22;
    float total = 0;
    for (int k = 0; k < nbr; k++) total += icone + 8 + largeur_texte(app->police_petite, elements[k].texte) + espace;
    float x = (app->largeur - total + espace) / 2;
    for (int k = 0; k < nbr; k++) {
        dessiner_icone(app, elements[k].motif, x, y, icone, 0);
        x += icone + 8;
        x += texte(app, app->police_petite, elements[k].texte, x, y + 4, TEXTE_DOUX, GAUCHE) + espace;
    }
}

static int page_apercu(App *app, void *donnees, int defilement) {
    Apercu *a = donnees;
    (void) defilement;
    texte(app, app->police_titre, a->titre, app->largeur / 2.0f, 18, TEXTE, CENTRE);
    int w = largeur_texte(app->police_gras, nom_mode(a->grille->mode)) + 24;
    badge_mode(app, a->grille->mode, (app->largeur - w) / 2.0f, 70);
    SDL_Rect zone = { MARGE, 112, app->largeur - 2 * MARGE, app->hauteur - 112 - 44 - 50 };
    dessiner_labyrinthe(app, a->grille, zone, NULL);
    legende(app, app->hauteur - 44 - 38.0f, a->grille->mode);
    return 0;
}

typedef struct {
    Grille *grille;
    const char *nom;
} Scores;

static int page_scores(App *app, void *donnees, int defilement) {
    Scores *s = donnees;
    char ligne[TAILLE_NOM + 32];
    (void) defilement;

    int largeur = mini(560, app->largeur - 2 * MARGE);
    int hauteur = 150 + NOMBRE_SCORE * 44;
    float x = (app->largeur - largeur) / 2.0f;
    float y = maxf(MARGE, (app->hauteur - 44 - hauteur) / 2.0f);
    panneau(app, x, y, (float) largeur, (float) hauteur, PANNEAU, PANNEAU_BORD);

    texte(app, app->police_titre, "Meilleurs scores", app->largeur / 2.0f, y + 24, ACCENT, CENTRE);
    snprintf(ligne, sizeof ligne, "Labyrinthe « %s » — mode %s", s->nom, nom_mode(s->grille->mode));
    texte(app, app->police, ligne, app->largeur / 2.0f, y + 78, TEXTE_DOUX, CENTRE);

    float cy = y + 124;
    int affiches = 0;
    const SDL_Color medailles[3] = { OR, {200, 205, 220, 255}, {205, 127, 50, 255} };
    for (int i = 0; i < NOMBRE_SCORE; i++) {
        Top_Player *tp = &s->grille->top_scores[i];
        if (tp->name == NULL) continue;
        if (affiches % 2 == 0) rectangle_arrondi(app, x + 24, cy - 6, largeur - 48.0f, 40, 8, BOUTON);
        snprintf(ligne, sizeof ligne, "%d", affiches + 1);
        if (affiches < 3) {
            disque(app, x + 52, cy + 14, 14, medailles[affiches]);
            texte(app, app->police_gras, ligne, x + 52, cy + 3, TEXTE_SOMBRE, CENTRE);
        } else {
            texte(app, app->police, ligne, x + 52, cy + 3, TEXTE_DOUX, CENTRE);
        }
        texte(app, affiches == 0 ? app->police_gras : app->police, tp->name, x + 84, cy + 3, TEXTE, GAUCHE);
        snprintf(ligne, sizeof ligne, "%d", tp->score);
        texte(app, app->police_gras, ligne, x + largeur - 44.0f, cy + 3, affiches == 0 ? ACCENT : TEXTE, DROITE);
        cy += 44;
        affiches++;
    }
    if (affiches == 0) {
        texte(app, app->police, "Aucun score enregistré pour ce labyrinthe.", app->largeur / 2.0f, cy + 40,
              TEXTE_DOUX, CENTRE);
    }
    return 0;
}

static int page_regles(App *app, void *donnees, int defilement) {
    (void) donnees;
    char tampon[256];
    int largeur = mini(860, app->largeur - 2 * MARGE);
    float x = (app->largeur - largeur) / 2.0f;
    float y = 30.0f - defilement;
    float y_depart = y;

    texte(app, app->police_titre, "Règles du jeu", app->largeur / 2.0f, y, ACCENT, CENTRE);
    y += 70;

    struct { const char *titre; const char *texte; } sections[] = {
        {"But du jeu",
         "Partez de l'entrée, ramassez la clé puis atteignez la sortie avec le score le plus bas possible, "
         "sans vous faire attraper par l'ogre."},
        {"Déplacements",
         "Flèches ou Z Q S D (W A S D marche aussi). Gardez une touche enfoncée pour avancer en continu. "
         "Échap abandonne la partie. Dans les menus : flèches ou souris, Entrée ou clic pour valider, "
         "chiffres 1 à 9 pour choisir directement, Échap pour revenir."},
    };
    for (int k = 0; k < 2; k++) {
        texte(app, app->police_gras, sections[k].titre, x, y, ACCENT, GAUCHE);
        y += 32;
        y += paragraphe(app, app->police, sections[k].texte, x, y, largeur, TEXTE, 0, 1) + 22;
    }

    texte(app, app->police_gras, "Ce que vous verrez dans le labyrinthe", x, y, ACCENT, GAUCHE);
    y += 38;
    struct { char motif; const char *texte; } elements[9] = {
        {MOTIF_MUR,     "Mur : infranchissable."},
        {MOTIF_ENTREE,  "Entrée : votre point de départ."},
        {MOTIF_SORTIE,  "Sortie : fermée par un cadenas tant que vous n'avez pas la clé."},
        {MOTIF_JOUEUR,  "Vous, le joueur."},
        {MOTIF_KEY,     "Clé : ouvre la sortie (aucun effet sur le score)."},
        {MOTIF_TRAP,    NULL},
        {MOTIF_BOOST,   NULL},
        {MOTIF_OGRE,    NULL},
        {MOTIF_FANTOME, "Fantôme (mode Difficile) : erre au hasard et traverse les murs. S'il vous attrape, "
                        "votre score est doublé et il réapparaît ailleurs."},
    };
    char piege[160], bonus[160], ogre[256];
    snprintf(piege, sizeof piege, "Piège : +%d points. Il reste en place et pique à chaque passage.", PENALITE_PIEGE);
    snprintf(bonus, sizeof bonus, "Bonus : -%d points (minimum 0). Il disparaît une fois pris.", GAIN_BONUS);
    snprintf(ogre, sizeof ogre, "Ogre (modes Normal et Difficile) : ne traverse pas les murs. En Normal il erre ; "
             "en Difficile il vous chasse dès que vous êtes à moins de %d cases. S'il vous attrape, "
             "la partie est perdue.", RAYON_CHASSE);
    elements[5].texte = piege;
    elements[6].texte = bonus;
    elements[7].texte = ogre;
    for (int k = 0; k < 9; k++) {
        dessiner_icone(app, elements[k].motif, x, y, 36, 0);
        int h = paragraphe(app, app->police, elements[k].texte, x + 56, y + 6, largeur - 56, TEXTE, 0, 1);
        y += maxf(48, h + 18);
    }
    y += 10;

    struct { const char *titre; const char *texte; } fin[] = {
        {"Modes de jeu",
         "Facile : ni ogre ni fantôme.\nNormal : un ogre qui erre sans chasser ; pièges et bonus intermédiaires.\n"
         "Difficile : ogre chasseur et fantôme, deux fois plus de pièges et deux fois moins de bonus."},
        {"Score", NULL},
    };
    snprintf(tampon, sizeof tampon,
             "Chaque déplacement ajoute 1 point (même contre un mur). Plus le score est bas, meilleur il est : "
             "les %d meilleurs sont enregistrés pour chaque labyrinthe.", NOMBRE_SCORE);
    fin[1].texte = tampon;
    for (int k = 0; k < 2; k++) {
        texte(app, app->police_gras, fin[k].titre, x, y, ACCENT, GAUCHE);
        y += 32;
        y += paragraphe(app, app->police, fin[k].texte, x, y, largeur, TEXTE, 0, 1) + 22;
    }
    return (int) (y - y_depart) + 60 + 30;
}

/* ------------------------------------------------------------------------------------------
 * Les actions du menu principal
 * ------------------------------------------------------------------------------------------ */

/* Fait choisir un labyrinthe existant. Renvoie 0 si l'utilisateur revient au menu. */
static int gui_choisir_labyrinthe(App *app, const char *titre, char *nom) {
    static char fichiers[NBR_LABYRINTHES_MAX][TAILLE_NOM];
    const char *options[NBR_LABYRINTHES_MAX + 1];
    int nbr = lister_labyrinthes(fichiers);

    if (nbr == 0) {
        gui_message(app, "Aucun labyrinthe", "Créez d'abord un labyrinthe depuis le menu principal.", ORANGE);
        return 0;
    }
    for (int i = 0; i < nbr; i++) options[i] = fichiers[i];
    options[nbr] = "Revenir au menu";

    Menu m = { .titre = titre, .texte = "Choisissez un labyrinthe.", .options = options,
               .nbr_options = nbr + 1, .echap = 1 };
    int choix = gui_choisir(app, &m);
    if (choix == nbr) return 0;
    snprintf(nom, TAILLE_NOM, "%s", fichiers[choix]);
    return 1;
}

/* Demande une dimension impaire entre DIMENSION_MIN et DIMENSION_MAX. Renvoie 0 pour revenir. */
static int gui_saisir_dimension(App *app, const char *titre, const char *libelle, const char *defaut) {
    char saisie[8];
    char aide[128];
    char erreur[128] = "";
    snprintf(saisie, sizeof saisie, "%s", defaut);
    snprintf(aide, sizeof aide, "%s du labyrinthe : un nombre impair entre %d et %d.", libelle,
             DIMENSION_MIN, DIMENSION_MAX);

    while (gui_saisir(app, titre, aide, saisie, 3, 1, erreur[0] ? erreur : NULL)) {
        int valeur = atoi(saisie);
        if (valeur < DIMENSION_MIN || valeur > DIMENSION_MAX) {
            snprintf(erreur, sizeof erreur, "%d n'est pas entre %d et %d.", valeur, DIMENSION_MIN, DIMENSION_MAX);
        } else if (valeur % 2 == 0) {
            snprintf(erreur, sizeof erreur, "%d est pair : choisissez un nombre impair.", valeur);
        } else {
            return valeur;
        }
    }
    return 0;
}

/* Construit le labyrinthe sous les yeux du joueur (une touche ou un clic accélère). */
static void gui_construire(App *app, Cellule *cellules, Grille *grille, const char *nom) {
    char titre[TAILLE_NOM + 32];
    int nbr_cellules = cellules->nbr_cellules_horizontales * cellules->nbr_cellules_verticales;
    int pas_par_image = nbr_cellules / 25 + 1;
    Uint32 debut = SDL_GetTicks();
    int accelerer = 0;

    snprintf(titre, sizeof titre, "Construction de « %s »...", nom);
    while (!labyrinthe_complet(cellules, grille)) {
        if (accelerer || SDL_GetTicks() - debut > DUREE_CONSTRUCTION) {
            construire_labyrinthe(cellules, grille);
            continue;
        }

        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) { app->quitter = 1; accelerer = 1; }
            if (e.type == SDL_KEYDOWN || e.type == SDL_MOUSEBUTTONDOWN) accelerer = 1;
        }
        for (int k = 0; k < pas_par_image; k++) construire_labyrinthe(cellules, grille);

        debut_image(app);
        fond_decoratif(app);
        texte(app, app->police_titre, titre, app->largeur / 2.0f, 18, TEXTE, CENTRE);
        SDL_Rect zone = { MARGE, 90, app->largeur - 2 * MARGE, app->hauteur - 90 - 60 };
        dessiner_labyrinthe(app, grille, zone, NULL);
        texte(app, app->police_petite, "Appuyez sur une touche pour terminer tout de suite",
              app->largeur / 2.0f, app->hauteur - 40.0f, TEXTE_DOUX, CENTRE);
        fin_image(app);
    }
}

static void option_creer(App *app) {
    const char *options[] = {
        "Facile — pas de monstres",
        "Normal — un ogre qui erre",
        "Difficile — ogre chasseur et fantôme",
        "Revenir au menu"
    };
    const int modes[] = { MODE_FACILE, MODE_NORMAL, MODE_DIFFICILE };
    Menu m = { .titre = "Créer un labyrinthe", .texte = "Choisissez le mode de jeu.",
               .options = options, .nbr_options = 4, .echap = 1 };
    int choix = gui_choisir(app, &m);
    if (choix == 3 || app->quitter) return;
    int mode = modes[choix];

    int hauteur = gui_saisir_dimension(app, "Hauteur", "Hauteur", "21");
    if (hauteur == 0) return;
    int largeur = gui_saisir_dimension(app, "Largeur", "Largeur", "41");
    if (largeur == 0) return;

    /* Nom du labyrinthe, avec confirmation avant d'écraser un labyrinthe existant */
    char nom[TAILLE_NOM] = "";
    char erreur[TAILLE_NOM + 64] = "";
    while (1) {
        if (!gui_saisir(app, "Nom du labyrinthe", "Lettres sans accent, chiffres, _ ou -.", nom,
                        TAILLE_NOM_MAX, 0, erreur[0] ? erreur : NULL)) return;
        if (!nom_valide(nom)) {
            snprintf(erreur, sizeof erreur, "Ce nom n'est pas valide.");
            continue;
        }
        if (!labyrinthe_existe(nom)) break;

        char titre[TAILLE_NOM + 32];
        const char *confirmation[] = {
            "Oui, l'écraser (son top 10 sera perdu)",
            "Non, choisir un autre nom",
            "Revenir au menu"
        };
        snprintf(titre, sizeof titre, "« %s » existe déjà", nom);
        Menu c = { .titre = titre, .texte = "Voulez-vous remplacer ce labyrinthe ?", .options = confirmation,
                   .nbr_options = 3, .echap = 1, .couleur_titre = ORANGE };
        int reponse = gui_choisir(app, &c);
        if (reponse == 0) break;
        if (reponse == 2) return;
        erreur[0] = '\0';
    }

    Cellule cellules;
    Grille grille;
    allouer_cellules(&cellules, (hauteur - 1) / 2, (largeur - 1) / 2);
    allouer_grille(&grille, hauteur, largeur);
    initialiser_cellules(&cellules);
    initialiser_grille(&grille, &cellules);

    gui_construire(app, &cellules, &grille, nom);
    placer_motifs(&grille, mode);
    sauvegarder_labyrinthe(&grille, nom);

    if (!app->quitter) {
        char titre[TAILLE_NOM + 32];
        snprintf(titre, sizeof titre, "« %s » est prêt !", nom);
        Apercu a = { &grille, titre };
        gui_page(app, page_apercu, &a);
    }

    liberer_grille(&grille);
    liberer_cellules(&cellules);
}

static void option_charger(App *app) {
    char nom[TAILLE_NOM];
    Grille grille;

    if (!gui_choisir_labyrinthe(app, "Voir un labyrinthe", nom)) return;
    if (!charger_labyrinthe(&grille, nom)) {
        gui_message(app, "Erreur", "Ce labyrinthe n'a pas pu être chargé.", ROUGE);
        return;
    }
    Apercu a = { &grille, nom };
    gui_page(app, page_apercu, &a);
    liberer_grille(&grille);
}

/* Joue une partie. Renvoie 1 si le joueur veut rejouer le même labyrinthe. */
static int gui_partie(App *app, const char *nom_joueur, char *nom_labyrinthe) {
    Partie p;
    memset(&p, 0, sizeof p);
    if (!charger_labyrinthe(&p.grille, nom_labyrinthe)) {
        gui_message(app, "Erreur", "Ce labyrinthe n'a pas pu être chargé.", ROUGE);
        return 0;
    }
    p.nom_joueur = nom_joueur;
    p.nom_labyrinthe = nom_labyrinthe;
    initialiser_joueur(&p.joueur);
    demarrer_partie(&p.grille, &p.joueur, &p.ogre, &p.fantome);
    snprintf(p.message, sizeof p.message, "%s", p.joueur.message);
    p.message_debut = SDL_GetTicks();
    p.joueur_l = (float) p.joueur.x;  p.joueur_c = (float) p.joueur.y;
    p.ogre_l = (float) p.ogre.x;      p.ogre_c = (float) p.ogre.y;
    p.fantome_l = (float) p.fantome.x; p.fantome_c = (float) p.fantome.y;
    p.dir_ligne = 1; /* L'entrée est en haut : le joueur regarde vers le bas */

    int etat = PARTIE_EN_COURS;
    int abandon = 0;
    Uint32 fin = 0, dernier_pas = 0, precedent = SDL_GetTicks();

    while (1) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                app->quitter = 1;
                liberer_grille(&p.grille);
                return 0;
            }
            if (e.type != SDL_KEYDOWN || etat != PARTIE_EN_COURS || abandon) continue;

            int touche = touche_jeu(e.key.keysym.sym);
            Uint32 maintenant = SDL_GetTicks();
            if (touche == TOUCHE_ECHAP) {
                abandon = 1;
                fin = maintenant;
            } else if (touche != 0 && (!e.key.repeat || maintenant - dernier_pas >= DELAI_REPETITION)) {
                dernier_pas = maintenant;
                etat = jouer_coup(&p, touche);
                if (etat != PARTIE_EN_COURS) fin = maintenant;
            }
        }

        Uint32 maintenant = SDL_GetTicks();
        float dt = (maintenant - precedent) / 1000.0f;
        precedent = maintenant;
        glisser(&p.joueur_l, p.joueur.x, dt);
        glisser(&p.joueur_c, p.joueur.y, dt);
        glisser(&p.ogre_l, p.ogre.x, dt);
        glisser(&p.ogre_c, p.ogre.y, dt);
        glisser(&p.fantome_l, p.fantome.x, dt);
        glisser(&p.fantome_c, p.fantome.y, dt);

        debut_image(app);
        dessiner_partie(app, &p);
        fin_image(app);

        if ((etat != PARTIE_EN_COURS || abandon) && maintenant - fin > (abandon ? 0 : DUREE_FIN)) break;
    }

    /* Écran de fin, par-dessus la partie */
    char resultat[3 * TAILLE_MESSAGE];
    char details[2 * TAILLE_MESSAGE] = "";
    const char *titre;
    SDL_Color couleur;
    if (etat == PARTIE_GAGNEE) {
        enregistrer_victoire(&p.grille, &p.joueur, nom_joueur, nom_labyrinthe, details, sizeof details);
        snprintf(resultat, sizeof resultat, "Score : %d\n%s", get_score_joueur(&p.joueur), details);
        titre = "Victoire !";
        couleur = ACCENT;
    } else if (etat == PARTIE_PERDUE) {
        snprintf(resultat, sizeof resultat, "L'ogre vous a attrapé. Votre score n'est pas enregistré.");
        titre = "Perdu !";
        couleur = ROUGE;
    } else {
        snprintf(resultat, sizeof resultat, "Vous avez quitté la partie.");
        titre = "Partie abandonnée";
        couleur = TEXTE;
    }
    p.message[0] = '\0';

    const char *options[] = { "Rejouer ce labyrinthe", "Revenir au menu" };
    Menu m = { .titre = titre, .texte = resultat, .options = options, .nbr_options = 2, .echap = 1,
               .fond = dessiner_partie, .donnees = &p, .couleur_titre = couleur };
    int choix = gui_choisir(app, &m);

    liberer_grille(&p.grille);
    return choix == 0 && !app->quitter;
}

/* Fait choisir le joueur (ou en créer un nouveau). Renvoie 0 pour revenir au menu. */
static int gui_choisir_joueur(App *app, char *nom_joueur) {
    char noms[NBR_JOUEURS_MAX][TAILLE_NOM];
    const char *options[NBR_JOUEURS_MAX + 2];
    int nbr = charger_noms_joueurs(noms);

    for (int i = 0; i < nbr; i++) options[i] = noms[i];
    options[nbr] = "Nouveau joueur";
    options[nbr + 1] = "Revenir au menu";

    Menu m = { .titre = "Qui joue ?", .options = options, .nbr_options = nbr + 2, .echap = 1 };
    int choix = gui_choisir(app, &m);
    if (choix == nbr + 1) return 0;

    if (choix == nbr) {
        nom_joueur[0] = '\0';
        if (!gui_saisir(app, "Nouveau joueur", "Votre nom : lettres sans accent, chiffres, _ ou -.",
                        nom_joueur, TAILLE_NOM_MAX, 0, NULL)) return 0;
    } else {
        snprintf(nom_joueur, TAILLE_NOM, "%s", noms[choix]);
    }
    enregistrer_nom_joueur(nom_joueur);
    return 1;
}

static void option_jouer(App *app) {
    char nom_joueur[TAILLE_NOM];
    char nom_labyrinthe[TAILLE_NOM];

    if (!labyrinthe_disponible()) {
        gui_message(app, "Aucun labyrinthe", "Créez d'abord un labyrinthe depuis le menu principal.", ORANGE);
        return;
    }
    if (!gui_choisir_joueur(app, nom_joueur)) return;
    if (!gui_choisir_labyrinthe(app, "Jouer", nom_labyrinthe)) return;

    while (gui_partie(app, nom_joueur, nom_labyrinthe)) {
        /* On rejoue avec le même joueur sur le même labyrinthe */
    }
}

static void option_scores(App *app) {
    char nom[TAILLE_NOM];
    Grille grille;

    if (!gui_choisir_labyrinthe(app, "Meilleurs scores", nom)) return;
    if (!charger_labyrinthe(&grille, nom)) {
        gui_message(app, "Erreur", "Ce labyrinthe n'a pas pu être chargé.", ROUGE);
        return;
    }
    Scores s = { &grille, nom };
    gui_page(app, page_scores, &s);
    liberer_grille(&grille);
}

static void option_supprimer(App *app) {
    char nom[TAILLE_NOM];
    char titre[TAILLE_NOM + 32];
    char message[TAILLE_NOM + 64];

    if (!gui_choisir_labyrinthe(app, "Supprimer un labyrinthe", nom)) return;

    const char *options[] = { "Oui, le supprimer (son top 10 sera perdu)", "Non, revenir au menu" };
    snprintf(titre, sizeof titre, "Supprimer « %s » ?", nom);
    Menu m = { .titre = titre, .texte = "Cette action est définitive.", .options = options,
               .nbr_options = 2, .echap = 1, .couleur_titre = ROUGE };
    if (gui_choisir(app, &m) != 0 || app->quitter) return;

    if (supprimer_labyrinthe(nom)) {
        snprintf(message, sizeof message, "Le labyrinthe « %s » a été supprimé.", nom);
        gui_message(app, "Supprimé", message, TEXTE);
    } else {
        gui_message(app, "Erreur", "Impossible de supprimer ce labyrinthe.", ROUGE);
    }
}

/* ------------------------------------------------------------------------------------------
 * Programme principal
 * ------------------------------------------------------------------------------------------ */

static TTF_Font *ouvrir_police(const char *chemins[], int nbr, int taille) {
    for (int i = 0; i < nbr; i++) {
        TTF_Font *police = TTF_OpenFont(chemins[i], taille);
        if (police != NULL) return police;
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    (void) argc;
    (void) argv;
    App app;
    memset(&app, 0, sizeof app);
    srand((unsigned) time(NULL)); /* Le seul appel à srand du programme */

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Impossible d'initialiser SDL : %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    if (TTF_Init() != 0) {
        fprintf(stderr, "Impossible d'initialiser SDL_ttf : %s\n", TTF_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    int nbr_polices = (int) (sizeof POLICES / sizeof POLICES[0]);
    app.police_titre  = ouvrir_police(POLICES_GRAS, nbr_polices, 34);
    app.police_gras   = ouvrir_police(POLICES_GRAS, nbr_polices, 19);
    app.police        = ouvrir_police(POLICES, nbr_polices, 19);
    app.police_petite = ouvrir_police(POLICES, nbr_polices, 15);
    if (!app.police_titre || !app.police_gras || !app.police || !app.police_petite) {
        fprintf(stderr, "Aucune police trouvée. Copiez un fichier .ttf dans assets/police.ttf "
                        "et assets/police-gras.ttf (ou installez fonts-dejavu-core).\n");
        TTF_Quit();
        SDL_Quit();
        return EXIT_FAILURE;
    }

    app.fenetre = SDL_CreateWindow("Labyrinthe", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   FENETRE_LARGEUR, FENETRE_HAUTEUR, SDL_WINDOW_RESIZABLE);
    if (app.fenetre == NULL) {
        fprintf(stderr, "Impossible de créer la fenêtre : %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_SetWindowMinimumSize(app.fenetre, FENETRE_LARGEUR_MIN, FENETRE_HAUTEUR_MIN);
    app.rendu = SDL_CreateRenderer(app.fenetre, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (app.rendu == NULL) app.rendu = SDL_CreateRenderer(app.fenetre, -1, SDL_RENDERER_SOFTWARE);
    if (app.rendu == NULL) {
        fprintf(stderr, "Impossible de créer le rendu : %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    SDL_SetRenderDrawBlendMode(app.rendu, SDL_BLENDMODE_BLEND);
    SDL_StopTextInput(); /* La saisie de texte n'est activée que dans les champs de saisie */

    const char *options[] = {
        "Jouer",
        "Créer un labyrinthe",
        "Voir un labyrinthe",
        "Meilleurs scores",
        "Règles du jeu",
        "Supprimer un labyrinthe",
        "Quitter"
    };
    Menu principal = { .titre = "LABYRINTHE", .texte = "Trouvez la clé, évitez l'ogre, atteignez la sortie.",
                       .options = options, .nbr_options = 7, .echap = 0, .couleur_titre = ACCENT };

    while (!app.quitter) {
        switch (gui_choisir(&app, &principal)) {
            case 0: option_jouer(&app);     break;
            case 1: option_creer(&app);     break;
            case 2: option_charger(&app);   break;
            case 3: option_scores(&app);    break;
            case 4: gui_page(&app, page_regles, NULL); break;
            case 5: option_supprimer(&app); break;
            default: app.quitter = 1;       break;
        }
    }

    TTF_CloseFont(app.police_titre);
    TTF_CloseFont(app.police_gras);
    TTF_CloseFont(app.police);
    TTF_CloseFont(app.police_petite);
    SDL_DestroyRenderer(app.rendu);
    SDL_DestroyWindow(app.fenetre);
    TTF_Quit();
    SDL_Quit();
    return EXIT_SUCCESS;
}
