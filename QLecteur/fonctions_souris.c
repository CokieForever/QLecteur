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

#include "fonctions_souris.h"


int cliquer_deplacer (SDL_Event event, SDL_Surface *surface, SDL_Rect *positionSurface, SDL_Rect *differenceAuClic, int *clicGauche)
{
     int deplacement = 0;
     switch (event.type)
     {
            case SDL_MOUSEBUTTONUP:
                 if (event.button.button == SDL_BUTTON_LEFT)
                    *clicGauche = 0;
                 break;
            case SDL_MOUSEBUTTONDOWN:
                 if ((*clicGauche != 1) && (event.button.button == SDL_BUTTON_LEFT) && ((event.button.x >= positionSurface->x) && (event.button.x - positionSurface->x <= surface->w)) && ((event.button.y >= positionSurface->y) && (event.button.y - positionSurface->y <= surface->h)))
                 {
                    *clicGauche = 1;
                    differenceAuClic->x = event.button.x - positionSurface->x;
                    differenceAuClic->y = event.button.y - positionSurface->y;
                 }
                 break;
            case SDL_MOUSEMOTION:
                 if (*clicGauche == 1)
                 {
                    positionSurface->x = event.button.x - differenceAuClic->x;
                    positionSurface->y = event.button.y - differenceAuClic->y;
                    deplacement = 1;
                 }
                 break;
     }
     return deplacement;
}


int test_button (SDL_Event event, SDL_Surface *ecran, SDL_Surface *bouton, SDL_Rect positionBouton, Uint32 couleurCadre, Uint32 couleurCadreAuClic, char texteBulleInfo[], int tempsDAttente, int *tempsDernierMvtSouris, int *clicGaucheSurBouton)
{
    int clicUpSurBouton = 0;

    if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT && ((event.button.x >= positionBouton.x) && (event.button.x - positionBouton.x <= bouton->w)) && ((event.button.y >= positionBouton.y) && (event.button.y - positionBouton.y <= bouton->h)))
    {
       clicUpSurBouton = 1;
       if (tempsDernierMvtSouris != NULL)
          *tempsDernierMvtSouris = SDL_GetTicks();
    }

    if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_RIGHT && ((event.button.x >= positionBouton.x) && (event.button.x - positionBouton.x <= bouton->w)) && ((event.button.y >= positionBouton.y) && (event.button.y - positionBouton.y <= bouton->h)))
    {
       clicUpSurBouton = 2;
       if (tempsDernierMvtSouris != NULL)
          *tempsDernierMvtSouris = SDL_GetTicks();
    }


    if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT && clicGaucheSurBouton != NULL)
       *clicGaucheSurBouton = 0;

    if (clicGaucheSurBouton != NULL && event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT && ((event.button.x >= positionBouton.x) && (event.button.x - positionBouton.x <= bouton->w)) && ((event.button.y >= positionBouton.y) && (event.button.y - positionBouton.y <= bouton->h)))
       *clicGaucheSurBouton = 1;

    if (event.type == SDL_MOUSEMOTION && tempsDernierMvtSouris != NULL)
       *tempsDernierMvtSouris = SDL_GetTicks();

    if ((couleurCadre != -1) && ((event.button.x >= positionBouton.x) && (event.button.x - positionBouton.x <= bouton->w)) && ((event.button.y >= positionBouton.y) && (event.button.y - positionBouton.y <= bouton->h)))
    {
       SDL_Surface *cadre = NULL;
       cadre = SDL_CreateRGBSurface(SDL_HWSURFACE, bouton->w + 2, bouton->h + 2, 32, 0, 0, 0, 0);
       SDL_FillRect(cadre, NULL, couleurCadre);

       SDL_Rect positionCadre;
       positionCadre.x = positionBouton.x - 1;
       positionCadre.y = positionBouton.y - 1;

       SDL_Surface *interieurCadre = NULL;
       interieurCadre = SDL_CreateRGBSurface(SDL_HWSURFACE, bouton->w, bouton->h, 32, 0, 0, 0, 0);
       Uint32 couleurInterieurCadre = SDL_MapRGB(ecran->format, 0, 0, 0);
       if (couleurInterieurCadre == couleurCadre)
          couleurInterieurCadre = SDL_MapRGB(ecran->format, 255, 255, 255);
       SDL_FillRect(interieurCadre, NULL, couleurInterieurCadre);

       SDL_Rect positionInterieurCadre;
       positionInterieurCadre.x = 1;
       positionInterieurCadre.y = 1;

       SDL_BlitSurface(interieurCadre, NULL, cadre, &positionInterieurCadre);
       SDL_SetColorKey(cadre, SDL_SRCCOLORKEY, couleurInterieurCadre);

       SDL_BlitSurface(cadre, NULL, ecran, &positionCadre);

       SDL_FreeSurface(interieurCadre);
       SDL_FreeSurface(cadre);
    }

    if (couleurCadreAuClic != -1 && clicGaucheSurBouton != NULL && *clicGaucheSurBouton)
    {
       SDL_Surface *cadre = NULL;
       cadre = SDL_CreateRGBSurface(SDL_HWSURFACE, bouton->w + 2, bouton->h + 2, 32, 0, 0, 0, 0);
       SDL_FillRect(cadre, NULL, couleurCadreAuClic);

       SDL_Rect positionCadre;
       positionCadre.x = positionBouton.x - 1;
       positionCadre.y = positionBouton.y - 1;

       SDL_Surface *interieurCadre = NULL;
       interieurCadre = SDL_CreateRGBSurface(SDL_HWSURFACE, bouton->w, bouton->h, 32, 0, 0, 0, 0);
       Uint32 couleurInterieurCadre = SDL_MapRGB(ecran->format, 0, 0, 0);
       if (couleurInterieurCadre == couleurCadreAuClic)
          couleurInterieurCadre = SDL_MapRGB(ecran->format, 255, 255, 255);
       SDL_FillRect(interieurCadre, NULL, couleurInterieurCadre);

       SDL_Rect positionInterieurCadre;
       positionInterieurCadre.x = 1;
       positionInterieurCadre.y = 1;

       SDL_BlitSurface(interieurCadre, NULL, cadre, &positionInterieurCadre);
       SDL_SetColorKey(cadre, SDL_SRCCOLORKEY, couleurInterieurCadre);

       SDL_BlitSurface(cadre, NULL, ecran, &positionCadre);

       SDL_FreeSurface(interieurCadre);
       SDL_FreeSurface(cadre);
    }

    if (tempsDernierMvtSouris != NULL && SDL_GetTicks() - *tempsDernierMvtSouris >= tempsDAttente && texteBulleInfo != NULL && ((event.button.x >= positionBouton.x) && (event.button.x - positionBouton.x <= bouton->w)) && ((event.button.y >= positionBouton.y) && (event.button.y - positionBouton.y <= bouton->h)))
    {
       SDL_Surface *surfaceBulleInfo = NULL;
       TTF_Font *policeBulleInfo = NULL;
       policeBulleInfo = TTF_OpenFont("fonts\\micross.ttf", 10);
       SDL_Color couleurNoire = {0, 0, 0};
       SDL_Color couleurBlanc = {255, 255, 255};
       surfaceBulleInfo = TTF_RenderText_Shaded(policeBulleInfo, texteBulleInfo, couleurNoire, couleurBlanc);
       SDL_Rect positionBulleInfo;
       positionBulleInfo.x = event.button.x;
       positionBulleInfo.y = event.button.y - surfaceBulleInfo->h;
       if (positionBulleInfo.x + surfaceBulleInfo->w > ecran->w)
          positionBulleInfo.x = ecran->w - surfaceBulleInfo->w;

       SDL_BlitSurface(surfaceBulleInfo, NULL, ecran, &positionBulleInfo);

       SDL_FreeSurface(surfaceBulleInfo);
       TTF_CloseFont(policeBulleInfo);
    }

    return clicUpSurBouton;
}



