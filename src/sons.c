/*
 * sons.c : effets sonores de la version 2D.
 *
 * Les sons ne viennent pas de fichiers : ils sont calculés au démarrage, échantillon par
 * échantillon, comme sur un petit synthétiseur (notes sinusoïdales, carrées, en dents de
 * scie ou bruit). Chaque son est rangé dans un fichier WAV en mémoire puis chargé par
 * SDL2_mixer, qui s'occupe de le convertir au format de la carte son et de mélanger
 * plusieurs sons joués en même temps.
 */
#include <SDL.h>
#include <SDL_mixer.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "sons.h"

#define FREQUENCE 44100   /* Échantillons par seconde */
#define CANAUX_MIXAGE 16  /* Nombre de sons pouvant être joués en même temps */

typedef enum { SINUS, CARRE, SCIE, BRUIT } Forme;

static Mix_Chunk *sons[NBR_SONS];
static int audio_ouvert = 0;
static int muet = 0;

/* Ajoute une note dans le tampon (valeurs entre -1 et 1).
 * La fréquence glisse de f0 à f1 pendant la note ; le volume monte en 5 ms puis
 * redescend jusqu'à 0 à la fin de la note (sans clic). */
static void ajouter_note(float *tampon, int taille, float debut, float duree, float f0, float f1,
                         Forme forme, float volume) {
    int premier = (int) (debut * FREQUENCE);
    int nombre = (int) (duree * FREQUENCE);
    int attaque = FREQUENCE / 200;
    float phase = 0;

    for (int k = 0; k < nombre && premier + k < taille; k++) {
        float progression = (float) k / nombre;
        float frequence = f0 + (f1 - f0) * progression;
        phase += frequence / FREQUENCE;
        phase -= floorf(phase);

        float onde;
        switch (forme) {
            case CARRE: onde = phase < 0.5f ? 1.0f : -1.0f; break;
            case SCIE:  onde = 2 * phase - 1; break;
            case BRUIT: onde = 2.0f * rand() / RAND_MAX - 1; break;
            default:    onde = sinf(2 * (float) M_PI * phase); break;
        }

        float enveloppe = k < attaque ? (float) k / attaque : 1 - progression;
        tampon[premier + k] += onde * volume * enveloppe * enveloppe;
    }
}

/* Écrit un entier en petit-boutiste (ordre des octets d'un fichier WAV). */
static void ecrire_entier(Uint8 *p, Uint32 valeur, int octets) {
    for (int i = 0; i < octets; i++) p[i] = (Uint8) (valeur >> (8 * i));
}

/* Transforme le tampon en fichier WAV (16 bits, mono) en mémoire et le charge dans SDL2_mixer. */
static Mix_Chunk *creer_son(const float *tampon, int taille) {
    Uint32 octets_donnees = (Uint32) taille * 2;
    Uint8 *wav = malloc(44 + octets_donnees);
    if (wav == NULL) return NULL;

    memcpy(wav, "RIFF", 4);
    ecrire_entier(wav + 4, 36 + octets_donnees, 4);
    memcpy(wav + 8, "WAVEfmt ", 8);
    ecrire_entier(wav + 16, 16, 4);              /* Taille du bloc "fmt " */
    ecrire_entier(wav + 20, 1, 2);               /* Format PCM */
    ecrire_entier(wav + 22, 1, 2);               /* Mono */
    ecrire_entier(wav + 24, FREQUENCE, 4);
    ecrire_entier(wav + 28, FREQUENCE * 2, 4);   /* Octets par seconde */
    ecrire_entier(wav + 32, 2, 2);               /* Octets par échantillon */
    ecrire_entier(wav + 34, 16, 2);              /* Bits par échantillon */
    memcpy(wav + 36, "data", 4);
    ecrire_entier(wav + 40, octets_donnees, 4);

    for (int k = 0; k < taille; k++) {
        float v = tampon[k];
        if (v > 1) v = 1;
        if (v < -1) v = -1;
        ecrire_entier(wav + 44 + 2 * k, (Uint32) (Uint16) (Sint16) (v * 32767), 2);
    }

    Mix_Chunk *son = Mix_LoadWAV_RW(SDL_RWFromConstMem(wav, (int) (44 + octets_donnees)), 1);
    free(wav);
    return son;
}

