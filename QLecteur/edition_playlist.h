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

#ifndef EDITION_PLAYLIST

#define EDITION_PLAYLIST

#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string.h>
#include "fonctions_repertoire.h"
#include "fonctions_texte.h"
#include "main.h"
#include "fonctions_fichier.h"

typedef struct Zone Zone;
struct Zone
{
     int largeur;
     int abscisse;
     SDL_Rect positionListe;
     char nomFichierListe[TAILLE_MAX_NOM];
     char nomFichierPlaylist[TAILLE_MAX_NOM];
     int ligneDebut;
     int ligneFin;
     int nombreAffichables;
     int nombreTotal;
     TTF_Font *police;
     SDL_Surface *tableauSurfaces[TAILLE_MAX_PLAYLIST];
     SDL_Rect tableauPositions[TAILLE_MAX_PLAYLIST];
     char *tableauNoms[TAILLE_MAX_PLAYLIST];
     int tableauSelectionnes[TAILLE_MAX_PLAYLIST];
     SDL_Surface *boutonDossier;
     SDL_Rect positionBoutonDossier;
     SDL_Surface *echantillonTexte;
     int clicGauche[TAILLE_MAX_PLAYLIST];
     SDL_Rect differenceAuClic[TAILLE_MAX_PLAYLIST];
     SDL_Rect positionFantome[TAILLE_MAX_PLAYLIST];
     SDL_Surface *barreDefilement;
     SDL_Rect positionBarreDefilement;
     int hauteurBarreDefilement;
     SDL_Rect positionLigneSeparation;
     SDL_Rect differenceAuClicBarreDefil;
     int clicGaucheSurBarreDefil;
     TTF_Font *policeBoutonTypePlaylist;
     SDL_Rect positionBoutonTypePlaylist;
     SDL_Surface *boutonTypePlaylist;
     TypePlaylist typePlaylist;
};


typedef struct MenuEditPlaylist MenuEditPlaylist;
struct MenuEditPlaylist
{
     SDL_Surface *surfaceBlit;
     SDL_Surface *surfaceSauvegarde;
     SDL_Surface *ligneSeparation;
     SDL_Rect positionZero;
     SDL_Rect positionSurfaceBlit;
     int hauteur;
     char repertoireVirtuel[TAILLE_MAX_NOM];
     char adresseAScanner[TAILLE_MAX_NOM];
     char dossierBase[TAILLE_MAX_NOM];
     int *tempsDernierMvtSouris;
     SDL_Surface *curseurInterdit;
     SDL_Surface *barreRouge;
     SDL_Rect positionBarreRouge;
     SDL_Surface *posteTravail;
     SDL_Rect positionPosteTravail;
     SDL_Surface *nouvellePlaylist;
     SDL_Rect positionNouvellePlaylist;
     SDL_Surface *ecran;

     Zone zoneDossier;
     Zone zoneMusique;
     Zone zonePlaylist;
};


int editer_playlist(MenuEditPlaylist *editMenu, Musique *musique, MenuConfigLecteur configMenu, AffichageParoles *affichageParoles, SDL_Event event, int cutPosition, int *ctrl, int *maj);
void recreer_surface(MenuEditPlaylist *editMenu);
void recreer_barre_defil(MenuEditPlaylist *editMenu);

#endif
