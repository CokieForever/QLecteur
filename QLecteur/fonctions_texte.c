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

#include "fonctions_texte.h"


void afficher_texte(char texteAAfficherOld[], TTF_Font *policeTexte, SDL_Color couleurTexte, SDL_Color couleurFond, int typeRender, int ecartEntreLignes, int largeurMax, SDL_Rect positionTexte, SDL_Surface *surfaceBlit)
{
     SDL_Surface *surfaceTexteAAfficher = NULL;
     char texteAAfficher[5000] = " ";
     strcpy(texteAAfficher, texteAAfficherOld);
     char texteRestantAAfficher[5000] = " ";
     strcpy(texteRestantAAfficher, texteAAfficherOld);
     char *positionEntree = NULL;

     renvoyer_texte(texteAAfficher, largeurMax, policeTexte);

     do
     {
          if ((positionEntree = strchr(texteAAfficher, '\n')) != NULL)
          {
               *positionEntree = '\0';
               strcpy(texteRestantAAfficher, positionEntree + 1);
          }

          if (texteAAfficher[0] != '\0')
          {

              switch (typeRender)
              {
                     case 0:
                          surfaceTexteAAfficher = TTF_RenderText_Solid(policeTexte, texteAAfficher, couleurTexte);
                          break;
                     case 1:
                          surfaceTexteAAfficher = TTF_RenderText_Shaded(policeTexte, texteAAfficher, couleurTexte, couleurFond);
                          break;
                     default:
                             surfaceTexteAAfficher = TTF_RenderText_Blended(policeTexte, texteAAfficher, couleurTexte);
                             break;
              }

              SDL_BlitSurface(surfaceTexteAAfficher, NULL, surfaceBlit, &positionTexte);
              SDL_FreeSurface(surfaceTexteAAfficher);
          }
          positionTexte.y += (surfaceTexteAAfficher->h + ecartEntreLignes);

          strcpy(texteAAfficher, texteRestantAAfficher);

     } while (strchr(texteAAfficher, '\n') != NULL);


     switch (typeRender)
     {
            case 0:
                 surfaceTexteAAfficher = TTF_RenderText_Solid(policeTexte, texteAAfficher, couleurTexte);
                 break;
            case 1:
                 surfaceTexteAAfficher = TTF_RenderText_Shaded(policeTexte, texteAAfficher, couleurTexte, couleurFond);
                 break;
            default:
                      surfaceTexteAAfficher = TTF_RenderText_Blended(policeTexte, texteAAfficher, couleurTexte);
                      break;
     }

     SDL_BlitSurface(surfaceTexteAAfficher, NULL, surfaceBlit, &positionTexte);
     SDL_FreeSurface(surfaceTexteAAfficher);
}



int afficher_fichier_texte(char nomDuFichier[], TTF_Font *policeTexte, SDL_Color couleurTexte, SDL_Color couleurFond, int typeRender, int ecartEntreLignes, int largeurMax, int ligneDebut, int ligneFin, SDL_Rect positionTexte, SDL_Surface *surfaceBlit, SDL_Surface *tableauDeSurfaces[5000], SDL_Rect tableauDePositions[5000], char *tableauDeTextes[5000])
{
     int i = 0;
     char ligneAAfficher[200] = " ";
     char *positionEntree = NULL;

     FILE *fichierTexte = NULL;
     if ((fichierTexte = fopen(nomDuFichier, "r")) == NULL)
        {
                       fprintf(stderr, "ERR afficher_fichier_texte %d : Le fichier %s n'a pas pu etre ouvert.\n", __LINE__, nomDuFichier);
                       return 0;
        }

     for (i = 0 ; i < ligneDebut ; i++)
     {
         fgets(ligneAAfficher, 200, fichierTexte);
     }

     while (fgets(ligneAAfficher, 200, fichierTexte) != NULL && i < ligneFin)
     {
           if((positionEntree = strrchr(ligneAAfficher, '\n')) != NULL)
                              *positionEntree = '\0';
           if (tableauDeSurfaces[i] != NULL)
              SDL_FreeSurface(tableauDeSurfaces[i]);

           couper_texte(ligneAAfficher, largeurMax, policeTexte);

           switch (typeRender)
           {
                  case 0:
                       tableauDeSurfaces[i] = TTF_RenderText_Solid(policeTexte, ligneAAfficher, couleurTexte);
                       break;
                  case 1:
                       tableauDeSurfaces[i] = TTF_RenderText_Shaded(policeTexte, ligneAAfficher, couleurTexte, couleurFond);
                       break;
                  default:
                          tableauDeSurfaces[i] = TTF_RenderText_Blended(policeTexte, ligneAAfficher, couleurTexte);
                          break;
           }

           SDL_BlitSurface(tableauDeSurfaces[i], NULL, surfaceBlit, &positionTexte);

           tableauDePositions[i] = positionTexte;
           strcpy(tableauDeTextes[i], ligneAAfficher);

           positionTexte.y += (tableauDeSurfaces[i]->h + ecartEntreLignes);
           i++;
     }

     tableauDeSurfaces[i] = NULL;

     while(fclose(fichierTexte) == EOF);

     return 1;
}



