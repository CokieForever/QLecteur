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

#ifndef RESIZE_IMAGE

#define RESIZE_IMAGE

#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>


int RSZ_redimensionner_image(SDL_Rect dimensionFinale, SDL_Surface **surface);
SDL_Surface* RSZ_extraire_ligne(int numeroLigne, SDL_Surface *surface);
SDL_Surface* RSZ_extraire_colonne(int numeroColonne, SDL_Surface *surface);
SDL_Surface* RSZ_extraire_pixel(SDL_Rect coordonneesPixel, SDL_Surface *surface);


#endif
