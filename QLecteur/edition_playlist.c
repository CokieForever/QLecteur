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

#include "edition_playlist.h"


int editer_playlist(MenuEditPlaylist *editMenu, Musique *musique, MenuConfigLecteur configMenu, AffichageParoles *affichageParoles, SDL_Event event, int cutPosition, int *ctrl, int *maj)
{
         char chaine[TAILLE_MAX_NOM] = " ";

         if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_LCTRL)
            *ctrl = 1;
         else if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_LCTRL)
              *ctrl = 0;

         SDL_BlitSurface(editMenu->surfaceSauvegarde, NULL, editMenu->surfaceBlit, &editMenu->positionZero);

         int i, j, out = 0;
         SDL_Color couleurBlanc = {255, 255, 255}, couleurBleuNuit = {0, 0, 160}, couleurRougeFonce = {120, 0, 0}, couleurVertBizarre = {128, 255, 0};
         char *positionAntiSlash = NULL;

         event.button.x -= editMenu->positionSurfaceBlit.x;
         event.button.y -= editMenu->positionSurfaceBlit.y;

         if (test_button(event, editMenu->surfaceBlit, editMenu->zoneDossier.boutonDossier, editMenu->zoneDossier.positionBoutonDossier, SDL_MapRGB(editMenu->surfaceBlit->format, 255, 0, 0), -1, "répertoire parent", 1500, editMenu->tempsDernierMvtSouris, NULL) == 1)
         {
                  if ((positionAntiSlash = strrchr(editMenu->repertoireVirtuel, '\\')) != NULL)
                     *positionAntiSlash = '\0';
                  if ((positionAntiSlash = strrchr(editMenu->repertoireVirtuel, '\\')) == NULL)
                     sprintf(editMenu->repertoireVirtuel, "%s\\", editMenu->repertoireVirtuel);
                  for (j = 0 ; j < TAILLE_MAX_PLAYLIST ; j++)
                  {
                      editMenu->zoneDossier.tableauSelectionnes[j] = 0;
                  }
                  strcpy(editMenu->adresseAScanner, editMenu->repertoireVirtuel);
                  lister_extension(editMenu->adresseAScanner, editMenu->zoneMusique.nomFichierListe, ".mp3 ; .wma ; .wav ; .aac ; .ogg", "w");
                  creer_playlist(editMenu->zoneMusique.nomFichierListe, editMenu->zoneMusique.nomFichierPlaylist, editMenu->zoneMusique.typePlaylist);
                  classer_alphabetique_fichier_texte(editMenu->zoneMusique.nomFichierPlaylist, TAILLE_MAX_NOM, editMenu->zoneMusique.nomFichierListe);
                  lister_dossiers(editMenu->adresseAScanner, editMenu->zoneDossier.nomFichierPlaylist, "w");
                  effacer_adresse(editMenu->zoneDossier.nomFichierPlaylist, TAILLE_MAX_NOM);
                  classer_alphabetique_fichier_texte(editMenu->zoneDossier.nomFichierPlaylist, TAILLE_MAX_NOM, NULL);
                  editMenu->zoneMusique.ligneDebut = 0;
                  editMenu->zoneMusique.ligneFin = editMenu->zoneMusique.nombreAffichables;
                  editMenu->zoneDossier.ligneDebut = 0;
                  editMenu->zoneDossier.ligneFin = editMenu->zoneDossier.nombreAffichables;
                  recreer_surface(editMenu);
                  out = 1;
         }

         else if (test_button(event, editMenu->surfaceBlit, editMenu->posteTravail, editMenu->positionPosteTravail, SDL_MapRGB(editMenu->surfaceBlit->format, 255, 0, 0), -1, "poste de travail", 1500, editMenu->tempsDernierMvtSouris, NULL) == 1)
         {
                  for (j = 0 ; j < TAILLE_MAX_PLAYLIST ; j++)
                  {
                      editMenu->zoneDossier.tableauSelectionnes[j] = 0;
                  }
                  strcpy(editMenu->repertoireVirtuel, "pdt");
                  lister_disques(editMenu->zoneDossier.nomFichierPlaylist, "w");
                  classer_alphabetique_fichier_texte(editMenu->zoneDossier.nomFichierPlaylist, TAILLE_MAX_NOM, NULL);
                  editMenu->zoneDossier.ligneDebut = 0;
                  editMenu->zoneDossier.ligneFin = editMenu->zoneDossier.nombreAffichables;
                  recreer_surface(editMenu);
                  out = 1;
         }

         else if (strstr(editMenu->zonePlaylist.nomFichierPlaylist, ".npl") == NULL && test_button(event, editMenu->surfaceBlit, editMenu->nouvellePlaylist, editMenu->positionNouvellePlaylist, SDL_MapRGB(editMenu->surfaceBlit->format, 255, 0, 0), -1, "nouvelle playlist", 1500, editMenu->tempsDernierMvtSouris, NULL) == 1)
         {
                 char nomNouvellePlaylist[TAILLE_MAX_NOM] = {'\0'};
                 SDL_Rect positionEntree;
                 positionEntree.x = editMenu->positionSurfaceBlit.x + editMenu->ligneSeparation->w;
                 positionEntree.y = editMenu->hauteur - 30;

                 if (entrer_texte(nomNouvellePlaylist, TAILLE_MAX_NOM, editMenu->surfaceBlit->w - editMenu->ligneSeparation->w, positionEntree, editMenu->ecran, editMenu->zonePlaylist.police, couleurBlanc, couleurRougeFonce, "Entrez un nom : ", "bui") && nomNouvellePlaylist[0] != '\0')
                 {
                     sprintf(chaine, "playlists\\%s.npl", nomNouvellePlaylist);
                     strcpy(nomNouvellePlaylist, chaine);

                     FILE *nouvellePlaylist = NULL;
                     if ((nouvellePlaylist = fopen(nomNouvellePlaylist, "w+")) == NULL)
                        fprintf(stderr, "ERR editer_playlist %d : Impossible de créer la playlist %s.\n", __LINE__, nomNouvellePlaylist);
                     else
                     {
                         fprintf(nouvellePlaylist, "ouverture\n");
                         while(fclose(nouvellePlaylist) == EOF);
                     }

                     strcpy(strstr(nomNouvellePlaylist, ".npl"), ".list");
                     if ((nouvellePlaylist = fopen(nomNouvellePlaylist, "w+")) == NULL)
                        fprintf(stderr, "ERR editer_playlist %d : Impossible de créer la playlist %s.\n", __LINE__, nomNouvellePlaylist);
                     else
                     {
                         fprintf(nouvellePlaylist, "ouverture.wav\n");
                         while(fclose(nouvellePlaylist) == EOF);
                     }
                 }
         }

         else if (strstr(editMenu->zonePlaylist.nomFichierPlaylist, ".npl") != NULL && test_button(event, editMenu->surfaceBlit, editMenu->zonePlaylist.boutonDossier, editMenu->zonePlaylist.positionBoutonDossier, SDL_MapRGB(editMenu->surfaceBlit->format, 255, 0, 0), -1, "ouvrir la liste", 1500, editMenu->tempsDernierMvtSouris, NULL))
         {
                  strcpy(editMenu->zonePlaylist.nomFichierPlaylist, "playlists\\Playlist_playlist.plpl");
                  strcpy(editMenu->zonePlaylist.nomFichierListe, "playlists\\Liste_playlists.spl");
                  editMenu->zonePlaylist.nombreAffichables += 2;
                  editMenu->zonePlaylist.ligneDebut = 0;
                  editMenu->zonePlaylist.ligneFin = editMenu->zonePlaylist.nombreAffichables;
                  recreer_surface(editMenu);
         }

         else if (test_button(event, editMenu->surfaceBlit, editMenu->zoneMusique.boutonTypePlaylist, editMenu->zoneMusique.positionBoutonTypePlaylist, SDL_MapRGB(editMenu->surfaceBlit->format, 255, 0, 0), -1, "type d'affichage", 1500, editMenu->tempsDernierMvtSouris, NULL))
         {
              editMenu->zoneMusique.typePlaylist++;
              if (editMenu->zoneMusique.typePlaylist > TITRE_ET_AUTEUR)
              editMenu->zoneMusique.typePlaylist = NOM;
              creer_playlist(editMenu->zoneMusique.nomFichierListe, editMenu->zoneMusique.nomFichierPlaylist, editMenu->zoneMusique.typePlaylist);
              classer_alphabetique_fichier_texte(editMenu->zoneMusique.nomFichierPlaylist, TAILLE_MAX_NOM, editMenu->zoneMusique.nomFichierListe);
              SDL_FreeSurface(editMenu->zoneMusique.boutonTypePlaylist);
              switch(editMenu->zoneMusique.typePlaylist)
              {
                                              case NOM:
                                                   editMenu->zoneMusique.boutonTypePlaylist = TTF_RenderText_Blended(editMenu->zoneMusique.policeBoutonTypePlaylist, "NoM", couleurVertBizarre);
                                                   break;
                                              case TITRE:
                                                   editMenu->zoneMusique.boutonTypePlaylist = TTF_RenderText_Blended(editMenu->zoneMusique.policeBoutonTypePlaylist, "TtL", couleurVertBizarre);
                                                   break;
                                              case AUTEUR:
                                                   editMenu->zoneMusique.boutonTypePlaylist = TTF_RenderText_Blended(editMenu->zoneMusique.policeBoutonTypePlaylist, "AsT", couleurVertBizarre);
                                                   break;
                                              case NOM_SIMPLIFIE:
                                                   editMenu->zoneMusique.boutonTypePlaylist = TTF_RenderText_Blended(editMenu->zoneMusique.policeBoutonTypePlaylist, "NsP", couleurVertBizarre);
                                                   break;
                                              case TITRE_ET_AUTEUR:
                                                   editMenu->zoneMusique.boutonTypePlaylist = TTF_RenderText_Blended(editMenu->zoneMusique.policeBoutonTypePlaylist, "T+A", couleurVertBizarre);
                                                   break;
              }
              editMenu->zoneMusique.positionBoutonTypePlaylist.x = editMenu->zoneMusique.positionLigneSeparation.x + editMenu->ligneSeparation->w + 10;

              recreer_surface(editMenu);
         }

         else if (strstr(editMenu->zonePlaylist.nomFichierPlaylist, ".npl") != NULL && test_button(event, editMenu->surfaceBlit, editMenu->zonePlaylist.boutonTypePlaylist, editMenu->zonePlaylist.positionBoutonTypePlaylist, SDL_MapRGB(editMenu->surfaceBlit->format, 255, 0, 0), -1, "type d'affichage", 1500, editMenu->tempsDernierMvtSouris, NULL))
         {
              editMenu->zonePlaylist.typePlaylist++;
              if (editMenu->zonePlaylist.typePlaylist > TITRE_ET_AUTEUR)
                 editMenu->zonePlaylist.typePlaylist = NOM;
              creer_playlist(editMenu->zonePlaylist.nomFichierListe, editMenu->zonePlaylist.nomFichierPlaylist, editMenu->zonePlaylist.typePlaylist);
              SDL_FreeSurface(editMenu->zonePlaylist.boutonTypePlaylist);
              switch(editMenu->zonePlaylist.typePlaylist)
              {
                                              case NOM:
                                                   editMenu->zonePlaylist.boutonTypePlaylist = TTF_RenderText_Blended(editMenu->zonePlaylist.policeBoutonTypePlaylist, "NoM", couleurVertBizarre);
                                                   break;
                                              case TITRE:
                                                   editMenu->zonePlaylist.boutonTypePlaylist = TTF_RenderText_Blended(editMenu->zonePlaylist.policeBoutonTypePlaylist, "TtL", couleurVertBizarre);
                                                   break;
                                              case AUTEUR:
                                                   editMenu->zonePlaylist.boutonTypePlaylist = TTF_RenderText_Blended(editMenu->zonePlaylist.policeBoutonTypePlaylist, "AsT", couleurVertBizarre);
                                                   break;
                                              case NOM_SIMPLIFIE:
                                                   editMenu->zonePlaylist.boutonTypePlaylist = TTF_RenderText_Blended(editMenu->zonePlaylist.policeBoutonTypePlaylist, "NsP", couleurVertBizarre);
                                                   break;
                                              case TITRE_ET_AUTEUR:
                                                   editMenu->zonePlaylist.boutonTypePlaylist = TTF_RenderText_Blended(editMenu->zonePlaylist.policeBoutonTypePlaylist, "T+A", couleurVertBizarre);
                                                   break;
              }
              editMenu->zonePlaylist.positionBoutonTypePlaylist.x = editMenu->zonePlaylist.positionLigneSeparation.x + editMenu->ligneSeparation->w + 10;

              recreer_surface(editMenu);
         }


         for (i = editMenu->zoneDossier.ligneDebut ; i < editMenu->zoneDossier.ligneFin && editMenu->zoneDossier.tableauSurfaces[i] != NULL ; i++)
         {
             if (editMenu->zoneDossier.tableauSelectionnes[i])
             {
                SDL_FreeSurface(editMenu->zoneDossier.tableauSurfaces[i]);
                editMenu->zoneDossier.tableauSurfaces[i] = TTF_RenderText_Shaded(editMenu->zoneDossier.police, editMenu->zoneDossier.tableauNoms[i], couleurBleuNuit, couleurBlanc);
                SDL_BlitSurface(editMenu->zoneDossier.tableauSurfaces[i], NULL, editMenu->surfaceBlit, &editMenu->zoneDossier.tableauPositions[i]);
             }
         }


         for (i = editMenu->zoneDossier.ligneDebut ; i < editMenu->zoneDossier.ligneFin && editMenu->zoneDossier.tableauSurfaces[i] != NULL && out == 0; i++)
         {
             if (test_button(event, editMenu->surfaceBlit, editMenu->zoneDossier.tableauSurfaces[i], editMenu->zoneDossier.tableauPositions[i], -1, -1, NULL, 0, NULL, NULL) == 1 && editMenu->zoneDossier.tableauSelectionnes[i] != 1)
             {
                  if (*ctrl == 0)
                  {
                            for (j = 0 ; j < TAILLE_MAX_PLAYLIST ; j++)
                            {
                                editMenu->zoneDossier.tableauSelectionnes[j] = 0;
                            }
                            effacer_fichier(editMenu->zoneMusique.nomFichierPlaylist);
                            effacer_fichier(editMenu->zoneMusique.nomFichierListe);
                  }
                  lire_ligne(editMenu->zoneDossier.nomFichierPlaylist, i + 1, editMenu->zoneDossier.tableauNoms[i], TAILLE_MAX_NOM);
                  sprintf(editMenu->adresseAScanner, "%s\\%s", editMenu->repertoireVirtuel, editMenu->zoneDossier.tableauNoms[i]);
                  if (strcmp(editMenu->repertoireVirtuel, "pdt") == 0)
                     strcpy(editMenu->adresseAScanner, editMenu->zoneDossier.tableauNoms[i]);
                  if (lister_extension(editMenu->adresseAScanner, editMenu->zoneMusique.nomFichierListe, ".mp3 ; .wma ; .wav ; .aac ; .ogg", "a+") == -1)
                     strcpy(editMenu->adresseAScanner, strrchr(editMenu->adresseAScanner, '\\'));
                  else
                  {
                      editMenu->zoneDossier.tableauSelectionnes[i] = 1;
                      creer_playlist(editMenu->zoneMusique.nomFichierListe, editMenu->zoneMusique.nomFichierPlaylist, editMenu->zoneMusique.typePlaylist);
                      classer_alphabetique_fichier_texte(editMenu->zoneMusique.nomFichierPlaylist, TAILLE_MAX_NOM, editMenu->zoneMusique.nomFichierListe);
                      editMenu->zoneMusique.ligneDebut = 0;
                      editMenu->zoneMusique.ligneFin = editMenu->zoneMusique.nombreAffichables;
                  }
                  recreer_surface(editMenu);
                  out = 1;
             }
             else if (test_button(event, editMenu->surfaceBlit, editMenu->zoneDossier.tableauSurfaces[i], editMenu->zoneDossier.tableauPositions[i], -1, -1, NULL, 0, NULL, NULL) == 2)
             {
                  lire_ligne(editMenu->zoneDossier.nomFichierPlaylist, i + 1, editMenu->zoneDossier.tableauNoms[i], TAILLE_MAX_NOM);
                  if (strcmp(editMenu->repertoireVirtuel, "pdt") == 0)
                     strcpy(editMenu->adresseAScanner, editMenu->zoneDossier.tableauNoms[i]);
                  else if (editMenu->repertoireVirtuel[strlen(editMenu->repertoireVirtuel) - 1] == '\\')
                       sprintf(editMenu->adresseAScanner, "%s%s", editMenu->repertoireVirtuel, editMenu->zoneDossier.tableauNoms[i]);
                  else sprintf(editMenu->adresseAScanner, "%s\\%s", editMenu->repertoireVirtuel, editMenu->zoneDossier.tableauNoms[i]);
                  for (j = 0 ; j < TAILLE_MAX_PLAYLIST ; j++)
                  {
                      editMenu->zoneDossier.tableauSelectionnes[j] = 0;
                  }
                  if (lister_extension(editMenu->adresseAScanner, editMenu->zoneMusique.nomFichierListe, ".mp3 ; .wma ; .wav ; .aac ; .ogg", "w") == -1)
                     strcpy(editMenu->adresseAScanner, editMenu->repertoireVirtuel);
                  else
                  {
                      strcpy(editMenu->repertoireVirtuel, editMenu->adresseAScanner);
                      creer_playlist(editMenu->zoneMusique.nomFichierListe, editMenu->zoneMusique.nomFichierPlaylist, editMenu->zoneMusique.typePlaylist);
                      classer_alphabetique_fichier_texte(editMenu->zoneMusique.nomFichierPlaylist, TAILLE_MAX_NOM, editMenu->zoneMusique.nomFichierListe);
                      lister_dossiers(editMenu->adresseAScanner, editMenu->zoneDossier.nomFichierPlaylist, "w");
                      effacer_adresse(editMenu->zoneDossier.nomFichierPlaylist, TAILLE_MAX_NOM);
                      classer_alphabetique_fichier_texte(editMenu->zoneDossier.nomFichierPlaylist, TAILLE_MAX_NOM, NULL);
                      editMenu->zoneMusique.ligneDebut = 0;
                      editMenu->zoneMusique.ligneFin = editMenu->zoneMusique.nombreAffichables;
                      editMenu->zoneDossier.ligneDebut = 0;
                      editMenu->zoneDossier.ligneFin = editMenu->zoneDossier.nombreAffichables;
                  }
                  recreer_surface(editMenu);
                  out = 1;
             }
         }


         if (out == 0 && test_position(event, editMenu->zoneDossier.positionListe, editMenu->zoneDossier.largeur, editMenu->hauteur) && event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT)
         {
                            for (j = 0 ; j < TAILLE_MAX_PLAYLIST ; j++)
                            {
                                editMenu->zoneDossier.tableauSelectionnes[j] = 0;
                                editMenu->zoneMusique.tableauSelectionnes[j] = 0;
                            }
                            lister_extension(editMenu->repertoireVirtuel, editMenu->zoneMusique.nomFichierListe, ".mp3 ; .wma ; .wav ; .aac ; .ogg", "w");
                            creer_playlist(editMenu->zoneMusique.nomFichierListe, editMenu->zoneMusique.nomFichierPlaylist, editMenu->zoneMusique.typePlaylist);
                            classer_alphabetique_fichier_texte(editMenu->zoneMusique.nomFichierPlaylist, TAILLE_MAX_NOM, editMenu->zoneMusique.nomFichierListe);
                            recreer_surface(editMenu);
         }

         out = 0;

         if (*ctrl && event.key.keysym.sym == SDLK_q)
         {
                   int nombreLignes = compter_lignes(editMenu->zoneMusique.nomFichierListe);
                   for (i = 0 ; i < nombreLignes ; i++)
                   {
                       editMenu->zoneMusique.tableauSelectionnes[i] = 1;
                   }
         }

         for (i = editMenu->zoneMusique.ligneDebut ; i < editMenu->zoneMusique.ligneFin && editMenu->zoneMusique.tableauSurfaces[i] != NULL; i++)
         {
             if (editMenu->zoneMusique.tableauSelectionnes[i])
             {
                SDL_FreeSurface(editMenu->zoneMusique.tableauSurfaces[i]);
                editMenu->zoneMusique.tableauSurfaces[i] = TTF_RenderText_Shaded(editMenu->zoneMusique.police, editMenu->zoneMusique.tableauNoms[i], couleurBleuNuit, couleurBlanc);
                SDL_BlitSurface(editMenu->zoneMusique.tableauSurfaces[i], NULL, editMenu->surfaceBlit, &editMenu->zoneMusique.tableauPositions[i]);
             }
         }


         for (i = editMenu->zoneMusique.ligneDebut ; i < editMenu->zoneMusique.ligneFin && editMenu->zoneMusique.tableauSurfaces[i] != NULL && out == 0; i++)
         {
             if (strstr(editMenu->zonePlaylist.nomFichierPlaylist, ".npl") != NULL)
             {
                     if (editMenu->zoneMusique.clicGauche[i] != 1)
                     {
                        editMenu->zoneMusique.positionFantome[i].x = editMenu->zoneMusique.tableauPositions[i].x;
                        editMenu->zoneMusique.positionFantome[i].y = editMenu->zoneMusique.tableauPositions[i].y;
                     }

                     cliquer_deplacer(event, editMenu->zoneMusique.tableauSurfaces[i], &editMenu->zoneMusique.positionFantome[i], &editMenu->zoneMusique.differenceAuClic[i], &editMenu->zoneMusique.clicGauche[i]);
                     if (editMenu->zoneMusique.clicGauche[i] && editMenu->zoneMusique.positionFantome[i].x != editMenu->zoneMusique.tableauPositions[i].x && editMenu->zoneMusique.positionFantome[i].y != editMenu->zoneMusique.tableauPositions[i].y)
                     {
                                                 if (*ctrl == 0)
                                                 {
                                                    if (editMenu->zoneMusique.tableauSelectionnes[i] == 0)
                                                    {
                                                         for (j = 0 ; j < TAILLE_MAX_PLAYLIST ; j++)
                                                         {
                                                            editMenu->zoneMusique.tableauSelectionnes[j] = 0;
                                                         }
                                                    }
                                                    editMenu->zoneMusique.tableauSelectionnes[i] = 1;
                                                 }
                                                 if (test_position(event, editMenu->zonePlaylist.positionListe, editMenu->zonePlaylist.largeur, editMenu->hauteur))
                                                 {
                                                         SDL_ShowCursor(1);
                                                         editMenu->positionBarreRouge.y = (event.button.y / (editMenu->zonePlaylist.echantillonTexte->h + 5)) * (editMenu->zonePlaylist.echantillonTexte->h + 5) + 8;
                                                         int positionMax = editMenu->zonePlaylist.nombreAffichables * (editMenu->zonePlaylist.echantillonTexte->h + 5) + 8;
                                                         if (editMenu->positionBarreRouge.y > positionMax)
                                                                editMenu->positionBarreRouge.y = positionMax;

                                                         SDL_BlitSurface(editMenu->barreRouge, NULL, editMenu->surfaceBlit, &editMenu->positionBarreRouge);
                                                         if (editMenu->positionBarreRouge.y <= 8)
                                                         {
                                                                  editMenu->zonePlaylist.ligneDebut -= 1;
                                                                  if (editMenu->zonePlaylist.ligneDebut < 0)
                                                                     editMenu->zonePlaylist.ligneDebut = 0;
                                                                  editMenu->zonePlaylist.ligneFin = editMenu->zonePlaylist.ligneDebut + editMenu->zonePlaylist.nombreAffichables;
                                                                  recreer_surface(editMenu);
                                                         }
                                                         else if (editMenu->positionBarreRouge.y >= positionMax)
                                                         {
                                                                  editMenu->zonePlaylist.ligneDebut += 1;
                                                                  if (editMenu->zonePlaylist.ligneDebut + editMenu->zonePlaylist.nombreAffichables > editMenu->zonePlaylist.nombreTotal)
                                                                     editMenu->zonePlaylist.ligneDebut = editMenu->zonePlaylist.nombreTotal - editMenu->zonePlaylist.nombreAffichables;
                                                                  editMenu->zonePlaylist.ligneFin = editMenu->zonePlaylist.ligneDebut + editMenu->zonePlaylist.nombreAffichables;
                                                                  recreer_surface(editMenu);
                                                         }
                                                         else
                                                         {
                                                                 SDL_Rect position;
                                                                 int k = 0;
                                                                 for (j = i ; j >= 0 && editMenu->zoneMusique.tableauSurfaces[j] != NULL ; j--)
                                                                 {
                                                                     if (editMenu->zoneMusique.tableauSelectionnes[j] == 1)
                                                                     {
                                                                           position.x = editMenu->zoneMusique.positionFantome[i].x;
                                                                           position.y = editMenu->zoneMusique.positionFantome[i].y - (k * (editMenu->zoneMusique.tableauSurfaces[j]->h + 5));
                                                                           if (position.y >= 0)
                                                                           {
                                                                                       SDL_SetAlpha(editMenu->zoneMusique.tableauSurfaces[j], SDL_SRCALPHA, 180);
                                                                                       SDL_BlitSurface(editMenu->zoneMusique.tableauSurfaces[j], NULL, editMenu->surfaceBlit, &position);
                                                                                       SDL_SetAlpha(editMenu->zoneMusique.tableauSurfaces[j], SDL_SRCALPHA, 0);
                                                                           }
                                                                           k++;
                                                                     }
                                                                 }

                                                                 k = 1;
                                                                 for (j = i + 1 ; j < TAILLE_MAX_PLAYLIST && editMenu->zoneMusique.tableauSurfaces[j] != NULL ; j++)
                                                                 {
                                                                     if (editMenu->zoneMusique.tableauSelectionnes[j] == 1)
                                                                     {
                                                                           position.x = editMenu->zoneMusique.positionFantome[i].x;
                                                                           position.y = editMenu->zoneMusique.positionFantome[i].y + (k * (editMenu->zoneMusique.tableauSurfaces[j]->h + 5));
                                                                           if (position.y >= 0)
                                                                           {
                                                                                       SDL_SetAlpha(editMenu->zoneMusique.tableauSurfaces[j], SDL_SRCALPHA, 180);
                                                                                       SDL_BlitSurface(editMenu->zoneMusique.tableauSurfaces[j], NULL, editMenu->surfaceBlit, &position);
                                                                                       SDL_SetAlpha(editMenu->zoneMusique.tableauSurfaces[j], SDL_SRCALPHA, 0);
                                                                           }
                                                                           k++;
                                                                     }
                                                                 }
                                                         }
                                                 }
                                                 else
                                                 {
                                                     SDL_ShowCursor(0);
                                                     SDL_Rect souris;
                                                     souris.x = event.button.x - (editMenu->curseurInterdit->w / 2);
                                                     souris.y = event.button.y - (editMenu->curseurInterdit->h / 2);
                                                     SDL_BlitSurface(editMenu->curseurInterdit, NULL, editMenu->surfaceBlit, &souris);
                                                 }
                                                 out = 1;
                     }
                     else if (out == 1 || editMenu->zoneMusique.positionFantome[i].x != editMenu->zoneMusique.tableauPositions[i].x || editMenu->zoneMusique.positionFantome[i].y != editMenu->zoneMusique.tableauPositions[i].y)
                     {
                             SDL_ShowCursor(1);
                             if (test_position(event, editMenu->zonePlaylist.positionListe, editMenu->zonePlaylist.largeur, editMenu->hauteur))
                             {
                                 for (j = TAILLE_MAX_PLAYLIST - 1 ; j >= 0 ; j--)
                                 {
                                     if (editMenu->zoneMusique.tableauSelectionnes[j])
                                     {
                                         int numeroLigneInsert = (editMenu->positionBarreRouge.y - 5) / (editMenu->zonePlaylist.echantillonTexte->h + 5) + editMenu->zonePlaylist.ligneDebut;
                                         char adresseAInserer[TAILLE_MAX_NOM] = " ";
                                         lire_ligne(editMenu->zoneMusique.nomFichierListe, j + 1, adresseAInserer, TAILLE_MAX_NOM);
                                         char nomAInserer[TAILLE_MAX_NOM] = " ";
                                         lire_ligne(editMenu->zoneMusique.nomFichierPlaylist, j + 1, nomAInserer, TAILLE_MAX_NOM);
                                         if (configMenu.parametre.sauvegardeRelative && editMenu->dossierBase[0] == editMenu->adresseAScanner[0] && toupper(editMenu->adresseAScanner[0]) != 'C')
                                            convertir_en_relatif(adresseAInserer, editMenu->dossierBase);

                                         inserer_ligne(editMenu->zonePlaylist.nomFichierListe, TAILLE_MAX_NOM, numeroLigneInsert + 1, adresseAInserer);
                                         inserer_ligne(editMenu->zonePlaylist.nomFichierPlaylist, TAILLE_MAX_NOM, numeroLigneInsert + 1, nomAInserer);
                                     }
                                     editMenu->zonePlaylist.tableauSelectionnes[j] = 0;
                                 }

                                 recreer_surface(editMenu);
                             }
                             out = 1;
                     }
             }

             if (test_button(event, editMenu->surfaceBlit, editMenu->zoneMusique.tableauSurfaces[i], editMenu->zoneMusique.tableauPositions[i], -1, -1, NULL, 0, NULL, NULL) == 2)
             {
                  FILE *playlistTemp = NULL;
                  if((playlistTemp = fopen("playlists\\list.tmp", "w")) == NULL)
                                   fprintf(stderr, "ERR editer_playlist %d : Impossible de créer la playlist temporaire.\n", __LINE__);
                  else
                  {
                      char chaine[TAILLE_MAX_NOM] = " ";
                      lire_ligne(editMenu->zoneMusique.nomFichierListe, i + 1, chaine, TAILLE_MAX_NOM);
                      fprintf(playlistTemp, "%s", chaine);
                      while(fclose(playlistTemp) == EOF);
                      FSOUND_Stream_Stop(musique->pointeurStereo);
                      FSOUND_Stream_Stop(musique->pointeurMono);
                      FSOUND_Stream_Close(musique->pointeurStereo);
                      FSOUND_Stream_Close(musique->pointeurMono);
                      charger_musique(musique, configMenu, affichageParoles, "playlists\\list.tmp", i + 1, cutPosition, NULL);

                      musique->numero = TAILLE_MAX_PLAYLIST + 1;
                      if (musique->mode != ERREUR && musique->mode != STOP)
                         FSOUND_Stream_Play(1, musique->pointeurStereo);
                  }
                  out = 1;
             }
         }


         for (i = editMenu->zoneMusique.ligneDebut ; i < editMenu->zoneMusique.ligneFin && editMenu->zoneMusique.tableauSurfaces[i] != NULL && out == 0; i++)
         {
             if (test_button(event, editMenu->surfaceBlit, editMenu->zoneMusique.tableauSurfaces[i], editMenu->zoneMusique.tableauPositions[i], -1, -1, NULL, 0, NULL, NULL))
             {
                  if (*ctrl == 0)
                  {
                            for (j = 0 ; j < TAILLE_MAX_PLAYLIST ; j++)
                            {
                                editMenu->zoneMusique.tableauSelectionnes[j] = 0;
                            }
                            editMenu->zoneMusique.tableauSelectionnes[i] = 1;
                  }
                  else
                  {
                      if (editMenu->zoneMusique.tableauSelectionnes[i])
                         editMenu->zoneMusique.tableauSelectionnes[i] = 0;
                      else editMenu->zoneMusique.tableauSelectionnes[i] = 1;
                  }
                  out = 1;
             }
         }


         if (out == 0 && test_position(event, editMenu->zoneMusique.positionListe, editMenu->zoneMusique.largeur, editMenu->hauteur) && event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT)
         {
                            for (j = 0 ; j < TAILLE_MAX_PLAYLIST ; j++)
                            {
                                editMenu->zoneMusique.tableauSelectionnes[j] = 0;
                            }
         }


         out = 0;


         for (i = editMenu->zonePlaylist.ligneDebut ; i < editMenu->zonePlaylist.ligneFin && editMenu->zonePlaylist.tableauSurfaces[i] != NULL ; i++)
         {
             if (editMenu->zonePlaylist.tableauSelectionnes[i])
             {
                            SDL_FreeSurface(editMenu->zonePlaylist.tableauSurfaces[i]);
                            editMenu->zonePlaylist.tableauSurfaces[i] = TTF_RenderText_Shaded(editMenu->zonePlaylist.police, editMenu->zonePlaylist.tableauNoms[i], couleurBleuNuit, couleurBlanc);
                            SDL_BlitSurface(editMenu->zonePlaylist.tableauSurfaces[i], NULL, editMenu->surfaceBlit, &editMenu->zonePlaylist.tableauPositions[i]);
             }
         }


         for (i = editMenu->zonePlaylist.ligneDebut ; i < editMenu->zonePlaylist.ligneFin && editMenu->zonePlaylist.tableauSurfaces[i] != NULL && out == 0 ; i++)
         {
                if (strstr(editMenu->zonePlaylist.nomFichierPlaylist, ".npl") == NULL)
                {
                         if (test_button(event, editMenu->surfaceBlit, editMenu->zonePlaylist.tableauSurfaces[i], editMenu->zonePlaylist.tableauPositions[i], -1, -1, NULL, 0, NULL, NULL) == 2)
                         {
                              for (j = 0 ; j < TAILLE_MAX_PLAYLIST ; j++)
                              {
                                  editMenu->zonePlaylist.tableauSelectionnes[j] = 0;
                              }
                              lire_ligne(editMenu->zonePlaylist.nomFichierPlaylist, i + 1, editMenu->zonePlaylist.tableauNoms[i], TAILLE_MAX_NOM);
                              sprintf(editMenu->zonePlaylist.nomFichierPlaylist, "playlists\\%s.npl", editMenu->zonePlaylist.tableauNoms[i]);
                              sprintf(editMenu->zonePlaylist.nomFichierListe, "playlists\\%s.list", editMenu->zonePlaylist.tableauNoms[i]);
                              editMenu->zonePlaylist.nombreAffichables -= 2;
                              editMenu->zonePlaylist.ligneFin = editMenu->zonePlaylist.ligneDebut + editMenu->zonePlaylist.nombreAffichables;
                              recreer_surface(editMenu);
                              out = 1;
                         }
                         else if (test_position(event, editMenu->zonePlaylist.positionListe, editMenu->zonePlaylist.largeur, editMenu->hauteur) && event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT && *ctrl == 0)
                         {
                                        for (j = 0 ; j < TAILLE_MAX_PLAYLIST ; j++)
                                        {
                                            editMenu->zonePlaylist.tableauSelectionnes[j] = 0;
                                        }
                                        out = 1;
                         }

                         if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_DELETE)
                         {
                                                char nomPlaylistASupprimer[TAILLE_MAX_NOM] = " ";
                                                char nomListeASupprimer[TAILLE_MAX_NOM] = " ";
                                                for (j = TAILLE_MAX_PLAYLIST - 1 ; j >= 0 ; j--)
                                                {
                                                    if (editMenu->zonePlaylist.tableauSelectionnes[j])
                                                    {
                                                       lire_ligne("playlists\\Liste_playlists.spl", j + 1, nomPlaylistASupprimer, TAILLE_MAX_NOM);
                                                       strcpy(nomListeASupprimer, nomPlaylistASupprimer);
                                                       strcpy(strstr(nomListeASupprimer, ".npl"), ".list");
                                                       int suppression1 = remove(nomListeASupprimer);
                                                       int suppression2 = remove(nomPlaylistASupprimer);
                                                       supprimer_ligne("playlists\\Liste_playlists.spl", TAILLE_MAX_NOM, j + 1);
                                                       supprimer_ligne("playlists\\Playlist_playlist.plpl", TAILLE_MAX_NOM, j + 1);
                                                       if ((suppression1 | suppression2) != 0)
                                                          fprintf(stderr, "ERR editer_playlist %d : Impossible de supprimer le fichier %s ou le fichier %s.\n", __LINE__,  nomListeASupprimer, nomPlaylistASupprimer);
                                                       editMenu->zonePlaylist.tableauSelectionnes[j] = 0;
                                                    }
                                                }

                                                recreer_surface(editMenu);

                                                out = 1;
                         }
                }
                else
                {
                         if (editMenu->zonePlaylist.clicGauche[i] != 1)
                         {
                            editMenu->zonePlaylist.positionFantome[i].x = editMenu->zonePlaylist.tableauPositions[i].x;
                            editMenu->zonePlaylist.positionFantome[i].y = editMenu->zonePlaylist.tableauPositions[i].y;
                         }

                         cliquer_deplacer(event, editMenu->zonePlaylist.tableauSurfaces[i], &editMenu->zonePlaylist.positionFantome[i], &editMenu->zonePlaylist.differenceAuClic[i], &editMenu->zonePlaylist.clicGauche[i]);
                         if (editMenu->zonePlaylist.clicGauche[i] && editMenu->zonePlaylist.positionFantome[i].x != editMenu->zonePlaylist.tableauPositions[i].x && editMenu->zonePlaylist.positionFantome[i].y != editMenu->zonePlaylist.tableauPositions[i].y)
                         {
                                                     if (*ctrl == 0)
                                                     {
                                                        if (editMenu->zonePlaylist.tableauSelectionnes[i] == 0)
                                                        {
                                                             for (j = 0 ; j < TAILLE_MAX_PLAYLIST ; j++)
                                                             {
                                                                editMenu->zonePlaylist.tableauSelectionnes[j] = 0;
                                                             }
                                                        }
                                                        editMenu->zonePlaylist.tableauSelectionnes[i] = 1;
                                                     }
                                                     if (test_position(event, editMenu->zonePlaylist.positionListe, editMenu->zonePlaylist.largeur, editMenu->hauteur))
                                                     {
                                                             SDL_ShowCursor(1);
                                                             editMenu->positionBarreRouge.y = (event.button.y / (editMenu->zonePlaylist.echantillonTexte->h + 5)) * (editMenu->zonePlaylist.echantillonTexte->h + 5) + 8;
                                                             int positionMax = editMenu->zonePlaylist.nombreAffichables * (editMenu->zonePlaylist.echantillonTexte->h + 5) + 8;
                                                             if (editMenu->positionBarreRouge.y > positionMax)
                                                                editMenu->positionBarreRouge.y = positionMax;

                                                             SDL_BlitSurface(editMenu->barreRouge, NULL, editMenu->surfaceBlit, &editMenu->positionBarreRouge);
                                                             if (editMenu->positionBarreRouge.y <= 8)
                                                             {
                                                                      editMenu->zonePlaylist.ligneDebut -= 1;
                                                                      if (editMenu->zonePlaylist.ligneDebut < 0)
                                                                         editMenu->zonePlaylist.ligneDebut = 0;
                                                                      editMenu->zonePlaylist.ligneFin = editMenu->zonePlaylist.ligneDebut + editMenu->zonePlaylist.nombreAffichables;
                                                                      recreer_surface(editMenu);
                                                             }
                                                             else if (editMenu->positionBarreRouge.y >= positionMax)
                                                             {
                                                                      editMenu->zonePlaylist.ligneDebut += 1;
                                                                      if (editMenu->zonePlaylist.ligneDebut + editMenu->zonePlaylist.nombreAffichables > editMenu->zonePlaylist.nombreTotal)
                                                                         editMenu->zonePlaylist.ligneDebut = editMenu->zonePlaylist.nombreTotal - editMenu->zonePlaylist.nombreAffichables;
                                                                      editMenu->zonePlaylist.ligneFin = editMenu->zonePlaylist.ligneDebut + editMenu->zonePlaylist.nombreAffichables;
                                                                      recreer_surface(editMenu);
                                                             }
                                                             else
                                                             {
                                                                     SDL_Rect position;
                                                                     int k = 0;
                                                                     for (j = i ; j >= 0 && editMenu->zonePlaylist.tableauSurfaces[j] != NULL ; j--)
                                                                     {
                                                                         if (editMenu->zonePlaylist.tableauSelectionnes[j] == 1)
                                                                         {
                                                                               position.x = editMenu->zonePlaylist.positionFantome[i].x;
                                                                               position.y = editMenu->zonePlaylist.positionFantome[i].y - (k * (editMenu->zonePlaylist.tableauSurfaces[j]->h + 5));
                                                                               if (position.y >= 0)
                                                                               {
                                                                                           SDL_SetAlpha(editMenu->zonePlaylist.tableauSurfaces[j], SDL_SRCALPHA, 180);
                                                                                           SDL_BlitSurface(editMenu->zonePlaylist.tableauSurfaces[j], NULL, editMenu->surfaceBlit, &position);
                                                                                           SDL_SetAlpha(editMenu->zonePlaylist.tableauSurfaces[j], SDL_SRCALPHA, 0);
                                                                               }
                                                                               k++;
                                                                         }
                                                                     }

                                                                     k = 1;
                                                                     for (j = i + 1 ; j < TAILLE_MAX_PLAYLIST && editMenu->zonePlaylist.tableauSurfaces[j] != NULL ; j++)
                                                                     {
                                                                         if (editMenu->zonePlaylist.tableauSelectionnes[j] == 1)
                                                                         {
                                                                               position.x = editMenu->zonePlaylist.positionFantome[i].x;
                                                                               position.y = editMenu->zonePlaylist.positionFantome[i].y + (k * (editMenu->zonePlaylist.tableauSurfaces[j]->h + 5));
                                                                               if (position.y >= 0)
                                                                               {
                                                                                           SDL_SetAlpha(editMenu->zonePlaylist.tableauSurfaces[j], SDL_SRCALPHA, 180);
                                                                                           SDL_BlitSurface(editMenu->zonePlaylist.tableauSurfaces[j], NULL, editMenu->surfaceBlit, &position);
                                                                                           SDL_SetAlpha(editMenu->zonePlaylist.tableauSurfaces[j], SDL_SRCALPHA, 0);
                                                                               }
                                                                               k++;
                                                                         }
                                                                     }
                                                             }
                                                     }
                                                     else
                                                     {
                                                         SDL_ShowCursor(0);
                                                         SDL_Rect souris;
                                                         souris.x = event.button.x - (editMenu->curseurInterdit->w / 2);
                                                         souris.y = event.button.y - (editMenu->curseurInterdit->h / 2);
                                                         SDL_BlitSurface(editMenu->curseurInterdit, NULL, editMenu->surfaceBlit, &souris);
                                                     }
                                                     out = 1;
                         }
                         else if (editMenu->zonePlaylist.positionFantome[i].x != editMenu->zonePlaylist.tableauPositions[i].x || editMenu->zonePlaylist.positionFantome[i].y != editMenu->zonePlaylist.tableauPositions[i].y)
                         {
                                 SDL_ShowCursor(1);
                                 if (test_position(event, editMenu->zonePlaylist.positionListe, editMenu->zonePlaylist.largeur, editMenu->hauteur))
                                 {
                                     int k = 0, nombreSelectionnes = 0;

                                     for (j = 0 ; j < TAILLE_MAX_PLAYLIST ; j++)
                                     {
                                         if (editMenu->zonePlaylist.tableauSelectionnes[j])
                                            nombreSelectionnes++;
                                     }

                                     char **tableau1 = NULL;
                                     tableau1 = malloc(sizeof(char*) * nombreSelectionnes);

                                     for (j = 0 ; j < TAILLE_MAX_PLAYLIST ; j++)
                                     {
                                         tableau1[k] = NULL;
                                         if (editMenu->zonePlaylist.tableauSelectionnes[j])
                                         {
                                            tableau1[k] = malloc(sizeof(char) * TAILLE_MAX_NOM);
                                            lire_ligne(editMenu->zonePlaylist.nomFichierListe, j + 1, tableau1[k], TAILLE_MAX_NOM);
                                            k++;
                                         }
                                     }

                                     for (j = TAILLE_MAX_PLAYLIST - 1 ; j >= 0 && *ctrl == 0 ; j--)
                                     {
                                         if (editMenu->zonePlaylist.tableauSelectionnes[j])
                                         {
                                            supprimer_ligne(editMenu->zonePlaylist.nomFichierListe, TAILLE_MAX_NOM, j + 1);
                                            supprimer_ligne(editMenu->zonePlaylist.nomFichierPlaylist, TAILLE_MAX_NOM, j + 1);
                                            editMenu->zonePlaylist.tableauSelectionnes[j] = 0;
                                         }
                                     }

                                     int numeroLigneInsert = (editMenu->positionBarreRouge.y - 5) / (editMenu->zonePlaylist.echantillonTexte->h + 5) + editMenu->zonePlaylist.ligneDebut;
                                     if (numeroLigneInsert < 0)
                                        numeroLigneInsert = 0;

                                     for (j = nombreSelectionnes - 1 ; j >= 0 ; j--)
                                     {
                                         inserer_ligne(editMenu->zonePlaylist.nomFichierListe, TAILLE_MAX_NOM, numeroLigneInsert + 1, tableau1[j]);
                                     }

                                     creer_playlist(editMenu->zonePlaylist.nomFichierListe, editMenu->zonePlaylist.nomFichierPlaylist, TITRE);

                                     for (j = 0 ; j < nombreSelectionnes ; j++)
                                     {
                                         free(tableau1[j]);
                                     }

                                     free(tableau1);
                                     recreer_surface(editMenu);
                                 }
                                 out = 1;
                         }

                         if (test_button(event, editMenu->surfaceBlit, editMenu->zonePlaylist.tableauSurfaces[i], editMenu->zonePlaylist.tableauPositions[i], -1, -1, NULL, 0, NULL, NULL) == 2)
                         {
                                      FSOUND_Stream_Stop(musique->pointeurStereo);
                                      FSOUND_Stream_Stop(musique->pointeurMono);
                                      FSOUND_Stream_Close(musique->pointeurStereo);
                                      FSOUND_Stream_Close(musique->pointeurMono);
                                      char nomFichierListe[TAILLE_MAX_NOM] = " ";
                                      strcpy(nomFichierListe, editMenu->zonePlaylist.nomFichierPlaylist);
                                      strcpy(strstr(nomFichierListe, ".npl"), ".list");
                                      charger_musique(musique, configMenu, affichageParoles, nomFichierListe, i + 1, cutPosition, NULL);
                                      musique->numero = TAILLE_MAX_PLAYLIST + 1;
                                      if (musique->mode != ERREUR && musique->mode != STOP)
                                         FSOUND_Stream_Play(1, musique->pointeurStereo);
                         }

                         if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_DELETE)
                         {
                                                char nomFichierListe[TAILLE_MAX_NOM] = " ";
                                                strcpy(nomFichierListe, editMenu->zonePlaylist.nomFichierPlaylist);
                                                strcpy(strstr(nomFichierListe, ".npl"), ".list");
                                                for (j = TAILLE_MAX_PLAYLIST - 1 ; j >= 0 ; j--)
                                                {
                                                    if (editMenu->zonePlaylist.tableauSelectionnes[j])
                                                    {
                                                       supprimer_ligne(editMenu->zonePlaylist.nomFichierPlaylist, TAILLE_MAX_NOM, j + 1);
                                                       supprimer_ligne(nomFichierListe, TAILLE_MAX_NOM, j + 1);
                                                       editMenu->zonePlaylist.tableauSelectionnes[j] = 0;
                                                    }
                                                }

                                                recreer_surface(editMenu);

                                                out = 1;
                         }
                }
         }

         for (i = editMenu->zonePlaylist.ligneDebut ; i < editMenu->zonePlaylist.ligneFin && editMenu->zonePlaylist.tableauSurfaces[i] != NULL && out == 0 ; i++)
         {
                if (test_button(event, editMenu->surfaceBlit, editMenu->zonePlaylist.tableauSurfaces[i], editMenu->zonePlaylist.tableauPositions[i], -1, -1, NULL, 0, NULL, NULL) == 1)
                {
                              if (*ctrl == 0)
                              {
                                        for (j = 0 ; j < TAILLE_MAX_PLAYLIST ; j++)
                                        {
                                            editMenu->zonePlaylist.tableauSelectionnes[j] = 0;
                                        }
                                        editMenu->zonePlaylist.tableauSelectionnes[i] = 1;
                              }
                              else
                              {
                                  if (editMenu->zonePlaylist.tableauSelectionnes[i])
                                     editMenu->zonePlaylist.tableauSelectionnes[i] = 0;
                                  else editMenu->zonePlaylist.tableauSelectionnes[i] = 1;
                              }
                              out = 1;
                }
         }


         if (out == 0 && test_position(event, editMenu->zonePlaylist.positionListe, editMenu->zonePlaylist.largeur, editMenu->hauteur) && event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT)
         {
             for (j = 0 ; j < TAILLE_MAX_PLAYLIST ; j++)
             {
                 editMenu->zonePlaylist.tableauSelectionnes[j] = 0;
             }
         }



         if (editMenu->zoneDossier.nombreAffichables < editMenu->zoneDossier.nombreTotal && cliquer_deplacer(event, editMenu->zoneDossier.barreDefilement, &editMenu->zoneDossier.positionBarreDefilement, &editMenu->zoneDossier.differenceAuClicBarreDefil, &editMenu->zoneDossier.clicGaucheSurBarreDefil))
         {
            editMenu->zoneDossier.positionBarreDefilement.x = 0;
            if (editMenu->zoneDossier.positionBarreDefilement.y < 0)
                    editMenu->zoneDossier.positionBarreDefilement.y = 0;
            if (editMenu->zoneDossier.positionBarreDefilement.y > editMenu->hauteur - editMenu->zoneDossier.hauteurBarreDefilement)
                    editMenu->zoneDossier.positionBarreDefilement.y = editMenu->hauteur - editMenu->zoneDossier.hauteurBarreDefilement;
            editMenu->zoneDossier.ligneDebut = (editMenu->zoneDossier.positionBarreDefilement.y * (editMenu->zoneDossier.nombreTotal - editMenu->zoneDossier.nombreAffichables)) / ((editMenu->hauteur - editMenu->zoneDossier.hauteurBarreDefilement) / 1.0);
            editMenu->zoneDossier.ligneFin = editMenu->zoneDossier.ligneDebut + editMenu->zoneDossier.nombreAffichables;
            recreer_surface(editMenu);
         }

         if (editMenu->zoneMusique.nombreAffichables < editMenu->zoneMusique.nombreTotal && cliquer_deplacer(event, editMenu->zoneMusique.barreDefilement, &editMenu->zoneMusique.positionBarreDefilement, &editMenu->zoneMusique.differenceAuClicBarreDefil, &editMenu->zoneMusique.clicGaucheSurBarreDefil))
         {
            editMenu->zoneMusique.positionBarreDefilement.x = editMenu->zoneMusique.positionLigneSeparation.x;
            if (editMenu->zoneMusique.positionBarreDefilement.y < 0)
                    editMenu->zoneMusique.positionBarreDefilement.y = 0;
            if (editMenu->zoneMusique.positionBarreDefilement.y > editMenu->hauteur - editMenu->zoneMusique.hauteurBarreDefilement)
                    editMenu->zoneMusique.positionBarreDefilement.y = editMenu->hauteur - editMenu->zoneMusique.hauteurBarreDefilement;
            editMenu->zoneMusique.ligneDebut = (editMenu->zoneMusique.positionBarreDefilement.y * (editMenu->zoneMusique.nombreTotal - editMenu->zoneMusique.nombreAffichables)) / ((editMenu->hauteur - editMenu->zoneMusique.hauteurBarreDefilement) / 1.0);
            editMenu->zoneMusique.ligneFin = editMenu->zoneMusique.ligneDebut + editMenu->zoneMusique.nombreAffichables;
            recreer_surface(editMenu);
         }

         if (editMenu->zonePlaylist.nombreAffichables < editMenu->zonePlaylist.nombreTotal && cliquer_deplacer(event, editMenu->zonePlaylist.barreDefilement, &editMenu->zonePlaylist.positionBarreDefilement, &editMenu->zonePlaylist.differenceAuClicBarreDefil, &editMenu->zonePlaylist.clicGaucheSurBarreDefil))
         {
            editMenu->zonePlaylist.positionBarreDefilement.x = editMenu->zonePlaylist.positionLigneSeparation.x;
            if (editMenu->zonePlaylist.positionBarreDefilement.y < 0)
                    editMenu->zonePlaylist.positionBarreDefilement.y = 0;
            if (editMenu->zonePlaylist.positionBarreDefilement.y > editMenu->hauteur - editMenu->zonePlaylist.hauteurBarreDefilement)
                    editMenu->zonePlaylist.positionBarreDefilement.y = editMenu->hauteur - editMenu->zonePlaylist.hauteurBarreDefilement;
            editMenu->zonePlaylist.ligneDebut = (editMenu->zonePlaylist.positionBarreDefilement.y * (editMenu->zonePlaylist.nombreTotal - editMenu->zonePlaylist.nombreAffichables)) / ((editMenu->hauteur - editMenu->zonePlaylist.hauteurBarreDefilement) / 1.0);
            editMenu->zonePlaylist.ligneFin = editMenu->zonePlaylist.ligneDebut + editMenu->zonePlaylist.nombreAffichables;
            recreer_surface(editMenu);
         }

         return 1;

}




