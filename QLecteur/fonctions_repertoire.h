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

#ifndef FONCTIONS_REPERTOIRE

#define FONCTIONS_REPERTOIRE


#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include "defines.h"
#include "fonctions_fichier.h"


int lister_repertoire(char nomRepertoire[], char nomFichierDEnregistrement[]);
int lister_extension(char nomRepertoire[], char nomFichierDEnregistrement[], char extensionALister[], char modeDOuvertureFichier[]);
int test_exist(char nomFichier[]);
int lister_dossiers(char nomRepertoire[], char nomFichierDEnregistrement[], char modeDOuvertureFichier[]);
int effacer_adresse(char nomFichierListe[], int tailleMaxChaine);
int lister_disques(char nomFichierDEnregistrement[], char modeDOuverture[]);
void convertir_en_relatif(char adresseAConvertir[], char adresseDeBase[]);
int convertir_fichier_en_relatif(char nomFichier[], char adresseDeBase[]);


#endif