void couper_texte(char texteACouper[], int largeurMax, TTF_Font *policeTexte)
{
     int i = 0;

     SDL_Color couleurNoire = {0, 0, 0};

     SDL_Surface *surfaceTexte = NULL;
     surfaceTexte = TTF_RenderText_Blended(policeTexte, texteACouper, couleurNoire);

     for (i = strlen(texteACouper) ; surfaceTexte->w > largeurMax && i >= 3 ; i--)
             {
                   if (i < 3)
                   {
                      texteACouper[0] = ' ';
                      texteACouper[1] = '\0';
                   }
                   else if (strlen(texteACouper) > i)
                   {
                      texteACouper[i - 3] = '.';
                      texteACouper[i - 2] = '.';
                      texteACouper[i - 1] = '.';
                      texteACouper[i] = '\0';
                   }
                   SDL_FreeSurface(surfaceTexte);
                   surfaceTexte = TTF_RenderText_Blended(policeTexte, texteACouper, couleurNoire);
             }

     SDL_FreeSurface(surfaceTexte);
}


void renvoyer_texte(char texteARenvoyer[], int largeurMax, TTF_Font *policeTexte)
{
     int i = 0;


     SDL_Color couleurNoire = {0, 0, 0};

     SDL_Surface *surfaceTexte = NULL;


     char chaine[5000] = "a";
     surfaceTexte = TTF_RenderText_Blended(policeTexte, chaine, couleurNoire);

     for (i = 0 ; surfaceTexte->w < largeurMax ; i++)
     {
         sprintf(chaine, "%sa", chaine);
         SDL_FreeSurface(surfaceTexte);
         surfaceTexte = TTF_RenderText_Blended(policeTexte, chaine, couleurNoire);
     }

     int nombreMaxLettres = i - 1;
     char *positionEspace = NULL;
     int j = 0;


     for (j = 0 ; j <= strlen(texteARenvoyer) ; j++)
     {
         if (texteARenvoyer[j] == ' ')
            positionEspace = texteARenvoyer + j;

         if (texteARenvoyer[j] == '\n')
         {
            i = nombreMaxLettres + j;
            positionEspace = NULL;
         }

         if (j > i)
         {
               if (positionEspace != NULL)
                  *positionEspace = '\n';
               i = nombreMaxLettres + positionEspace - texteARenvoyer;
               j = positionEspace - texteARenvoyer;
               positionEspace = NULL;
         }
     }

     SDL_FreeSurface(surfaceTexte);
}



