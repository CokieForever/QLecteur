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

#include "fonctions_repertoire.h"



int lister_repertoire(char nomRepertoire[], char nomFichierDEnregistrement[])
{
    int nombreTrouves = 0;
    FILE *fichierDEnregistrement = NULL;
    fichierDEnregistrement = fopen(nomFichierDEnregistrement, "w");
    if (fichierDEnregistrement == NULL)
    {
       fprintf(stderr, "ERR lister_repertoires %d : Impossible d'ouvrir le fichier %s.\n", __LINE__, nomFichierDEnregistrement);
       return -1;
    }

    DIR *repertoireDIR = NULL;
    struct dirent *fichierTrouve;
    if ((repertoireDIR = opendir(nomRepertoire)) == NULL)
    {
        fprintf(stderr, "ERR lister_repertoires %d : Impossible d'ouvrir le repertoire %s.\n", __LINE__, nomRepertoire);
        return -1;
    }

    do
    {
        errno = 0;
        if ((fichierTrouve = readdir(repertoireDIR)) != NULL)
        {
            fprintf(fichierDEnregistrement, "%s\\%s\n", nomRepertoire, fichierTrouve->d_name);
            nombreTrouves++;
        }
    } while (fichierTrouve != NULL);


    if (errno != 0)
    {
        fprintf(stderr, "ERR lister_repertoires %d : Erreur de lecture du repertoire %s.\n", __LINE__, nomRepertoire);
        return -1;
    }

    closedir(repertoireDIR);
    while(fclose(fichierDEnregistrement) == EOF);
    return nombreTrouves;
}



int lister_extension(char nomRepertoire[], char nomFichierDEnregistrement[], char extensionALister2[], char modeDOuvertureFichier[])
{
    char *extensionALister = NULL;
    extensionALister = malloc(sizeof(char) * (strlen(extensionALister2) + 1));
    strcpy(extensionALister, extensionALister2);


    int i = 0;

    char *positionSeparateur = NULL;
    for (i = 0 ; (positionSeparateur = strstr(extensionALister, " ; ")) != NULL ; i++)
    {
        *(positionSeparateur + 1) = '#';
    }

    int nombreExtensions = i + 1;


    char **tableauExtensions = NULL;
    tableauExtensions = malloc(sizeof(char*) * nombreExtensions);
    for (i = 0 ; i < nombreExtensions ; i++)
    {
        tableauExtensions[i] = malloc(sizeof(char) * 52);
        positionSeparateur = strrchr(extensionALister, '#');
        if (positionSeparateur != NULL)
        {
                               strcpy(tableauExtensions[i], positionSeparateur + 2);
                               *(positionSeparateur - 1) = '\0';
        }
        else strcpy(tableauExtensions[i], extensionALister);
    }


    FILE *fichierDEnregistrement = NULL;
    fichierDEnregistrement = fopen(nomFichierDEnregistrement, modeDOuvertureFichier);
    if (fichierDEnregistrement == NULL)
    {
       fprintf(stderr, "ERR lister_extensions %d : Impossible d'ouvrir le fichier %s.\n", __LINE__, nomFichierDEnregistrement);
       return -1;
    }

    DIR *repertoireDIR = NULL;
    struct dirent *fichierTrouve;
    if ((repertoireDIR = opendir(nomRepertoire)) == NULL)
    {
        fprintf(stderr, "ERR lister_extensions %d : Impossible d'ouvrir le repertoire %s.\n", __LINE__, nomRepertoire);
        return -1;
    }

    int nombreTrouves = 0;


    do
    {
                errno = 0;
                if ((fichierTrouve = readdir(repertoireDIR)) != NULL)
                {
                    i = -1;
                    do
                    {
                        i++;
                        if (strstr(fichierTrouve->d_name, tableauExtensions[i]) != NULL)
                        {
                              fprintf(fichierDEnregistrement, "%s\\%s\n", nomRepertoire, fichierTrouve->d_name);
                              nombreTrouves++;
                        }
                    } while (i < nombreExtensions - 1 && strstr(fichierTrouve->d_name, tableauExtensions[i]) == NULL);
                }
    } while (fichierTrouve != NULL);


    if (errno != 0)
    {
                fprintf(stderr, "ERR lister_extensions %d : Erreur de lecture du repertoire %s.\n", __LINE__, nomRepertoire);
                return -1;
    }


    while(closedir(repertoireDIR) == -1);
    while(fclose(fichierDEnregistrement) == EOF);
    for (i = 0 ; i < nombreExtensions ; i++)
    {
          free(tableauExtensions[i]);
    }
    free(tableauExtensions);
    free(extensionALister);
    return nombreTrouves;
}


