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

#include "affichage_paroles.h"

void afficher_paroles (AffichageParoles *affichageParoles, Musique *musique, SDL_Event event)
{
    SDL_BlitSurface(affichageParoles->surfaceParoles, NULL, musique->surface.blit, &affichageParoles->positionSurfaceParoles);
    if (affichageParoles->surfaceBarreDefilement != NULL)
       SDL_BlitSurface(affichageParoles->surfaceBarreDefilement, NULL, musique->surface.blit, &affichageParoles->positionBarreDefilement);

    SDL_Rect positionBarreDefil = affichageParoles->positionBarreDefilement;
    if (affichageParoles->surfaceBarreDefilement != NULL && cliquer_deplacer (event, affichageParoles->surfaceBarreDefilement, &positionBarreDefil, &affichageParoles->differenceAuClic, &affichageParoles->clicGaucheSurBarreDefil))
    {
        positionBarreDefil.x = affichageParoles->positionBarreDefilement.x;
        if (positionBarreDefil.y + affichageParoles->hauteurBarreDefilement > affichageParoles->hauteurZone + affichageParoles->hauteurDiff)
           positionBarreDefil.y = affichageParoles->hauteurZone - affichageParoles->hauteurBarreDefilement + affichageParoles->hauteurDiff;
        if (positionBarreDefil.y < affichageParoles->hauteurDiff)
           positionBarreDefil.y = affichageParoles->hauteurDiff;
        affichageParoles->positionBarreDefilement.y = positionBarreDefil.y;

        actualiser_paroles(affichageParoles, musique);
    }

    return;
}

void actualiser_paroles (AffichageParoles *affichageParoles, Musique *musique)
{
     char paroles[TAILLE_MAX_PAROLES] = "\0", *positionDebut, *positionFin;
     int i, j = 0;
     strcpy(paroles, musique->paroles);

     SDL_Color couleurBlanc = {255, 255, 255},
               couleurNoire = {0, 0, 0};

     SDL_Surface *surface = TTF_RenderText_Shaded(affichageParoles->police, "A", couleurBlanc, couleurNoire);
     int nombreAffichables = affichageParoles->hauteurZone / (surface->h + 5) - 3,
         nombreTotal = 0;
     SDL_FreeSurface(surface);

     renvoyer_texte(paroles, affichageParoles->largeurZone, affichageParoles->police);
     for (i = 0 ; paroles[i] != '\0' ; i++)
     {
         if (paroles[i] == '\n')
            nombreTotal++;
     }

     int nouvelleHauteur = (nombreAffichables / (nombreTotal / 1.0)) * affichageParoles->hauteurZone;
     if (affichageParoles->hauteurBarreDefilement != nouvelleHauteur)
     {
         affichageParoles->hauteurBarreDefilement = nouvelleHauteur;
         if (nombreTotal > nombreAffichables)
            AFF_recreer_barre_defil (affichageParoles);
         else
         {
            if (affichageParoles->surfaceBarreDefilement != NULL)
               SDL_FreeSurface(affichageParoles->surfaceBarreDefilement);
            affichageParoles->surfaceBarreDefilement = NULL;
         }
     }

     int ligneDebut = ((affichageParoles->positionBarreDefilement.y - affichageParoles->hauteurDiff) * (nombreTotal - nombreAffichables)) / ((affichageParoles->hauteurZone - affichageParoles->hauteurBarreDefilement) / 1.0),
         ligneFin = ligneDebut + nombreAffichables;

     for (i = 0 ; j < ligneDebut && paroles[i] != '\0' ; i++)
     {
         if (paroles[i] == '\n')
            j++;
     }
     if (paroles[i] == '\0')
        positionDebut = paroles;
     else positionDebut = paroles + i;

     j = 0;
     for (i = 0 ; j <= ligneFin && paroles[i] != '\0' ; i++)
     {
         if (paroles[i] == '\n')
            j++;
     }
     if (paroles[i] == '\0')
        positionFin = paroles + strlen(paroles);
     else positionFin = paroles + i;

     *positionFin = '\0';

     char texteAAfficher[TAILLE_MAX_PAROLES];
     sprintf(texteAAfficher, "Paroles de \"%s\"\n\n%s", musique->titre, positionDebut);

     SDL_Rect positionZero = {0};
     SDL_FillRect(affichageParoles->surfaceParoles, NULL, SDL_MapRGB(affichageParoles->surfaceParoles->format, 255, 0, 0));
     afficher_texte(texteAAfficher, affichageParoles->police, couleurBlanc, couleurNoire, 1, 5, affichageParoles->largeurZone, positionZero, affichageParoles->surfaceParoles);
     SDL_SetColorKey(affichageParoles->surfaceParoles, SDL_SRCCOLORKEY, SDL_MapRGB(affichageParoles->surfaceParoles->format, 255, 0, 0));
     return;
}


void AFF_recreer_barre_defil (AffichageParoles *affichageParoles)
{
     affichageParoles->positionBarreDefilement.y = affichageParoles->hauteurDiff;

     if (affichageParoles->surfaceBarreDefilement != NULL)
        SDL_FreeSurface(affichageParoles->surfaceBarreDefilement);
     affichageParoles->surfaceBarreDefilement = SDL_CreateRGBSurface(SDL_HWSURFACE, 26, affichageParoles->hauteurBarreDefilement, 32, 0, 0, 0, 0);

     SDL_Surface *rectangle = NULL;
     rectangle = SDL_CreateRGBSurface(SDL_HWSURFACE, 1, affichageParoles->hauteurBarreDefilement, 32, 0, 0, 0, 0);
     SDL_Rect position;
     position.x = 0;
     position.y = 0;

     int i;

     for (i = 0 ; i <= 12 ; i++)
     {
         SDL_FillRect(rectangle, NULL, SDL_MapRGB(affichageParoles->surfaceBarreDefilement->format, (255 * i) / 12, (255 * i) / 12, (255 * i) / 12));
         position.x = i;
         SDL_BlitSurface(rectangle, NULL, affichageParoles->surfaceBarreDefilement, &position);
     }
     for (i = 0 ; i <= 12 ; i++)
     {
         SDL_FillRect(rectangle, NULL, SDL_MapRGB(affichageParoles->surfaceBarreDefilement->format, 255 - (255 * i) / 12, 255 - (255 * i) / 12, 255 - (255 * i) / 12));
         position.x = i + 13;
         SDL_BlitSurface(rectangle, NULL, affichageParoles->surfaceBarreDefilement, &position);
     }

     SDL_FreeSurface(rectangle);
     return;
}
