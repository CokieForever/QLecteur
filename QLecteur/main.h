/*
QLecteur - Multimedia player
Copyright (C) 2008-2013  Cokie

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef MAIN

#define MAIN

#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include "fmod.h"
#include <smpeg.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "defines.h"
#include "config.h"



typedef enum Mode Mode;
enum Mode
{
     PLAY, PAUSE, STOP, ERREUR
};

typedef enum TypeStream TypeStream;
enum TypeStream
{
     NORMAL, RADIO, INTERNET, VIDEO
};

typedef struct SurfacesMusique SurfacesMusique;
struct SurfacesMusique
{
       SDL_Surface *nom;
       SDL_Surface *titre;
       SDL_Surface *auteur;
       SDL_Surface *album;
       SDL_Surface *annee;
       SDL_Surface *longueur;
       SDL_Surface *poids;
       SDL_Surface *curseur;
       SDL_Surface *frequence;
       SDL_Surface *debit;
       SDL_Surface *format;
       SDL_Surface *blit;
       SDL_Surface *image;
};

typedef struct PositionsSurfacesMusique PositionsSurfacesMusique;
struct PositionsSurfacesMusique
{
       SDL_Rect nom;
       SDL_Rect titre;
       SDL_Rect auteur;
       SDL_Rect album;
       SDL_Rect annee;
       SDL_Rect longueur;
       SDL_Rect poids;
       SDL_Rect curseur;
       SDL_Rect frequence;
       SDL_Rect debit;
       SDL_Rect format;
       SDL_Rect image;
};

typedef struct Musique Musique;
struct Musique
{
       char nom[TAILLE_MAX_NOM];
       char adresse[TAILLE_MAX_NOM];

       char titre[TAILLE_MAX_NOM];
       char auteur[TAILLE_MAX_NOM];
       char album[TAILLE_MAX_NOM];
       char annee[TAILLE_MAX_NOM];
       char genre[TAILLE_MAX_NOM];
       char paroles[TAILLE_MAX_PAROLES];
       int numero;
       int longueur;
       double poids;
       double frequence;
       int debit;
       char format[10];

       FSOUND_STREAM *pointeurMono;
       FSOUND_STREAM *pointeurStereo;
       int curseur;

       Mode mode;
       TypeStream type;
       PositionsSurfacesMusique position;
       SurfacesMusique surface;

       int erreur;
       int statutInternet;
       int protocoleInternet;
};

#include "affichage_paroles.h"

typedef enum TypePlaylist TypePlaylist;
enum TypePlaylist
{
     NOM, TITRE, AUTEUR, NOM_SIMPLIFIE, TITRE_ET_AUTEUR
};


void setPixel(SDL_Surface *surface, int x, int y, Uint32 pixel);
void setPixel2(SDL_Surface *surface, int x, int y, Uint32 couleurPixel);
void setRectangle(SDL_Surface *surface, int x, int y, Uint32 couleurRectangle);
int test_quit (SDL_Event event, int *alt);
void charger_musique(Musique *musique, MenuConfigLecteur configMenu, AffichageParoles *affichageParoles, char nomFichierListe[], int numeroLigne, int cutPosition, int *numeroChargement);
int creer_playlist(char nomFichierListe[], char nomFichierSauvegarde2[], TypePlaylist type);
void mettre_en_stop(Musique *musique, SDL_Surface *surfaceBoutonPlayPause);
void mettre_a_jour_playlists(char nomFichierListePlaylists[], char nomFichierPlaylistDePlaylists[], int tableauTotalMusiques[TAILLE_MAX_PLAYLIST]);
int recuperer_tag(Musique *musique, int prendreImage, int prendreParoles);
void stocker_erreur(int *numeroErreur, char chaineStockage[]);
VOID CALLBACK OsmProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
VOID CALLBACK ReceiveProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

void display_callback (SDL_Surface* dst, int x, int y, unsigned int w, unsigned int h);
void resize_video(SMPEG* smpeg);

char* strstr2 (char string1[], char string2[], int max);
unsigned int inverser_nombre (unsigned int nombre);
SDL_Surface* recuperer_image (char nomDuFichier[]);


extern char dossierBase[TAILLE_MAX_NOM],
            nomBase[TAILLE_MAX_NOM];

extern float *spectre;
extern MenuConfigLecteur *copieConfigMenu;
extern int passerAuSuivant, passerAuPrecedent, mettreEnPlayOuPause, mettreEnStop;
extern HWND mainWnd;
extern void *sharedAuxMemory;
extern SDL_Surface *surfaceSMPEG;
extern SDL_Rect positionSurfaceSMPEG;
extern int numeroVisu, secondsSkipped;
extern int bigScreenMode, tempsDernierResize;
extern float agrandissement;

#endif
