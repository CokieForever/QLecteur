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

#ifndef CONVERSIONS

#define CONVERSIONS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "main.h"


int convertir_wpl_en_list(char nomFichierAConvertir[], char nomFichierDeSortie[]);
int convertir_m3u_en_list(char nomFichierAConvertir[], char nomFichierDeSortie[]);
int convertir_en_unl(char nomFichierAConvertir[], char nomFichierDeSortie[]);

#endif