void recreer_surface(MenuEditPlaylist *editMenu)
{
         SDL_Color couleurBlanc = {255, 255, 255};

         SDL_FillRect(editMenu->surfaceSauvegarde, NULL, SDL_MapRGB(editMenu->surfaceSauvegarde->format, 0, 0, 0));
         SDL_BlitSurface(editMenu->ligneSeparation, NULL, editMenu->surfaceSauvegarde, &editMenu->zoneDossier.positionLigneSeparation);
         SDL_BlitSurface(editMenu->ligneSeparation, NULL, editMenu->surfaceSauvegarde, &editMenu->zoneMusique.positionLigneSeparation);
         SDL_BlitSurface(editMenu->ligneSeparation, NULL, editMenu->surfaceSauvegarde, &editMenu->zonePlaylist.positionLigneSeparation);
         afficher_fichier_texte(editMenu->zoneDossier.nomFichierPlaylist, editMenu->zoneDossier.police, couleurBlanc, couleurBlanc, 2, 5, editMenu->zoneDossier.largeur - 5, editMenu->zoneDossier.ligneDebut, editMenu->zoneDossier.ligneFin, editMenu->zoneDossier.positionListe, editMenu->surfaceSauvegarde, editMenu->zoneDossier.tableauSurfaces, editMenu->zoneDossier.tableauPositions, editMenu->zoneDossier.tableauNoms);
         afficher_fichier_texte(editMenu->zoneMusique.nomFichierPlaylist, editMenu->zoneMusique.police, couleurBlanc, couleurBlanc, 2, 5, editMenu->zoneMusique.largeur - 5, editMenu->zoneMusique.ligneDebut, editMenu->zoneMusique.ligneFin, editMenu->zoneMusique.positionListe, editMenu->surfaceSauvegarde, editMenu->zoneMusique.tableauSurfaces, editMenu->zoneMusique.tableauPositions, editMenu->zoneMusique.tableauNoms);
         afficher_fichier_texte(editMenu->zonePlaylist.nomFichierPlaylist, editMenu->zonePlaylist.police, couleurBlanc, couleurBlanc, 2, 5, editMenu->zonePlaylist.largeur - 5, editMenu->zonePlaylist.ligneDebut, editMenu->zonePlaylist.ligneFin, editMenu->zonePlaylist.positionListe, editMenu->surfaceSauvegarde, editMenu->zonePlaylist.tableauSurfaces, editMenu->zonePlaylist.tableauPositions, editMenu->zonePlaylist.tableauNoms);
         SDL_BlitSurface(editMenu->zoneDossier.boutonDossier, NULL, editMenu->surfaceSauvegarde, &editMenu->zoneDossier.positionBoutonDossier);
         if (strstr(editMenu->zonePlaylist.nomFichierPlaylist, ".npl") != NULL)
         {
                SDL_BlitSurface(editMenu->zonePlaylist.boutonDossier, NULL, editMenu->surfaceSauvegarde, &editMenu->zonePlaylist.positionBoutonDossier);
                SDL_BlitSurface(editMenu->zonePlaylist.boutonTypePlaylist, NULL, editMenu->surfaceSauvegarde, &editMenu->zonePlaylist.positionBoutonTypePlaylist);
         }
         else SDL_BlitSurface(editMenu->nouvellePlaylist, NULL, editMenu->surfaceSauvegarde, &editMenu->positionNouvellePlaylist);
         SDL_BlitSurface(editMenu->posteTravail, NULL, editMenu->surfaceSauvegarde, &editMenu->positionPosteTravail);
         SDL_BlitSurface(editMenu->zoneMusique.boutonTypePlaylist, NULL, editMenu->surfaceSauvegarde, &editMenu->zoneMusique.positionBoutonTypePlaylist);

         recreer_barre_defil(editMenu);
         if (editMenu->zoneDossier.nombreAffichables < editMenu->zoneDossier.nombreTotal)
            SDL_BlitSurface(editMenu->zoneDossier.barreDefilement, NULL, editMenu->surfaceSauvegarde, &editMenu->zoneDossier.positionBarreDefilement);
         if (editMenu->zoneMusique.nombreAffichables < editMenu->zoneMusique.nombreTotal)
            SDL_BlitSurface(editMenu->zoneMusique.barreDefilement, NULL, editMenu->surfaceSauvegarde, &editMenu->zoneMusique.positionBarreDefilement);
         if (editMenu->zonePlaylist.nombreAffichables < editMenu->zonePlaylist.nombreTotal)
            SDL_BlitSurface(editMenu->zonePlaylist.barreDefilement, NULL, editMenu->surfaceSauvegarde, &editMenu->zonePlaylist.positionBarreDefilement);
}