/* Fabrique un son : une suite de notes dans un tampon de la durée donnée (en secondes). */
static Mix_Chunk *fabriquer(Son son) {
    float duree;
    switch (son) {
        case SON_PAS:      duree = 0.05f; break;
        case SON_MUR:      duree = 0.09f; break;
        case SON_PORTE:    duree = 0.25f; break;
        case SON_CLE:      duree = 0.45f; break;
        case SON_PIEGE:    duree = 0.30f; break;
        case SON_BONUS:    duree = 0.30f; break;
        case SON_FANTOME:  duree = 0.60f; break;
        case SON_OGRE:     duree = 0.90f; break;
        case SON_VICTOIRE: duree = 1.00f; break;
        case SON_MENU:     duree = 0.03f; break;
        default:           duree = 0.08f; break;
    }

    int taille = (int) (duree * FREQUENCE);
    float *t = calloc((size_t) taille, sizeof *t);
    if (t == NULL) return NULL;

    switch (son) {
        case SON_PAS:      /* Petit pas feutré */
            ajouter_note(t, taille, 0, 0.05f, 190, 110, SINUS, 0.30f);
            ajouter_note(t, taille, 0, 0.02f, 0, 0, BRUIT, 0.06f);
            break;
        case SON_MUR:      /* Choc sourd */
            ajouter_note(t, taille, 0, 0.09f, 110, 55, CARRE, 0.18f);
            ajouter_note(t, taille, 0, 0.04f, 0, 0, BRUIT, 0.12f);
            break;
        case SON_PORTE:    /* Deux notes graves : « non » */
            ajouter_note(t, taille, 0, 0.11f, 220, 220, CARRE, 0.12f);
            ajouter_note(t, taille, 0.12f, 0.13f, 165, 165, CARRE, 0.12f);
            break;
        case SON_CLE:      /* Arpège brillant qui monte */
            ajouter_note(t, taille, 0.00f, 0.15f, 988, 988, SINUS, 0.30f);
            ajouter_note(t, taille, 0.08f, 0.15f, 1319, 1319, SINUS, 0.30f);
            ajouter_note(t, taille, 0.16f, 0.29f, 1976, 1976, SINUS, 0.30f);
            break;
        case SON_PIEGE:    /* Grésillement qui descend */
            ajouter_note(t, taille, 0, 0.30f, 320, 90, SCIE, 0.25f);
            ajouter_note(t, taille, 0, 0.10f, 0, 0, BRUIT, 0.20f);
            break;
        case SON_BONUS:    /* Deux notes joyeuses */
            ajouter_note(t, taille, 0.00f, 0.12f, 784, 784, CARRE, 0.10f);
            ajouter_note(t, taille, 0.10f, 0.20f, 1175, 1175, CARRE, 0.10f);
            break;
        case SON_FANTOME:  /* Plainte qui ondule en descendant */
            for (int k = 0; k < 6; k++) {
                float f = 700 - k * 60;
                ajouter_note(t, taille, k * 0.09f, 0.12f, f + 40, f - 40, SINUS, 0.22f);
            }
            break;
        case SON_OGRE:     /* Grognement grave */
            ajouter_note(t, taille, 0, 0.90f, 95, 45, SCIE, 0.35f);
            ajouter_note(t, taille, 0, 0.90f, 70, 40, CARRE, 0.15f);
            ajouter_note(t, taille, 0, 0.50f, 0, 0, BRUIT, 0.12f);
            break;
        case SON_VICTOIRE: /* Petite fanfare : do mi sol do */
            ajouter_note(t, taille, 0.00f, 0.14f, 523, 523, CARRE, 0.12f);
            ajouter_note(t, taille, 0.13f, 0.14f, 659, 659, CARRE, 0.12f);
            ajouter_note(t, taille, 0.26f, 0.14f, 784, 784, CARRE, 0.12f);
            ajouter_note(t, taille, 0.39f, 0.60f, 1047, 1047, CARRE, 0.14f);
            ajouter_note(t, taille, 0.39f, 0.60f, 523, 523, SINUS, 0.20f);
            break;
        case SON_MENU:     /* Petit tic */
            ajouter_note(t, taille, 0, 0.03f, 1400, 1200, SINUS, 0.12f);
            break;
        default:           /* SON_VALIDER : tic plus franc */
            ajouter_note(t, taille, 0.00f, 0.04f, 900, 900, SINUS, 0.18f);
            ajouter_note(t, taille, 0.03f, 0.05f, 1350, 1350, SINUS, 0.18f);
            break;
    }

    Mix_Chunk *resultat = creer_son(t, taille);
    free(t);
    return resultat;
}

int sons_initialiser(void) {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) return 0;
    if (Mix_OpenAudio(FREQUENCE, AUDIO_S16SYS, 2, 1024) != 0) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return 0;
    }
    audio_ouvert = 1;
    Mix_AllocateChannels(CANAUX_MIXAGE);
    for (int i = 0; i < NBR_SONS; i++) {
        sons[i] = fabriquer((Son) i);
    }
    return 1;
}

void sons_jouer(Son son) {
    if (!audio_ouvert || muet || son < 0 || son >= NBR_SONS || sons[son] == NULL) return;
    Mix_PlayChannel(-1, sons[son], 0);
}

void sons_basculer_muet(void) {
    muet = !muet;
    if (muet && audio_ouvert) Mix_HaltChannel(-1);
}

int sons_muet(void) {
    return muet;
}

void sons_liberer(void) {
    if (!audio_ouvert) return;
    Mix_HaltChannel(-1);
    for (int i = 0; i < NBR_SONS; i++) {
        if (sons[i] != NULL) Mix_FreeChunk(sons[i]);
        sons[i] = NULL;
    }
    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    audio_ouvert = 0;
}
