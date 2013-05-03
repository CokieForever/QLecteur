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

#include "resize_image.h"



int RSZ_redimensionner_image(SDL_Rect dimensionFinale, SDL_Surface **surface)
{
//    fprintf(stdout, "Exécution de la fonction resize...\nParamètres en entrée :\nDimension entrée w : %d\nDimension entrée h : %d\nDimension sortie w : %d\nDimension sortie h : %d\n", (*surface)->w, (*surface)->h, dimensionFinale.w, dimensionFinale.h);

    if ((dimensionFinale.w / 1.0) / (*surface)->w > (dimensionFinale.h / 1.0) / (*surface)->h)
       dimensionFinale.w = dimensionFinale.h * (*surface)->w / ((*surface)->h / 1.0);
    else if ((dimensionFinale.w / 1.0) / (*surface)->w < (dimensionFinale.h / 1.0) / (*surface)->h)
         dimensionFinale.h = dimensionFinale.w * (*surface)->h / ((*surface)->w / 1.0);


    SDL_Surface *nouvelleSurface = NULL, *morceau = NULL;
    nouvelleSurface = SDL_CreateRGBSurface(SDL_HWSURFACE, dimensionFinale.w, (*surface)->h, 32, 0, 0, 0, 0);

    double i;
    int j;
    double facteurLigne = (*surface)->w / (dimensionFinale.w / 1.0), facteurColonne = (*surface)->h / (dimensionFinale.h / 1.0);
    if (facteurLigne < 1)
       facteurLigne = 1;
    if (facteurColonne < 1)
       facteurColonne = 1;

    SDL_Rect positionMorceau;
    positionMorceau.x = 0;
    positionMorceau.y = 0;

    int nombreCopies = 0;
    for (i = 1 ; nombreCopies < dimensionFinale.w ; i += facteurLigne)
    {
           j = i;
           morceau = RSZ_extraire_colonne(j - 1, *surface);
           positionMorceau.x = nombreCopies;
           SDL_BlitSurface(morceau, NULL, nouvelleSurface, &positionMorceau);
           nombreCopies++;
           SDL_FreeSurface(morceau);
    }


    nombreCopies = 0;
    positionMorceau.x = 0;
    SDL_FreeSurface(*surface);
    *surface = SDL_CreateRGBSurface(SDL_HWSURFACE, dimensionFinale.w, dimensionFinale.h, 32, 0, 0, 0, 0);

    for (i = 1 ; nombreCopies < dimensionFinale.h ; i += facteurLigne)
    {
           j = i;
           morceau = RSZ_extraire_ligne(j - 1, nouvelleSurface);
           positionMorceau.y = nombreCopies;
           SDL_BlitSurface(morceau, NULL, *surface, &positionMorceau);
           nombreCopies++;
           SDL_FreeSurface(morceau);
    }

    SDL_FreeSurface(nouvelleSurface);

//    fprintf(stdout, "Sortie de fonction resize :\nDimension sortie w : %d\nDimension sortie h : %d\nFacteur ligne : %lf\nFacteur colonne : %lf\n", (*surface)->w, (*surface)->h, facteurLigne, facteurColonne);


    return 1;
}



SDL_Surface* RSZ_extraire_ligne(int numeroLigne, SDL_Surface *surface)
{
             SDL_Surface *surfaceDeSortie = NULL;
             surfaceDeSortie = SDL_CreateRGBSurface(SDL_HWSURFACE, surface->w, 1, 32, 0, 0, 0, 0);

             SDL_Rect positionSurface;
             positionSurface.x = 0;
             positionSurface.y = numeroLigne;
             positionSurface.w = surface->w;
             positionSurface.h = 1;

             SDL_Rect positionZero;
             positionZero.x = 0;
             positionZero.y = 0;

             SDL_BlitSurface(surface, &positionSurface, surfaceDeSortie, &positionZero);

             return surfaceDeSortie;
}



SDL_Surface* RSZ_extraire_colonne(int numeroColonne, SDL_Surface *surface)
{
             SDL_Surface *surfaceDeSortie = NULL;
             surfaceDeSortie = SDL_CreateRGBSurface(SDL_HWSURFACE, 1, surface->h, 32, 0, 0, 0, 0);

             SDL_Rect positionSurface;
             positionSurface.x = numeroColonne;
             positionSurface.y = 0;
             positionSurface.w = 1;
             positionSurface.h = surface->h;

             SDL_Rect positionZero;
             positionZero.x = 0;
             positionZero.y = 0;

             SDL_BlitSurface(surface, &positionSurface, surfaceDeSortie, &positionZero);

             return surfaceDeSortie;
}


SDL_Surface* RSZ_extraire_pixel(SDL_Rect coordonneesPixel, SDL_Surface *surface)
{
             SDL_Surface *surfaceColonne = NULL;
             surfaceColonne = RSZ_extraire_colonne(coordonneesPixel.x, surface);

             SDL_Surface *surfaceDeSortie = NULL;
             surfaceDeSortie = RSZ_extraire_ligne(coordonneesPixel.y, surfaceColonne);

             SDL_FreeSurface(surfaceColonne);

             return surfaceDeSortie;
}