int classer_alphabetique(char *tableauAClasser[], int tailleTableau, int tailleMaxChaine, char *deuxiemeTableau[])
{
    int i, j, k, out = 0;
    char *pointeurEchange = NULL;
    char *chaine1 = NULL;
    chaine1 = malloc((sizeof(char)) * tailleMaxChaine);

    char *chaine2 = NULL;
    chaine2 = malloc((sizeof(char)) * tailleMaxChaine);

    for (i = 0 ; i < tailleTableau - 1 ; i++)
    {
        for (j = i + 1 ; j < tailleTableau ; j++)
        {
            strcpy(chaine1, tableauAClasser[i]);
            strcpy(chaine2, tableauAClasser[j]);
            out = 0;
            for (k = 0 ; out == 0 ; k++)
            {
                chaine1[k] = toupper(chaine1[k]);
                chaine2[k] = toupper(chaine2[k]);
                if (chaine1[k] == 39)
                   chaine1[k] = ' ';
                if (chaine2[k] == 39)
                   chaine2[k] = ' ';
                if (chaine1[k] == '\0' || chaine1[k] == '\n')
                   out = 1;
                else if (chaine2[k] == '\0' || chaine2[k] == '\n')
                     out = 2;
                else
                {
                    if (chaine1[k] < chaine2[k])
                       out = 1;
                    else if (chaine2[k] < chaine1[k])
                         out = 2;
                }
            }
            if (out == 2)
            {
                    pointeurEchange = tableauAClasser[j];
                    tableauAClasser[j] = tableauAClasser[i];
                    tableauAClasser[i] = pointeurEchange;
                    if (deuxiemeTableau != NULL)
                    {
                                       pointeurEchange = deuxiemeTableau[j];
                                       deuxiemeTableau[j] = deuxiemeTableau[i];
                                       deuxiemeTableau[i] = pointeurEchange;
                    }
            }
        }
    }

    free(chaine1);
    free(chaine2);
    return 1;
}


int classer_alphabetique_fichier_texte(char nomFichier[], int tailleMaxChaine, char deuxiemeFichier[])
{
    FILE *fichier = NULL;
    if ((fichier = fopen(nomFichier, "r")) == NULL)
    {
                fprintf(stderr, "ERR classer_alphabetique_fichier_texte %d : mpossible d'ouvrir le fichier %s.\n", __LINE__, nomFichier);
                return 0;
    }


    FILE *fichier2 = NULL;
    if (deuxiemeFichier != NULL)
    {
                        if ((fichier2 = fopen(deuxiemeFichier, "r")) == NULL)
                        {
                           fprintf(stderr, "ERR classer_alphabetique_fichier_texte %d : Impossible d'ouvrir le fichier %s.\n", __LINE__, deuxiemeFichier);
                           return 0;
                        }
    }

    int i = 0;
    char *chaine = NULL;
    chaine = malloc(sizeof(char) * (tailleMaxChaine + 1));
    while(fgets(chaine, tailleMaxChaine, fichier) != NULL)
                        i++;

    int nombreLignes = i;
    char *positionEntree = NULL;

    char **tableau = NULL;
    tableau = malloc(sizeof(char*) * nombreLignes);
    for (i = 0 ; i < nombreLignes ; i++)
    {
        tableau[i] = malloc(sizeof(char) * (tailleMaxChaine + 1));
    }

    char **tableau2 = NULL;
    tableau2 = malloc(sizeof(char*) * nombreLignes);
    for (i = 0 ; i < nombreLignes ; i++)
    {
        tableau2[i] = malloc(sizeof(char) * (tailleMaxChaine + 1));
    }


    rewind(fichier);
    if (deuxiemeFichier != NULL)
       rewind(fichier2);
    i = 0;

    while(fgets(tableau[i], tailleMaxChaine, fichier) != NULL)
    {
                        if ((positionEntree = strchr(tableau[i], '\n')) != NULL)
                           *positionEntree = '\0';
                        i++;
    }

    if (deuxiemeFichier != NULL)
    {
                        i = 0;
                        while(fgets(tableau2[i], tailleMaxChaine, fichier2) != NULL)
                        {
                            if ((positionEntree = strchr(tableau2[i], '\n')) != NULL)
                               *positionEntree = '\0';
                            i++;
                        }
    }



    if (deuxiemeFichier == NULL)
    {
                       if (classer_alphabetique(tableau, nombreLignes, tailleMaxChaine, NULL) != 1)
                       return 0;
    }
    else
    {
                       if (classer_alphabetique(tableau, nombreLignes, tailleMaxChaine, tableau2) != 1)
                       return 0;
    }


    while(fclose(fichier) == EOF);
    if ((fichier = fopen(nomFichier, "w")) == NULL)
    {
                fprintf(stderr, "ERR classer_alphabetique_fichier_texte %d : Impossible d'ouvrir le fichier %s.\n", __LINE__, nomFichier);
                return 0;
    }

    for (i = 0 ; i < nombreLignes ; i++)
    {
        fprintf(fichier, "%s\n", tableau[i]);
    }


    if (deuxiemeFichier != NULL)
    {
                        while(fclose(fichier2) == EOF);
                        if ((fichier2 = fopen(deuxiemeFichier, "w")) == NULL)
                        {
                           fprintf(stderr, "ERR classer_alphabetique_fichier_texte %d : Impossible d'ouvrir le fichier %s.\n", __LINE__, deuxiemeFichier);
                           return 0;
                        }

                        for (i = 0 ; i < nombreLignes ; i++)
                        {
                            fprintf(fichier2, "%s\n", tableau2[i]);
                        }
    }

    free(chaine);

    for (i = 0 ; i < nombreLignes ; i++)
    {
        free(tableau[i]);
    }
    free(tableau);
    while(fclose(fichier) == EOF);

    for (i = 0 ; i < nombreLignes ; i++)
    {
        free(tableau2[i]);
    }
    free(tableau2);

    if (deuxiemeFichier != NULL)
       while(fclose(fichier2) == EOF);


    return 1;
}



