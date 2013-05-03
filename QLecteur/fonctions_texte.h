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

#ifndef FONCTIONS_TEXTE

#define FONCTIONS_TEXTE

#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>
#include <SDL_ttf.h>
#include <string.h>
#include "main.h"


void afficher_texte(char texteAAfficher[], TTF_Font *policeTexte, SDL_Color couleurTexte, SDL_Color couleurFond /* (shaded uniquement) */, int typeRender /* 0 = ??, 1 = Shaded et 2 = Blended */, int ecartEntreLignes, int largeurMax, SDL_Rect positionTexte, SDL_Surface *surfaceBlit);
int afficher_fichier_texte(char nomDuFichier[], TTF_Font *policeTexte, SDL_Color couleurTexte, SDL_Color couleurFond /* (shaded uniquement) */, int typeRender /* 0 = Solid, 1 = Shaded et 2 = Blended */, int ecartEntreLignes, int largeurMax, int ligneDebut, int ligneFin, SDL_Rect positionTexte, SDL_Surface *surfaceBlit, SDL_Surface *tableauDeSurfaces[5000], SDL_Rect tableauDePositions[5000], char *tableauDeTextes[5000]);
void couper_texte(char texteACouper[], int largeurMax, TTF_Font *policeTexte);
int classer_alphabetique(char *tableauAClasser[], int tailleTableau, int tailleMaxChaine, char *deuxiemeTableau[]);
int classer_alphabetique_fichier_texte(char nomFichier[], int tailleMaxChaine, char deuxiemeFichier[]);
void renvoyer_texte(char texteARenvoyer[], int largeurMax, TTF_Font *policeTexte);
int entrer_texte(char chaine[], int tailleMaxChaine, int longueurZone, SDL_Rect positionSurface, SDL_Surface *surfaceBlit, TTF_Font *police, SDL_Color couleurPP, SDL_Color couleurAP, char texteBase[], char type[]);
int convertir_entree_SDL(SDL_Event event, int *maj, int *altgr);
int combiner_caractere(int *caractere1, int caractere2);

#endif