void recreer_barre_defil(MenuEditPlaylist *editMenu)
{
         editMenu->zoneDossier.nombreTotal = compter_lignes(editMenu->zoneDossier.nomFichierPlaylist);
         if (editMenu->zoneDossier.nombreTotal == 0)
            editMenu->zoneDossier.nombreTotal++;
         editMenu->zoneDossier.hauteurBarreDefilement = (editMenu->zoneDossier.nombreAffichables / (editMenu->zoneDossier.nombreTotal / 1.0)) * editMenu->hauteur;
         editMenu->zoneDossier.barreDefilement = SDL_CreateRGBSurface(SDL_HWSURFACE, 26, editMenu->zoneDossier.hauteurBarreDefilement, 32, 0, 0, 0, 0);


         SDL_Surface *rectangle = NULL;
         rectangle = SDL_CreateRGBSurface(SDL_HWSURFACE, 1, editMenu->zoneDossier.hauteurBarreDefilement, 32, 0, 0, 0, 0);
         SDL_Rect position;
         position.x = 0;
         position.y = 0;

         int i;

         for (i = 0 ; i <= 12 ; i++)
         {
                SDL_FillRect(rectangle, NULL, SDL_MapRGB(editMenu->zoneDossier.barreDefilement->format, (255 * i) / 12, (255 * i) / 12, (255 * i) / 12));
                position.x = i;
                SDL_BlitSurface(rectangle, NULL, editMenu->zoneDossier.barreDefilement, &position);
         }
         for (i = 0 ; i <= 12 ; i++)
         {
                SDL_FillRect(rectangle, NULL, SDL_MapRGB(editMenu->zoneDossier.barreDefilement->format, 255 - (255 * i) / 12, 255 - (255 * i) / 12, 255 - (255 * i) / 12));
                position.x = i + 13;
                SDL_BlitSurface(rectangle, NULL, editMenu->zoneDossier.barreDefilement, &position);
         }


         editMenu->zoneMusique.nombreTotal = compter_lignes(editMenu->zoneMusique.nomFichierPlaylist);
         if (editMenu->zoneMusique.nombreTotal == 0)
            editMenu->zoneMusique.nombreTotal++;
         editMenu->zoneMusique.hauteurBarreDefilement = (editMenu->zoneMusique.nombreAffichables / (editMenu->zoneMusique.nombreTotal / 1.0)) * editMenu->hauteur;
         editMenu->zoneMusique.barreDefilement = SDL_CreateRGBSurface(SDL_HWSURFACE, 26, editMenu->zoneMusique.hauteurBarreDefilement, 32, 0, 0, 0, 0);


         SDL_FreeSurface(rectangle);
         rectangle = SDL_CreateRGBSurface(SDL_HWSURFACE, 1, editMenu->zoneMusique.hauteurBarreDefilement, 32, 0, 0, 0, 0);
         position.y = 0;

         for (i = 0 ; i <= 12 ; i++)
         {
                SDL_FillRect(rectangle, NULL, SDL_MapRGB(editMenu->zoneMusique.barreDefilement->format, (255 * i) / 12, (255 * i) / 12, (255 * i) / 12));
                position.x = i;
                SDL_BlitSurface(rectangle, NULL, editMenu->zoneMusique.barreDefilement, &position);
         }
         for (i = 0 ; i <= 12 ; i++)
         {
                SDL_FillRect(rectangle, NULL, SDL_MapRGB(editMenu->zoneMusique.barreDefilement->format, 255 - (255 * i) / 12, 255 - (255 * i) / 12, 255 - (255 * i) / 12));
                position.x = i + 13;
                SDL_BlitSurface(rectangle, NULL, editMenu->zoneMusique.barreDefilement, &position);
         }


         editMenu->zonePlaylist.nombreTotal = compter_lignes(editMenu->zonePlaylist.nomFichierPlaylist);
         if (editMenu->zonePlaylist.nombreTotal == 0)
            editMenu->zonePlaylist.nombreTotal++;
         editMenu->zonePlaylist.hauteurBarreDefilement = (editMenu->zonePlaylist.nombreAffichables / (editMenu->zonePlaylist.nombreTotal / 1.0)) * editMenu->hauteur;
         editMenu->zonePlaylist.barreDefilement = SDL_CreateRGBSurface(SDL_HWSURFACE, 26, editMenu->zonePlaylist.hauteurBarreDefilement, 32, 0, 0, 0, 0);


         SDL_FreeSurface(rectangle);
         rectangle = SDL_CreateRGBSurface(SDL_HWSURFACE, 1, editMenu->zonePlaylist.hauteurBarreDefilement, 32, 0, 0, 0, 0);
         position.y = 0;

         for (i = 0 ; i <= 12 ; i++)
         {
                SDL_FillRect(rectangle, NULL, SDL_MapRGB(editMenu->zonePlaylist.barreDefilement->format, (255 * i) / 12, (255 * i) / 12, (255 * i) / 12));
                position.x = i;
                SDL_BlitSurface(rectangle, NULL, editMenu->zonePlaylist.barreDefilement, &position);
         }
         for (i = 0 ; i <= 12 ; i++)
         {
                SDL_FillRect(rectangle, NULL, SDL_MapRGB(editMenu->zonePlaylist.barreDefilement->format, 255 - (255 * i) / 12, 255 - (255 * i) / 12, 255 - (255 * i) / 12));
                position.x = i + 13;
                SDL_BlitSurface(rectangle, NULL, editMenu->zonePlaylist.barreDefilement, &position);
         }

         SDL_FreeSurface(rectangle);

         editMenu->zoneDossier.positionBarreDefilement.x = 0;
         editMenu->zoneMusique.positionBarreDefilement.x = editMenu->zoneMusique.positionLigneSeparation.x;
         editMenu->zonePlaylist.positionBarreDefilement.x = editMenu->zonePlaylist.positionLigneSeparation.x;
         editMenu->zoneDossier.positionBarreDefilement.y  = editMenu->zoneDossier.ligneDebut / ((editMenu->zoneDossier.nombreTotal - editMenu->zoneDossier.nombreAffichables) / ((editMenu->hauteur - editMenu->zoneDossier.hauteurBarreDefilement) / 1.0));
         editMenu->zoneMusique.positionBarreDefilement.y  = editMenu->zoneMusique.ligneDebut / ((editMenu->zoneMusique.nombreTotal - editMenu->zoneMusique.nombreAffichables) / ((editMenu->hauteur - editMenu->zoneMusique.hauteurBarreDefilement) / 1.0));
         editMenu->zonePlaylist.positionBarreDefilement.y  = editMenu->zonePlaylist.ligneDebut / ((editMenu->zonePlaylist.nombreTotal - editMenu->zonePlaylist.nombreAffichables) / ((editMenu->hauteur - editMenu->zonePlaylist.hauteurBarreDefilement) / 1.0));
}