int entrer_texte(char chaine[], int tailleMaxChaine, int longueurZone, SDL_Rect positionSurface, SDL_Surface *surfaceBlit, TTF_Font *police, SDL_Color couleurPP, SDL_Color couleurAP, char texteBase[], char type[])
{
    /* infos sur les types :
             la première lettre représente le type de texte : s = solid, h = shaded et b = blended
             la deuxième le type de donnée : u = universel, l = lettres uniquement, n = nombres uniquement et p = password (universel)
             la troisieme le type de boucle : i = interne et e = externe
    */

    //pré-effacement
    SDL_Surface *surface = NULL;
    switch (type[0])
    {
                     case 's':
                          surface = TTF_RenderText_Solid(police, "A", couleurPP);
                          break;
                     case 'h':
                          surface = TTF_RenderText_Shaded(police, "A", couleurPP, couleurAP);
                          break;
                     case 'b':
                          surface = TTF_RenderText_Blended(police, "A", couleurPP);
                          break;
    }
    int hauteurZone = surface->h;
    int nombreAffichables = longueurZone / surface->w;
    if (texteBase != NULL)
        nombreAffichables -= strlen(texteBase);

    SDL_FreeSurface(surface);
    surface = SDL_CreateRGBSurface(SDL_HWSURFACE, longueurZone, hauteurZone, 32, 0, 0, 0, 0);
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, couleurAP.r, couleurAP.g, couleurAP.b));

    SDL_BlitSurface(surface, NULL, surfaceBlit, &positionSurface);
    SDL_FreeSurface(surface);
    surface = NULL;


    SDL_Flip(surfaceBlit);
    SDL_EnableKeyRepeat(1000, 50);

    int continuer = 1, maj = 0, altgr = 0, i, debutChaine = 0;
    SDL_Event event;
    if (type[2] == 'e')
       chaine[0] = '\0';

    char *chaineAAfficher = NULL;
    chaineAAfficher = malloc(sizeof(char) * (tailleMaxChaine + 1));
    char *chaineCodee = NULL;
    chaineCodee = malloc(sizeof(char) * (tailleMaxChaine + 1));
    char *chaine2 = NULL;
    chaine2 = malloc(sizeof(char) * (tailleMaxChaine + 1));
    char *chaineAChariot = NULL;
    chaineAChariot = malloc(sizeof(char) * (tailleMaxChaine + 1));

    int numeroCase = 0, caractere = 0, alt = 0, combi = 0;
    for (numeroCase = 0 ; type[2] == 'i' && chaine[numeroCase] != '\0' ; numeroCase++);
    while (numeroCase - debutChaine > nombreAffichables && debutChaine < numeroCase)
          debutChaine++;

    if (texteBase != NULL)
    sprintf(chaineAAfficher, "%s%s", texteBase, chaine + debutChaine);
    else sprintf(chaineAAfficher, "%s", chaine + debutChaine);

    surface = SDL_CreateRGBSurface(SDL_HWSURFACE, longueurZone, hauteurZone, 32, 0, 0, 0, 0);
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, couleurAP.r, couleurAP.g, couleurAP.b));

    SDL_BlitSurface(surface, NULL, surfaceBlit, &positionSurface);
    SDL_FreeSurface(surface);
    surface = NULL;

    switch (type[0])
    {
                         case 's':
                              surface = TTF_RenderText_Solid(police, chaineAAfficher, couleurPP);
                              break;
                         case 'h':
                              surface = TTF_RenderText_Shaded(police, chaineAAfficher, couleurPP, couleurAP);
                              break;
                         case 'b':
                              surface = TTF_RenderText_Blended(police, chaineAAfficher, couleurPP);
                              break;
    }

    SDL_BlitSurface(surface, NULL, surfaceBlit, &positionSurface);
    SDL_FreeSurface(surface);
    surface = NULL;
    SDL_Flip(surfaceBlit);


    while (continuer)
    {
          event.type = -1;
          if (type[2] == 'e')
          {
             continuer = 0;
             SDL_PollEvent(&event);
          }
          else SDL_WaitEvent(&event);

          if (test_quit(event, &alt) == 0 && event.key.keysym.sym != SDLK_ESCAPE)
             return -1;
          else if (event.key.keysym.sym == SDLK_ESCAPE)
               return 0;

          if (event.key.keysym.sym == SDLK_LSHIFT && event.type == SDL_KEYUP)
              maj = 0;
          else if (event.key.keysym.sym == SDLK_RALT && event.type == SDL_KEYUP)
              altgr = 0;

          if (event.type == SDL_KEYDOWN)
          {
              switch(event.key.keysym.sym)
              {
                  case SDLK_LEFT:
                       if (numeroCase > 0)
                       {
                            numeroCase--;
                            while (numeroCase - debutChaine < nombreAffichables && debutChaine > 0)
                               debutChaine--;
                       }
                       break;
                  case SDLK_RIGHT:
                       if (chaine[numeroCase] != '\0')
                       {
                           numeroCase++;
                           if (numeroCase > tailleMaxChaine - 1)
                                 numeroCase = tailleMaxChaine - 1;
                           while (numeroCase - debutChaine > nombreAffichables && debutChaine < numeroCase)
                                    debutChaine++;
                       }
                       break;
                    default:
                        break;
              }

              caractere = convertir_entree_SDL(event, &maj, &altgr);

              if (caractere == '\n')
                 continuer = 0;
              else if (caractere != 0)
              {
                  if (caractere == '\b')
                  {
                     if (chaine[numeroCase] == '^' || chaine[numeroCase] == '¨' || chaine[numeroCase] == '`' || chaine[numeroCase] == '~')
                        numeroCase++;
                     if (numeroCase > 0)
                     {
                        strcpy(chaine2, chaine);
                        if (chaine[numeroCase] != '\0')
                             strcpy(chaine + numeroCase - 1, chaine2 + numeroCase);
                        else chaine[numeroCase - 1] = '\0';
                        numeroCase--;
                        while (numeroCase - debutChaine < nombreAffichables && debutChaine > 0)
                           debutChaine--;
                     }
                     else if (chaine[1] == '\0')
                          chaine[0] = '\0';
                  }
                  else if ((caractere == '^' || caractere == '¨' || caractere == '`' || caractere == '~') && toupper(type[1]) != 'N' && chaine[numeroCase] != '^' && chaine[numeroCase] != '¨' && chaine[numeroCase] != '`' && chaine[numeroCase] != '~')
                  {
                       strcpy(chaine2, chaine);
                       if (chaine[numeroCase] != '\0' && numeroCase < tailleMaxChaine)
                             strcpy(chaine + numeroCase + 1, chaine2 + numeroCase);
                       else if (numeroCase < tailleMaxChaine)
                               chaine[numeroCase + 1] = '\0';

                       chaine[numeroCase] = caractere;
                  }
                  else
                  {
                      if (toupper(type[1]) == 'N' && (caractere < '0' || caractere > '9') && (caractere != '-' || numeroCase != 0) && (caractere != '.' || numeroCase == 0));
                      else if (toupper(type[1]) == 'L' && (caractere >= '0' && caractere <= '9'));
                      else
                      {
                          combi = combiner_caractere(&caractere, chaine[numeroCase]);
                          if (combi == 0)
                             numeroCase++;
                          else if (combi == -1)
                          {
                              strcpy(chaine2, chaine);
                              if (chaine[numeroCase] != '\0' && numeroCase < tailleMaxChaine)
                                 strcpy(chaine + numeroCase + 1, chaine2 + numeroCase);
                              else if (numeroCase < tailleMaxChaine)
                                   chaine[numeroCase + 1] = '\0';
                          }

                          chaine[numeroCase] = caractere;
                          numeroCase++;
                          if (numeroCase > tailleMaxChaine - 1)
                                 numeroCase = tailleMaxChaine - 1;
                          while (numeroCase - debutChaine > nombreAffichables && debutChaine < numeroCase)
                                    debutChaine++;
                      }
                  }


                  if (toupper(type[1]) == 'P')
                  {

                              strcpy(chaineCodee, chaine);
                              for (i = 0 ; chaineCodee[i] != '\0' ; i++)
                              {
                                  chaineCodee[i] = '*';
                              }

                              if (texteBase != NULL)
                                 sprintf(chaineAAfficher, "%s%s", texteBase, chaineCodee + debutChaine);
                              else sprintf(chaineAAfficher, "%s", chaineCodee + debutChaine);
                  }
                  else
                  {
                      if (texteBase != NULL)
                         sprintf(chaineAAfficher, "%s%s", texteBase, chaine + debutChaine);
                      else sprintf(chaineAAfficher, "%s", chaine + debutChaine);
                  }
              }

              strcpy(chaineAChariot, chaineAAfficher);
              if (texteBase != NULL)
                         strcpy(chaineAChariot + numeroCase - debutChaine + strlen(texteBase), "_");
              else strcpy(chaineAChariot + numeroCase - debutChaine, "_");


              surface = SDL_CreateRGBSurface(SDL_HWSURFACE, longueurZone, hauteurZone, 32, 0, 0, 0, 0);
              SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, couleurAP.r, couleurAP.g, couleurAP.b));

              SDL_BlitSurface(surface, NULL, surfaceBlit, &positionSurface);
              SDL_FreeSurface(surface);
              surface = NULL;

              switch (type[0])
              {
                         case 's':
                              surface = TTF_RenderText_Solid(police, chaineAAfficher, couleurPP);
                              SDL_BlitSurface(surface, NULL, surfaceBlit, &positionSurface);
                              SDL_FreeSurface(surface);
                              surface = TTF_RenderText_Solid(police, chaineAChariot, couleurPP);
                              SDL_BlitSurface(surface, NULL, surfaceBlit, &positionSurface);
                              break;
                         case 'h':
                              surface = TTF_RenderText_Shaded(police, chaineAAfficher, couleurPP, couleurAP);
                              SDL_BlitSurface(surface, NULL, surfaceBlit, &positionSurface);
                              SDL_FreeSurface(surface);
                              surface = TTF_RenderText_Shaded(police, chaineAChariot, couleurPP, couleurAP);
                              SDL_BlitSurface(surface, NULL, surfaceBlit, &positionSurface);
                              break;
                         case 'b':
                              surface = TTF_RenderText_Blended(police, chaineAAfficher, couleurPP);
                              SDL_BlitSurface(surface, NULL, surfaceBlit, &positionSurface);
                              SDL_FreeSurface(surface);
                              surface = TTF_RenderText_Solid(police, chaineAChariot, couleurPP);
                              SDL_BlitSurface(surface, NULL, surfaceBlit, &positionSurface);
                              break;
              }

              SDL_FreeSurface(surface);
              surface = NULL;
              SDL_Flip(surfaceBlit);
          }
    }

    free(chaineAAfficher);
    return 1;
}



