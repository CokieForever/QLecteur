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

#ifndef AFFICHAGE_PAROLES

#define AFFICHAGE_PAROLES
#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <string.h>
#include "fonctions_souris.h"


typedef struct AffichageParoles
{
        SDL_Rect positionBarreDefilement;
        int hauteurBarreDefilement;
        int hauteurDiff;
        SDL_Rect positionSurfaceParoles;
        int clicGaucheSurBarreDefil;
        SDL_Rect differenceAuClic;
        int hauteurZone;
        int largeurZone;
        TTF_Font *police;
        SDL_Surface *surfaceParoles;
        SDL_Surface *surfaceBarreDefilement;
} AffichageParoles;

#include "main.h"
#include "fonctions_texte.h"

void afficher_paroles (AffichageParoles *affichageParoles, Musique *musique, SDL_Event event);
void actualiser_paroles (AffichageParoles *affichageParoles, Musique *musique);
void AFF_recreer_barre_defil (AffichageParoles *affichageParoles);

#endif
