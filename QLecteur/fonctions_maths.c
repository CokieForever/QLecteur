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

#include "fonctions_maths.h"


double frac(double nombre)
{
       double partieFrac = 0;
       int negatif;
       if (nombre < 0)
          negatif = 1;
       else
           negatif = 0;
       nombre = fabs(nombre);
       for (; nombre > 1 ; nombre--);
       partieFrac = nombre;
       if (negatif == 1)
          partieFrac = -partieFrac;
       return partieFrac;
}


int ent(double nombre)
{
    int partieEnt = nombre - frac(nombre);
    return partieEnt;
}


double arrondi(double nombre)
{
       double nombreArrondi = nombre;
       if (fabs(frac(nombre)) >= 0.5)
          nombreArrondi = (ent(nombre) + 1);
       else
           nombreArrondi = ent(nombre);
       return nombreArrondi;
}


double conversion(double euros)
{
    double francs = 0;

    francs = 6.55957 * euros;
    return francs;
}


int conversion_decimal_binaire(int nombreDecimal, int nombreBits, char chaineBinaire[])
{
    int nombreEnSortie = 0;
    int n = 0, init = 0;

    if (nombreBits < 0)
    {
                   for (n = 0 ; nombreDecimal / pow(2, n) >= 1 ; n++);
                   nombreBits = n;
    }

    if (chaineBinaire == NULL)
    {
       chaineBinaire = malloc(sizeof(char) * (nombreBits));
       init = 1;
    }

    for (n = 0 ; n < nombreBits ; n++)
    {
        chaineBinaire[n] = '0';
    }

    while(nombreDecimal > 0)
    {
                        for (n = 0 ; nombreDecimal / pow(2, n) >= 1 ; n++);
                        n--;
                        chaineBinaire[nombreBits - 1 - n] = '1';
                        nombreDecimal -= pow(2, n);
    }

    nombreEnSortie = strtol(chaineBinaire, NULL, 10);

    chaineBinaire[nombreBits] = '\0';

    if (init)
       free(chaineBinaire);

    return nombreEnSortie;
}