int convertir_entree_SDL(SDL_Event event, int *maj, int *altgr)
{
     //cette fonction a pour but de donner le code ASCII correct en fonction de event.key.keysym.sym
     //le clavier de base est un AZERTY

     SDLMod mod;
     mod = SDL_GetModState();
     int sortie = event.key.keysym.sym;

     if (event.type != SDL_KEYDOWN && event.type != SDL_KEYUP)
        return 0;

     if (sortie == SDLK_RALT && event.type == SDL_KEYDOWN)
     {
                *altgr = 1;
                return 0;
     }
     else if (sortie == SDLK_RALT && event.type == SDL_KEYUP)
     {
          *altgr = 0;
          return 0;
     }
     else if (sortie == SDLK_LSHIFT && event.type == SDL_KEYUP)
     {
          *maj = 0;
          return 0;
     }
     else if (sortie == SDLK_LSHIFT && event.type == SDL_KEYDOWN)
     {
          *maj = 1;
          return 0;
     }


     switch (sortie)
     {
                                case 13:
                                     sortie = '\n';
                                     break;
                                case 'a':
                                     sortie = 'q';
                                     break;
                                case 'q':
                                     sortie = 'a';
                                     break;
                                case 'z':
                                     sortie = 'w';
                                     break;
                                case 'w':
                                     sortie = 'z';
                                     break;
                                case 'm':
                                     sortie = ',';
                                     break;
                                case ',':
                                     sortie = ';';
                                     break;
                                case ';':
                                     sortie = 'm';
                                     break;
                                case '.':
                                     sortie = ':';
                                     break;
                                case '/':
                                     sortie = '!';
                                     break;
                                case '[':
                                     sortie = '^';
                                     break;
                                case ']':
                                     sortie = '$';
                                     break;
                                case 39:
                                     sortie = 'ù';
                                     break;
                                case '\\':
                                     sortie = '*';
                                     break;
                                case '-':
                                     sortie = ')';
                                     break;
     }

     if (*maj || (mod & KMOD_CAPS) == KMOD_CAPS)
     {
              switch (sortie)
              {
                     case '^':
                          sortie = '¨';
                          break;
                     case '$':
                          sortie = '£';
                          break;
                     case 'ù':
                          sortie = '%';
                          break;
                     case '*':
                          sortie = 'µ';
                          break;
                     case ',':
                          sortie = '?';
                          break;
                     case ';':
                          sortie = '.';
                          break;
                     case ':':
                          sortie = '/';
                          break;
                     case '!':
                          sortie = '§';
                          break;
                     case ')':
                          sortie = '°';
                          break;
                     case '=':
                          sortie = '+';
                          break;
                     case '<':
                          sortie = '>';
                          break;
              }
              sortie = toupper(sortie);
     }
     else if (*altgr == 0)
     {
         switch (sortie)
         {
                case '1':
                     sortie = '&';
                     break;
                case '2':
                     sortie = 'é';
                     break;
                case '3':
                     sortie = '"';
                     break;
                case '4':
                     sortie = 39;
                     break;
                case '5':
                     sortie = '(';
                     break;
                case '6':
                     sortie = '-';
                     break;
                case '7':
                     sortie = 'è';
                     break;
                case '8':
                     sortie = '_';
                     break;
                case '9':
                     sortie = 'ç';
                     break;
                case '0':
                     sortie = 'à';
                     break;
         }
     }
     else
     {
         switch (sortie)
         {
                case '1':
                     sortie = 0;
                     break;
                case '2':
                     sortie = '~';
                     break;
                case '3':
                     sortie = '#';
                     break;
                case '4':
                     sortie = '{';
                     break;
                case '5':
                     sortie = '[';
                     break;
                case '6':
                     sortie = '|';
                     break;
                case '7':
                     sortie = '`';
                     break;
                case '8':
                     sortie = '\\';
                     break;
                case '9':
                     sortie = '^';
                     break;
                case '0':
                     sortie = '@';
                     break;
                case ')':
                     sortie = ']';
                     break;
                case '=':
                     sortie = '}';
                     break;
                case '$':
                     sortie = '€';
                     break;
                case 'e':
                     sortie = '¤';
                     break;
         }
     }

     if (sortie >= 256 && sortie <= 265)
         sortie -= 208;

     switch (sortie)
     {
                   case SDLK_KP_PERIOD:
                        sortie = '.';
                        break;
                   case SDLK_KP_DIVIDE:
                        sortie = '/';
                        break;
                   case SDLK_KP_MULTIPLY:
                        sortie = '*';
                        break;
                   case SDLK_KP_MINUS:
                        sortie = '-';
                        break;
                   case SDLK_KP_PLUS:
                        sortie = '+';
                        break;
                   case SDLK_KP_ENTER:
                        sortie = '\n';
                        break;
                   case SDLK_KP_EQUALS:
                        sortie = '=';
                        break;
     }

     if (sortie > 127 || sortie < -127)
        sortie = 0;


     return sortie;
}