int test_exist(char nomFichier[])
{
    int i;

    FILE *fichier = NULL;
    for (i = 0 ; (fichier = fopen(nomFichier, "r")) == NULL && i <= 10 ; i++);
    if (fichier != NULL)
    {
       while(fclose(fichier) == EOF);
       return 1;
    }

    DIR *repertoire = NULL;
    for (i = 0 ; (repertoire = opendir(nomFichier)) == NULL && i <= 10 ; i++);
    if (repertoire != NULL)
    {
       closedir(repertoire);
       return 1;
    }

    return 0;
}


int lister_dossiers(char nomRepertoire[], char nomFichierDEnregistrement[], char modeDOuvertureFichier[])
{
    FILE *fichierDEnregistrement = NULL;
    fichierDEnregistrement = fopen(nomFichierDEnregistrement, modeDOuvertureFichier);
    if (fichierDEnregistrement == NULL)
    {
       fprintf(stderr, "ERR lister_dossiers %d : Impossible d'ouvrir le fichier %s.\n", __LINE__, nomFichierDEnregistrement);
       return -1;
    }

    DIR *repertoireDIR = NULL;
    struct dirent *fichierTrouve;
    if ((repertoireDIR = opendir(nomRepertoire)) == NULL)
    {
        fprintf(stderr, "ERR lister_dossiers %d : Impossible d'ouvrir le repertoire %s.\n", __LINE__, nomRepertoire);
        return -1;
    }

    int nombreTrouves = 0;


    if ((repertoireDIR = opendir(nomRepertoire)) == NULL)
    {
               fprintf(stderr, "ERR lister_dossiers %d : Impossible d'ouvrir le repertoire %s.\n", __LINE__, nomRepertoire);
               return -1;
    }

    do
    {
                errno = 0;
                if ((fichierTrouve = readdir(repertoireDIR)) != NULL)
                {
                    if (strchr(fichierTrouve->d_name, '.') == NULL)
                    {
                          fprintf(fichierDEnregistrement, "%s\\%s\n", nomRepertoire, fichierTrouve->d_name);
                          nombreTrouves++;
                    }
                }
    } while (fichierTrouve != NULL);


    if (errno != 0)
    {
                fprintf(stderr, "ERR lister_dossiers %d : Erreur de lecture du repertoire %s.\n", __LINE__, nomRepertoire);
                return -1;
    }


    closedir(repertoireDIR);
    while(fclose(fichierDEnregistrement) == EOF);
    return nombreTrouves;
}


int lister_disques(char nomFichierDEnregistrement[], char modeDOuverture[])
{
    FILE *fichierDEnregistrement = NULL;
    fichierDEnregistrement = fopen(nomFichierDEnregistrement, modeDOuverture);
    if (fichierDEnregistrement == NULL)
    {
       fprintf(stderr, "ERR fonction lister_disques %d : Impossible d'ouvrir le fichier %s.\n", __LINE__, nomFichierDEnregistrement);
       return -1;
    }

    DIR *disque = NULL;
    int numeroDisque = 'A';
    char nomDisque[5] = "A:\\";

    for (numeroDisque = 'A' ; numeroDisque <= 'Z' ; numeroDisque++)
    {
        nomDisque[0] = numeroDisque;
        if ((disque = opendir(nomDisque)) != NULL)
            fprintf(fichierDEnregistrement, "%s\n", nomDisque);
        closedir(disque);
    }

    while(fclose(fichierDEnregistrement) == EOF);

    return 1;
}



