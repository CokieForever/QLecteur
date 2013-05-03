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

#ifndef FONCTIONS_SOURIS

#define FONCTIONS_SOURIS

#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <math.h>


int cliquer_deplacer (SDL_Event event, SDL_Surface *surface, SDL_Rect *positionSurface, SDL_Rect *differenceAuClic, int *clicGauche);
int test_button (SDL_Event event, SDL_Surface *ecran, SDL_Surface *bouton, SDL_Rect positionBouton, Uint32 couleurDuCadre, Uint32 couleurCadreAuClic, char bulleInfo[], int tempsDAttente, int *tempsDernierMvtSouris, int *clicGaucheSurBouton);
int test_button_rond (SDL_Event event, SDL_Surface *ecran, SDL_Surface *bouton, SDL_Rect positionBouton, Uint32 couleurCadre, Uint32 couleurCadreAuClic, char texteBulleInfo[], int tempsDAttente, int *tempsDernierMvtSouris, int *clicGaucheSurBouton);
int test_position(SDL_Event event, SDL_Rect position, int largeur, int hauteur);
int test_position_rond(SDL_Event event, SDL_Rect positionCentre, int diametre);

#endif
