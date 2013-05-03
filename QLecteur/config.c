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

#include "config.h"


void CFG_configurer_lecteur(MenuConfigLecteur *configMenu, SDL_Event event)
{
     SDL_Rect positionZero, positionAbsolue;
     positionZero.x = 0;
     positionZero.y = 0;
     SDL_Surface *surface = NULL;

     SDL_BlitSurface(configMenu->surfaceSauvegarde, NULL, configMenu->surfaceBlit, &positionZero);

     int i, out = 0;
     char chaine[TAILLE_MAX_NOM] = " ";

     SDL_Color couleurRougeFonce = {120, 0, 0}, couleurJaune = {255, 255, 0};
     TTF_Font *police = TTF_OpenFont("fonts\\sdigit.ttf", 15);

     event.button.x -= configMenu->positionBlit.x;
     event.button.y -= configMenu->positionBlit.y;

     for (i = 0 ; i <= 5 && out == 0 ; i++)
     {
         if (test_button(event, configMenu->surfaceBlit, configMenu->tableauSurfaces[i], configMenu->tableauPositions[i], SDL_MapRGB(configMenu->surfaceBlit->format, 255, 0, 0), -1, NULL, 0, NULL, NULL))
         {
                                if (configMenu->tableauTextesReponses[i][0] == 'o')
                                   strcpy(configMenu->tableauTextesReponses[i], "non");
                                else strcpy(configMenu->tableauTextesReponses[i], "oui");
                                out = 1;
         }
     }

     for (i = 6 ; i <= 11 && out == 0 ; i++)
     {
         if (test_button(event, configMenu->surfaceBlit, configMenu->tableauSurfaces[i], configMenu->tableauPositions[i], SDL_MapRGB(configMenu->surfaceBlit->format, 255, 0, 0), -1, NULL, 0, NULL, NULL))
         {
                             strcpy(chaine, configMenu->tableauTextesReponses[i]);
                             positionAbsolue.x = configMenu->tableauPositionsReponses[i].x + configMenu->positionBlit.x;
                             positionAbsolue.y = configMenu->tableauPositionsReponses[i].y + configMenu->positionBlit.y;
                             if (entrer_texte(chaine, 30, configMenu->surfaceBlit->w - configMenu->tableauPositionsReponses[i].x, positionAbsolue, configMenu->ecran, police, couleurJaune, couleurRougeFonce, NULL, "bni"))
                             {
                                if (chaine[0] == '-')
                                   strcpy(chaine, chaine + 1);
                                if (strchr(chaine, '.') != NULL)
                                   *(strchr(chaine, '.')) = '\0';
                                if (chaine[0] != '\0')
                                   strcpy(configMenu->tableauTextesReponses[i], chaine);
                             }
                             out = 1;
         }
     }

     for (i = 12 ; i <= 13 && out == 0 ; i++)
     {
         if (test_button(event, configMenu->surfaceBlit, configMenu->tableauSurfaces[i], configMenu->tableauPositions[i], SDL_MapRGB(configMenu->surfaceBlit->format, 255, 0, 0), -1, NULL, 0, NULL, NULL))
         {
                             strcpy(chaine, configMenu->tableauTextesReponses[i]);
                             if (strcmp(chaine, "(none)") == 0)
                                   chaine[0] = '\0';
                             if (strstr(chaine, " (inexistant)") != NULL)
                                *(strstr(chaine, " (inexistant)")) = '\0';
                             positionAbsolue.x = configMenu->tableauPositionsReponses[i].x + configMenu->positionBlit.x;
                             positionAbsolue.y = configMenu->tableauPositionsReponses[i].y + configMenu->positionBlit.y;
                             if (entrer_texte(chaine, TAILLE_MAX_NOM, configMenu->surfaceBlit->w - configMenu->tableauPositionsReponses[i].x, positionAbsolue, configMenu->ecran, police, couleurJaune, couleurRougeFonce, NULL, "bui"))
                             {
                                if (chaine[0] == '\0')
                                   strcpy(chaine, "(none)");
                                else if (strcmp(chaine, "(none)") != 0 && (surface = IMG_Load(chaine)) == NULL)
                                   strcat(chaine, " (inexistant)");
                                strcpy(configMenu->tableauTextesReponses[i], chaine);
                             }
                             out = 1;
         }
     }


     if (out == 0 && test_button(event, configMenu->surfaceBlit, configMenu->tableauSurfaces[15], configMenu->tableauPositions[15], SDL_MapRGB(configMenu->surfaceBlit->format, 0, 0, 255), -1, NULL, 0, NULL, NULL))
     {
        CFG_RAZ_config(configMenu);
        out = 1;
     }
     else if (out == 0 && test_button(event, configMenu->surfaceBlit, configMenu->tableauSurfaces[16], configMenu->tableauPositions[16], SDL_MapRGB(configMenu->surfaceBlit->format, 0, 0, 255), -1, NULL, 0, NULL, NULL))
     {
        char command[TAILLE_MAX_NOM], adresseFichierIcone[TAILLE_MAX_NOM];
        sprintf(command, "\"%s\\%s\" \"%%0\"", dossierBase, nomBase);
        sprintf(adresseFichierIcone, "%s\\img\\icons\\icone_list.ico", nomBase);

        assoc_extension(".list", "playlist.qlecteur\nPlaylist QLecteur", adresseFichierIcone, command);
        out = 1;
     }
     else if (out == 0 && test_button(event, configMenu->surfaceBlit, configMenu->tableauSurfaces[17], configMenu->tableauPositions[17], SDL_MapRGB(configMenu->surfaceBlit->format, 0, 0, 255), -1, NULL, 0, NULL, NULL))
     {
        char command[TAILLE_MAX_NOM], adresseFichierIcone[TAILLE_MAX_NOM];
        sprintf(command, "\"%s\\%s\" \"%%0\"", dossierBase, nomBase);
        sprintf(adresseFichierIcone, "%s\\img\\icons\\icone_reduite.ico", nomBase);

        assoc_extension(".mp3", "file.mp3\nMusique MP3", adresseFichierIcone, command);
        assoc_extension(".wma", "file.wma\nWindows Media Audio", adresseFichierIcone, command);
        assoc_extension(".wav", "file.wav\nSon Wave", adresseFichierIcone, command);
        assoc_extension(".mp3", "file.aac\nMusique AAC", adresseFichierIcone, command);
        assoc_extension(".mp3", "file.ogg\nMusique OGG", adresseFichierIcone, command);
        out = 1;
     }


     if (out)
     {
             CFG_associer_parametres(configMenu);
             CFG_recreer_surface(configMenu);
     }

     if (surface != NULL)
        SDL_FreeSurface(surface);

}