int effacer_adresse(char nomFichierListe[], int tailleMaxChaine)
{
    FILE *fichier = NULL;

    if ((fichier = fopen(nomFichierListe, "r")) == NULL)
    {
                 fprintf(stderr, "ERR effacer_adresse %d : Le fichier %s n'a pas pu être ouvert.\n", __LINE__, nomFichierListe);
                 return 0;
    }

    FILE *fichierTemp = NULL;
    if ((fichierTemp = fopen("$%tmp.txt", "w")) == NULL)
    {
                 fprintf(stderr, "ERR effacer_adresse %d : Le fichier $%%tmp.txt n'a pas pu être créé.\n", __LINE__);
                 return 0;
    }

    char *chaine = NULL;
    chaine = malloc(sizeof(char) * (tailleMaxChaine + 1));
    char *positionNom = NULL;

    while(fgets(chaine, tailleMaxChaine, fichier) != NULL)
    {
                        if ((positionNom = strrchr(chaine, '\\')) != NULL)
                           strcpy(chaine, positionNom + 1);
                        fprintf(fichierTemp, "%s", chaine);
    }

    while(fclose(fichier) == EOF);
    while(fclose(fichierTemp) == EOF);

    if ((fichier = fopen(nomFichierListe, "w")) == NULL)
    {
                 fprintf(stderr, "ERR effacer_adresse %d : Le fichier %s n'a pas pu être effacé.\n", __LINE__, nomFichierListe);
                 return 0;
    }

    if ((fichierTemp = fopen("$%tmp.txt", "r")) == NULL)
    {
                 fprintf(stderr, "ERR effacer_adresse %d : Le fichier $%%tmp.txt n'a pas pu être ouvert.\n", __LINE__);
                 return 0;
    }

    while(fgets(chaine, tailleMaxChaine, fichierTemp) != NULL)
                        fprintf(fichier, "%s", chaine);

    while(fclose(fichier) == EOF);
    while(fclose(fichierTemp) == EOF);
    remove("$%tmp.txt");

    return 1;
}



void convertir_en_relatif(char adresseAConvertir[], char adresseDeBase[])
{
     if (adresseAConvertir[0] != adresseDeBase[0])
        return;

     char nouvelleAdresse[5000] = {'\0'};
     char nouvelleAdresse2[5000] = {'\0'};

     if (strstr(adresseAConvertir, adresseDeBase) != NULL)
     {
        strcpy(adresseAConvertir, strlen(adresseDeBase) + adresseAConvertir + 1);
        return;
     }


     int i, j, out = 0, nombreSlashs = 0;
     char *positionDernierSlash = NULL;

     for (i = 1 ; adresseDeBase[i] != '\0' ; i++)
     {
         if (adresseDeBase[i] == '\\')
            nombreSlashs++;
     }


     for (i = 1 ; out == 0 ; i++)
     {
         if (adresseAConvertir[i] == '\\')
         {
            nombreSlashs--;
            positionDernierSlash = adresseAConvertir + i;
         }

         if (adresseAConvertir[i] != adresseDeBase[i] || adresseAConvertir[i] == '\0')
            out = 1;
     }


     for (j = 0 ; j <= nombreSlashs ; j++)
     {
         if (nouvelleAdresse[0] == '\0')
            strcpy(nouvelleAdresse, "..");
         else
             strcpy(nouvelleAdresse + strlen(nouvelleAdresse), "\\..");
     }


     if (adresseAConvertir[i] != '\0' && positionDernierSlash != NULL)
     {
        sprintf(nouvelleAdresse2, "%s%s", nouvelleAdresse, positionDernierSlash);
        strcpy(adresseAConvertir, nouvelleAdresse2);
     }
     else strcpy(adresseAConvertir, nouvelleAdresse);

     return;
}



int convertir_fichier_en_relatif(char nomFichier[], char adresseDeBase[])
{
    FILE *fichier = NULL;
    int tailleTableau = compter_lignes(nomFichier) + 1, i;

    char **tableau = NULL;
    tableau = malloc(sizeof(char*) * tailleTableau);

    if ((fichier = fopen(nomFichier, "r")) == NULL)
    {
                 fprintf(stderr, "ERR convertir_fichier_en_relatif %d : Impossible d'ouvrir le fichier %s.\n", __LINE__, nomFichier);
                 free(tableau);
                 return 0;
    }

    for (i = 0 ; i < tailleTableau ; i++)
    {
        tableau[i] = malloc(sizeof(char) * 1001);
        if (fgets(tableau[i], 1000, fichier) != NULL)
        {
            if (strrchr(tableau[i], '\n') != NULL)
               *(strrchr(tableau[i], '\n')) = '\0';
            convertir_en_relatif(tableau[i], adresseDeBase);
        }
    }

    while(fclose(fichier) == EOF);

    if ((fichier = fopen(nomFichier, "w+")) == NULL)
    {
                 fprintf(stderr, "ERR convertir_fichier_en_relatif %d : Impossible de rouvrir le fichier %s.\n", __LINE__, nomFichier);
                 for (i = 0 ; i < tailleTableau ; i++)
                 {
                    free(tableau[i]);
                 }
                 free(tableau);
                 return 0;
    }

    for (i = 0 ; i < tailleTableau ; i++)
    {
        if (tableau[i] != NULL)
           fprintf(fichier, "%s\n", tableau[i]);
    }

    while(fclose(fichier) == EOF);
    for (i = 0 ; i < tailleTableau ; i++)
    {
        free(tableau[i]);
    }
    free(tableau);

    return 1;
}