int combiner_caractere(int *caractere1, int caractere2)
{
                          //cette fonction combine un caractere1 avec un accent2 pour en tirer un autre caractère
                          //elle renvoie 1 si une combinaison a été effectuée, 0 si non et -1 si le caractere2 n'était pas un accent.

                          int sortie = 1;

                          if (caractere2 == '^')
                          {
                                         switch (*caractere1)
                                         {
                                                case 'a':
                                                     *caractere1 = 'â';
                                                     break;
                                                case 'e':
                                                     *caractere1 = 'ê';
                                                     break;
                                                case 'i':
                                                     *caractere1 = 'î';
                                                     break;
                                                case 'o':
                                                     *caractere1 = 'ô';
                                                     break;
                                                case 'u':
                                                     *caractere1 = 'û';
                                                     break;
                                                case 'A':
                                                     *caractere1 = 'Â';
                                                     break;
                                                case 'E':
                                                     *caractere1 = 'Ê';
                                                     break;
                                                case 'I':
                                                     *caractere1 = 'Î';
                                                     break;
                                                case 'O':
                                                     *caractere1 = 'Ô';
                                                     break;
                                                case 'U':
                                                     *caractere1 = 'Û';
                                                     break;
                                                default :
                                                        sortie = 0;
                                         }
                          }
                          else if (caractere2 == '¨')
                          {
                                         switch (*caractere1)
                                         {
                                                case 'a':
                                                     *caractere1 = 'ä';
                                                     break;
                                                case 'e':
                                                     *caractere1 = 'ë';
                                                     break;
                                                case 'i':
                                                     *caractere1 = 'ï';
                                                     break;
                                                case 'o':
                                                     *caractere1 = 'ö';
                                                     break;
                                                case 'u':
                                                     *caractere1 = 'ü';
                                                     break;
                                                case 'A':
                                                     *caractere1 = 'Ä';
                                                     break;
                                                case 'E':
                                                     *caractere1 = 'Ë';
                                                     break;
                                                case 'I':
                                                     *caractere1 = 'Ï';
                                                     break;
                                                case 'O':
                                                     *caractere1 = 'Ö';
                                                     break;
                                                case 'U':
                                                     *caractere1 = 'Ü';
                                                     break;
                                                default:
                                                        sortie = 0;
                                                        break;
                                         }
                          }
                          else if (caractere2 == '`')
                          {
                                         switch (*caractere1)
                                         {
                                                case 'a':
                                                     *caractere1 = 'à';
                                                     break;
                                                case 'e':
                                                     *caractere1 = 'è';
                                                     break;
                                                case 'i':
                                                     *caractere1 = 'ì';
                                                     break;
                                                case 'o':
                                                     *caractere1 = 'ò';
                                                     break;
                                                case 'u':
                                                     *caractere1 = 'ù';
                                                     break;
                                                case 'A':
                                                     *caractere1 = 'À';
                                                     break;
                                                case 'E':
                                                     *caractere1 = 'È';
                                                     break;
                                                case 'I':
                                                     *caractere1 = 'Ì';
                                                     break;
                                                case 'O':
                                                     *caractere1 = 'Ò';
                                                     break;
                                                case 'U':
                                                     *caractere1 = 'Ù';
                                                     break;
                                                default:
                                                        sortie = 0;
                                                        break;
                                         }
                          }
                          else if (caractere2 == '~')
                          {
                                         switch (*caractere1)
                                         {
                                                case 'a':
                                                     *caractere1 = 'ã';
                                                     break;
                                                case 'o':
                                                     *caractere1 = 'õ';
                                                     break;
                                                case 'A':
                                                     *caractere1 = 'Ã';
                                                     break;
                                                case 'O':
                                                     *caractere1 = 'Õ';
                                                     break;
                                                default:
                                                        sortie = 0;
                                                        break;
                                         }
                          }
                          else sortie = -1;

                          return sortie;
}
