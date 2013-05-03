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

#ifndef FONCTIONS_FICHIER

#define FONCTIONS_FICHIER

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fonctions_maths.h"

void lire_caracteres(FILE *fichier, char chaine[], int nombreCaracteres);
int convertir_en_binaire(char nomFichierAConvertir[], char nomFichierDeSortie[], int nombreMax);
int effacer_fichier(char nomFichier[]);
int test_vide(char nomFichier[]);
int compter_lignes(char nomFichier[]);
int supprimer_ligne(char nomFichier[], int tailleMaxChaine, int numeroLigne);
int inserer_ligne(char nomFichier[], int tailleMaxChaine, int numeroLigne, char chaineAInserer[]);
int lire_ligne(char nomFichier[], int numeroLigne, char chaine[], int tailleMaxChaine);

#endif
