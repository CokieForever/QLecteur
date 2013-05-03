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

#include "conversions.h"


int convertir_wpl_en_list(char nomFichierAConvertir[], char nomFichierDeSortie[])
{
    char ligneLecture[TAILLE_MAX_NOM] = " ";

    FILE *fichierAConvertir = NULL;
    if ((fichierAConvertir = fopen(nomFichierAConvertir, "r")) == NULL)
    {
                      fprintf(stderr, "ERR Fonction convertir_wpl_en_list %d : Le fichier %s n'a pas pu etre ouvert.\n", __LINE__, nomFichierAConvertir);
                      return 0;
    }

    FILE *fichierDeSortie = NULL;
    if ((fichierDeSortie = fopen(nomFichierDeSortie, "w")) == NULL)
    {
                      fprintf(stderr, "ERR Fonction convertir_wpl_en_list %d : Le fichier %s n'a pas pu etre créé.\n", __LINE__, nomFichierDeSortie);
                      return 0;
    }

    char *positionExtension = NULL;
    char *positionGuillemet = NULL;
    char *positionGuillemetSimple = NULL;

   while (fgets(ligneLecture, TAILLE_MAX_NOM, fichierAConvertir) != NULL)
   {
         if ((positionExtension = strstr(ligneLecture, ".mp3")) != NULL || (positionExtension = strstr(ligneLecture, ".MP3")) != NULL
          || (positionExtension = strstr(ligneLecture, ".wma")) != NULL || (positionExtension = strstr(ligneLecture, ".WMA")) != NULL
          || (positionExtension = strstr(ligneLecture, ".wav")) != NULL || (positionExtension = strstr(ligneLecture, ".WAV")) != NULL
          || (positionExtension = strstr(ligneLecture, ".aac")) != NULL || (positionExtension = strstr(ligneLecture, ".AAC")) != NULL
          || (positionExtension = strstr(ligneLecture, ".ogg")) != NULL || (positionExtension = strstr(ligneLecture, ".OGG")) != NULL)
         {
                                *(positionExtension + 4) = '\0';
                                positionGuillemet = strchr(ligneLecture, '"');
                                strcpy(ligneLecture, positionGuillemet + 1);
                                while ((positionGuillemetSimple = strstr(ligneLecture, "&apos;")) != NULL)
                                {
                                                             *positionGuillemetSimple = 39;
                                                             strcpy(positionGuillemetSimple + 1, positionGuillemetSimple + 6);
                                }
                                fprintf(fichierDeSortie, "%s\n", ligneLecture);
         }
   }

   while(fclose(fichierDeSortie) == EOF);
   while(fclose(fichierAConvertir) == EOF);

   remove(nomFichierAConvertir);

   return 1;
}


int convertir_m3u_en_list(char nomFichierAConvertir[], char nomFichierDeSortie[])
{
    char ligneLecture[TAILLE_MAX_NOM] = " ";

    FILE *fichierAConvertir = NULL;
    if ((fichierAConvertir = fopen(nomFichierAConvertir, "r")) == NULL)
    {
                      fprintf(stderr, "ERR convertir_m3u_en_list %d : Le fichier %s n'a pas pu etre ouvert.\n", __LINE__, nomFichierAConvertir);
                      return 0;
    }

    FILE *fichierDeSortie = NULL;
    if ((fichierDeSortie = fopen(nomFichierDeSortie, "w")) == NULL)
    {
                      fprintf(stderr, "ERR convertir_m3u_en_list %d : Le fichier %s n'a pas pu etre ouvert.\n", __LINE__, nomFichierDeSortie);
                      return 0;
    }

    while (fgets(ligneLecture, TAILLE_MAX_NOM, fichierAConvertir) != NULL)
    {
         if ((strstr(ligneLecture, ".mp3") != NULL || strstr(ligneLecture, ".MP3") != NULL
          || strstr(ligneLecture, ".wma") != NULL || strstr(ligneLecture, ".WMA") != NULL
          || strstr(ligneLecture, ".wav") != NULL || strstr(ligneLecture, ".WAV") != NULL
          || strstr(ligneLecture, ".aac") != NULL || strstr(ligneLecture, ".AAC") != NULL
          || strstr(ligneLecture, ".ogg") != NULL || strstr(ligneLecture, ".OGG") != NULL)
          && strstr(ligneLecture, "#EXTINF:0,") == NULL)
         {
                                fprintf(fichierDeSortie, "%s", ligneLecture);
         }
    }

    while(fclose(fichierDeSortie) == EOF);
    while(fclose(fichierAConvertir) == EOF);

    remove(nomFichierAConvertir);

    return 1;
}


int convertir_en_unl(char nomFichierAConvertir[], char nomFichierDeSortie[])
{
     char *positionExtension = NULL;
     if ((positionExtension = strstr(nomFichierAConvertir, ".pls")) != NULL)
     {
            FILE *fichierAConvertir = NULL;
            if ((fichierAConvertir = fopen(nomFichierAConvertir, "r")) == NULL)
            {
               fprintf(stderr, "ERR convertir_en_unl %d : Impossible d'ouvrir le fichier %s.\n", __LINE__, nomFichierAConvertir);
               return 0;
            }

            FILE *fichierDeSortie = NULL;
            if ((fichierDeSortie = fopen(nomFichierDeSortie, "w")) == NULL)
            {
               fprintf(stderr, "ERR convertir_en_unl %d : Impossible d'ouvrir le fichier %s.\n", __LINE__, nomFichierAConvertir);
               return 0;
            }

            char *positionAdresse = NULL;
            char chaine[TAILLE_MAX_NOM] = " ";
            do
            {
                  fgets(chaine, TAILLE_MAX_NOM, fichierAConvertir);
            } while ((positionAdresse = strstr(chaine, "http")) == NULL);

            fprintf(fichierDeSortie, "%s", positionAdresse);
            while(fclose(fichierAConvertir) == EOF);
            while(fclose(fichierDeSortie) == EOF);
     }

     return 1;
}

