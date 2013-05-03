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

#include "calculs_temps.h"


void diviser_temps(int millisecondes, int *centiemes, int *secondes, int *minutes, int *heures, int *jours, int *annees)
{
     if (annees != NULL)
     {
        *annees = millisecondes / (365.0 * 24 * 3600 * 1000);
        millisecondes = millisecondes % (long long int)(365.0 * 24 * 3600 * 1000);
     }

     if (jours != NULL)
     {
        *jours = millisecondes / (24 * 3600 * 1000);
        millisecondes = millisecondes % (24 * 3600 * 1000);
     }

     if (heures != NULL)
     {
        *heures = millisecondes / (3600 * 1000);
        millisecondes = millisecondes % (3600 * 1000);
     }

     if (minutes != NULL)
     {
        *minutes = millisecondes / (60 * 1000);
        millisecondes = millisecondes % (60 * 1000);
     }

     if (secondes != NULL)
     {
        *secondes = millisecondes / 1000;
        millisecondes = millisecondes % 1000;
     }

     if (centiemes != NULL)
        *centiemes = millisecondes / 10;
}


void diviser_temps_et_stocker(int millisecondes, int *centiemes, int *secondes, int *minutes, int *heures, char texteTemps[])
{
     diviser_temps(millisecondes, centiemes, secondes, minutes, heures, NULL, NULL);

     texteTemps[0] = '\0';
     if (heures != NULL)
     {
                if (*heures < 10)
                   sprintf(texteTemps, "0%d:", *heures);
                else
                    sprintf(texteTemps, "%d:", *heures);
     }

     if (minutes != NULL)
     {
                if (*minutes < 10)
                   sprintf(texteTemps, "%s0%d:", texteTemps, *minutes);
                else
                    sprintf(texteTemps, "%s%d:", texteTemps, *minutes);
     }

     if (secondes != NULL)
     {
                if (*secondes < 10)
                   sprintf(texteTemps, "%s0%d:", texteTemps, *secondes);
                else
                    sprintf(texteTemps, "%s%d:", texteTemps, *secondes);
     }

     if (centiemes != NULL)
     {
                if (*centiemes < 10)
                   sprintf(texteTemps, "%s0%d:", texteTemps, *centiemes);
                else
                    sprintf(texteTemps, "%s%d:", texteTemps, *centiemes);
     }


     *(strlen(texteTemps) + texteTemps - 1) = '\0';
}
