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

#ifndef CALCULS_TEMPS

#define CALCULS_TEMPS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


void diviser_temps(int millisecondes, int *centiemes, int *secondes, int *minutes, int *heures, int *jours, int *annees);
void diviser_temps_et_stocker(int millisecondes, int *centiemes, int *secondes, int *minutes, int *heures, char texteTemps[]);

#endif