int test_button_rond (SDL_Event event, SDL_Surface *ecran, SDL_Surface *bouton, SDL_Rect positionBouton, Uint32 couleurCadre, Uint32 couleurCadreAuClic, char texteBulleInfo[], int tempsDAttente, int *tempsDernierMvtSouris, int *clicGaucheSurBouton)
{
    int clicGaucheUpSurBouton = 0;
    SDL_Rect centreBouton;
    centreBouton.x = positionBouton.x + (bouton->w / 2);
    centreBouton.y = positionBouton.y + (bouton->h / 2);
    int distanceAuCentre = sqrt(pow(event.button.x - centreBouton.x, 2) + pow(event.button.y - centreBouton.y, 2));

    if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT && distanceAuCentre <= bouton->w / 2)
    {
       clicGaucheUpSurBouton = 1;
       if (tempsDernierMvtSouris != NULL)
          *tempsDernierMvtSouris = SDL_GetTicks();
    }

    if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_RIGHT && distanceAuCentre <= bouton->w / 2)
    {
       clicGaucheUpSurBouton = 2;
       if (tempsDernierMvtSouris != NULL)
          *tempsDernierMvtSouris = SDL_GetTicks();
    }

    if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT && clicGaucheSurBouton != NULL)
       *clicGaucheSurBouton = 0;

    if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT && distanceAuCentre <= bouton->w / 2 && clicGaucheSurBouton != NULL)
       *clicGaucheSurBouton = 1;

    if (event.type == SDL_MOUSEMOTION && tempsDernierMvtSouris != NULL)
       *tempsDernierMvtSouris = SDL_GetTicks();

    if ((couleurCadre != -1) && (distanceAuCentre <= bouton->w / 2))
    {
       SDL_Surface *cadre = NULL;
       cadre = SDL_CreateRGBSurface(SDL_HWSURFACE, bouton->w + 2, bouton->h + 2, 32, 0, 0, 0, 0);
       SDL_FillRect(cadre, NULL, couleurCadre);

       SDL_Rect positionCadre;
       positionCadre.x = positionBouton.x - 1;
       positionCadre.y = positionBouton.y - 1;

       SDL_Surface *interieurCadre = NULL;
       interieurCadre = SDL_CreateRGBSurface(SDL_HWSURFACE, bouton->w, bouton->h, 32, 0, 0, 0, 0);
       Uint32 couleurInterieurCadre = SDL_MapRGB(ecran->format, 0, 0, 0);
       if (couleurInterieurCadre == couleurCadre)
          couleurInterieurCadre = SDL_MapRGB(ecran->format, 255, 255, 255);
       SDL_FillRect(interieurCadre, NULL, couleurInterieurCadre);

       SDL_Rect positionInterieurCadre;
       positionInterieurCadre.x = 1;
       positionInterieurCadre.y = 1;

       SDL_BlitSurface(interieurCadre, NULL, cadre, &positionInterieurCadre);
       SDL_SetColorKey(cadre, SDL_SRCCOLORKEY, couleurInterieurCadre);

       SDL_BlitSurface(cadre, NULL, ecran, &positionCadre);

       SDL_FreeSurface(interieurCadre);
       SDL_FreeSurface(cadre);
    }

    if (couleurCadreAuClic != -1 && clicGaucheSurBouton != NULL && *clicGaucheSurBouton)
    {
       SDL_Surface *cadre = NULL;
       cadre = SDL_CreateRGBSurface(SDL_HWSURFACE, bouton->w + 2, bouton->h + 2, 32, 0, 0, 0, 0);
       SDL_FillRect(cadre, NULL, couleurCadreAuClic);

       SDL_Rect positionCadre;
       positionCadre.x = positionBouton.x - 1;
       positionCadre.y = positionBouton.y - 1;

       SDL_Surface *interieurCadre = NULL;
       interieurCadre = SDL_CreateRGBSurface(SDL_HWSURFACE, bouton->w, bouton->h, 32, 0, 0, 0, 0);
       Uint32 couleurInterieurCadre = SDL_MapRGB(ecran->format, 0, 0, 0);
       if (couleurInterieurCadre == couleurCadreAuClic)
          couleurInterieurCadre = SDL_MapRGB(ecran->format, 255, 255, 255);
       SDL_FillRect(interieurCadre, NULL, couleurInterieurCadre);

       SDL_Rect positionInterieurCadre;
       positionInterieurCadre.x = 1;
       positionInterieurCadre.y = 1;

       SDL_BlitSurface(interieurCadre, NULL, cadre, &positionInterieurCadre);
       SDL_SetColorKey(cadre, SDL_SRCCOLORKEY, couleurInterieurCadre);

       SDL_BlitSurface(cadre, NULL, ecran, &positionCadre);

       SDL_FreeSurface(interieurCadre);
       SDL_FreeSurface(cadre);
    }

    if (tempsDernierMvtSouris != NULL && SDL_GetTicks() - *tempsDernierMvtSouris >= tempsDAttente && texteBulleInfo != NULL && distanceAuCentre <= bouton->w / 2)
    {
       SDL_Surface *surfaceBulleInfo = NULL;
       TTF_Font *policeBulleInfo = NULL;
       policeBulleInfo = TTF_OpenFont("fonts\\micross.ttf", 10);
       SDL_Color couleurNoire = {0, 0, 0};
       SDL_Color couleurBlanc = {255, 255, 255};
       surfaceBulleInfo = TTF_RenderText_Shaded(policeBulleInfo, texteBulleInfo, couleurNoire, couleurBlanc);
       SDL_Rect positionBulleInfo;
       positionBulleInfo.x = event.button.x;
       positionBulleInfo.y = event.button.y - surfaceBulleInfo->h;

       SDL_BlitSurface(surfaceBulleInfo, NULL, ecran, &positionBulleInfo);

       SDL_FreeSurface(surfaceBulleInfo);
       TTF_CloseFont(policeBulleInfo);
    }

    return clicGaucheUpSurBouton;
}



int test_position(SDL_Event event, SDL_Rect position, int largeur, int hauteur)
{
    if (event.button.x >= position.x && event.button.x <= position.x + largeur && event.button.y >= position.y && event.button.y <= position.y + hauteur)
       return 1;
    else return 0;
}


int test_position_rond(SDL_Event event, SDL_Rect positionCentre, int diametre)
{
    int distanceAuCentre = sqrt(pow(event.button.x - positionCentre.x, 2) + pow(event.button.y - positionCentre.y, 2));

    if (distanceAuCentre <= diametre / 2.0)
       return 1;
    else return 0;
}
