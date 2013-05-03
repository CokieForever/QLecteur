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

#ifndef CONFIG

#define CONFIG

#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string.h>
#include "defines.h"
#include "fonctions_registre.h"
#include "resize_image.h"


typedef struct Parametre Parametre;
struct Parametre
{
       int sauvegarde;
       int chargementAccurate;
       int chargementNonBlocking;
       int modeReduit;
       int osmoCom;
       int sauvegardeRelative;
       int dureeMaxChargement;
       int dureeMaxEnregistrement;
       int nombreChargementsMaxi;
       int vitesseDefilement;
       int rafraichissement;
       int vitesseTransition;
       int numeroImageDiaporama;
       int nombreImagesDiaporama;
       char dossierDiaporama[TAILLE_MAX_NOM];
       char imageDeFond[TAILLE_MAX_NOM];
       char fichierListeDiaporama[TAILLE_MAX_NOM];
};


typedef struct MenuConfigLecteur MenuConfigLecteur;
struct MenuConfigLecteur
{
       SDL_Surface *surfaceSauvegarde;
       SDL_Rect positionBlit;
       SDL_Surface *surfaceBlit;
       SDL_Surface *ecran;
       SDL_Surface *imageDeFond;
       SDL_Rect positionFond;

       SDL_Surface *tableauSurfaces[100];
       SDL_Rect tableauPositions[100];
       char *tableauTextes[100];
       SDL_Surface *tableauSurfacesReponses[100];
       SDL_Rect tableauPositionsReponses[100];
       char *tableauTextesReponses[100];

       Parametre parametre;
};

#include "main.h"


void CFG_configurer_lecteur(MenuConfigLecteur *configMenu, SDL_Event event);
void CFG_init_config(MenuConfigLecteur *configMenu, SDL_Surface *ecran, SDL_Rect positionBlit, int largeur, int hauteur);
void CFG_RAZ_config(MenuConfigLecteur *configMenu);
void CFG_recreer_surface(MenuConfigLecteur *configMenu);
void CFG_associer_parametres(MenuConfigLecteur *configMenu);

#endif