void CFG_init_config(MenuConfigLecteur *configMenu, SDL_Surface *ecran, SDL_Rect positionBlit, int largeur, int hauteur)
{
     //initialise toutes les sous-variables de configMenu pour un première utilisation et créé la surface de sauvegarde


     configMenu->ecran = ecran;
     configMenu->positionBlit = positionBlit;

     int i;
     for (i = 0 ; i < 100 ; i++)
     {
         configMenu->tableauTextes[i] = malloc(sizeof(char) * TAILLE_MAX_NOM );
         configMenu->tableauTextesReponses[i] = malloc(sizeof(char) * TAILLE_MAX_NOM );
         configMenu->tableauSurfaces[i] = NULL;
     }

     CFG_RAZ_config(configMenu);
     strcpy(configMenu->tableauTextes[0], "Sauvegarder les paramètres en fermeture :");
     strcpy(configMenu->tableauTextes[1], "Utiliser le chargement MPEG-Accurate :");
     strcpy(configMenu->tableauTextes[2], "Utiliser le chargement Non-blocking :");
     strcpy(configMenu->tableauTextes[3], "Utiliser le mode basse consommation :");
     strcpy(configMenu->tableauTextes[4], "Communiquer le spectre à Osmoled :");
     strcpy(configMenu->tableauTextes[5], "Enregistrer les playlists en adresse relative :");
     strcpy(configMenu->tableauTextes[6], "Délai maximum de chargement (ms) :");
     strcpy(configMenu->tableauTextes[7], "Durée maximum d'un enregistrement (sec) :");
     strcpy(configMenu->tableauTextes[8], "Nombre maximum de chargements :");
     strcpy(configMenu->tableauTextes[9], "Temps de défilement (ms) :");
     strcpy(configMenu->tableauTextes[10], "Temps de rafraichissement (ms) :");
     strcpy(configMenu->tableauTextes[11], "Temps de transition (ms) :");
     strcpy(configMenu->tableauTextes[12], "Dossier de diaporama :");
     strcpy(configMenu->tableauTextes[13], "Image de fond :");
     strcpy(configMenu->tableauTextes[14], "(none)");
     strcpy(configMenu->tableauTextes[15], "Rétablir par défaut");
     strcpy(configMenu->tableauTextes[16], "Associer l'extension .list au logiciel");
     strcpy(configMenu->tableauTextes[17], "Associer les fichiers musicaux au logiciel");
     strcpy(configMenu->tableauTextes[18], "(none)");


     configMenu->surfaceSauvegarde = SDL_CreateRGBSurface(SDL_HWSURFACE, largeur, hauteur, 32, 0, 0, 0, 0);
     SDL_FillRect(configMenu->surfaceSauvegarde, NULL, SDL_MapRGB(configMenu->surfaceSauvegarde->format, 0, 0, 0));

     configMenu->imageDeFond = SDL_CreateRGBSurface (SDL_HWSURFACE, largeur, hauteur, 32, 0, 0, 0, 0);
     SDL_FillRect(configMenu->imageDeFond, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
     configMenu->positionFond = positionBlit;


     CFG_recreer_surface(configMenu);

     configMenu->surfaceBlit = SDL_CreateRGBSurface(SDL_HWSURFACE, largeur, hauteur, 32, 0, 0, 0, 0);
     SDL_Rect positionZero;
     positionZero.x = 0;
     positionZero.y = 0;
     SDL_BlitSurface(configMenu->surfaceSauvegarde, NULL, configMenu->surfaceBlit, &positionZero);
}



void CFG_RAZ_config(MenuConfigLecteur *configMenu)
{
     //remet à zéro tous les parametres de réglage de configMenu avec les valeurs par défaut

     strcpy(configMenu->tableauTextesReponses[0], "oui");
     strcpy(configMenu->tableauTextesReponses[1], "oui");
     strcpy(configMenu->tableauTextesReponses[2], "oui");
     strcpy(configMenu->tableauTextesReponses[3], "oui");
     strcpy(configMenu->tableauTextesReponses[4], "non");
     strcpy(configMenu->tableauTextesReponses[5], "oui");
     strcpy(configMenu->tableauTextesReponses[6], "7500");
     strcpy(configMenu->tableauTextesReponses[7], "300");
     strcpy(configMenu->tableauTextesReponses[8], "10");
     strcpy(configMenu->tableauTextesReponses[9], "7500");
     strcpy(configMenu->tableauTextesReponses[10], "35");
     strcpy(configMenu->tableauTextesReponses[11], "1000");
     strcpy(configMenu->tableauTextesReponses[12], "(none)");
     strcpy(configMenu->tableauTextesReponses[13], "img\\other\\icone.bmp");

     CFG_associer_parametres(configMenu);
}


void CFG_recreer_surface(MenuConfigLecteur *configMenu)
{
     //recréée la surface d'enregistrement de configMenu.
     //Toutes les surfaces, positions et paramètres doivent avoir été initialisés au préalable.

     SDL_FillRect(configMenu->surfaceSauvegarde, NULL, SDL_MapRGB(configMenu->surfaceSauvegarde->format, 0, 0, 0));

     TTF_Font *police = TTF_OpenFont("fonts\\sdigit.ttf", 15);
     SDL_Color couleurBlanc = {255, 255, 255};
     SDL_Color couleurJaune = {255, 255, 0};
     SDL_Rect positionFond;
     positionFond.x = 0;
     positionFond.y = 0;
     SDL_Surface *surface = NULL;


     SDL_FillRect(configMenu->imageDeFond, NULL, SDL_MapRGB(configMenu->imageDeFond->format, 0, 0, 0));

     surface = IMG_Load(configMenu->parametre.imageDeFond);
     if (surface != NULL)
     {
           if (configMenu->imageDeFond->w < surface->w || configMenu->imageDeFond->h < surface->h)
           {
              SDL_Rect nouvelleDimension;
              nouvelleDimension.w = configMenu->imageDeFond->w;
              nouvelleDimension.h = configMenu->imageDeFond->h;
              RSZ_redimensionner_image(nouvelleDimension, &surface);
           }
           positionFond.x = (configMenu->imageDeFond->w - surface->w) / 2;
           positionFond.y = (configMenu->imageDeFond->h - surface->h) / 2;
           SDL_BlitSurface(surface, NULL, configMenu->imageDeFond, &positionFond);
           SDL_FreeSurface(surface);
     }



     configMenu->tableauPositions[0].y = 10;


     int i;
     for (i = 0 ; strcmp(configMenu->tableauTextes[i], "(none)") != 0 && i < 100 ; i++)
     {
         configMenu->tableauPositions[i].x = 5;
         if (i > 0)
            configMenu->tableauPositions[i].y = configMenu->tableauPositions[i - 1].y + configMenu->tableauSurfaces[i - 1]->h + 5;

         if (configMenu->tableauSurfaces[i] != NULL)
            SDL_FreeSurface(configMenu->tableauSurfaces[i]);

         //renvoyer_texte(configMenu->tableauTextes[i], configMenu->surfaceSauvegarde->w, police);
         configMenu->tableauSurfaces[i] = TTF_RenderText_Blended(police, configMenu->tableauTextes[i], couleurBlanc);
         SDL_BlitSurface(configMenu->tableauSurfaces[i], NULL, configMenu->surfaceSauvegarde, &configMenu->tableauPositions[i]);

         if (configMenu->tableauSurfacesReponses[i] != NULL)
            SDL_FreeSurface(configMenu->tableauSurfacesReponses[i]);

         configMenu->tableauSurfacesReponses[i] = TTF_RenderText_Blended(police, configMenu->tableauTextesReponses[i], couleurJaune);

         configMenu->tableauPositionsReponses[i].x = 5 + configMenu->tableauSurfaces[i]->w + 10;
         configMenu->tableauPositionsReponses[i].y = configMenu->tableauPositions[i].y + configMenu->tableauSurfaces[i]->h - configMenu->tableauSurfacesReponses[i]->h;

         SDL_BlitSurface(configMenu->tableauSurfacesReponses[i], NULL, configMenu->surfaceSauvegarde, &configMenu->tableauPositionsReponses[i]);
     }


     SDL_Color couleurVert = {0, 255, 0};
     configMenu->tableauPositions[i].y = configMenu->tableauPositions[i - 1].y + configMenu->tableauSurfaces[i - 1]->h + 5;
     configMenu->tableauSurfaces[i] = configMenu->tableauSurfaces[i - 1];
     configMenu->tableauPositions[i].x = 5;

     for (i = i + 1 ; strcmp(configMenu->tableauTextes[i], "(none)") != 0 && i < 100 ; i++)
     {
         if (configMenu->tableauSurfaces[i] != NULL)
                SDL_FreeSurface(configMenu->tableauSurfaces[i]);
         configMenu->tableauPositions[i].y = configMenu->tableauPositions[i - 1].y + configMenu->tableauSurfaces[i - 1]->h + 5;
         configMenu->tableauSurfaces[i] = TTF_RenderText_Blended(police, configMenu->tableauTextes[i], couleurVert);
         SDL_BlitSurface(configMenu->tableauSurfaces[i], NULL, configMenu->surfaceSauvegarde, &configMenu->tableauPositions[i]);
     }

}


void CFG_associer_parametres(MenuConfigLecteur *configMenu)
{
     if (configMenu->tableauTextesReponses[0][0] == 'o')
        configMenu->parametre.sauvegarde = 1;
     else configMenu->parametre.sauvegarde = 0;

     if (configMenu->tableauTextesReponses[1][0] == 'o')
        configMenu->parametre.chargementAccurate = 1;
     else configMenu->parametre.chargementAccurate = 0;

     if (configMenu->tableauTextesReponses[2][0] == 'o')
        configMenu->parametre.chargementNonBlocking = 1;
     else configMenu->parametre.chargementNonBlocking = 0;

     if (configMenu->tableauTextesReponses[3][0] == 'o')
        configMenu->parametre.modeReduit = 1;
     else configMenu->parametre.modeReduit = 0;

     if (configMenu->tableauTextesReponses[4][0] == 'o')
        configMenu->parametre.osmoCom = 1;
     else configMenu->parametre.osmoCom = 0;

     if (configMenu->tableauTextesReponses[5][0] == 'o')
        configMenu->parametre.sauvegardeRelative = 1;
     else configMenu->parametre.sauvegardeRelative = 0;


     configMenu->parametre.dureeMaxChargement = strtol(configMenu->tableauTextesReponses[6], NULL, 10);
     if (configMenu->parametre.dureeMaxChargement < 1000)
        configMenu->parametre.dureeMaxChargement = 1000;
     else if (configMenu->parametre.dureeMaxChargement > 100000)
          configMenu->parametre.dureeMaxChargement = 100000;
     sprintf(configMenu->tableauTextesReponses[6], "%d", configMenu->parametre.dureeMaxChargement);

     configMenu->parametre.dureeMaxEnregistrement = strtol(configMenu->tableauTextesReponses[7], NULL, 10);
     if (configMenu->parametre.dureeMaxEnregistrement < 10)
        configMenu->parametre.dureeMaxEnregistrement = 10;
     else if (configMenu->parametre.dureeMaxEnregistrement > 3600)
          configMenu->parametre.dureeMaxEnregistrement = 3600;
     sprintf(configMenu->tableauTextesReponses[7], "%d", configMenu->parametre.dureeMaxEnregistrement);

     configMenu->parametre.nombreChargementsMaxi = strtol(configMenu->tableauTextesReponses[8], NULL, 10);
     if (configMenu->parametre.nombreChargementsMaxi < 2)
        configMenu->parametre.nombreChargementsMaxi = 2;
     else if (configMenu->parametre.nombreChargementsMaxi > 1000)
          configMenu->parametre.nombreChargementsMaxi = 1000;
     sprintf(configMenu->tableauTextesReponses[8], "%d", configMenu->parametre.nombreChargementsMaxi);

     configMenu->parametre.vitesseDefilement = strtol(configMenu->tableauTextesReponses[9], NULL, 10);
     if (configMenu->parametre.vitesseDefilement > 360000)
          configMenu->parametre.vitesseDefilement = 360000;

     configMenu->parametre.rafraichissement = strtol(configMenu->tableauTextesReponses[10], NULL, 10);
     if (configMenu->parametre.rafraichissement < 5)
        configMenu->parametre.rafraichissement = 5;
     else if (configMenu->parametre.rafraichissement > 100)
          configMenu->parametre.rafraichissement = 100;
     sprintf(configMenu->tableauTextesReponses[10], "%d", configMenu->parametre.rafraichissement);

     configMenu->parametre.vitesseTransition = strtol(configMenu->tableauTextesReponses[11], NULL, 10);
     if (configMenu->parametre.vitesseTransition > 60000)
          configMenu->parametre.vitesseTransition = 60000;
     else if (configMenu->parametre.vitesseTransition + configMenu->parametre.vitesseDefilement < 500)
          configMenu->parametre.vitesseDefilement = 500 - configMenu->parametre.vitesseTransition;
     sprintf(configMenu->tableauTextesReponses[11], "%d", configMenu->parametre.vitesseTransition);
     sprintf(configMenu->tableauTextesReponses[9], "%d", configMenu->parametre.vitesseDefilement);


     strcpy(configMenu->parametre.dossierDiaporama, configMenu->tableauTextesReponses[12]);
     strcpy(configMenu->parametre.imageDeFond, configMenu->tableauTextesReponses[13]);
}
