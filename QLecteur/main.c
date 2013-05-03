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



//NE SURTOUT PAS MODIFIER L'ORDRE D'INCLUSION
#include "fonctions_registre.h"
#include "fonctions_texte.h"
#include "main.h"
#include "fonctions_repertoire.h"
#include "edition_playlist.h"
#include "calculs_temps.h"
#include "conversions.h"
#include "fonctions_fichier.h"
#include "fonctions_maths.h"
#include "fonctions_souris.h"
#include "fonctions_wav.h"
#include "resize_image.h"

char dossierBase[TAILLE_MAX_NOM] = "\0",
     nomBase[TAILLE_MAX_NOM] = "\0";

float *spectre = NULL;   //pointeur vers float
MenuConfigLecteur *copieConfigMenu;
int passerAuSuivant = 0, passerAuPrecedent = 0, mettreEnPlayOuPause = 0, mettreEnStop = 0;
HWND mainWnd = NULL;
void *sharedAuxMemory = NULL;
SDL_Surface *surfaceSMPEG = NULL;
SDL_Rect positionSurfaceSMPEG;
int numeroVisu = 0, secondsSkipped = 0;
int bigScreenMode = 0, tempsDernierResize = 0;
float agrandissement = 1.0;


int main(int argc, char *argv[])
{
    //vérification de l'existence d'une éventuelle zone de mémoire partagée
    int mustClose = 0;
    HWND *targetWnd = NULL;

    HANDLE handle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "qlecshared");
    if (handle != NULL)
    {
        targetWnd = (HWND*) MapViewOfFile(handle, FILE_MAP_READ, 0, 0, sizeof(HWND));
        if (IsWindow(*targetWnd) && argc <= 1)
        {
           CloseHandle(handle);
           ShowWindow(*targetWnd, SW_RESTORE);
           SetForegroundWindow(*targetWnd);
           UnmapViewOfFile(targetWnd);
           exit(EXIT_SUCCESS);
        }

        UnmapViewOfFile(targetWnd);
        targetWnd = NULL;
        mustClose = 1;
    }

    //création de la zone de mémoire partagée
    if (handle == NULL)
       handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(HWND) + (TAILLE_MAX_NOM * 500), "qlecshared");
    if (handle == NULL)
    {
       if (MessageBox(NULL, "Impossible de créer la mémoire partagée (1). Continuer ?", "Erreur ! ", MB_YESNO | MB_ICONERROR) == IDNO)
          exit(EXIT_FAILURE);
    }

    void *sharedMemory = MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(HWND) + (TAILLE_MAX_NOM * 500));
    if (sharedMemory == NULL)
    {
      if (MessageBox(NULL, "Impossible de créer la mémoire partagée (2). Continuer ?", "Erreur ! ", MB_YESNO | MB_ICONERROR) == IDNO)
      {
          CloseHandle(handle);
          exit(EXIT_FAILURE);
      }
    }

    memset(sharedMemory, '\0', sizeof(HWND) + (TAILLE_MAX_NOM * 500));


    int i;
    strcpy(dossierBase, argv[0]);

    if (strrchr(dossierBase, '\\') != NULL)  // en cas de lancement normal : "adresse complète de l'application" "paramètres"
       *(strrchr(dossierBase, '\\')) = '\0';
    chdir(dossierBase);  //rétablissement du dossier de lancement de l'application

    if (strrchr(argv[0], '\\') != NULL)
       strcpy(nomBase, strrchr(argv[0], '\\') + 1);  //rétablissement du nom de l'application


    char *adressChar = NULL;
    if (argc > 1)  //si des paramètres ont été envoyés au programme
    {
             for (i = 0 ; i < 500 ; i++)
             {
                 adressChar = (sharedMemory + sizeof(HWND) + (i * TAILLE_MAX_NOM));
                 if (*adressChar == '\0')
                    break;
             }
             if (*adressChar != '\0')
                exit(EXIT_FAILURE);

             FILE *fichier = NULL;
             for (i = 1 ; i < argc ; i++)
             {
                 if (strstr(argv[i], ".list") == NULL)
                 {
                     if ((fichier = fopen("playlists\\Originelle.list", "a+")) == NULL)
                     {
                         fprintf(stderr, "ERR main %d : Impossible d'ouvrir le fichier playlists\\Originelle.list.\n", __LINE__);
                         exit(EXIT_FAILURE);
                     }
                     else
                     {
                         fprintf(fichier, "%s\n", argv[i]);  //ecriture des paramètres envoyés, en ajout sur la playlist
                         fclose(fichier);
                         creer_playlist("playlists\\Originelle.list", "playlists\\Originelle.npl", TITRE);
                         if (mustClose)
                         {
                             strcpy(adressChar, "playlists\\Originelle.list");
                             if (i == argc - 1)
                                exit(EXIT_SUCCESS);
                         }
                     }
                 }
                 else if (mustClose)
                 {
                     strcpy(adressChar, argv[i]);
                     if (i == argc - 1)
                        exit(EXIT_SUCCESS);
                 }
             }
    }




    SDL_Surface *ecran = NULL;
    SDL_Event event;
    Musique musique;
    TypePlaylist typePlaylist = TITRE;
    SDL_Rect positionZero;
    positionZero.x = 0;
    positionZero.y = 0;

    musique.surface.nom = NULL;
    musique.surface.auteur = NULL;
    musique.surface.curseur = NULL;
    musique.surface.debit = NULL;
    musique.surface.frequence = NULL;
    musique.surface.poids = NULL;
    musique.surface.longueur = NULL;
    musique.surface.image = NULL;


    srand(time(NULL));
    int noMusic = 0;

    char nomFichierListe[TAILLE_MAX_NOM] = "playlists\\Dossier musiques.list";
    if (lister_extension("musiques", nomFichierListe, ".mp3 ; .url ; .wma ; .wav ; .aac ; .ogg ; .mpg", "w") == 0)
          noMusic = 1;

    char nomFichierListePlaylists[TAILLE_MAX_NOM] = "playlists\\Liste_playlists.spl";

    char nomPlaylist[TAILLE_MAX_NOM] = " ";
    char *nomPlaylist2 = NULL;
    nomPlaylist2 = malloc(sizeof(char) * (TAILLE_MAX_NOM + 1));
    char nomPlaylist3[TAILLE_MAX_NOM] = " ";
    char *positionExtension = NULL;
    char *positionEntree = NULL;

    lister_extension("playlists", nomFichierListePlaylists, ".wpl", "w");

    FILE *fichierListePlaylists = NULL;
    if ((fichierListePlaylists = fopen(nomFichierListePlaylists, "r")) == NULL)
       fprintf(stderr, "ERR main %d : Le fichier %s n'a pas pu etre ouvert.\n", __LINE__, nomFichierListePlaylists);
    else
    {
        while(fgets(nomPlaylist, TAILLE_MAX_NOM, fichierListePlaylists) != NULL)
        {
                                 if ((positionEntree = strchr(nomPlaylist, '\n')) != NULL)
                                    *positionEntree = '\0';
                                 strcpy(nomPlaylist2, nomPlaylist);
                                 positionExtension = strstr(nomPlaylist2, ".wpl");
                                 if (positionExtension != NULL)
                                 {
                                    strcpy(positionExtension, ".list");
                                    convertir_wpl_en_list(nomPlaylist, nomPlaylist2);
                                 }
        }
        while(fclose(fichierListePlaylists) == EOF);
    }

    lister_extension("playlists", nomFichierListePlaylists, ".m3u", "w");
    if ((fichierListePlaylists = fopen(nomFichierListePlaylists, "r")) == NULL)
       fprintf(stderr, "ERR main %d : Le fichier %s n'a pas pu etre ouvert.\n", __LINE__, nomFichierListePlaylists);
    else
    {
        while(fgets(nomPlaylist, TAILLE_MAX_NOM, fichierListePlaylists) != NULL)
        {
                                 if ((positionEntree = strchr(nomPlaylist, '\n')) != NULL)
                                    *positionEntree = '\0';
                                 strcpy(nomPlaylist2, nomPlaylist);
                                 if ((positionExtension = strstr(nomPlaylist2, ".m3u")) != NULL)
                                 {
                                    strcpy(positionExtension, ".list");
                                    convertir_m3u_en_list(nomPlaylist, nomPlaylist2);
                                 }
        }
        while(fclose(fichierListePlaylists) == EOF);
    }

    char nomFichierPlaylist[TAILLE_MAX_NOM] = "playlists\\Dossier musiques.npl";
    char nomFichierPlaylistDePlaylists[TAILLE_MAX_NOM] = "playlists\\Playlist_playlist.plpl";
    creer_playlist(nomFichierListe, nomFichierPlaylist, typePlaylist);
    classer_alphabetique_fichier_texte(nomFichierPlaylist, TAILLE_MAX_NOM, nomFichierListe);
    lister_extension("playlists", nomFichierListePlaylists, ".npl", "w");
    creer_playlist(nomFichierListePlaylists, nomFichierPlaylistDePlaylists, NOM);

    char nomListe[TAILLE_MAX_NOM] = " ";
    lister_extension("playlists", "playlists\\temp.tmp", ".list", "w");
    FILE *fichierTemp = NULL;
    if ((fichierTemp = fopen("playlists\\temp.tmp", "r")) == NULL)
       fprintf(stderr, "ERR main %d : Impossible d'ouvrir le fichier playlists\\temp.tmp", __LINE__);
    else
    {
        while(fgets(nomListe, TAILLE_MAX_NOM, fichierTemp) != NULL)
        {
                                 if ((positionEntree = strchr(nomListe, '\n')) != NULL)
                                    *positionEntree = '\0';
                                 if (strcmp(nomListe, nomFichierListe) != 0 && strcmp(nomListe, nomFichierListePlaylists) != 0)
                                 {
                                     strcpy(nomPlaylist2, nomListe);
                                     if ((positionExtension = strstr(nomPlaylist2, ".list")) != NULL)
                                     {
                                        strcpy(positionExtension, ".npl");
                                        creer_playlist(nomListe, nomPlaylist2, typePlaylist);
                                     }
                                 }
        }
        while(fclose(fichierTemp) == EOF);
        remove("playlists\\temp.tmp");
    }



    //webradios
    if (lister_extension("radios", "playlists\\Radios.list", ".pls ; .asx ; .url", "w") == 0)
    {
       FILE *fichierNRJ = NULL;
       if ((fichierNRJ = fopen("radios\\NRJ.pls", "w")) == NULL)
          fprintf(stderr, "ERR main %d : Le fichier NRJ.pls n'a pas pu etre créé.\n", __LINE__);
       else
       {
           fprintf(fichierNRJ, "[playlist]\nnumberofentries=1\nFile1=http://80.236.126.2:708\nTitle1=(#1 - 13/100) NRJ\nLength1=-1\nVersion=2\n");
           while(fclose(fichierNRJ) == EOF);
       }

       FILE *fichierSkyrock = NULL;
       if ((fichierSkyrock = fopen("radios\\Skyrock.pls", "w")) == NULL)
          fprintf(stderr, "ERR main %d : Le fichier Skyrock.pls n'a pas pu etre créé.\n", __LINE__);
       else
       {
           fprintf(fichierSkyrock, "[playlist]\nnumberofentries=1\nFile1=http://80.236.126.2:710\nTitle1=(#1 - 6/100) SKYROCK\nLength1=-1\nVersion=2\n");
           while(fclose(fichierSkyrock) == EOF);
       }

       FILE *fichierFunRadio = NULL;
       char chaineFunRadio[1000] = " ";
       char *positionPointInterrogation = NULL;
       if ((fichierFunRadio = fopen("radios\\FunRadio.asx", "w")) == NULL)
          fprintf(stderr, "ERR main %d : Le fichier FunRadio.asx n'a pas pu etre créé.\n", __LINE__);
       else
       {
           sprintf(chaineFunRadio, "<ASX version=?3.0?> \n\n<entry> \n<ref href = ?http://streaming.radio.funradio.fr:80/fun-1-44-96?/> \n<Abstract>Funradio 96kbps</abstract> \n<Copyright>Funradio</Copyright>\n<Title>Funradio</title>\n</entry>\n\n</ASX>\n\n\n\n");
           while ((positionPointInterrogation = strchr(chaineFunRadio, '?')) != NULL)
                 *positionPointInterrogation = '"';
           fprintf(fichierFunRadio, "%s", chaineFunRadio);
           while(fclose(fichierFunRadio) == EOF);
       }

       FILE *fichierVoltage = NULL;
       if ((fichierVoltage = fopen("radios\\Voltage.url", "w")) == NULL)
          fprintf(stderr, "ERR main %d : Le fichier Voltage.url n'a pas pu etre créé.\n", __LINE__);
       else
       {
           fprintf(fichierVoltage, "[DEFAULT]\nBASEURL=http://broadcast.infomaniak.net/start-voltage-high.mp3\n[InternetShortcut]\nURL=http://broadcast.infomaniak.net/start-voltage-high.mp3");
           while(fclose(fichierVoltage) == EOF);
       }

    }

    lister_extension("radios", "playlists\\Radios.list", ".pls ; .asx ; .url", "w");
    creer_playlist("playlists\\Radios.list", "playlists\\Radios.npl", typePlaylist);
    musique.type = NORMAL;


    if (noMusic)
    {
       remove(nomFichierPlaylist);
       strcpy(nomFichierListe, "playlists\\Radios.list");
       strcpy(nomFichierPlaylist, "playlists\\Radios.npl");
       classer_alphabetique_fichier_texte(nomFichierPlaylist, TAILLE_MAX_NOM, nomFichierListe);
    }


    int numeroRecord = lister_extension("records", "playlists\\Records.list", "Enregistrement", "w");
    numeroRecord++;
    lister_extension("records", "playlists\\Records.list", ".wav", "w");
    creer_playlist("playlists\\Records.list", "playlists\\Records.npl", typePlaylist);


    FILE *saveFile = NULL;
    int tempsAMettre = 0;
    musique.numero = -1;
    char parametre[TAILLE_MAX_NOM] = "\0";

    if (argc > 1 && strstr(argv[1], ".list") == NULL)
    {
             strcpy(nomFichierListe, "playlists\\Originelle.list");
             strcpy(nomFichierPlaylist, "playlists\\Originelle.npl");
    }
    else if (argc > 1 && strstr(argv[1], ".list") != NULL)
    {
         strcpy(nomFichierListe, argv[1]);
         strcpy(nomFichierPlaylist, argv[1]);
         strcpy(strstr(nomFichierPlaylist, ".list"), ".npl");
         creer_playlist(nomFichierListe, nomFichierPlaylist, TITRE);
    }
    else
    {
        if ((saveFile = fopen("saveFile.sf", "r")) == NULL)
           fprintf(stderr, "ERR main %d : Le fichier de sauvegarde n'a pas pu etre ouvert.\n", __LINE__);
        else
        {

            while (fgets(parametre, 1000, saveFile) != NULL && strcmp(parametre, "[saved]\n") != 0);
            if (strcmp(parametre, "[saved]\n") == 0)
            {
                              fgets(parametre, 1000, saveFile);
                              fgets(parametre, 1000, saveFile);
                              fgets(parametre, 1000, saveFile);
                              strcpy(nomFichierListe, parametre);
                              if ((positionEntree = strchr(nomFichierListe, '\n')) != NULL)
                                 *positionEntree = '\0';

                              fgets(parametre, 1000, saveFile);
                              strcpy(nomFichierPlaylist, parametre);
                              if ((positionEntree = strchr(nomFichierPlaylist, '\n')) != NULL)
                                 *positionEntree = '\0';

                              fgets(parametre, 1000, saveFile);
                              musique.numero = strtol(parametre, NULL, 10);

                              if (test_exist(nomFichierListe) == 0 || test_exist(nomFichierPlaylist) == 0 || test_vide(nomFichierListe) == 0 || test_vide(nomFichierPlaylist) == 0)
                              {
                                 remove(nomFichierPlaylist);
                                 remove(nomFichierListe);
                                 lister_extension("playlists", nomFichierListePlaylists, ".npl", "w");
                                 creer_playlist(nomFichierListePlaylists, nomFichierPlaylistDePlaylists, typePlaylist);
                                 strcpy(nomFichierListe, "playlists\\Radios.list");
                                 strcpy(nomFichierPlaylist, "playlists\\Radios.npl");
                              }
            }
        }
    }

    int modeListe = 0;   // 0 = musique ; 1 = playlist

    FILE *fichierPlaylist = NULL;
    if ((fichierPlaylist = fopen(nomFichierPlaylist, "r")) == NULL)
       fprintf(stderr, "ERR main %d : Le fichier %s n'a pas pu etre ouvert.\n", __LINE__, nomFichierPlaylist);

    int nombreMusiques = 0;
    char temp[TAILLE_MAX_NOM] = " ";
    while(fgets(temp, TAILLE_MAX_NOM, fichierPlaylist) != NULL)
                      nombreMusiques++;

    while(fclose(fichierPlaylist) == EOF);

    FILE *fichierPlaylistDePlaylists = NULL;



    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == -1) // Démarrage de la SDL avec timer. Si erreur alors...
    {
        fprintf(stderr, "ERR main %d : Erreur d'initialisation de la SDL : %s\n", __LINE__, SDL_GetError());
        exit(EXIT_FAILURE);
    }

    if(TTF_Init() == -1) // Démarrage de SDL_ttf. Si erreur alors...
    {
                  fprintf(stderr, "ERR main %d : Erreur d'initialisation de TTF_Init : %s\n", __LINE__, TTF_GetError());
                  exit(EXIT_FAILURE);
    }

    if (FSOUND_Init(44100, 32, FSOUND_INIT_ACCURATEVULEVELS) == 0)  // Initialisation de FMOD. Si erreur alors...
    {
        fprintf(stderr, "ERR main %d : Erreur d'initialisation de FMOD.\n", __LINE__);
        exit(EXIT_FAILURE);
    }


    SDL_WM_SetIcon(SDL_LoadBMP("img\\other\\sdl_icone.bmp"), NULL);

    ecran = SDL_SetVideoMode(900, 660, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_RESIZABLE);
    if (ecran == NULL)
    {
        fprintf(stderr, "ERR main %d : Impossible de charger le mode vidéo : %s\n", __LINE__, SDL_GetError());
        exit(EXIT_FAILURE);
    }

    musique.surface.blit = ecran;
    SDL_WM_SetCaption("Prog0001;ID=\"QLecteur\"", NULL);

    mainWnd = FindWindow(NULL, "Prog0001;ID=\"QLecteur\"");
    CopyMemory(sharedMemory, &mainWnd, sizeof(HWND));

    SetWindowText(mainWnd, "CokieForever");

    int largeurZoneReglage = 100;
    int largeurZoneListe = 200;
    int largeurZoneGraphique = ecran->w - largeurZoneReglage - largeurZoneListe;
    int hauteurZonePlayPause = 100;
    int hauteurZoneLecteur = ecran->h - (hauteurZonePlayPause + 25);
    int hauteurZoneBoutons = 15 * 2 + 25;
    int hauteurZoneGraphique = hauteurZoneLecteur - hauteurZoneBoutons;



    //Dans la zone reglage
    //Réglage de volume par bouton
    SDL_Surface *boutonVolume = NULL;
    boutonVolume = IMG_Load ("img\\other\\bouton_reglage_volume.jpg");
    SDL_Surface *ligneVolume = NULL;
    ligneVolume = IMG_Load ("img\\other\\ligne_volume_reduite.bmp");

    SDL_Rect positionBoutonVolumeGauche;
    positionBoutonVolumeGauche.x = (largeurZoneReglage - (boutonVolume->w * 2 + 10)) / 2;
    positionBoutonVolumeGauche.y = 0;
    int abscisseBoutonVolumeGauche = positionBoutonVolumeGauche.x;

    SDL_Rect positionLigneVolumeGauche;
    positionLigneVolumeGauche.x = (boutonVolume->w - ligneVolume->w) / 2 + positionBoutonVolumeGauche.x;
    positionLigneVolumeGauche.y = 100;
    positionBoutonVolumeGauche.y = (ligneVolume->h - boutonVolume->h) / 2 + positionLigneVolumeGauche.y;

    SDL_Rect positionBoutonVolumeDroit;
    positionBoutonVolumeDroit.x = abscisseBoutonVolumeGauche + boutonVolume->w + 10;
    positionBoutonVolumeDroit.y = positionBoutonVolumeGauche.y;
    int abscisseBoutonVolumeDroit = positionBoutonVolumeDroit.x;

    SDL_Rect positionLigneVolumeDroit;
    positionLigneVolumeDroit.x = (boutonVolume->w - ligneVolume->w) / 2 + positionBoutonVolumeDroit.x;
    positionLigneVolumeDroit.y = positionLigneVolumeGauche.y;

    SDL_BlitSurface(ligneVolume, NULL, ecran, &positionLigneVolumeDroit);
    SDL_BlitSurface(ligneVolume, NULL, ecran, &positionLigneVolumeGauche);
    SDL_BlitSurface(boutonVolume, NULL, ecran, &positionBoutonVolumeDroit);
    SDL_BlitSurface(boutonVolume, NULL, ecran, &positionBoutonVolumeGauche);

    int volume = 128, volumeDroit = 128, volumeGauche = 128;

    //Mono - stéréo
    int indicStereoMono = 1;   // 1 = stéréo, 0 = mono
    TTF_Font *policeIndicMonoStereo = NULL;
    policeIndicMonoStereo = TTF_OpenFont("fonts\\sdigit.ttf", 20);
    SDL_Surface *surfaceIndicStereoMono = NULL;
    SDL_Color couleurJaune = {255, 255, 0};
    surfaceIndicStereoMono = TTF_RenderText_Blended(policeIndicMonoStereo, "Stereo", couleurJaune);
    SDL_Rect positionIndicStereoMono;
    positionIndicStereoMono.x = (largeurZoneReglage - surfaceIndicStereoMono->w) / 2;
    positionIndicStereoMono.y = 10;

    SDL_BlitSurface(surfaceIndicStereoMono, NULL, ecran, &positionIndicStereoMono);

    //ligne de séparation
    SDL_Surface *ligneSeparation = NULL;
    SDL_Rect positionLigneSeparation;
    positionLigneSeparation.x = largeurZoneReglage;
    positionLigneSeparation.y = 0;
    ligneSeparation = IMG_Load("img\\other\\ligne_separation_reduite.jpg");

    SDL_BlitSurface(ligneSeparation, NULL, ecran, &positionLigneSeparation);


    //affichage du temps écoulé (chanson)
    musique.surface.curseur = NULL;
    musique.curseur = 0;
    int minutesMusiqueEcoulees = musique.curseur / 60;
    int secondesMusiqueEcoulees = musique.curseur % 60;
    char texteTempsMusiqueEcoule[10] = "00:00";
    TTF_Font *policeTempsMusiqueEcoule = NULL;
    policeTempsMusiqueEcoule = TTF_OpenFont("fonts\\sdigit.ttf", 20);
    SDL_Color couleurRouge = {255, 0, 0};
    musique.surface.curseur = TTF_RenderText_Blended(policeTempsMusiqueEcoule, texteTempsMusiqueEcoule, couleurRouge);

    musique.position.curseur.x = (largeurZoneReglage - musique.surface.curseur->w) / 2;
    musique.position.curseur.y = 40;

    SDL_BlitSurface(musique.surface.curseur, NULL, ecran, &musique.position.curseur);


    //indicateur de verrouillage
    SDL_Surface *surfaceLockIndic = NULL;
    int lockIndic = 1;  //1 = verrouillé, 0 = déverrouillé
    TTF_Font *policeLockIndic = NULL;
    policeLockIndic = TTF_OpenFont("fonts\\sdigit.ttf", 10);
    SDL_Color couleurVert = {0, 255, 0};
    surfaceLockIndic = TTF_RenderText_Blended(policeLockIndic, "Locked", couleurRouge);


    //bouton d'ajustement audio droite-gauche
    SDL_Surface *boutonAjustementAudio = NULL;
    TTF_Font *policeBoutonAjustementAudio = NULL;
    policeBoutonAjustementAudio = TTF_OpenFont("fonts\\sdigit.ttf", 10);
    boutonAjustementAudio = TTF_RenderText_Blended(policeBoutonAjustementAudio, "Adjust", couleurJaune);

    SDL_Rect positionLockIndic;
    positionLockIndic.x = (largeurZoneReglage - (surfaceLockIndic->w + boutonAjustementAudio->w + 10)) / 2;
    positionLockIndic.y = 400;

    SDL_Rect positionBoutonAjustementAudio;
    positionBoutonAjustementAudio.x = positionLockIndic.x + surfaceLockIndic->w + 10;
    positionBoutonAjustementAudio.y = positionLockIndic.y;

    SDL_BlitSurface(surfaceLockIndic, NULL, ecran, &positionLockIndic);
    SDL_BlitSurface(boutonAjustementAudio, NULL, ecran, &positionBoutonAjustementAudio);


    //affichage de la longueur de la piste
    musique.surface.longueur = NULL;
    musique.longueur = 0;
    char texteLongueurPiste[10] = "00:00";
    TTF_Font *policeLongueurPiste = NULL;
    policeLongueurPiste = TTF_OpenFont("fonts\\sdigit.ttf", 20);
    SDL_Color couleurRose = {255, 0, 100};
    musique.surface.longueur = TTF_RenderText_Blended(policeLongueurPiste, texteLongueurPiste, couleurRose);

    musique.position.longueur.x = (largeurZoneReglage - musique.surface.longueur->w) / 2;
    musique.position.longueur.y = positionLockIndic.y + 20;

    SDL_BlitSurface(musique.surface.longueur, NULL, ecran, &musique.position.longueur);


    //affichage du poids du fichier
    musique.surface.poids = NULL;
    musique.poids = 0;
    char textePoidsFichier[10] = "0.00 Mo";
    TTF_Font *policePoidsFichier = NULL;
    policePoidsFichier = TTF_OpenFont("fonts\\sdigit.ttf", 20);
    SDL_Color couleurBleuClair = {0, 100, 255};
    musique.surface.poids = TTF_RenderText_Blended(policePoidsFichier, textePoidsFichier, couleurBleuClair);

    musique.position.poids.x = (largeurZoneReglage - musique.surface.poids->w) / 2;
    musique.position.poids.y = musique.position.longueur.y + 20;

    SDL_BlitSurface(musique.surface.poids, NULL, ecran, &musique.position.poids);


    //affichage de la fréquence du canal
    musique.surface.frequence = NULL;
    char texteFrequenceCanal[10] = "00.0 kHz";
    TTF_Font *policeFrequenceCanal = NULL;
    policeFrequenceCanal = TTF_OpenFont("fonts\\sdigit.ttf", 20);
    musique.surface.frequence = TTF_RenderText_Blended(policeFrequenceCanal, texteFrequenceCanal, couleurVert);

    musique.position.frequence.x = (largeurZoneReglage - musique.surface.frequence->w) / 2;
    musique.position.frequence.y = musique.position.poids.y + 20;

    SDL_BlitSurface(musique.surface.frequence, NULL, ecran, &musique.position.frequence);


    //affichage du format musique
    musique.surface.format = NULL;
    strcpy(musique.format, "...");
    TTF_Font *policeFormatMusique = NULL;
    policeFormatMusique = TTF_OpenFont("fonts\\sdigit.ttf", 20);
    musique.surface.format = TTF_RenderText_Blended(policeFormatMusique, musique.format, couleurJaune);

    musique.position.format.x = (largeurZoneReglage - musique.surface.format->w) / 2;
    musique.position.format.y = musique.position.frequence.y + 35;

    SDL_BlitSurface(musique.surface.format, NULL, ecran, &musique.position.format);



    //Dans la zone Liste
    //ligne de séparation
    SDL_Surface *ligneSeparation3 = NULL;
    ligneSeparation3 = IMG_Load("img\\other\\ligne_separation_3.jpg");

    SDL_Rect positionLigneSeparation3;
    positionLigneSeparation3.x = ecran->w - largeurZoneListe;
    positionLigneSeparation3.y = 0;
    int abscisseLigneSeparation3 = positionLigneSeparation3.x;

    SDL_BlitSurface(ligneSeparation3, NULL, ecran, &positionLigneSeparation3);


    //playlist
    int ligneDebut = 0;
    int ligneFin = 5000;

    SDL_Surface *tableauSurfacesPlaylist[TAILLE_MAX_PLAYLIST] = {NULL};
    SDL_Rect tableauPositionsPlaylist[TAILLE_MAX_PLAYLIST];
    char *tableauTextesPlaylist[TAILLE_MAX_PLAYLIST] = {NULL};
    for (i = 0 ; i < TAILLE_MAX_PLAYLIST ; i++)
    {
          tableauTextesPlaylist[i] = malloc(sizeof(char) * 200);
    }

    SDL_Rect positionPlaylist;
    positionPlaylist.x = ecran->w - largeurZoneListe + 30;
    positionPlaylist.y = 10;

    TTF_Font *policePlaylist = NULL;
    policePlaylist = TTF_OpenFont("fonts\\sdigit.ttf", 15);
    SDL_Color couleurBlanc = {255, 255, 255};

    afficher_fichier_texte(nomFichierPlaylist, policePlaylist, couleurBlanc, couleurBlanc, 2, 5, largeurZoneListe - 35, ligneDebut, ligneFin, positionPlaylist, ecran, tableauSurfacesPlaylist, tableauPositionsPlaylist, tableauTextesPlaylist);


    //bouton de changement de mode de liste
    SDL_Surface *boutonModeListe = NULL;
    boutonModeListe = IMG_Load("img\\other\\bouton_playlists.jpg");

    SDL_Rect positionBoutonModeListe;
    positionBoutonModeListe.x = ecran->w - largeurZoneListe + 25 + ((largeurZoneListe - 25 - boutonModeListe->w) / 2);
    positionBoutonModeListe.y = hauteurZoneLecteur - boutonModeListe->h - 10;

    SDL_BlitSurface(boutonModeListe, NULL, ecran, &positionBoutonModeListe);


    ligneFin = ((hauteurZoneLecteur - boutonModeListe->h - 20) / (tableauSurfacesPlaylist[0]->h + 5)) + ligneDebut;
    afficher_fichier_texte(nomFichierPlaylist, policePlaylist, couleurBlanc, couleurBlanc, 2, 5, largeurZoneListe - 35, ligneDebut, ligneFin, positionPlaylist, ecran, tableauSurfacesPlaylist, tableauPositionsPlaylist, tableauTextesPlaylist);
    int nombreMusiquesAffichables = ligneFin - ligneDebut;

    //barre de défilement
    SDL_Surface *barreDefilement = NULL;
    int hauteurBarreDefilement = (nombreMusiquesAffichables / (nombreMusiques / 1.0)) * hauteurZoneLecteur;
    barreDefilement = SDL_CreateRGBSurface(SDL_HWSURFACE, 26, hauteurBarreDefilement, 32, 0, 0, 0, 0);

    SDL_Surface *rectangle = NULL;
    rectangle = SDL_CreateRGBSurface(SDL_HWSURFACE, 1, hauteurBarreDefilement, 32, 0, 0, 0, 0);
    SDL_Rect position;
    position.x = 0;
    position.y = 0;

    for (i = 0 ; i <= 12 ; i++)
    {
        SDL_FillRect(rectangle, NULL, SDL_MapRGB(ecran->format, (255 * i) / 12, (255 * i) / 12, (255 * i) / 12));
        position.x = i;
        SDL_BlitSurface(rectangle, NULL, barreDefilement, &position);
    }
    for (i = 0 ; i <= 12 ; i++)
    {
        SDL_FillRect(rectangle, NULL, SDL_MapRGB(ecran->format, 255 - (255 * i) / 12, 255 - (255 * i) / 12, 255 - (255 * i) / 12));
        position.x = i + 13;
        SDL_BlitSurface(rectangle, NULL, barreDefilement, &position);
    }

    SDL_Rect positionBarreDefilement;
    positionBarreDefilement.x = positionLigneSeparation3.x;
    positionBarreDefilement.y = 0;

    if (nombreMusiques > nombreMusiquesAffichables  && (musique.numero < ligneDebut || musique.numero > ligneFin) && musique.numero > 0)
    {
       ligneDebut = musique.numero;
       ligneFin = musique.numero + nombreMusiquesAffichables;
       positionBarreDefilement.y = (ligneDebut * (hauteurZoneLecteur - hauteurBarreDefilement)) / ((nombreMusiques - nombreMusiquesAffichables) / 1.0);
    }

    if (nombreMusiquesAffichables < nombreMusiques)
       SDL_BlitSurface(barreDefilement, NULL, ecran, &positionBarreDefilement);


    // bouton de classement alphabétique
    SDL_Surface *boutonAlpha = NULL;
    TTF_Font *policeAlpha = NULL;
    policeAlpha = TTF_OpenFont("fonts\\sdigit.ttf", 15);
    SDL_Color couleurVertBizarre = {128, 255, 0};
    boutonAlpha = TTF_RenderText_Blended(policeAlpha, "AbC", couleurVertBizarre);

    SDL_Rect positionBoutonAlpha;
    positionBoutonAlpha.x = ecran->w - largeurZoneListe + 25 + (((positionBoutonModeListe.x - (ecran->w - largeurZoneListe + 25)) - boutonAlpha->w) / 2);
    positionBoutonAlpha.y = positionBoutonModeListe.y + ((boutonModeListe->h - boutonAlpha->h) / 2);

    SDL_BlitSurface(boutonAlpha, NULL, ecran, &positionBoutonAlpha);


    // bouton de changement de type playlist
    SDL_Surface *boutonTypePlaylist = NULL;
    TTF_Font *policeBoutonTypePlaylist = NULL;
    policeBoutonTypePlaylist = TTF_OpenFont("fonts\\sdigit.ttf", 15);
    boutonTypePlaylist = TTF_RenderText_Blended(policeBoutonTypePlaylist, "TtL", couleurVertBizarre);

    SDL_Rect positionBoutonTypePlaylist;
    positionBoutonTypePlaylist.x = positionBoutonModeListe.x + boutonModeListe->w + ((ecran->w - positionBoutonModeListe.x - boutonModeListe->w - boutonTypePlaylist->w) / 2);
    positionBoutonTypePlaylist.y = positionBoutonModeListe.y + ((boutonModeListe->h - boutonTypePlaylist->h) / 2);

    SDL_BlitSurface(boutonTypePlaylist, NULL, ecran, &positionBoutonTypePlaylist);



    //Dans la zone Play-Pause
    //surface Play-Pause
    SDL_Surface *surfaceZonePlayPause = NULL;
    surfaceZonePlayPause = SDL_CreateRGBSurface(SDL_HWSURFACE, ecran->w, hauteurZonePlayPause, 32, 0, 0, 0, 0);
    SDL_FillRect(surfaceZonePlayPause, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));

    SDL_Rect positionZonePlayPause;
    positionZonePlayPause.x = 0;
    positionZonePlayPause.y = hauteurZoneLecteur + 25;

    SDL_BlitSurface(surfaceZonePlayPause, NULL, ecran, &positionZonePlayPause);


    //Bouton Play/pause
    SDL_Surface *surfaceBoutonPlay = IMG_Load("img\\other\\bouton_play.jpg");
    SDL_Surface *surfaceBoutonPause = IMG_Load("img\\other\\bouton_pause.jpg");

    SDL_Surface *surfaceBoutonPlayPause = surfaceBoutonPlay;
    musique.mode = PLAY;

    SDL_Rect positionBoutonPlayPause;
    positionBoutonPlayPause.x = (ecran->w - surfaceBoutonPlayPause->w) / 2;
    positionBoutonPlayPause.y = hauteurZoneLecteur + 25 + ((hauteurZonePlayPause - surfaceBoutonPlayPause->h) / 2);

    SDL_BlitSurface(surfaceBoutonPlayPause, NULL, ecran, &positionBoutonPlayPause);


    //ligne de séparation
    SDL_Surface *ligneSeparation2 = NULL;
    ligneSeparation2 = IMG_Load("img\\other\\ligne_separation_2_reduite.jpg");

    SDL_Rect positionLigneSeparation2;
    positionLigneSeparation2.x = 0;
    positionLigneSeparation2.y = hauteurZoneLecteur;

    SDL_BlitSurface(ligneSeparation2, NULL, ecran, &positionLigneSeparation2);


    //indicateur de position dans la musique : "flèche lecture"
    SDL_Surface *flecheLecture = NULL;
    flecheLecture = IMG_Load("img\\other\\fleche_lecture_reduite.gif");

    SDL_Rect positionFlecheLecture;
    positionFlecheLecture.x = 0;
    positionFlecheLecture.y = hauteurZoneLecteur + 1;
    int ordonneeFlecheLecture = positionFlecheLecture.y;

    SDL_BlitSurface(flecheLecture, NULL, ecran, &positionFlecheLecture);


    //Bouton avance rapide
    SDL_Surface *boutonAvanceRapide = NULL;
    boutonAvanceRapide = IMG_Load("img\\other\\bouton_avance_rapide_reduit.jpg");

    SDL_Rect positionBoutonAvanceRapide;
    positionBoutonAvanceRapide.x = positionBoutonPlayPause.x + surfaceBoutonPlayPause->w + 30;
    positionBoutonAvanceRapide.y = hauteurZoneLecteur + 25 + ((hauteurZonePlayPause - boutonAvanceRapide->h) / 2);

    SDL_BlitSurface(boutonAvanceRapide, NULL, ecran, &positionBoutonAvanceRapide);


    //Bouton recul rapide
    SDL_Surface *boutonReculRapide = NULL;
    boutonReculRapide = IMG_Load("img\\other\\bouton_recul_rapide_reduit.jpg");

    SDL_Rect positionBoutonReculRapide;
    positionBoutonReculRapide.x = positionBoutonPlayPause.x - boutonReculRapide->w - 30;
    positionBoutonReculRapide.y = hauteurZoneLecteur + 25 + ((hauteurZonePlayPause - boutonReculRapide->h) / 2);

    SDL_BlitSurface(boutonReculRapide, NULL, ecran, &positionBoutonReculRapide);


    //nom de la musique, de l'auteur, de l'album, de l'image d'album
    musique.surface.titre = NULL;
    musique.surface.auteur = NULL;
    musique.surface.album = NULL;
    strcpy(musique.titre, "Unknow");
    strcpy(musique.auteur, "Artiste Inconnu");
    strcpy(musique.album, "Album Inconnu");

    TTF_Font *policeNomMusique = NULL;
    policeNomMusique = TTF_OpenFont("fonts\\sdigit.ttf", 15);
    musique.surface.titre = TTF_RenderText_Blended(policeNomMusique, musique.titre, couleurBlanc);
    musique.surface.auteur = TTF_RenderText_Blended(policeNomMusique, musique.auteur, couleurBlanc);
    musique.surface.album = TTF_RenderText_Blended(policeNomMusique, musique.album, couleurBlanc);

    musique.position.titre.y = hauteurZoneLecteur + 25 + ((hauteurZonePlayPause - (musique.surface.titre->h * 3 + 10)) / 2);
    musique.position.auteur.y = musique.position.titre.y + musique.surface.titre->h + 5;
    musique.position.album.y = musique.position.auteur.y + musique.surface.auteur->h + 5;
    musique.position.image.y = musique.position.titre.y;

    musique.position.titre.x = 40 + musique.position.album.y + musique.surface.album->h - musique.position.titre.y;
    musique.position.auteur.x = musique.position.titre.x;
    musique.position.album.x = musique.position.titre.x;
    musique.position.image.x = 20;

    SDL_Rect tailleImageAlbum;
    tailleImageAlbum.w = musique.position.album.y + musique.surface.album->h - musique.position.titre.y;
    tailleImageAlbum.h = tailleImageAlbum.w;

    SDL_Surface *imageAlbumDefautMusique = IMG_Load("img\\other\\464.png");
    RSZ_redimensionner_image(tailleImageAlbum, &imageAlbumDefautMusique);

    SDL_Surface *imageAlbumDefautVideo = IMG_Load("img\\other\\503.png");
    RSZ_redimensionner_image(tailleImageAlbum, &imageAlbumDefautVideo);

    SDL_Surface *imageAlbumDefautInternet = IMG_Load("img\\other\\471.png");
    RSZ_redimensionner_image(tailleImageAlbum, &imageAlbumDefautInternet);

    SDL_Surface *imageAlbumDefautRadio = IMG_Load("img\\other\\600.png");
    RSZ_redimensionner_image(tailleImageAlbum, &imageAlbumDefautRadio);

    SDL_Surface *imageAlbumDefautErreur = IMG_Load("img\\other\\704.png");
    RSZ_redimensionner_image(tailleImageAlbum, &imageAlbumDefautErreur);

    SDL_BlitSurface(musique.surface.titre, NULL, ecran, &musique.position.titre);
    SDL_BlitSurface(musique.surface.auteur, NULL, ecran, &musique.position.auteur);
    SDL_BlitSurface(musique.surface.album, NULL, ecran, &musique.position.album);

    //bouton lecture aléatoire
    SDL_Surface *boutonAleatoire = NULL;
    boutonAleatoire = IMG_Load("img\\other\\bouton_lecture_aleatoire_reduit.jpg");
    int modeAleatoire = 1;

    SDL_Rect positionBoutonAleatoire;
    positionBoutonAleatoire.x = positionBoutonAvanceRapide.x + boutonAvanceRapide->w + 50;
    positionBoutonAleatoire.y = hauteurZoneLecteur + 25 + ((hauteurZonePlayPause - boutonAleatoire->h) / 2);

    SDL_BlitSurface(boutonAleatoire, NULL, ecran, &positionBoutonAleatoire);


    //bouton lecture en boucle
    SDL_Surface *boutonBoucle = NULL;
    boutonBoucle = IMG_Load("img\\other\\bouton_lecture_sans_boucle_reduit.jpg");
    int modeBoucle = 0;

    SDL_Rect positionBoutonBoucle;
    positionBoutonBoucle.x = positionBoutonAleatoire.x + boutonAleatoire->w + 30;
    positionBoutonBoucle.y = hauteurZoneLecteur + 25 + ((hauteurZonePlayPause - boutonBoucle->h) / 2);

    SDL_BlitSurface(boutonBoucle, NULL, ecran, &positionBoutonBoucle);


    //bouton lecture A-B
    SDL_Surface *boutonAB = NULL, *lettreA = NULL, *lettreB = NULL;
    boutonAB = IMG_Load("img\\other\\bouton_lecture_AB_reduit.jpg");

    int modeAB = 0;   // 0 = désactivé, 1 = A placé, 2 = activé
    int tempsPointA = 0, tempsPointB = 0;

    TTF_Font *policeAB = NULL;
    policeAB = TTF_OpenFont("fonts\\sdigit.ttf", 20);

    SDL_Rect positionBoutonAB, positionLettreA, positionLettreB;
    positionBoutonAB.x = positionBoutonBoucle.x + boutonBoucle->w + 30;
    positionBoutonAB.y = hauteurZoneLecteur + 25 + ((hauteurZonePlayPause - boutonAB->h) / 2);

    SDL_BlitSurface(boutonAB, NULL, ecran, &positionBoutonAB);


    //bouton mettre à jour (radio uniquement)
    SDL_Surface *boutonActualiser = NULL;
    boutonActualiser = IMG_Load("img\\other\\arobase.bmp");

    SDL_Rect positionBoutonActualiser;
    positionBoutonActualiser.x = positionBoutonBoucle.x;
    positionBoutonActualiser.y = hauteurZoneLecteur + 25 + ((hauteurZonePlayPause - boutonActualiser->h) / 2);


    //bouton enregistrer (radio uniquement)
    SDL_Surface *boutonRecord = NULL;
    boutonRecord = IMG_Load("img\\other\\bouton_record_play_reduit.jpg");
    int modeRecord = 0;
    int tempsDernierRecord = 0;

    FSOUND_SAMPLE *enregistrement = NULL;

    char nomFichierRecord[100] = " ";
    sprintf(nomFichierRecord, "records\\Enregistrement %d.wav", numeroRecord);

    SDL_Rect positionBoutonRecord;
    positionBoutonRecord.x = positionBoutonAB.x;
    positionBoutonRecord.y = hauteurZoneLecteur + 25 + ((hauteurZonePlayPause - boutonRecord->h) / 2);


    //zone graphique

    //boutons de réglages
    //bouton changement visu
    SDL_Surface *boutonChangementVisu = NULL;
    boutonChangementVisu = IMG_Load("img\\other\\affichage.bmp");

    SDL_Rect positionBoutonChangementVisu;
    positionBoutonChangementVisu.x = positionLigneSeparation3.x - boutonChangementVisu->w - 15;
    positionBoutonChangementVisu.y = 10;

    SDL_BlitSurface(boutonChangementVisu, NULL, ecran, &positionBoutonChangementVisu);

    //bouton settings
    SDL_Surface *boutonSettings = NULL;
    boutonSettings = IMG_Load("img\\other\\settings.bmp");

    SDL_Rect positionBoutonSettings;
    positionBoutonSettings.x = positionBoutonChangementVisu.x - boutonSettings->w - 15;
    positionBoutonSettings.y = positionBoutonChangementVisu.y;

    SDL_BlitSurface(boutonSettings, NULL, ecran, &positionBoutonSettings);

    //bouton rechercher
    SDL_Surface *boutonResearch = NULL;
    boutonResearch = IMG_Load("img\\other\\research.bmp");

    SDL_Rect positionBoutonResearch;
    positionBoutonResearch.x = positionBoutonSettings.x - boutonResearch->w - 15;
    positionBoutonResearch.y = positionBoutonChangementVisu.y;

    SDL_BlitSurface(boutonResearch, NULL, ecran, &positionBoutonResearch);

    //ligne separation
    SDL_Surface *ligneSeparation4 = NULL;
    ligneSeparation4 = SDL_CreateRGBSurface(SDL_HWSURFACE, largeurZoneGraphique, 5, 32, 0, 0, 0, 0);
    SDL_FillRect(ligneSeparation4, NULL, SDL_MapRGB(ligneSeparation4->format, 200, 0, 0));
    SDL_BlitSurface(IMG_Load("img\\other\\ligne_separation_4_reduite.jpg"), NULL, ligneSeparation4, &positionZero);
    SDL_Rect positionLigneSeparation4;
    positionLigneSeparation4.x = positionLigneSeparation.x;
    positionLigneSeparation4.y = hauteurZoneBoutons - ligneSeparation4->h;


    //affichage du temps de rafraichissement
    SDL_Surface *surfaceTempsRafraichissement = NULL;
    TTF_Font *policeTempsRafraichissement = TTF_OpenFont("fonts\\cour.ttf", 15);
    char texteTempsRafraichissement[20] = "00 ms";
    surfaceTempsRafraichissement = TTF_RenderText_Solid(policeTempsRafraichissement, texteTempsRafraichissement, couleurBlanc);

    SDL_Rect positionTempsRafraichissement;
    positionTempsRafraichissement.x = positionLigneSeparation.x + ligneSeparation->w + 10;
    positionTempsRafraichissement.y = 10;

    SDL_BlitSurface(surfaceTempsRafraichissement, NULL, ecran, &positionTempsRafraichissement);



    //préparation pour visu2
    int hauteurBarre = (hauteurZoneGraphique / 2.0) - 100;
    int positionBarreVisu2 = 0;

    SDL_Surface *barreNoire = NULL;
    barreNoire = SDL_CreateRGBSurface(SDL_HWSURFACE, 5, hauteurZoneGraphique, 32, 0, 0, 0, 0);
    SDL_FillRect(barreNoire, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));

    SDL_Rect positionBarreNoire;
    positionBarreNoire.x = 0;
    positionBarreNoire.y = 0;

    SDL_Surface *surfaceVisu = NULL;
    surfaceVisu = SDL_CreateRGBSurface(SDL_HWSURFACE, largeurZoneGraphique, hauteurZoneGraphique, 32, 0, 0, 0, 0);
    SDL_FillRect(surfaceVisu, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));

    SDL_Rect positionVisu2;
    positionVisu2.x = largeurZoneReglage;
    positionVisu2.y = hauteurZoneBoutons;

    SDL_Surface *triangle1 = NULL;
    triangle1 = IMG_Load("img\\other\\triangle1_reduit.bmp");
    SDL_Surface *triangle2 = NULL;
    triangle2 = IMG_Load("img\\other\\triangle2_reduit.bmp");

    SDL_Rect positionTriangle1;
    positionTriangle1.x = positionBarreVisu2 - 12;
    positionTriangle1.y = hauteurZoneGraphique - triangle1->h;
    SDL_Rect positionTriangle2;
    positionTriangle2.x = positionBarreVisu2 - 12;
    positionTriangle2.y = 0;

    SDL_Surface *surfaceCache1 = NULL;
    surfaceCache1 = SDL_CreateRGBSurface(SDL_HWSURFACE, largeurZoneGraphique, 100, 32, 0, 0, 0, 0);
    SDL_FillRect(surfaceCache1, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));

    SDL_Rect positionSurfaceCache1;
    positionSurfaceCache1.x = 0;
    positionSurfaceCache1.y = 0;

    SDL_Rect positionSurfaceCache2;
    positionSurfaceCache2.x = 0;
    positionSurfaceCache2.y = hauteurZoneGraphique - surfaceCache1->h;

    SDL_Surface *barreInterTriangles = NULL;
    barreInterTriangles = SDL_CreateRGBSurface(SDL_HWSURFACE, 1, 2000, 32, 0, 0, 0, 0);
    SDL_FillRect(barreInterTriangles, NULL, SDL_MapRGB(ecran->format, 255, 0, 0));

    SDL_Rect positionBarreInterTriangles;
    positionBarreInterTriangles.x = positionBarreVisu2;
    positionBarreInterTriangles.y = 0;


    // affichage du texte d'erreur
    TTF_Font *policeDErreur = NULL;
    policeDErreur = TTF_OpenFont("fonts\\cour.ttf", 15);
    TTF_SetFontStyle(policeDErreur, TTF_STYLE_NORMAL);
    SDL_Color couleurNoir = {0};

    SDL_Rect positionTexteErreur;
    positionTexteErreur.x = largeurZoneReglage + 30;
    positionTexteErreur.y = hauteurZoneBoutons + 10;



    //Gestion de données - non graphique
    //tableau récapitulant les 10 dernières musiques écoutées
    int tableauDernieresMusiques[10] = {0};
    for (i = 0 ; i < 10 ; i++)
    {
        tableauDernieresMusiques[i] = -1;
    }
    int positionDansTableauDernieresMusiques = 0;

    //tableau récapitulant les musiques déjà écoutées
    int tableauTotalMusiques[TAILLE_MAX_PLAYLIST] = {0};

    strcpy(parametre, "NULL");

    //fichier de sauvegarde des paramètres du lecteur
    if ((saveFile = fopen("saveFile.sf", "r")) == NULL)
       fprintf(stderr, "ERR main %d : Le fichier de sauvegarde n'a pas pu etre ouvert.\n", __LINE__);
    else
    {

        while (fgets(parametre, 1000, saveFile) != NULL && strcmp(parametre, "[saved]\n") != 0);
        if (strcmp(parametre, "[saved]\n") == 0)
        {
                          fgets(parametre, 1000, saveFile);
                          volumeDroit = strtol(parametre, NULL, 10);
                          positionBoutonVolumeDroit.y = 255 - (volumeDroit + (boutonVolume->h / 2) - positionLigneVolumeDroit.y);

                          fgets(parametre, 1000, saveFile);
                          volumeGauche = strtol(parametre, NULL, 10);
                          positionBoutonVolumeGauche.y = 255 - (volumeGauche + (boutonVolume->h / 2) - positionLigneVolumeGauche.y);

                          fgets(parametre, 1000, saveFile);
                          fgets(parametre, 1000, saveFile);
                          fgets(parametre, 1000, saveFile);
                          fgets(parametre, 1000, saveFile);
                          if (argc <= 1)
                             tempsAMettre = strtol(parametre, NULL, 10);

                          fgets(parametre, 1000, saveFile);
                          indicStereoMono = strtol(parametre, NULL, 10);
                          SDL_FreeSurface(surfaceIndicStereoMono);
                          if (indicStereoMono)
                             surfaceIndicStereoMono = TTF_RenderText_Blended(policeIndicMonoStereo, "Stereo", couleurJaune);
                          else surfaceIndicStereoMono = TTF_RenderText_Blended(policeIndicMonoStereo, "Mono", couleurJaune);

                          fgets(parametre, 1000, saveFile);
                          lockIndic = strtol(parametre, NULL, 10);
                          SDL_FreeSurface(surfaceLockIndic);
                          if (lockIndic)
                             surfaceLockIndic = TTF_RenderText_Blended(policeLockIndic, "Locked", couleurRouge);
                          else surfaceLockIndic = TTF_RenderText_Blended(policeLockIndic, "Unlocked", couleurVert);

                          fgets(parametre, 1000, saveFile);
                          modeAleatoire = strtol(parametre, NULL, 10);
                          SDL_FreeSurface(boutonAleatoire);
                          if (modeAleatoire)
                             boutonAleatoire = IMG_Load("img\\other\\bouton_lecture_aleatoire_reduit.jpg");
                          else boutonAleatoire = IMG_Load("img\\other\\bouton_lecture_normale_reduit.jpg");

                          fgets(parametre, 1000, saveFile);
                          modeBoucle = strtol(parametre, NULL, 10);
                          SDL_FreeSurface(boutonBoucle);
                          if (modeBoucle)
                             boutonBoucle = IMG_Load("img\\other\\bouton_lecture_boucle_reduit.jpg");
                          else boutonBoucle = IMG_Load("img\\other\\bouton_lecture_sans_boucle_reduit.jpg");

                          fgets(parametre, 1000, saveFile);
                          numeroVisu = strtol(parametre, NULL, 10);

                          fgets(parametre, 1000, saveFile);
                          if (strrchr(parametre, '\n') != NULL)
                             *(strrchr(parametre, '\n')) = '\0';

                          while(fclose(saveFile) == EOF);
        }
    }


    // initialisation des paramètres pour l'édition de playlist
    MenuEditPlaylist editMenu;

         // paramètres généraux
         editMenu.ecran = ecran;
         editMenu.surfaceBlit = SDL_CreateRGBSurface(SDL_HWSURFACE, largeurZoneGraphique, hauteurZoneGraphique, 32, 0, 0, 0, 0);
         SDL_FillRect(editMenu.surfaceBlit, NULL, SDL_MapRGB(editMenu.surfaceBlit->format, 0, 0, 0));
         editMenu.positionSurfaceBlit = positionVisu2;

         editMenu.ligneSeparation = IMG_Load("img\\other\\ligne_separation_reduite.jpg");
         editMenu.zoneDossier.positionLigneSeparation.y = 0;
         editMenu.zoneMusique.positionLigneSeparation.y = 0;
         editMenu.zonePlaylist.positionLigneSeparation.y = 0;

         editMenu.positionZero.x = 0;
         editMenu.positionZero.y = 0;

         editMenu.hauteur = editMenu.surfaceBlit->h;
         editMenu.zoneDossier.largeur = (editMenu.surfaceBlit->w - (editMenu.ligneSeparation->w * 3)) / 3;
         editMenu.zoneMusique.largeur = editMenu.zoneDossier.largeur;
         editMenu.zonePlaylist.largeur = editMenu.zoneDossier.largeur;

         editMenu.zoneDossier.abscisse = editMenu.ligneSeparation->w;
         editMenu.zoneMusique.abscisse = editMenu.zoneDossier.abscisse + editMenu.zoneDossier.largeur + editMenu.ligneSeparation->w;
         editMenu.zonePlaylist.abscisse = editMenu.zoneMusique.abscisse + editMenu.zoneMusique.largeur + editMenu.ligneSeparation->w;

         editMenu.zoneDossier.positionLigneSeparation.x = 0;
         editMenu.zoneMusique.positionLigneSeparation.x = editMenu.zoneDossier.abscisse + editMenu.zoneDossier.largeur;
         editMenu.zonePlaylist.positionLigneSeparation.x = editMenu.zoneMusique.abscisse + editMenu.zoneMusique.largeur;

         editMenu.zoneDossier.barreDefilement = NULL;
         editMenu.zoneMusique.barreDefilement = NULL;
         editMenu.zonePlaylist.barreDefilement = NULL;

         if (strcmp(parametre, "NULL") != 0 && test_exist(parametre))
            strcpy(editMenu.repertoireVirtuel, parametre);
         else strcpy(editMenu.repertoireVirtuel, "C:\\");
         strcpy(editMenu.adresseAScanner, editMenu.repertoireVirtuel);
         strcpy(editMenu.dossierBase, dossierBase);

         editMenu.surfaceSauvegarde = SDL_CreateRGBSurface(SDL_HWSURFACE, surfaceVisu->w, surfaceVisu->h, 32, 0, 0, 0, 0);
         SDL_FillRect(editMenu.surfaceSauvegarde, NULL, SDL_MapRGB(editMenu.surfaceSauvegarde->format, 0, 0, 0));
         SDL_BlitSurface(editMenu.ligneSeparation, NULL, editMenu.surfaceSauvegarde, &editMenu.zoneDossier.positionLigneSeparation);
         SDL_BlitSurface(editMenu.ligneSeparation, NULL, editMenu.surfaceSauvegarde, &editMenu.zoneMusique.positionLigneSeparation);
         SDL_BlitSurface(editMenu.ligneSeparation, NULL, editMenu.surfaceSauvegarde, &editMenu.zonePlaylist.positionLigneSeparation);



         //paramètres de la liste de dossiers
         strcpy(editMenu.zoneDossier.nomFichierListe, "Dirs.spl");
         strcpy(editMenu.zoneDossier.nomFichierPlaylist, "Dirs.npl");
         lister_dossiers(editMenu.repertoireVirtuel, editMenu.zoneDossier.nomFichierPlaylist, "w");
         effacer_adresse(editMenu.zoneDossier.nomFichierPlaylist, TAILLE_MAX_NOM);
         classer_alphabetique_fichier_texte(editMenu.zoneDossier.nomFichierPlaylist, TAILLE_MAX_NOM, NULL);

         editMenu.zoneDossier.ligneDebut = 0;
         editMenu.zoneDossier.ligneFin = 0;

         editMenu.zoneDossier.police = TTF_OpenFont("fonts\\sdigit.ttf", 15);
         editMenu.zoneDossier.positionListe.x = editMenu.zoneDossier.abscisse + 5;
         editMenu.zoneDossier.positionListe.y = 10;

         editMenu.zoneDossier.echantillonTexte = TTF_RenderText_Blended(editMenu.zoneDossier.police, "A", couleurBlanc);
         editMenu.zoneDossier.nombreAffichables = ((editMenu.hauteur - 5) / (editMenu.zoneDossier.echantillonTexte->h + 5)) - 2;
         editMenu.zoneDossier.ligneFin = editMenu.zoneDossier.ligneDebut + editMenu.zoneDossier.nombreAffichables;

         for (i = 0 ; i < TAILLE_MAX_PLAYLIST ; i++)
         {
             editMenu.zoneDossier.tableauSurfaces[i] = NULL;
             editMenu.zoneDossier.tableauPositions[i].x = 0;
             editMenu.zoneDossier.tableauPositions[i].y = 0;
             editMenu.zoneDossier.tableauNoms[i] = malloc(sizeof(char) * TAILLE_MAX_NOM);
             editMenu.zoneDossier.tableauSelectionnes[i] = 0;
             editMenu.zoneDossier.clicGauche[i] = 0;
             editMenu.zoneDossier.positionFantome[i].x = 0;
             editMenu.zoneDossier.positionFantome[i].y = 0;
             editMenu.zoneDossier.differenceAuClic[i].x = 0;
             editMenu.zoneDossier.differenceAuClic[i].y = 0;
         }



         //paramètres de la liste de musiques
         strcpy(editMenu.zoneMusique.nomFichierListe, "Musics.list");
         strcpy(editMenu.zoneMusique.nomFichierPlaylist, "Musics.npl");
         lister_extension(editMenu.repertoireVirtuel, editMenu.zoneMusique.nomFichierListe, ".mp3 ; .wma ; .wav ; .aac ; .ogg ; .mpg ; .url", "w");
         creer_playlist(editMenu.zoneMusique.nomFichierListe, editMenu.zoneMusique.nomFichierPlaylist, editMenu.zoneMusique.typePlaylist);
         classer_alphabetique_fichier_texte(editMenu.zoneMusique.nomFichierPlaylist, TAILLE_MAX_NOM, NULL);

         editMenu.zoneMusique.ligneDebut = 0;
         editMenu.zoneMusique.ligneFin = 0;

         editMenu.zoneMusique.police = TTF_OpenFont("fonts\\sdigit.ttf", 15);
         editMenu.zoneMusique.positionListe.x = editMenu.zoneMusique.abscisse + 5;
         editMenu.zoneMusique.positionListe.y = 10;

         editMenu.zoneMusique.echantillonTexte = TTF_RenderText_Blended(editMenu.zoneMusique.police, "A", couleurBlanc);
         editMenu.zoneMusique.nombreAffichables = (editMenu.hauteur - 5) / (editMenu.zoneMusique.echantillonTexte->h + 5) - 2;
         editMenu.zoneMusique.ligneFin = editMenu.zoneMusique.ligneDebut + editMenu.zoneMusique.nombreAffichables;

         for (i = 0 ; i < TAILLE_MAX_PLAYLIST ; i++)
         {
             editMenu.zoneMusique.tableauSurfaces[i] = NULL;
             editMenu.zoneMusique.tableauPositions[i].x = 0;
             editMenu.zoneMusique.tableauPositions[i].y = 0;
             editMenu.zoneMusique.tableauNoms[i] = malloc(sizeof(char) * TAILLE_MAX_NOM);
             editMenu.zoneMusique.tableauSelectionnes[i] = 0;
             editMenu.zoneMusique.clicGauche[i] = 0;
             editMenu.zoneMusique.positionFantome[i].x = 0;
             editMenu.zoneMusique.positionFantome[i].y = 0;
             editMenu.zoneMusique.differenceAuClic[i].x = 0;
             editMenu.zoneMusique.differenceAuClic[i].y = 0;
         }



         //paramètres de la liste de playlists
         strcpy(editMenu.zonePlaylist.nomFichierListe, nomFichierListePlaylists);
         strcpy(editMenu.zonePlaylist.nomFichierPlaylist, nomFichierPlaylistDePlaylists);

         editMenu.zonePlaylist.ligneDebut = 0;
         editMenu.zonePlaylist.ligneFin = 0;

         editMenu.zonePlaylist.police = TTF_OpenFont("fonts\\sdigit.ttf", 15);
         editMenu.zonePlaylist.positionListe.x = editMenu.zonePlaylist.abscisse + 5;
         editMenu.zonePlaylist.positionListe.y = 10;

         editMenu.zonePlaylist.echantillonTexte = TTF_RenderText_Blended(editMenu.zonePlaylist.police, "A", couleurBlanc);
         editMenu.zonePlaylist.nombreAffichables = (editMenu.hauteur - 5) / (editMenu.zonePlaylist.echantillonTexte->h + 5);
         editMenu.zonePlaylist.ligneFin = editMenu.zonePlaylist.ligneDebut + editMenu.zonePlaylist.nombreAffichables;

         for (i = 0 ; i < TAILLE_MAX_PLAYLIST ; i++)
         {
             editMenu.zonePlaylist.tableauSurfaces[i] = NULL;
             editMenu.zonePlaylist.tableauPositions[i].x = 0;
             editMenu.zonePlaylist.tableauPositions[i].y = 0;
             editMenu.zonePlaylist.tableauNoms[i] = malloc(sizeof(char) * TAILLE_MAX_NOM);
             editMenu.zonePlaylist.tableauSelectionnes[i] = 0;
             editMenu.zonePlaylist.clicGauche[i] = 0;
             editMenu.zonePlaylist.positionFantome[i].x = 0;
             editMenu.zonePlaylist.positionFantome[i].y = 0;
             editMenu.zonePlaylist.differenceAuClic[i].x = 0;
             editMenu.zonePlaylist.differenceAuClic[i].y = 0;
         }

         recreer_barre_defil(&editMenu);

         if (editMenu.zoneDossier.nombreAffichables < editMenu.zoneDossier.nombreTotal)
            SDL_BlitSurface(editMenu.zoneDossier.barreDefilement, NULL, editMenu.surfaceSauvegarde, &editMenu.zoneDossier.positionBarreDefilement);
         if (editMenu.zoneMusique.nombreAffichables < editMenu.zoneMusique.nombreTotal)
            SDL_BlitSurface(editMenu.zoneMusique.barreDefilement, NULL, editMenu.surfaceSauvegarde, &editMenu.zoneMusique.positionBarreDefilement);
         if (editMenu.zonePlaylist.nombreAffichables < editMenu.zonePlaylist.nombreTotal)
            SDL_BlitSurface(editMenu.zonePlaylist.barreDefilement, NULL, editMenu.surfaceSauvegarde, &editMenu.zonePlaylist.positionBarreDefilement);


         //mise à jour de la surface de sauvegarde
         afficher_fichier_texte(editMenu.zoneDossier.nomFichierPlaylist, editMenu.zoneDossier.police, couleurBlanc, couleurBlanc, 2, 5, editMenu.zoneDossier.largeur, editMenu.zoneDossier.ligneDebut, editMenu.zoneDossier.ligneFin, editMenu.zoneDossier.positionListe, editMenu.surfaceSauvegarde, editMenu.zoneDossier.tableauSurfaces, editMenu.zoneDossier.tableauPositions, editMenu.zoneDossier.tableauNoms);
         afficher_fichier_texte(editMenu.zoneMusique.nomFichierPlaylist, editMenu.zoneMusique.police, couleurBlanc, couleurBlanc, 2, 5, editMenu.zoneMusique.largeur, editMenu.zoneMusique.ligneDebut, editMenu.zoneMusique.ligneFin, editMenu.zoneMusique.positionListe, editMenu.surfaceSauvegarde, editMenu.zoneMusique.tableauSurfaces, editMenu.zoneMusique.tableauPositions, editMenu.zoneMusique.tableauNoms);
         afficher_fichier_texte(editMenu.zonePlaylist.nomFichierPlaylist, editMenu.zonePlaylist.police, couleurBlanc, couleurBlanc, 2, 5, editMenu.zonePlaylist.largeur, editMenu.zonePlaylist.ligneDebut, editMenu.zonePlaylist.ligneFin, editMenu.zonePlaylist.positionListe, editMenu.surfaceSauvegarde, editMenu.zonePlaylist.tableauSurfaces, editMenu.zonePlaylist.tableauPositions, editMenu.zonePlaylist.tableauNoms);


         //bouton dossier
         editMenu.zoneDossier.boutonDossier = IMG_Load("img\\other\\dossier.bmp");
         editMenu.zoneDossier.positionBoutonDossier.x = editMenu.zoneMusique.positionLigneSeparation.x - editMenu.zoneDossier.boutonDossier->w - 10;
         editMenu.zoneDossier.positionBoutonDossier.y = editMenu.hauteur - 10 - editMenu.zoneDossier.boutonDossier->h;

         SDL_BlitSurface(editMenu.zoneDossier.boutonDossier, NULL, editMenu.surfaceSauvegarde, &editMenu.zoneDossier.positionBoutonDossier);


         //bouton dossier 2
         editMenu.zonePlaylist.boutonDossier = IMG_Load("img\\other\\dossier_musique.bmp");
         editMenu.zonePlaylist.positionBoutonDossier.x = editMenu.surfaceBlit->w - editMenu.zonePlaylist.boutonDossier->w - 10;
         editMenu.zonePlaylist.positionBoutonDossier.y = editMenu.zoneDossier.positionBoutonDossier.y;


         //curseur "interdit"
         editMenu.curseurInterdit = IMG_Load("img\\other\\forbid.gif");


         //barre rouge
         editMenu.barreRouge = SDL_CreateRGBSurface(SDL_HWSURFACE, 2000, 2, 32, 0, 0, 0, 0);
         SDL_FillRect(editMenu.barreRouge, NULL, SDL_MapRGB(editMenu.barreRouge->format, 255, 0, 0));

         editMenu.positionBarreRouge.x = editMenu.zonePlaylist.abscisse;
         editMenu.positionBarreRouge.y = 0;


         //poste de travail
         editMenu.posteTravail = IMG_Load("img\\other\\poste_travail.bmp");
         editMenu.positionPosteTravail.x = editMenu.zoneDossier.positionBoutonDossier.x - editMenu.posteTravail->w - 15;
         editMenu.positionPosteTravail.y = editMenu.zoneDossier.positionBoutonDossier.y;

         SDL_BlitSurface(editMenu.posteTravail, NULL, editMenu.surfaceSauvegarde, &editMenu.positionPosteTravail);


         //bouton nouvelle playlist
         editMenu.nouvellePlaylist = IMG_Load("img\\other\\nouvelle_playlist.bmp");
         editMenu.positionNouvellePlaylist.x = editMenu.zonePlaylist.positionBoutonDossier.x;
         editMenu.positionNouvellePlaylist.y = editMenu.zonePlaylist.positionBoutonDossier.y;

         SDL_BlitSurface(editMenu.nouvellePlaylist, NULL, editMenu.surfaceSauvegarde, &editMenu.positionNouvellePlaylist);


         //boutons types playlists
         editMenu.zoneMusique.policeBoutonTypePlaylist = TTF_OpenFont("fonts\\sdigit.ttf", 15);
         editMenu.zoneMusique.boutonTypePlaylist = TTF_RenderText_Blended(policeBoutonTypePlaylist, "NoM", couleurVertBizarre);
         editMenu.zoneMusique.typePlaylist = NOM;

         editMenu.zoneMusique.positionBoutonTypePlaylist.x = editMenu.zoneMusique.positionLigneSeparation.x + editMenu.ligneSeparation->w + 10;
         editMenu.zoneMusique.positionBoutonTypePlaylist.y = editMenu.zonePlaylist.positionBoutonDossier.y;

         SDL_BlitSurface(editMenu.zoneMusique.boutonTypePlaylist, NULL, editMenu.surfaceSauvegarde, &editMenu.zoneMusique.positionBoutonTypePlaylist);


         editMenu.zonePlaylist.policeBoutonTypePlaylist = TTF_OpenFont("fonts\\sdigit.ttf", 15);
         editMenu.zonePlaylist.boutonTypePlaylist = TTF_RenderText_Blended(policeBoutonTypePlaylist, "NoM", couleurVertBizarre);
         editMenu.zonePlaylist.typePlaylist = NOM;

         editMenu.zonePlaylist.positionBoutonTypePlaylist.x = editMenu.zonePlaylist.positionLigneSeparation.x + editMenu.ligneSeparation->w + 10;
         editMenu.zonePlaylist.positionBoutonTypePlaylist.y = editMenu.zonePlaylist.positionBoutonDossier.y;

     // fin de l'initialisation


    //initialisation des paramètres pour l'affichage de paroles
    AffichageParoles affichageParoles;
    affichageParoles.positionBarreDefilement = positionLigneSeparation;
    affichageParoles.positionSurfaceParoles = positionTexteErreur;
    affichageParoles.clicGaucheSurBarreDefil = 0;
    affichageParoles.hauteurZone = hauteurZoneGraphique;
    affichageParoles.largeurZone = largeurZoneGraphique - 60;
    affichageParoles.hauteurDiff = hauteurZoneBoutons;
    affichageParoles.police = TTF_OpenFont("fonts\\cour.ttf", 15);
    TTF_SetFontStyle(affichageParoles.police, TTF_STYLE_BOLD);
    affichageParoles.surfaceParoles = SDL_CreateRGBSurface(SDL_HWSURFACE, affichageParoles.largeurZone, affichageParoles.hauteurZone, 32, 0, 0, 0, 0);
    affichageParoles.surfaceBarreDefilement = NULL;

    //initialisation des paramètres pour le mode vidéo grand écran
    bigScreenMode = 0;
    SDL_Surface *boutonModeGrandEcran = IMG_Load("img\\other\\bouton_mode_grand_ecran.bmp");
    SDL_Rect positionBoutonModeGrandEcran;
    positionBoutonModeGrandEcran.x = positionBoutonChangementVisu.x;
    positionBoutonModeGrandEcran.y = positionBoutonChangementVisu.y;

    SDL_Surface *boutonTailleVideo = IMG_Load("img\\other\\bouton_taille_video.png");
    SDL_Rect positionBoutonTailleVideo;
    positionBoutonTailleVideo.x = (ecran->w - boutonTailleVideo->w) / 2;
    positionBoutonTailleVideo.y = ecran->h - boutonTailleVideo->h - 20;
    int ordonneeBoutonTailleVideo = positionBoutonTailleVideo.y;
    float oldAgrandissement = agrandissement;

    SDL_Surface *ligneTailleVideo = IMG_Load("img\\other\\ligne_taille_video.png");
    SDL_Rect positionLigneTailleVideo;
    positionLigneTailleVideo.x = (ecran->w - ligneTailleVideo->w) / 2;
    positionLigneTailleVideo.y = (boutonTailleVideo->h - ligneTailleVideo->h) / 2 + positionBoutonTailleVideo.y;

    SDL_Surface *boutonTailleVideoMoins = IMG_Load("img\\other\\bouton_taille_video_moins.png");
    SDL_Rect positionBoutonTailleVideoMoins;
    positionBoutonTailleVideoMoins.x = positionLigneTailleVideo.x - boutonTailleVideo->w - 10;
    positionBoutonTailleVideoMoins.y = positionBoutonTailleVideo.y;

    SDL_Surface *boutonTailleVideoPlus = IMG_Load("img\\other\\bouton_taille_video_plus.png");
    SDL_Rect positionBoutonTailleVideoPlus;
    positionBoutonTailleVideoPlus.x = positionLigneTailleVideo.x + ligneTailleVideo->w + 10;
    positionBoutonTailleVideoPlus.y = positionBoutonTailleVideo.y;

    SDL_Surface *boutonModePetitEcran = IMG_Load("img\\other\\bouton_mode_grand_ecran.bmp");
    SDL_Rect positionBoutonModePetitEcran;
    positionBoutonModePetitEcran.x = ecran->w - boutonModePetitEcran->w - 10;
    positionBoutonModePetitEcran.y = ecran->h - boutonModePetitEcran->h - 10;


    // initialisation des paramètres pour la configuration du lecteur
    MenuConfigLecteur configMenu;
    copieConfigMenu = &configMenu;
    position.x = largeurZoneReglage + ligneSeparation->w;
    position.y = hauteurZoneBoutons;
    CFG_init_config(&configMenu, ecran, position, largeurZoneGraphique - ligneSeparation->w, hauteurZoneGraphique);
    position.y = 0;


    //recupération d'éventuelles données sauvegardées
    if ((saveFile = fopen("saveFile.sf", "r")) == NULL)
       fprintf(stderr, "ERR main %d : Le fichier de sauvegarde n'a pas pu etre ouvert.\n", __LINE__);
    else
    {

        while (fgets(parametre, 1000, saveFile) != NULL && strcmp(parametre, "[parameters]\n") != 0);
        if (strcmp(parametre, "[parameters]\n") == 0)
        {
                          for (i = 0 ; fgets(parametre, 1000, saveFile) != NULL && i < 100 && strcmp(parametre, "[over]") != 0 ; i++)
                          {
                              if (strchr(parametre, '\n') != NULL)
                                 *(strchr(parametre, '\n')) = '\0';
                              strcpy(configMenu.tableauTextesReponses[i], parametre);
                          }

                          strcpy(configMenu.tableauTextes[i + 1], "(none)");
                          CFG_associer_parametres(&configMenu);
                          CFG_recreer_surface(&configMenu);

                          while(fclose(saveFile) == EOF);
        }
    }



    SDL_Flip(ecran);



    FSOUND_SAMPLE *son1 = NULL;
    son1 = FSOUND_Sample_Load(FSOUND_FREE, "ouverture.wav", 0, 0, 0);
    if (son1 == NULL)
       fprintf(stderr, "ERR main %d : Le chargement de son1 a echoue.\n", __LINE__);

    FSOUND_PlaySound(FSOUND_FREE, son1);

    SDL_Delay(2000);

    //FSOUND_Stream_SetBufferSize(800);

    musique.pointeurStereo = NULL;
    musique.pointeurMono = NULL;
    strcpy(musique.adresse, " ");
    i = 0;
    charger_musique(&musique, configMenu, &affichageParoles, nomFichierListe, musique.numero, positionBoutonReculRapide.x - musique.position.titre.x - 40, &i);
    i++;

    while (i < configMenu.parametre.nombreChargementsMaxi && musique.mode == ERREUR)
    {
          charger_musique(&musique, configMenu, &affichageParoles, nomFichierListe, -1, positionBoutonReculRapide.x - musique.position.titre.x - 40, &i);
          i++;
    }

    FSOUND_SAMPLE *sampleMusique = NULL;

    if (musique.mode == STOP || musique.mode == ERREUR)
             mettre_en_stop(&musique, surfaceBoutonPlayPause);
    else
    {
            if (musique.type != VIDEO)
            {
                if (indicStereoMono)
                   FSOUND_Stream_Play(1, musique.pointeurStereo);   //Lecture d'un son long sur le canal 1
                else FSOUND_Stream_Play(1, musique.pointeurMono);
            }
            else SMPEG_play((SMPEG*) musique.pointeurStereo);

            if (tempsAMettre > 0 && musique.type != VIDEO)
            {
               FSOUND_Stream_SetTime(musique.pointeurStereo, tempsAMettre);
               FSOUND_Stream_SetTime(musique.pointeurMono, tempsAMettre);
            }
            else if (tempsAMettre > 0)
            {
                secondsSkipped = tempsAMettre / 1000.0;
                SMPEG_rewind((SMPEG*) musique.pointeurStereo);
                SMPEG_play((SMPEG*) musique.pointeurStereo);
                SMPEG_skip((SMPEG*) musique.pointeurStereo, secondsSkipped);
            }

            musique.mode = PLAY;
            if (musique.type != VIDEO)
               sampleMusique = FSOUND_Stream_GetSample(musique.pointeurStereo);
            tableauDernieresMusiques[positionDansTableauDernieresMusiques] = musique.numero;
            tableauTotalMusiques[musique.numero] = 1;
    }


   /* int typeTagField = 0;
    char nameTagField[TAILLE_MAX_NOM] = "niet";
    void **value = NULL;
    int lenght = 0;
    FSOUND_Stream_GetTagField(musique.pointeurStereo, 10, &typeTagField, &nameTagField, value, &lenght);
    fprintf(stdout, "%d%d%d%d", nameTagField[0], nameTagField[1], nameTagField[2], nameTagField[3]);*/


    float niveauSonGauche = 0;
    float niveauSonDroit = 0;
    FSOUND_GetCurrentLevels(1, &niveauSonGauche, &niveauSonDroit);  //Prélèvement du volume sur les canaux droit et gauche

    FSOUND_SetVolume(FSOUND_ALL, 128);  //réglage du volume. Ici tous canaux. 0 à 255.


    //Création du système de visualisation spectrale
    spectre = FSOUND_DSP_GetSpectrum();   //renvoi par FMOD d'un pointeur de 512 float, représentant chacun une fréquence.
    int j;


    int continuer = 1, tempsActuel = 0, tempsPrecedent = 0, tempsDernierMvtSouris = 0, tempsDernierEvenement = 0, tempsDernierClicGaucheSurBoutonAvanceRapide = -1, tempsDernierClicGaucheSurBoutonReculRapide = -1;
    editMenu.tempsDernierMvtSouris = &tempsDernierMvtSouris;
    int alt = 0, ctrl = 0, maj = 0, toucheActive = 0;
    int clicGaucheSurBoutonVolumeDroit = 0, clicGaucheSurBoutonVolumeGauche = 0, clicGaucheSurFlecheLecture = 0, clicGaucheSurBoutonAvanceRapide = 0, clicGaucheSurBoutonReculRapide = 0, clicGaucheSurLigneSeparation3 = 0, clicGaucheSurBarreDefilement = 0, clicGaucheSurLettreA = 0, clicGaucheSurLettreB = 0, clicGaucheSurBoutonTailleVideo = 0;
    int clicGaucheUpSurIndicStereoMono = 0, clicGaucheUpSurBoutonPlayPause = 0, clicGaucheUpSurFlecheLecture = 0, clicGaucheUpSurBoutonAvanceRapide = 0, clicGaucheUpSurBoutonReculRapide = 0, clicUpSurBoutonAjustementAudio = 0, clicUpSurBoutonAB = 0;
    int clicGaucheUpSurPlaylist[TAILLE_MAX_PLAYLIST] = {0};
    int differenceAltitude = 0;
    int tempsRafraichissement = configMenu.parametre.rafraichissement;
    int deplacementLigneSeparation3 = 0;
    int action = 0;
    SDL_Rect positionBoutonVolumeAuClic, positionBoutonVolume2AuClic, positionFlecheLectureAuClic, positionLigneSeparation3AuClic, positionBarreDefilementAuClic, positionLettreAAuClic, positionLettreBAuClic, positionBoutonTailleVideoAuClic;
    char *title = NULL;
    title = malloc(sizeof(char) * ((TAILLE_MAX_NOM * 2) + 4));

    SDL_Surface *imageDiaporama1 = NULL, *imageDiaporama2 = NULL;
    SDL_Rect positionImageDiaporama1, positionImageDiaporama2;
    int tempsDernierEffetDiaporama = -configMenu.parametre.vitesseDefilement;

    //SDL_EnableKeyRepeat(10, 10);


    SetTimer(mainWnd, 1, 40, (TIMERPROC)OsmProc);
    SetTimer(mainWnd, 2, 100, (TIMERPROC)ReceiveProc);

    handle = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(HWND) + TAILLE_MAX_NOM, "qauxshared");
    if (handle == NULL)
    {
       if (MessageBox(NULL, "Impossible de créer la mémoire partagée pour QAux.\nIl ne sera donc pas utilisable, continuer ?", "Erreur ! ", MB_YESNO | MB_ICONERROR) == IDNO)
          exit(EXIT_FAILURE);
    }
    else
    {
        sharedAuxMemory = MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(HWND) + TAILLE_MAX_NOM);
        if (sharedAuxMemory == NULL)
        {
          if (MessageBox(NULL, "Impossible de créer la mémoire partagée pour QAux.\nIl ne sera donc pas utilisable, continuer ?", "Erreur ! ", MB_YESNO | MB_ICONERROR) == IDNO)
              exit(EXIT_FAILURE);
        }
        else
        {
            CopyMemory(sharedAuxMemory, &mainWnd, sizeof(HWND));
            sharedAuxMemory += sizeof(HWND);
            ShellExecute(mainWnd, "open", "qaux.exe", NULL, NULL, SW_SHOWMINIMIZED);
        }
    }


    while (continuer)
    {
          action = 0;
          event.type = 0;
          SDL_PollEvent(&event);

          if (event.type == 0 && SDL_GetTicks() - tempsDernierEvenement > 5000)
          {
             action = 1;
             SDL_ShowCursor(0);
          }
          else if (event.type != 0)
          {
               tempsDernierEvenement = SDL_GetTicks();
               SDL_ShowCursor(1);
          }

          if (bigScreenMode)
             action = 1;

          continuer = test_quit(event, &alt);

          differenceAltitude = positionBoutonVolumeGauche.y - positionBoutonVolumeDroit.y;


          if (action == 0 && (cliquer_deplacer(event, boutonVolume, &positionBoutonVolumeDroit, &positionBoutonVolumeAuClic, &clicGaucheSurBoutonVolumeDroit) || cliquer_deplacer(event, boutonVolume, &positionBoutonVolumeGauche, &positionBoutonVolume2AuClic, &clicGaucheSurBoutonVolumeGauche) || clicUpSurBoutonAjustementAudio))
          {
              positionBoutonVolumeDroit.x = abscisseBoutonVolumeDroit;
              positionBoutonVolumeGauche.x = abscisseBoutonVolumeGauche;

              if (lockIndic)
              {
                 if (clicGaucheSurBoutonVolumeDroit)
                        positionBoutonVolumeGauche.y = positionBoutonVolumeDroit.y + differenceAltitude;
                 if (clicGaucheSurBoutonVolumeGauche)
                        positionBoutonVolumeDroit.y = positionBoutonVolumeGauche.y - differenceAltitude;
              }

              if ((positionBoutonVolumeDroit.y + (boutonVolume->h / 2)) > (positionLigneVolumeDroit.y + ligneVolume->h))
              {
                 positionBoutonVolumeDroit.y = positionLigneVolumeDroit.y + ligneVolume->h - (boutonVolume->h / 2);
                 if (lockIndic)
                    positionBoutonVolumeGauche.y = positionBoutonVolumeDroit.y + differenceAltitude;
              }
              if ((positionBoutonVolumeDroit.y + (boutonVolume->h / 2)) < positionLigneVolumeDroit.y)
              {
                 positionBoutonVolumeDroit.y = positionLigneVolumeDroit.y - (boutonVolume->h / 2);
                 if (lockIndic)
                    positionBoutonVolumeGauche.y = positionBoutonVolumeDroit.y + differenceAltitude;
              }
              if ((positionBoutonVolumeGauche.y + (boutonVolume->h / 2)) > (positionLigneVolumeGauche.y + ligneVolume->h))
              {
                 positionBoutonVolumeGauche.y = positionLigneVolumeGauche.y + ligneVolume->h - (boutonVolume->h / 2);
                 if (lockIndic)
                    positionBoutonVolumeDroit.y = positionBoutonVolumeGauche.y - differenceAltitude;
              }
              if ((positionBoutonVolumeGauche.y + (boutonVolume->h / 2)) < positionLigneVolumeGauche.y)
              {
                 positionBoutonVolumeGauche.y = positionLigneVolumeGauche.y - (boutonVolume->h / 2);
                 if (lockIndic)
                    positionBoutonVolumeDroit.y = positionBoutonVolumeGauche.y - differenceAltitude;
              }

              volumeDroit = 255 - (positionBoutonVolumeDroit.y + (boutonVolume->h / 2) - positionLigneVolumeDroit.y);
              volumeGauche = 255 - (positionBoutonVolumeGauche.y + (boutonVolume->h / 2) - positionLigneVolumeGauche.y);

              action = 2;
          }

          if (action == 0 || action == 2)
          {
                     if (action == 2)
                        action = 1;
                     test_button(event, ecran, boutonVolume, positionBoutonVolumeDroit, SDL_MapRGB(ecran->format, 255, 0, 0), SDL_MapRGB(ecran->format, 255, 255, 0), "volume droit", 1500, &tempsDernierMvtSouris, &clicGaucheSurBoutonVolumeDroit);
                     test_button(event, ecran, boutonVolume, positionBoutonVolumeGauche, SDL_MapRGB(ecran->format, 255, 0, 0), SDL_MapRGB(ecran->format, 255, 255, 0), "volume gauche", 1500, &tempsDernierMvtSouris, &clicGaucheSurBoutonVolumeGauche);
          }

          if (toucheActive == SDLK_UP || toucheActive == SDLK_DOWN)
          {
             int ajout = 5, difference = volumeDroit - volumeGauche;
             if (ctrl)
                ajout = 1;
             else if (maj)
                  ajout = 20;

             if (toucheActive == SDLK_DOWN)
                ajout = -ajout;

             volumeGauche += ajout;
             if (volumeGauche > 255)
                volumeGauche = 255;
             if (volumeGauche < 0)
                volumeGauche = 0;

             if (lockIndic)
                volumeDroit = volumeGauche + difference;
             else volumeDroit += ajout;
             if (volumeDroit > 255)
                volumeDroit = 255;
             if (volumeDroit < 0)
                volumeDroit = 0;

             if (lockIndic)
                volumeGauche = volumeDroit - difference;

             positionBoutonVolumeDroit.y = ceil(255 - (boutonVolume->h / 2) + positionLigneVolumeDroit.y - volumeDroit);
             positionBoutonVolumeGauche.y = ceil(255 - (boutonVolume->h / 2) + positionLigneVolumeDroit.y - volumeGauche);
          }

          if (musique.type != RADIO && action == 0)
          {
                  if (cliquer_deplacer(event, flecheLecture, &positionFlecheLecture, &positionFlecheLectureAuClic, &clicGaucheSurFlecheLecture))
                     action = 1;
                  positionFlecheLecture.y = ordonneeFlecheLecture;
                  if (positionFlecheLecture.x > ecran->w - flecheLecture->w - 1)
                         positionFlecheLecture.x = ecran->w - flecheLecture->w - 1;
                  clicGaucheUpSurFlecheLecture = test_button(event, ecran, flecheLecture, positionFlecheLecture, -1, SDL_MapRGB(ecran->format, 0, 255, 0), texteTempsMusiqueEcoule, 0, &tempsDernierMvtSouris, &clicGaucheSurFlecheLecture);
                  if (clicGaucheUpSurFlecheLecture && indicStereoMono)
                  {
                     int newTime = (positionFlecheLecture.x * musique.longueur) / (ecran->w - flecheLecture->w);
                     if (musique.type != VIDEO && indicStereoMono)
                         FSOUND_Stream_SetTime(musique.pointeurStereo, newTime);
                     else if (musique.type != VIDEO)
                          FSOUND_Stream_SetTime(musique.pointeurMono, newTime);
                     else
                     {
                         secondsSkipped = newTime / 1000.0;
                         SMPEG_rewind((SMPEG*) musique.pointeurStereo);
                         SMPEG_play((SMPEG*) musique.pointeurStereo);
                         SMPEG_skip((SMPEG*) musique.pointeurStereo, secondsSkipped);
                     }
                  }
                  clicGaucheUpSurFlecheLecture = 0;
          }

          if (musique.type != RADIO && musique.type != VIDEO && action == 0)
          {
              clicGaucheUpSurIndicStereoMono = test_button(event, ecran, surfaceIndicStereoMono, positionIndicStereoMono, SDL_MapRGB(ecran->format, 255, 0, 0), -1, "stereo / mono", 1500, &tempsDernierMvtSouris, NULL);
              if ((clicGaucheUpSurIndicStereoMono || toucheActive == 'y')&& indicStereoMono)
              {
                    if (toucheActive == 'y')
                       toucheActive = 0;
                    indicStereoMono = 0;
                    SDL_FreeSurface(surfaceIndicStereoMono);
                    surfaceIndicStereoMono = TTF_RenderText_Blended(policeIndicMonoStereo, "Mono", couleurJaune);
                    positionIndicStereoMono.x = (largeurZoneReglage - surfaceIndicStereoMono->w) / 2;
                    positionIndicStereoMono.y = 10;
                    int position = FSOUND_Stream_GetTime(musique.pointeurStereo);
                    FSOUND_Stream_Play(1, musique.pointeurMono);
                    FSOUND_Stream_SetTime(musique.pointeurMono, position);
                    FSOUND_Stream_Stop(musique.pointeurStereo);
                    FSOUND_Stream_SetLoopCount(musique.pointeurStereo, -1);
                    action = 1;
              }
              else if ((clicGaucheUpSurIndicStereoMono || toucheActive == 'y') && indicStereoMono == 0)
              {
                    if (toucheActive == 'y')
                       toucheActive = 0;
                    indicStereoMono = 1;
                    SDL_FreeSurface(surfaceIndicStereoMono);
                    surfaceIndicStereoMono = TTF_RenderText_Blended(policeIndicMonoStereo, "Stereo", couleurJaune);
                    positionIndicStereoMono.x = (largeurZoneReglage - surfaceIndicStereoMono->w) / 2;
                    positionIndicStereoMono.y = 10;
                    int position = FSOUND_Stream_GetTime(musique.pointeurMono);
                    FSOUND_Stream_Play(1, musique.pointeurStereo);
                    FSOUND_Stream_SetTime(musique.pointeurStereo, position);
                    FSOUND_Stream_Stop(musique.pointeurMono);
                    FSOUND_Stream_SetLoopCount(musique.pointeurMono, -1);
                    action = 1;
              }
              clicGaucheUpSurIndicStereoMono = 0;
          }

          if (action == 0 || mettreEnPlayOuPause || mettreEnStop || toucheActive == SDLK_PAUSE || toucheActive == 'x')
          {
              if (musique.mode != ERREUR)
                 clicGaucheUpSurBoutonPlayPause = test_button_rond(event, ecran, surfaceBoutonPlayPause, positionBoutonPlayPause, SDL_MapRGB(ecran->format, 255, 0, 0), -1, "play / pause", 1500, &tempsDernierMvtSouris, NULL);
              if ((clicGaucheUpSurBoutonPlayPause == 1 || mettreEnPlayOuPause || toucheActive == SDLK_PAUSE) && musique.mode == PLAY)
              {
                 if (toucheActive == SDLK_PAUSE)
                    toucheActive = 0;
                 musique.mode = PAUSE;
                 if (musique.type != VIDEO)
                    FSOUND_SetPaused(FSOUND_ALL, 1);
                 else SMPEG_pause((SMPEG*) musique.pointeurStereo);
                 action = 1;
              }
              else if ((clicGaucheUpSurBoutonPlayPause == 1 || mettreEnPlayOuPause || toucheActive == SDLK_PAUSE) && musique.mode == PAUSE)
              {
                   if (toucheActive == SDLK_PAUSE)
                      toucheActive = 0;
                   musique.mode = PLAY;
                   if (musique.type != VIDEO)
                      FSOUND_SetPaused(FSOUND_ALL, 0);
                   else SMPEG_play((SMPEG*) musique.pointeurStereo);
                   action = 1;
              }
              else if ((clicGaucheUpSurBoutonPlayPause == 1 || mettreEnPlayOuPause || toucheActive == SDLK_PAUSE) && musique.mode == STOP)
              {
                   if (toucheActive == SDLK_PAUSE)
                      toucheActive = 0;
                   musique.mode = PLAY;
                   if (musique.type != VIDEO)
                   {
                       if (indicStereoMono)
                          FSOUND_Stream_Play(1, musique.pointeurStereo);
                       else FSOUND_Stream_Play(1, musique.pointeurMono);
                   }
                   else SMPEG_play((SMPEG*) musique.pointeurStereo);
                   action = 1;
              }
              else if (clicGaucheUpSurBoutonPlayPause == 2 || mettreEnStop || toucheActive == 'x')
              {
                   if (toucheActive == 'x')
                      toucheActive = 0;
                   mettre_en_stop(&musique, surfaceBoutonPlayPause);
                   action = 1;
              }

              clicGaucheUpSurBoutonPlayPause = 0;
              mettreEnPlayOuPause = 0;
              mettreEnStop = 0;
          }


          if (action == 0)
          {
                     clicGaucheUpSurBoutonAvanceRapide = test_button(event, ecran, boutonAvanceRapide, positionBoutonAvanceRapide, SDL_MapRGB(ecran->format, 255, 0, 0), SDL_MapRGB(ecran->format, 255, 255, 0), "suivant / avance rapide", 1500, &tempsDernierMvtSouris, &clicGaucheSurBoutonAvanceRapide);
                     clicGaucheUpSurBoutonReculRapide = test_button(event, ecran, boutonReculRapide, positionBoutonReculRapide, SDL_MapRGB(ecran->format, 255, 0, 0), SDL_MapRGB(ecran->format, 255, 255, 0), "début / précédent / recul rapide", 1500, &tempsDernierMvtSouris, &clicGaucheSurBoutonReculRapide);
          }


          if (clicGaucheSurBoutonAvanceRapide && tempsDernierClicGaucheSurBoutonAvanceRapide == -1)
             tempsDernierClicGaucheSurBoutonAvanceRapide = SDL_GetTicks();
          if ((clicGaucheUpSurBoutonAvanceRapide && action == 0) || passerAuSuivant || toucheActive == 's')
          {
                action = 1;
                if (SDL_GetTicks() - tempsDernierClicGaucheSurBoutonAvanceRapide <= 1500 || passerAuSuivant || toucheActive == 's')
                {
                                if (toucheActive == 's')
                                   toucheActive = 0;
                                if (musique.type != VIDEO)
                                {
                                    FSOUND_Stream_Stop(musique.pointeurStereo);
                                    FSOUND_Stream_Stop(musique.pointeurMono);
                                }
                                else SMPEG_rewind((SMPEG*) musique.pointeurStereo);

                                positionDansTableauDernieresMusiques++;
                                if (positionDansTableauDernieresMusiques > 10)
                                   positionDansTableauDernieresMusiques = 0;

                                i = 0;
                                do
                                {
                                    if (musique.type != VIDEO)
                                    {
                                        FSOUND_Stream_Close(musique.pointeurStereo);
                                        FSOUND_Stream_Close(musique.pointeurMono);
                                    }
                                    else SMPEG_delete((SMPEG*) musique.pointeurStereo);

                                    if (tableauDernieresMusiques[positionDansTableauDernieresMusiques] >= 6000)
                                    {
                                       charger_musique(&musique, configMenu, &affichageParoles, nomFichierListe, tableauDernieresMusiques[positionDansTableauDernieresMusiques] - 6000, positionBoutonReculRapide.x - musique.position.titre.x - 40, &i);
                                       while (musique.mode == ERREUR && i < configMenu.parametre.nombreChargementsMaxi)
                                       {
                                           tableauTotalMusiques[-musique.numero] = -1;
                                           charger_musique(&musique, configMenu, &affichageParoles, nomFichierListe, -1, positionBoutonReculRapide.x - musique.position.titre.x - 40, &i);
                                           i++;
                                       }
                                    }
                                    else
                                    {
                                        do
                                        {
                                            if (musique.mode == ERREUR)
                                                 tableauTotalMusiques[musique.numero] = -1;
                                            if (modeAleatoire)
                                                 charger_musique(&musique, configMenu, &affichageParoles, nomFichierListe, -1, positionBoutonReculRapide.x - musique.position.titre.x - 40, &i);
                                            else
                                            {
                                                 musique.numero++;
                                                 if (musique.numero > nombreMusiques)
                                                    musique.numero = 1;
                                                 charger_musique(&musique, configMenu, &affichageParoles, nomFichierListe, musique.numero, positionBoutonReculRapide.x - musique.position.titre.x - 40, &i);
                                            }
                                            i++;
                                        } while (musique.mode == ERREUR && i < configMenu.parametre.nombreChargementsMaxi);
                                    }
                                    if (i >= configMenu.parametre.nombreChargementsMaxi)
                                    {
                                          for (j = 0 ; j < TAILLE_MAX_PLAYLIST ; j++)
                                          {
                                              if (tableauTotalMusiques[j] > 0)
                                                 tableauTotalMusiques[j] = 0;
                                          }
                                          if (musique.mode != ERREUR)
                                             musique.mode = STOP;
                                    }
                                } while (tableauTotalMusiques[musique.numero] && modeAleatoire && i < configMenu.parametre.nombreChargementsMaxi);

                                if (nombreMusiques > nombreMusiquesAffichables  && (musique.numero < ligneDebut || musique.numero > ligneFin))
                                {
                                              ligneDebut = musique.numero - 1;
                                              ligneFin = ligneDebut + nombreMusiquesAffichables;
                                              positionBarreDefilement.y = (ligneDebut * (hauteurZoneLecteur - hauteurBarreDefilement)) / ((nombreMusiques - nombreMusiquesAffichables) / 1.0);
                                }

                                if (musique.mode != PLAY)
                                   mettre_en_stop(&musique, surfaceBoutonPlayPause);
                                else
                                {
                                    if (musique.type != VIDEO)
                                    {
                                        if (indicStereoMono)
                                           FSOUND_Stream_Play(1, musique.pointeurStereo);
                                        else FSOUND_Stream_Play(1, musique.pointeurMono);
                                    }
                                    else SMPEG_play((SMPEG*) musique.pointeurStereo);
                                    tableauDernieresMusiques[positionDansTableauDernieresMusiques] = musique.numero;
                                    tableauTotalMusiques[musique.numero] = 1;
                                }

                                if (musique.type != VIDEO)
                                {
                                   FSOUND_SetVolume(1, volume);
                                   FSOUND_SetPan(1, ceil((255 - volumeGauche + volumeDroit) / 2));
                                }
                                else SMPEG_setvolume((SMPEG*) musique.pointeurStereo, volume * 100 / 255);
                }

                tempsDernierClicGaucheSurBoutonAvanceRapide = -1;
                passerAuSuivant = 0;
          }
          if (clicGaucheSurBoutonReculRapide && tempsDernierClicGaucheSurBoutonReculRapide == -1)
             tempsDernierClicGaucheSurBoutonReculRapide = SDL_GetTicks();
          if ((clicGaucheUpSurBoutonReculRapide && action == 0) || passerAuPrecedent || toucheActive == 'p')
          {
             action = 1;
             if (SDL_GetTicks() - tempsDernierClicGaucheSurBoutonReculRapide <= 1500 || passerAuPrecedent || toucheActive == 'p')
             {
                                if (toucheActive == 'p')
                                   toucheActive = 0;
                                if (musique.curseur >= 5000)
                                {
                                   if (musique.type != VIDEO)
                                   {
                                       FSOUND_Stream_SetTime(musique.pointeurStereo, 0);
                                       FSOUND_Stream_SetTime(musique.pointeurMono, 0);
                                   }
                                   else
                                   {
                                       secondsSkipped = 0;
                                       SMPEG_rewind((SMPEG*) musique.pointeurStereo);
                                       SMPEG_play((SMPEG*) musique.pointeurStereo);
                                   }
                                }
                                else
                                {
                                    if (musique.type != VIDEO)
                                    {
                                        FSOUND_Stream_Stop(musique.pointeurStereo);
                                        FSOUND_Stream_Stop(musique.pointeurMono);
                                        FSOUND_Stream_Close(musique.pointeurStereo);
                                        FSOUND_Stream_Close(musique.pointeurMono);
                                    }
                                    else
                                    {
                                        SMPEG_stop((SMPEG*) musique.pointeurStereo);
                                        SMPEG_delete((SMPEG*) musique.pointeurStereo);
                                    }

                                    j = 0;
                                    if (modeAleatoire)
                                    {
                                       tableauDernieresMusiques[positionDansTableauDernieresMusiques] += 6000;
                                       tableauTotalMusiques[musique.numero] = 0;
                                       positionDansTableauDernieresMusiques--;
                                       if (positionDansTableauDernieresMusiques < 0)
                                          positionDansTableauDernieresMusiques = 10;
                                       charger_musique(&musique, configMenu, &affichageParoles, nomFichierListe, tableauDernieresMusiques[positionDansTableauDernieresMusiques], positionBoutonReculRapide.x - musique.position.titre.x - 40, &j);
                                       while (musique.mode == ERREUR && j < configMenu.parametre.nombreChargementsMaxi)
                                       {
                                           tableauTotalMusiques[musique.numero] = -1;
                                           charger_musique(&musique, configMenu, &affichageParoles, nomFichierListe, -1, positionBoutonReculRapide.x - musique.position.titre.x - 40, &j);
                                           j++;
                                       }
                                    }
                                    else
                                    {
                                        do
                                        {
                                                       if (musique.mode == ERREUR)
                                                          tableauTotalMusiques[musique.numero] = -1;
                                                       musique.numero--;
                                                       if (musique.numero < 1)
                                                          musique.numero = nombreMusiques;
                                                       charger_musique(&musique, configMenu, &affichageParoles, nomFichierListe, musique.numero, positionBoutonReculRapide.x - musique.position.titre.x - 40, &j);
                                                       j++;
                                        } while (musique.mode == ERREUR && j < configMenu.parametre.nombreChargementsMaxi);
                                    }
                                    if (nombreMusiques > nombreMusiquesAffichables  && (musique.numero < ligneDebut || musique.numero > ligneFin))
                                    {
                                              ligneDebut = musique.numero - 1;
                                              ligneFin = ligneDebut + nombreMusiquesAffichables;
                                              positionBarreDefilement.y = (ligneDebut * (hauteurZoneLecteur - hauteurBarreDefilement)) / ((nombreMusiques - nombreMusiquesAffichables) / 1.0);
                                    }
                                   if (musique.mode != PLAY)
                                      mettre_en_stop(&musique, surfaceBoutonPlayPause);
                                   else
                                   {
                                        if (musique.type != VIDEO)
                                        {
                                            if (indicStereoMono)
                                               FSOUND_Stream_Play(1, musique.pointeurStereo);
                                            else FSOUND_Stream_Play(1, musique.pointeurMono);
                                        }
                                        else SMPEG_play((SMPEG*) musique.pointeurStereo);

                                        tableauTotalMusiques[musique.numero] = 1;
                                   }

                                   if (musique.type != VIDEO)
                                   {
                                       FSOUND_SetVolume(1, volume);
                                       FSOUND_SetPan(1, ceil((255 - volumeGauche + volumeDroit) / 2));
                                   }
                                   else SMPEG_setvolume((SMPEG*) musique.pointeurStereo, volume * 100 / 255);
                                }
             }

             tempsDernierClicGaucheSurBoutonReculRapide = -1;
             passerAuPrecedent = 0;
          }


          if ((clicGaucheSurBoutonAvanceRapide && SDL_GetTicks() - tempsDernierClicGaucheSurBoutonAvanceRapide >= 1500) || toucheActive == SDLK_RIGHT)
          {
             if (musique.type != VIDEO && indicStereoMono)
                FSOUND_Stream_SetTime(musique.pointeurStereo, musique.curseur + 1000);
             else if (musique.type != VIDEO)
                  FSOUND_Stream_SetTime(musique.pointeurMono, musique.curseur + 1000);
             else
             {
                 secondsSkipped += 1.0;
                 SMPEG_skip((SMPEG*) musique.pointeurStereo, 1.0);
             }
          }

          if ((clicGaucheSurBoutonReculRapide && SDL_GetTicks() - tempsDernierClicGaucheSurBoutonReculRapide >= 1500) || toucheActive == SDLK_LEFT)
          {
             if (musique.type != VIDEO && indicStereoMono)
                FSOUND_Stream_SetTime(musique.pointeurStereo, musique.curseur - 1000);
             else if (musique.type != VIDEO)
                  FSOUND_Stream_SetTime(musique.pointeurMono, musique.curseur - 1000);
             else
             {
                 secondsSkipped = (musique.curseur - 1000) / 1000.0;
                 SMPEG_rewind((SMPEG*) musique.pointeurStereo);
                 SMPEG_play((SMPEG*) musique.pointeurStereo);
                 SMPEG_skip((SMPEG*) musique.pointeurStereo, secondsSkipped);
             }
          }

          if (action == 0 && (test_button(event, ecran, surfaceLockIndic, positionLockIndic, SDL_MapRGB(ecran->format, 255, 0, 0), -1, "attacher / détacher les canaux gauche et droite", 1500, &tempsDernierMvtSouris, NULL) || toucheActive == 'v'))
          {
             if (toucheActive == 'v')
                toucheActive = 0;
             action = 1;
             SDL_FreeSurface(surfaceLockIndic);
             if (lockIndic)
             {
                surfaceLockIndic = TTF_RenderText_Blended(policeLockIndic, "Unlocked", couleurVert);
                positionLockIndic.x = (largeurZoneReglage - (surfaceLockIndic->w + boutonAjustementAudio->w + 10)) / 2;
                lockIndic = 0;
             }
             else
             {
                 surfaceLockIndic = TTF_RenderText_Blended(policeLockIndic, "Locked", couleurRouge);
                 positionLockIndic.x = (largeurZoneReglage - (surfaceLockIndic->w + boutonAjustementAudio->w + 10)) / 2;
                 lockIndic = 1;
             }
          }


          if (action == 0)
          {
              clicUpSurBoutonAjustementAudio = test_button(event, ecran, boutonAjustementAudio, positionBoutonAjustementAudio, SDL_MapRGB(ecran->format, 255, 0, 0), -1, "égaliser les canaux gauche et droite", 1500, &tempsDernierMvtSouris, NULL);
              if (clicUpSurBoutonAjustementAudio == 1 || (toucheActive == 'e' && !ctrl))
              {
                 action = 1;
                 if (toucheActive == 'e')
                    toucheActive = 0;
                 positionBoutonVolumeDroit.y = positionBoutonVolumeGauche.y;
              }
              else if (clicUpSurBoutonAjustementAudio == 2 || (toucheActive == 'e' && ctrl))
              {
                   action = 1;
                   if (toucheActive == 'e')
                      toucheActive = 0;
                   positionBoutonVolumeGauche.y = positionBoutonVolumeDroit.y;
              }

              clicUpSurBoutonAjustementAudio = 0;
              volumeDroit = 255 - (positionBoutonVolumeDroit.y + (boutonVolume->h / 2) - positionLigneVolumeDroit.y);
              volumeGauche = 255 - (positionBoutonVolumeGauche.y + (boutonVolume->h / 2) - positionLigneVolumeGauche.y);
          }


          for (i = ligneDebut ; tableauSurfacesPlaylist[i] != NULL && i < TAILLE_MAX_PLAYLIST && action == 0 ; i++)
          {
                     if (strstr(tableauTextesPlaylist[i], "...") != NULL)
                     {
                        lire_ligne(nomFichierPlaylist, i + 1, parametre, TAILLE_MAX_NOM);
                        clicGaucheUpSurPlaylist[i] = test_button(event, ecran, tableauSurfacesPlaylist[i], tableauPositionsPlaylist[i], SDL_MapRGB(ecran->format, 255, 0, 0), -1, parametre, 500, &tempsDernierMvtSouris, NULL);
                     }
                     else
                         clicGaucheUpSurPlaylist[i] = test_button(event, ecran, tableauSurfacesPlaylist[i], tableauPositionsPlaylist[i], SDL_MapRGB(ecran->format, 255, 0, 0), -1, NULL, 0, NULL, NULL);
                     if ((clicGaucheUpSurPlaylist[i] && modeListe == 0) || (musique.type == RADIO && musique.numero - 1 == i && (toucheActive == 'q' || test_button(event, ecran, boutonActualiser, positionBoutonActualiser, SDL_MapRGB(ecran->format, 255, 0, 0), -1, "actualiser la radio", 1500, &tempsDernierMvtSouris, NULL))))
                     {
                           if (toucheActive == 'q')
                              toucheActive = 0;
                           action = 1;

                           if (musique.type != VIDEO)
                           {
                               FSOUND_Stream_Stop(musique.pointeurStereo);
                               FSOUND_Stream_Stop(musique.pointeurMono);
                               FSOUND_Stream_Close(musique.pointeurStereo);
                               FSOUND_Stream_Close(musique.pointeurMono);
                           }
                           else
                           {
                               SMPEG_stop((SMPEG*) musique.pointeurStereo);
                               SMPEG_delete((SMPEG*) musique.pointeurStereo);
                           }

                           j = 0;
                           charger_musique(&musique, configMenu, &affichageParoles, nomFichierListe, i + 1, positionBoutonReculRapide.x - musique.position.titre.x - 40, &j);
                           j++;
                           while (musique.mode == ERREUR && j < configMenu.parametre.nombreChargementsMaxi)
                           {
                                   if (musique.mode == ERREUR)
                                       tableauTotalMusiques[musique.numero] = -1;
                                   if (modeAleatoire)
                                      charger_musique(&musique, configMenu, &affichageParoles, nomFichierListe, -1, positionBoutonReculRapide.x - musique.position.titre.x - 40, &j);
                                   else
                                   {
                                       musique.numero++;
                                       if (musique.numero > nombreMusiques)
                                          musique.numero = 1;
                                       charger_musique(&musique, configMenu, &affichageParoles, nomFichierListe, musique.numero, positionBoutonReculRapide.x - musique.position.titre.x - 40, &j);
                                   }
                                   j++;
                           }
                           for (j = 0 ; j < TAILLE_MAX_PLAYLIST ; j++)
                           {
                               if (tableauTotalMusiques[j] > 0)
                                  tableauTotalMusiques[j] = 0;
                           }

                           if (musique.mode == STOP || musique.mode == ERREUR)
                                   mettre_en_stop(&musique, surfaceBoutonPlayPause);
                           else
                           {
                               if (musique.type != VIDEO)
                               {
                                   if (indicStereoMono)
                                      FSOUND_Stream_Play(1, musique.pointeurStereo);
                                   else FSOUND_Stream_Play(1, musique.pointeurMono);
                               }
                               else SMPEG_play((SMPEG*) musique.pointeurStereo);

                               positionDansTableauDernieresMusiques++;
                               if (positionDansTableauDernieresMusiques > 10)
                                  positionDansTableauDernieresMusiques = 0;
                               tableauDernieresMusiques[positionDansTableauDernieresMusiques] = musique.numero;
                               tableauTotalMusiques[musique.numero] = 1;

                               musique.mode = PLAY;
                           }
                     }
                     else if (clicGaucheUpSurPlaylist[i] && modeListe)
                     {
                           action = 1;
                           if ((fichierListePlaylists = fopen(nomFichierListePlaylists, "r")) == NULL)
                              fprintf(stderr, "ERR main %d : Le fichier %s n'a pas pu etre ouvert", __LINE__, nomFichierListePlaylists);
                           else
                           {
                               char nomNouvellePlaylist[TAILLE_MAX_NOM] = " ";
                               char nomNouvelleListe[TAILLE_MAX_NOM] = " ";

                               for (j = 1 ; j <= i + 1 ; j++)
                               {
                                   fgets(nomNouvellePlaylist, TAILLE_MAX_NOM, fichierListePlaylists);
                               }

                               while(fclose(fichierListePlaylists) == EOF);

                               if ((positionEntree = strchr(nomNouvellePlaylist, '\n')) != NULL)
                                      *positionEntree = '\0';

                               mettre_a_jour_playlists(nomFichierListePlaylists, nomFichierPlaylistDePlaylists, tableauTotalMusiques);

                               char *positionExtension = NULL;
                               strcpy(nomNouvelleListe, nomNouvellePlaylist);
                               if ((positionExtension = strstr(nomNouvelleListe, ".npl")) != NULL)
                                      strcpy(positionExtension, ".list");

                               if (test_exist(nomNouvellePlaylist) == 1 && test_vide(nomNouvellePlaylist) == 1 && test_exist(nomNouvelleListe) == 1 && test_vide(nomNouvelleListe) == 1)
                               {
                                   strcpy(nomFichierPlaylist, nomNouvellePlaylist);
                                   strcpy(nomFichierListe, nomNouvelleListe);

                                   modeListe = 0;
                                   SDL_FreeSurface(boutonModeListe);
                                   boutonModeListe = IMG_Load("img\\other\\bouton_playlists.jpg");

                                   if ((fichierPlaylist = fopen(nomFichierPlaylist, "r")) == NULL)
                                      fprintf(stderr, "ERR main %d : Le fichier %s n'a pas pu etre ouvert.\n", __LINE__, nomFichierPlaylist);
                                   nombreMusiques = 0;
                                   while(fgets(temp, TAILLE_MAX_NOM, fichierPlaylist) != NULL)
                                                     nombreMusiques++;
                                   while(fclose(fichierPlaylist) == EOF);

                                   hauteurBarreDefilement = (nombreMusiquesAffichables / (nombreMusiques / 1.0)) * hauteurZoneLecteur;
                                   SDL_FreeSurface(barreDefilement);
                                   barreDefilement = SDL_CreateRGBSurface(SDL_HWSURFACE, 26, hauteurBarreDefilement, 32, 0, 0, 0, 0);
                                   SDL_FreeSurface(rectangle);
                                   rectangle = SDL_CreateRGBSurface(SDL_HWSURFACE, 1, hauteurBarreDefilement, 32, 0, 0, 0, 0);
                                   for (i = 0 ; i <= 12 ; i++)
                                   {
                                       SDL_FillRect(rectangle, NULL, SDL_MapRGB(ecran->format, (255 * i) / 12, (255 * i) / 12, (255 * i) / 12));
                                       position.x = i;
                                       SDL_BlitSurface(rectangle, NULL, barreDefilement, &position);
                                   }
                                   for (i = 0 ; i <= 12 ; i++)
                                   {
                                       SDL_FillRect(rectangle, NULL, SDL_MapRGB(ecran->format, 255 - (255 * i) / 12, 255 - (255 * i) / 12, 255 - (255 * i) / 12));
                                       position.x = i + 13;
                                       SDL_BlitSurface(rectangle, NULL, barreDefilement, &position);
                                   }
                                   positionBarreDefilement.x = positionLigneSeparation3.x;
                                   positionBarreDefilement.y = 0;
                               }
                               else
                               {
                                   remove(nomNouvelleListe);
                                   remove(nomNouvellePlaylist);
                                   mettre_a_jour_playlists(nomFichierListePlaylists, nomFichierPlaylistDePlaylists, tableauTotalMusiques);
                               }
                           }
                     }
          }


          if (nombreMusiquesAffichables < nombreMusiques && action == 0)
          {
             if (cliquer_deplacer (event, barreDefilement, &positionBarreDefilement, &positionBarreDefilementAuClic, &clicGaucheSurBarreDefilement))
                action = 1;
             positionBarreDefilement.x = positionLigneSeparation3.x;
             if (positionBarreDefilement.y < 0)
                    positionBarreDefilement.y = 0;
             if (positionBarreDefilement.y > hauteurZoneLecteur - hauteurBarreDefilement)
                    positionBarreDefilement.y = hauteurZoneLecteur - hauteurBarreDefilement;
             ligneDebut = (positionBarreDefilement.y * (nombreMusiques - nombreMusiquesAffichables)) / ((hauteurZoneLecteur - hauteurBarreDefilement) / 1.0);
             ligneFin = ligneDebut + nombreMusiquesAffichables;
          }

          abscisseLigneSeparation3 = positionLigneSeparation3.x;
          if (action == 0 && cliquer_deplacer (event, ligneSeparation3, &positionLigneSeparation3, &positionLigneSeparation3AuClic, &clicGaucheSurLigneSeparation3))
          {
              action = 1;
              deplacementLigneSeparation3 = 1;
              positionLigneSeparation3.y = 0;
              if (clicGaucheSurBarreDefilement)
                 positionLigneSeparation3.x = abscisseLigneSeparation3;
              if (positionLigneSeparation3.x > ecran->w - ligneSeparation3->w)
                    positionLigneSeparation3.x = ecran->w - ligneSeparation3->w;
              if (positionLigneSeparation3.x < largeurZoneReglage + ligneSeparation->w)
                    positionLigneSeparation3.x = largeurZoneReglage + ligneSeparation->w;
              largeurZoneListe = ecran->w - positionLigneSeparation3.x;
              largeurZoneGraphique = ecran->w - largeurZoneReglage - largeurZoneListe;
              positionPlaylist.x = ecran->w - largeurZoneListe + 30;
              positionBoutonModeListe.x = ecran->w - largeurZoneListe + 25 + ((largeurZoneListe - 25 - boutonModeListe->w) / 2);
              if (positionBoutonModeListe.x > ecran->w - boutonModeListe->w - 5)
                    positionBoutonModeListe.x = 2000;
              positionBoutonAlpha.x = ecran->w - largeurZoneListe + 25 + (((positionBoutonModeListe.x - (ecran->w - largeurZoneListe + 25)) - boutonAlpha->w) / 2);
              if (positionBoutonAlpha.x >= positionBoutonModeListe.x - 5 - boutonAlpha->w)
                 positionBoutonAlpha.x = 2000;
              positionBoutonTypePlaylist.x = positionBoutonModeListe.x + boutonModeListe->w + ((ecran->w - positionBoutonModeListe.x - boutonModeListe->w - boutonTypePlaylist->w) / 2);
              if (positionBoutonTypePlaylist.x <= positionBoutonModeListe.x + 5 + boutonModeListe->w)
                 positionBoutonTypePlaylist.x = 2000;
              positionBarreDefilement.x = positionLigneSeparation3.x;
          }
          else if (deplacementLigneSeparation3)
          {
              event.type = SDL_VIDEORESIZE;
              event.resize.h = -1;
              deplacementLigneSeparation3 = 0;
              action = 1;
          }

          if (modeListe == 0 && action == 0)
          {
             if (toucheActive == 'l' || test_button(event, ecran, boutonModeListe, positionBoutonModeListe, SDL_MapRGB(ecran->format, 255, 0, 0), -1, "afficher les playlists", 1500, &tempsDernierMvtSouris, NULL))
             {
                if (toucheActive == 'l')
                   toucheActive = 0;
                action = 1;
                modeListe = 1;
                SDL_FreeSurface(boutonModeListe);
                boutonModeListe = IMG_Load("img\\other\\bouton_music_list.jpg");

                if ((fichierPlaylistDePlaylists = fopen(nomFichierPlaylistDePlaylists, "r")) == NULL)
                   fprintf(stderr, "ERR main %d : Le fichier %s n'a pas pu etre ouvert.\n", __LINE__, nomFichierPlaylist);
                else
                {
                    nombreMusiques = 0;
                    while(fgets(temp, TAILLE_MAX_NOM, fichierPlaylistDePlaylists) != NULL)
                       nombreMusiques++;
                    while(fclose(fichierPlaylistDePlaylists) == EOF);

                    hauteurBarreDefilement = (nombreMusiquesAffichables / (nombreMusiques / 1.0)) * hauteurZoneLecteur;
                    SDL_FreeSurface(barreDefilement);
                    barreDefilement = SDL_CreateRGBSurface(SDL_HWSURFACE, 26, hauteurBarreDefilement, 32, 0, 0, 0, 0);
                    SDL_FreeSurface(rectangle);
                    rectangle = SDL_CreateRGBSurface(SDL_HWSURFACE, 1, hauteurBarreDefilement, 32, 0, 0, 0, 0);
                    for (i = 0 ; i <= 12 ; i++)
                    {
                        SDL_FillRect(rectangle, NULL, SDL_MapRGB(ecran->format, (255 * i) / 12, (255 * i) / 12, (255 * i) / 12));
                        position.x = i;
                        SDL_BlitSurface(rectangle, NULL, barreDefilement, &position);
                    }
                    for (i = 0 ; i <= 12 ; i++)
                    {
                        SDL_FillRect(rectangle, NULL, SDL_MapRGB(ecran->format, 255 - (255 * i) / 12, 255 - (255 * i) / 12, 255 - (255 * i) / 12));
                        position.x = i + 13;
                        SDL_BlitSurface(rectangle, NULL, barreDefilement, &position);
                    }
                    positionBarreDefilement.x = positionLigneSeparation3.x;
                    positionBarreDefilement.y = 0;
                    ligneDebut = 0;
                    ligneFin = nombreMusiquesAffichables;
                }
             }
          }
          else if (action == 0)
          {
              if (toucheActive == 'l' || test_button(event, ecran, boutonModeListe, positionBoutonModeListe, SDL_MapRGB(ecran->format, 255, 0, 0), -1, "afficher les musiques", 1500, &tempsDernierMvtSouris, NULL))
              {
                 if (toucheActive == 'l')
                    toucheActive = 0;
                 action = 1;
                 modeListe = 0;
                 SDL_FreeSurface(boutonModeListe);
                 boutonModeListe = IMG_Load("img\\other\\bouton_playlists.jpg");

                 if ((fichierPlaylist = fopen(nomFichierPlaylist, "r")) == NULL)
                    fprintf(stderr, "ERR main %d : Le fichier %s n'a pas pu etre ouvert.\n", __LINE__, nomFichierPlaylist);
                 else
                 {
                     nombreMusiques = 0;
                     while(fgets(temp, TAILLE_MAX_NOM, fichierPlaylist) != NULL)
                                       nombreMusiques++;
                     while(fclose(fichierPlaylist) == EOF);

                     hauteurBarreDefilement = (nombreMusiquesAffichables / (nombreMusiques / 1.0)) * hauteurZoneLecteur;
                     SDL_FreeSurface(barreDefilement);
                     barreDefilement = SDL_CreateRGBSurface(SDL_HWSURFACE, 26, hauteurBarreDefilement, 32, 0, 0, 0, 0);
                     SDL_FreeSurface(rectangle);
                     rectangle = SDL_CreateRGBSurface(SDL_HWSURFACE, 1, hauteurBarreDefilement, 32, 0, 0, 0, 0);
                     for (i = 0 ; i <= 12 ; i++)
                     {
                         SDL_FillRect(rectangle, NULL, SDL_MapRGB(ecran->format, (255 * i) / 12, (255 * i) / 12, (255 * i) / 12));
                         position.x = i;
                         SDL_BlitSurface(rectangle, NULL, barreDefilement, &position);
                     }
                     for (i = 0 ; i <= 12 ; i++)
                     {
                         SDL_FillRect(rectangle, NULL, SDL_MapRGB(ecran->format, 255 - (255 * i) / 12, 255 - (255 * i) / 12, 255 - (255 * i) / 12));
                         position.x = i + 13;
                         SDL_BlitSurface(rectangle, NULL, barreDefilement, &position);
                     }
                     positionBarreDefilement.x = positionLigneSeparation3.x;
                     positionBarreDefilement.y = 0;
                     ligneDebut = 0;
                     ligneFin = nombreMusiquesAffichables;
                 }

              }
          }


          if (modeAleatoire && action == 0)
          {
                            if ((toucheActive == 'q' && musique.type != RADIO) || test_button(event, ecran, boutonAleatoire, positionBoutonAleatoire, SDL_MapRGB(ecran->format, 255, 0, 0), -1, "lecture aléatoire activée", 1500, &tempsDernierMvtSouris, NULL))
                            {
                                 if (toucheActive == 'q')
                                    toucheActive = 0;
                                 action = 1;
                                 modeAleatoire = 0;
                                 SDL_FreeSurface(boutonAleatoire);
                                 boutonAleatoire = IMG_Load("img\\other\\bouton_lecture_normale_reduit.jpg");
                            }
          }
          else if (action == 0)
          {
               if ((toucheActive == 'q' && musique.type != RADIO) || test_button(event, ecran, boutonAleatoire, positionBoutonAleatoire, SDL_MapRGB(ecran->format, 255, 0, 0), -1, "lecture aléatoire désactivée", 1500, &tempsDernierMvtSouris, NULL))
               {
                                 if (toucheActive == 'q')
                                    toucheActive = 0;
                                 action = 1;
                                 modeAleatoire = 1;
                                 SDL_FreeSurface(boutonAleatoire);
                                 boutonAleatoire = IMG_Load("img\\other\\bouton_lecture_aleatoire_reduit.jpg");
                                 for (j = 0 ; j < TAILLE_MAX_PLAYLIST ; j++)
                                 {
                                     if (tableauTotalMusiques[j] > 0)
                                        tableauTotalMusiques[j] = 0;
                                     if (j <= 10)
                                        tableauDernieresMusiques[j] = -1;
                                 }
               }
          }


          if (modeBoucle && musique.type != RADIO && action == 0)
          {
                         if (toucheActive == 'b' || test_button(event, ecran, boutonBoucle, positionBoutonBoucle, SDL_MapRGB(ecran->format, 255, 0, 0), -1, "lecture en boucle activée", 1500, &tempsDernierMvtSouris, NULL))
                            {
                                 if (toucheActive == 'b')
                                    toucheActive = 0;
                                 action = 1;
                                 modeBoucle = 0;
                                 SDL_FreeSurface(boutonBoucle);
                                 boutonBoucle = IMG_Load("img\\other\\bouton_lecture_sans_boucle_reduit.jpg");
                            }
          }
          else if (musique.type != RADIO && action == 0)
          {
                         if (toucheActive == 'b' || test_button(event, ecran, boutonBoucle, positionBoutonBoucle, SDL_MapRGB(ecran->format, 255, 0, 0), -1, "lecture en boucle désactivée", 1500, &tempsDernierMvtSouris, NULL))
                            {
                                 if (toucheActive == 'b')
                                    toucheActive = 0;
                                 action = 1;
                                 modeBoucle = 1;
                                 SDL_FreeSurface(boutonBoucle);
                                 boutonBoucle = IMG_Load("img\\other\\bouton_lecture_boucle_reduit.jpg");
                            }
          }


          if (musique.type != RADIO && action == 0)
          {
                  switch(modeAB)
                  {
                                case 0:
                                     if (toucheActive == ';' || test_button(event, ecran, boutonAB, positionBoutonAB, SDL_MapRGB(ecran->format, 255, 0, 0), -1, "placer le repère A", 1500, &tempsDernierMvtSouris, NULL))
                                     {
                                         if (toucheActive == ';')
                                            toucheActive = 0;
                                         action = 1;
                                         modeAB = 1;
                                         lettreA = TTF_RenderText_Blended(policeAB, "A", couleurJaune);
                                         positionLettreA = positionFlecheLecture;
                                         tempsPointA = musique.curseur;
                                     }
                                     break;
                                case 1:
                                     clicUpSurBoutonAB = test_button(event, ecran, boutonAB, positionBoutonAB, SDL_MapRGB(ecran->format, 255, 0, 0), -1, "placer le repère B", 1500, &tempsDernierMvtSouris, NULL);
                                     if (clicUpSurBoutonAB == 1 || (toucheActive == ';' && !ctrl))
                                     {
                                         if (toucheActive == ';')
                                            toucheActive = 0;
                                         action = 1;
                                         modeAB = 2;
                                         lettreB = TTF_RenderText_Blended(policeAB, "B", couleurVert);
                                         positionLettreB = positionFlecheLecture;
                                         tempsPointB = musique.curseur;
                                         SDL_FreeSurface(boutonAB);
                                         boutonAB = IMG_Load("img\\other\\bouton_lecture_AB_ok_reduit.jpg");

                                         if (musique.type != VIDEO)
                                         {
                                             FSOUND_Stream_SetTime(musique.pointeurStereo, tempsPointA);
                                             FSOUND_Stream_SetTime(musique.pointeurMono, tempsPointA);
                                         }
                                         else
                                         {
                                             secondsSkipped = tempsPointA / 1000.0;
                                             SMPEG_rewind((SMPEG*) musique.pointeurStereo);
                                             SMPEG_play((SMPEG*) musique.pointeurStereo);
                                             SMPEG_skip((SMPEG*) musique.pointeurStereo, secondsSkipped);
                                         }
                                     }
                                     else if (clicUpSurBoutonAB == 2 || (toucheActive == ';' && ctrl))
                                     {
                                          if (toucheActive == ';')
                                             toucheActive = 0;
                                          action = 1;
                                          modeAB = 0;
                                          SDL_FreeSurface(lettreA);
                                          lettreA = NULL;
                                     }
                                     clicUpSurBoutonAB = 0;
                                     break;
                                case 2:
                                     if (toucheActive == ';' || test_button(event, ecran, boutonAB, positionBoutonAB, SDL_MapRGB(ecran->format, 255, 0, 0), -1, "effacer les repères", 1500, &tempsDernierMvtSouris, NULL))
                                     {
                                         if (toucheActive == ';')
                                            toucheActive = 0;
                                         action = 1;
                                         modeAB = 0;
                                         SDL_FreeSurface(lettreA);
                                         lettreA = NULL;
                                         SDL_FreeSurface(lettreB);
                                         lettreB = NULL;
                                         SDL_FreeSurface(boutonAB);
                                         boutonAB = IMG_Load("img\\other\\bouton_lecture_AB_reduit.jpg");
                                     }
                                     break;
                  }

                  if (lettreA != NULL && clicGaucheSurFlecheLecture != 1 && action == 0)
                  {
                              if (cliquer_deplacer (event, lettreA, &positionLettreA, &positionLettreAAuClic, &clicGaucheSurLettreA))
                              {
                                  action = 1;
                                  positionLettreA.y = ordonneeFlecheLecture;
                                  if (positionLettreA.x > ecran->w - lettreA->w - 1)
                                     positionLettreA.x = ecran->w - lettreA->w - 1;
                                  if (test_button(event, ecran, lettreA, positionLettreA, -1, SDL_MapRGB(ecran->format, 0, 0, 255), NULL, 0, &tempsDernierMvtSouris, &clicGaucheSurLettreA));
                                     tempsPointA = (positionLettreA.x * musique.longueur) / (ecran->w - lettreA->w);
                              }
                  }

                  if (lettreB != NULL && clicGaucheSurFlecheLecture != 1 && action == 0)
                  {
                              if (cliquer_deplacer (event, lettreB, &positionLettreB, &positionLettreBAuClic, &clicGaucheSurLettreB))
                              {
                                  action = 1;
                                  positionLettreB.y = ordonneeFlecheLecture;
                                  if (positionLettreB.x > ecran->w - lettreB->w - 1)
                                     positionLettreB.x = ecran->w - lettreB->w - 1;
                                  if (test_button(event, ecran, lettreB, positionLettreB, -1, SDL_MapRGB(ecran->format, 0, 0, 255), NULL, 0, &tempsDernierMvtSouris, &clicGaucheSurLettreB));
                                     tempsPointB = (positionLettreB.x * musique.longueur) / (ecran->w - lettreB->w);
                              }
                  }
          }


          if (action == 0 && (toucheActive == 'c' || test_button(event, ecran, boutonAlpha, positionBoutonAlpha, SDL_MapRGB(ecran->format, 0, 0, 255), -1, "classer", 1500, &tempsDernierMvtSouris, NULL)))
          {
                      if (toucheActive == 'c')
                         toucheActive = 0;
                      action = 1;
                      if (modeListe == 0)
                         classer_alphabetique_fichier_texte(nomFichierPlaylist, TAILLE_MAX_NOM, nomFichierListe);
                      else
                          classer_alphabetique_fichier_texte(nomFichierPlaylistDePlaylists, TAILLE_MAX_NOM, nomFichierListePlaylists);
                      for (i = 0 ; i < TAILLE_MAX_PLAYLIST ; i++)
                      {
                          if (tableauTotalMusiques[i] == -1)
                             tableauTotalMusiques[i] = 0;
                      }
          }


          if (modeRecord && musique.type == RADIO && action == 0)
          {
                         if (toucheActive == 'r' || test_button(event, ecran, boutonRecord, positionBoutonRecord, SDL_MapRGB(ecran->format, 0, 0, 255), -1, "arrêter l'enregistrement", 1500, &tempsDernierMvtSouris, NULL))
                         {
                                                if (toucheActive == 'r')
                                                   toucheActive = 0;
                                                action = 1;
                                                FSOUND_Record_Stop();
                                                sauver_en_wav(enregistrement, nomFichierRecord, SDL_GetTicks() - tempsDernierRecord);
                                                numeroRecord++;
                                                sprintf(nomFichierRecord, "records\\Enregistrement %d.wav", numeroRecord);
                                                FSOUND_Sample_Free(enregistrement);
                                                modeRecord = 0;

                                                SDL_FreeSurface(boutonRecord);
                                                boutonRecord = IMG_Load("img\\other\\bouton_record_play_reduit.jpg");
                                                lister_extension("records", "playlists\\Records.list", "Enregistrement", "w");
                                                creer_playlist("playlists\\Records.list", "playlists\\Records.npl", typePlaylist);
                                                lister_extension("playlists", nomFichierListePlaylists, ".npl", "w");
                                                creer_playlist(nomFichierListePlaylists, nomFichierPlaylistDePlaylists, typePlaylist);
                         }
                         else if (SDL_GetTicks() - tempsDernierRecord >= configMenu.parametre.dureeMaxEnregistrement * 1000)
                         {
                                                 action = 1;
                                                 FSOUND_SetPaused(FSOUND_ALL, 1);
                                                 FSOUND_Record_Stop();
                                                 sauver_en_wav(enregistrement, nomFichierRecord, SDL_GetTicks() - tempsDernierRecord);
                                                 numeroRecord++;
                                                 sprintf(nomFichierRecord, "records\\Enregistrement %d.wav", numeroRecord);
                                                 FSOUND_Sample_Free(enregistrement);
                                                 enregistrement = FSOUND_Sample_Alloc(FSOUND_FREE, configMenu.parametre.dureeMaxEnregistrement * musique.frequence * 1000, 0, 44100, 128, 128, 255);
                                                 if (enregistrement == NULL)
                                                    fprintf(stderr, "ERR main %d : Le chargement de enregistrement a echoue.\n", __LINE__);
                                                 else
                                                 {
                                                     tempsDernierRecord = SDL_GetTicks();
                                                     FSOUND_Record_StartSample(enregistrement, 0);
                                                     FSOUND_SetPaused(FSOUND_ALL, 0);
                                                 }
                         }

          }
          else if (musique.type == RADIO && action == 0)
          {
                       if (toucheActive == 'r' || test_button(event, ecran, boutonRecord, positionBoutonRecord, SDL_MapRGB(ecran->format, 0, 0, 255), -1, "démarrer l'enregistrement", 1500, &tempsDernierMvtSouris, NULL))
                       {
                                 if (toucheActive == 'r')
                                    toucheActive = 0;
                                 action = 1;
                                 enregistrement = FSOUND_Sample_Alloc(FSOUND_FREE, configMenu.parametre.dureeMaxEnregistrement * musique.frequence * 1000, 0, 44100, 128, 128, 255);
                                 if (enregistrement == NULL)
                                    fprintf(stderr, "ERR main %d : Le chargement de enregistrement a echoue.\n", __LINE__);
                                 else
                                 {
                                     FSOUND_Record_StartSample(enregistrement, 0);
                                     modeRecord = 1;
                                     SDL_FreeSurface(boutonRecord);
                                     boutonRecord = IMG_Load("img\\other\\bouton_record_pause_reduit.jpg");
                                     tempsDernierRecord = SDL_GetTicks();
                                 }
                       }
          }

          if (action == 0 && numeroVisu >= 0 && (musique.type != VIDEO || numeroVisu >= NOMBRE_VISUS) && (toucheActive == SDLK_TAB || test_button(event, ecran, boutonChangementVisu, positionBoutonChangementVisu, SDL_MapRGB(ecran->format, 255, 0, 0), -1, "changer la visualisation", 1500, &tempsDernierMvtSouris, NULL)))
          {
                                 if (toucheActive == SDLK_TAB)
                                    toucheActive = 0;
                                 action = 1;
                                 numeroVisu++;
                                 if (numeroVisu >= NOMBRE_VISUS)
                                    numeroVisu = 0;
                                 SDL_FillRect(surfaceVisu, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
                                 positionBarreVisu2 = 0;
          }

          else if (action == 0 && (toucheActive == 'n' || test_button(event, ecran, boutonResearch, positionBoutonResearch, SDL_MapRGB(ecran->format, 255, 0, 0), -1, "rechercher sur l'ordinateur", 1500, &tempsDernierMvtSouris, NULL)))
          {
                                 if (toucheActive == 'n')
                                    toucheActive = 0;
                                 action = 1;
                                 numeroVisu = NOMBRE_VISUS;
                                 if (musique.mode == ERREUR)
                                    musique.mode = STOP;
                                 SDL_FillRect(surfaceVisu, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
          }

          else if (action == 0 && (toucheActive == 'a' || test_button(event, ecran, boutonSettings, positionBoutonSettings, SDL_MapRGB(ecran->format, 255, 0, 0), -1, "modifier les réglages", 1500, &tempsDernierMvtSouris, NULL)))
          {
                                 if (toucheActive == 'a')
                                    toucheActive = 0;
                                 action = 1;
                                 numeroVisu = NOMBRE_VISUS + 1;
                                 if (musique.mode == ERREUR)
                                    musique.mode = STOP;
                                 SDL_FillRect(surfaceVisu, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
          }

          else if (action == 0 && (toucheActive == 't' || test_button(event, ecran, boutonTypePlaylist, positionBoutonTypePlaylist, SDL_MapRGB(ecran->format, 255, 0, 0), -1, "type d'affichage", 1500, &tempsDernierMvtSouris, NULL)))
          {
                          if (toucheActive == 't')
                             toucheActive = 0;
                          action = 1;
                          typePlaylist++;
                          if (typePlaylist > TITRE_ET_AUTEUR)
                             typePlaylist = NOM;
                          creer_playlist(nomFichierListe, nomFichierPlaylist, typePlaylist);
                          SDL_FreeSurface(boutonTypePlaylist);
                          switch(typePlaylist)
                          {
                                              case NOM:
                                                   boutonTypePlaylist = TTF_RenderText_Blended(policeBoutonTypePlaylist, "NoM", couleurVertBizarre);
                                                   break;
                                              case TITRE:
                                                   boutonTypePlaylist = TTF_RenderText_Blended(policeBoutonTypePlaylist, "TtL", couleurVertBizarre);
                                                   break;
                                              case AUTEUR:
                                                   boutonTypePlaylist = TTF_RenderText_Blended(policeBoutonTypePlaylist, "AsT", couleurVertBizarre);
                                                   break;
                                              case NOM_SIMPLIFIE:
                                                   boutonTypePlaylist = TTF_RenderText_Blended(policeBoutonTypePlaylist, "NsP", couleurVertBizarre);
                                                   break;
                                              case TITRE_ET_AUTEUR:
                                                   boutonTypePlaylist = TTF_RenderText_Blended(policeBoutonTypePlaylist, "T+A", couleurVertBizarre);
                                                   break;
                          }
                          positionBoutonTypePlaylist.x = positionBoutonModeListe.x + boutonModeListe->w + ((ecran->w - positionBoutonModeListe.x - boutonModeListe->w - boutonTypePlaylist->w) / 2);

          }

          if (action == 0 && numeroVisu < NOMBRE_VISUS && numeroVisu >= 0 && musique.type == VIDEO && (toucheActive == 'g' || test_button(event, ecran, boutonModeGrandEcran, positionBoutonModeGrandEcran, SDL_MapRGB(ecran->format, 255, 0, 0), -1, "mode grand écran", 1500, &tempsDernierMvtSouris, NULL)))
          {
             if (toucheActive == 'g')
                toucheActive = 0;
             bigScreenMode = 1;
          }
          if (musique.type != VIDEO)
          {
             agrandissement = 1.0;
             bigScreenMode = 0;
          }

          if (bigScreenMode)
          {
             SMPEG_Info SInfo;
             SMPEG_getinfo((SMPEG*) musique.pointeurStereo, &SInfo);

             char buffer[TAILLE_MAX_NOM];
             sprintf(buffer, "x%.2lf", agrandissement);
             test_button(event, ecran, boutonTailleVideo, positionBoutonTailleVideo, SDL_MapRGB(ecran->format, 255, 255, 0), -1, buffer, 0, &tempsDernierMvtSouris, NULL);

             if (cliquer_deplacer(event, boutonTailleVideo, &positionBoutonTailleVideo, &positionBoutonTailleVideoAuClic, &clicGaucheSurBoutonTailleVideo))
             {
                 positionBoutonTailleVideo.y = ordonneeBoutonTailleVideo;
                 if (positionBoutonTailleVideo.x + (boutonTailleVideo->w / 2) >= positionLigneTailleVideo.x + ligneTailleVideo->w)
                    positionBoutonTailleVideo.x = positionLigneTailleVideo.x + ligneTailleVideo->w - (boutonTailleVideo->w / 2);
                 if (positionBoutonTailleVideo.x + (boutonTailleVideo->w / 2) <= positionLigneTailleVideo.x)
                    positionBoutonTailleVideo.x = positionLigneTailleVideo.x - (boutonTailleVideo->w / 2);

                 agrandissement = (positionBoutonTailleVideo.x - positionLigneTailleVideo.x + (boutonTailleVideo->w / 2)) * 3.0 / ligneTailleVideo->w;
             }
             else
             {
                 if (toucheActive == SDLK_PLUS || test_button(event, ecran, boutonTailleVideoPlus, positionBoutonTailleVideoPlus, SDL_MapRGB(ecran->format, 255, 0, 0), -1, "agrandir la vidéo", 1500, &tempsDernierMvtSouris, NULL))
                 {
                    if (toucheActive == SDLK_PLUS)
                       toucheActive = 0;
                    agrandissement += 0.1;
                 }
                 else if (toucheActive == SDLK_MINUS || test_button(event, ecran, boutonTailleVideoMoins, positionBoutonTailleVideoMoins, SDL_MapRGB(ecran->format, 255, 0, 0), -1, "réduire la vidéo", 1500, &tempsDernierMvtSouris, NULL))
                 {
                    if (toucheActive == SDLK_MINUS)
                       toucheActive = 0;
                    agrandissement -= 0.1;
                 }

                 if (agrandissement > 3)
                    agrandissement = 3.0;

                 positionBoutonTailleVideo.x = agrandissement * ligneTailleVideo->w / 3 + positionLigneTailleVideo.x - (boutonTailleVideo->w / 2);

                 if (oldAgrandissement != agrandissement)
                    resize_video((SMPEG*) musique.pointeurStereo);
                 oldAgrandissement = agrandissement;
             }

             if (toucheActive == 'g' || test_button(event, ecran, boutonModePetitEcran, positionBoutonModePetitEcran, SDL_MapRGB(ecran->format, 255, 0, 0), -1, "revenir à l'affichage classique", 1500, &tempsDernierMvtSouris, NULL))
             {
                if (toucheActive == 'g')
                   toucheActive = 0;
                agrandissement = 1.0;
                bigScreenMode = 0;
                resize_video((SMPEG*) musique.pointeurStereo);
             }
          }


          sprintf(title, "%s - %s", musique.titre, musique.auteur);

          if (IsIconic(mainWnd) || !IsWindowVisible(mainWnd))
          {
                if (musique.type == NORMAL)
                       SetWindowText(mainWnd, title);
                else SetWindowText(mainWnd, musique.titre);
          }
          else SetWindowText(mainWnd, "CokieForever");

          if (configMenu.parametre.modeReduit && (IsIconic(mainWnd) || !IsWindowVisible(mainWnd)))  //En cas de réduction de la fenêtre
          {
                int tempsMaximum = 0;

                while (continuer && (event.active.gain != 1 || (event.active.state & SDL_APPACTIVE) != SDL_APPACTIVE))  //boucle ultra-simplifiée en attente de la fin de la musique
                {
                    SDL_PollEvent(&event);

                    sprintf(title, "%s - %s", musique.titre, musique.auteur);

                    if (musique.type == NORMAL)
                       SetWindowText(mainWnd, title);
                    else SetWindowText(mainWnd, musique.titre);

                    SMPEG_Info SInfo;
                    if (musique.type == VIDEO && musique.mode != ERREUR)
                       SMPEG_getinfo((SMPEG*) musique.pointeurStereo, &SInfo);

                    if (musique.mode == ERREUR || musique.mode == STOP)
                       musique.curseur = 0;
                    else
                    {
                        if (musique.type != VIDEO)
                        {
                            if (indicStereoMono)
                               musique.curseur = FSOUND_Stream_GetTime(musique.pointeurStereo);
                            else musique.curseur = FSOUND_Stream_GetTime(musique.pointeurMono);
                        }
                        else if (musique.mode == PLAY)
                             musique.curseur = (SInfo.current_time + secondsSkipped) * 1000;
                    }

                    if (musique.mode != ERREUR && musique.type != VIDEO)
                       musique.longueur = FSOUND_Stream_GetLengthMs(musique.pointeurStereo);
                    else if (musique.mode != ERREUR)
                         musique.longueur = SInfo.total_time * 1000;

                    if (modeBoucle && musique.type == VIDEO && musique.curseur >= musique.longueur - 500)
                    {
                         SMPEG_rewind((SMPEG*) musique.pointeurStereo);
                         SMPEG_play((SMPEG*) musique.pointeurStereo);
                         secondsSkipped = 0;
                    }

                    if (modeBoucle == 0 && musique.type != RADIO && musique.mode != ERREUR && musique.mode != STOP && musique.curseur >= musique.longueur - 500)
                    {
                                    if (musique.type != VIDEO)
                                    {
                                        FSOUND_Stream_Stop(musique.pointeurStereo);
                                        FSOUND_Stream_Stop(musique.pointeurMono);
                                    }
                                    else SMPEG_rewind((SMPEG*) musique.pointeurStereo);

                                    i = 0;
                                    do
                                    {
                                        if (musique.type != VIDEO)
                                        {
                                            FSOUND_Stream_Close(musique.pointeurStereo);
                                            FSOUND_Stream_Close(musique.pointeurMono);
                                        }
                                        else SMPEG_delete((SMPEG*) musique.pointeurStereo);

                                        do
                                        {
                                            if (musique.mode == ERREUR)
                                                 tableauTotalMusiques[-musique.numero] = -1;
                                            if (modeAleatoire)
                                               charger_musique(&musique, configMenu, &affichageParoles, nomFichierListe, -1, positionBoutonReculRapide.x - musique.position.titre.x - 40, &i);
                                            else
                                            {
                                                 musique.numero++;
                                                 if (musique.numero > nombreMusiques)
                                                    musique.numero = 1;
                                                 charger_musique(&musique, configMenu, &affichageParoles, nomFichierListe, musique.numero,positionBoutonReculRapide.x - musique.position.titre.x - 40, &i);
                                            }
                                            i++;
                                        } while (musique.mode == ERREUR && i < configMenu.parametre.nombreChargementsMaxi);
                                        if (i >= configMenu.parametre.nombreChargementsMaxi)
                                        {
                                              for (j = 0 ; j < TAILLE_MAX_PLAYLIST ; j++)
                                              {
                                                  tableauTotalMusiques[j] = 0;
                                              }
                                              if (musique.mode != ERREUR)
                                                 musique.mode = STOP;
                                        }
                                    } while (tableauTotalMusiques[musique.numero] && modeAleatoire && i < configMenu.parametre.nombreChargementsMaxi);


                                    if (nombreMusiques > nombreMusiquesAffichables  && (musique.numero < ligneDebut || musique.numero > ligneFin))
                                    {
                                                  ligneDebut = musique.numero - 1;
                                                  ligneFin = ligneDebut + nombreMusiquesAffichables;
                                                  positionBarreDefilement.y = (ligneDebut * (hauteurZoneLecteur - hauteurBarreDefilement)) / ((nombreMusiques - nombreMusiquesAffichables) / 1.0);
                                    }

                                    if (musique.mode == ERREUR || musique.mode == STOP)
                                       mettre_en_stop(&musique, surfaceBoutonPlayPause);
                                    else
                                    {
                                        if (musique.type != VIDEO)
                                        {
                                            if (indicStereoMono)
                                               FSOUND_Stream_Play(1, musique.pointeurStereo);
                                            else FSOUND_Stream_Play(1, musique.pointeurMono);
                                        }
                                        else SMPEG_play((SMPEG*) musique.pointeurStereo);

                                        positionDansTableauDernieresMusiques++;
                                        if (positionDansTableauDernieresMusiques > 10)
                                           positionDansTableauDernieresMusiques = 0;
                                        tableauDernieresMusiques[positionDansTableauDernieresMusiques] = musique.numero;
                                        tableauTotalMusiques[musique.numero] = 1;

                                        musique.mode = PLAY;
                                    }
                                    volume = (volumeDroit + volumeGauche) / 2;
                                    if (musique.type != VIDEO)
                                    {
                                        FSOUND_SetVolume(1, volume);
                                        FSOUND_SetPan(1, ceil((255 - volumeGauche + volumeDroit) / 2));
                                    }
                                    else SMPEG_setvolume((SMPEG*) musique.pointeurStereo, volume * 100 / 255);
                    }

                    if (modeRecord && musique.type == RADIO)
                    {
                                     if (SDL_GetTicks() - tempsDernierRecord >= 295000)
                                     {
                                                         FSOUND_SetPaused(FSOUND_ALL, 1);
                                                         FSOUND_Record_Stop();
                                                         sauver_en_wav(enregistrement, nomFichierRecord, SDL_GetTicks() - tempsDernierRecord);
                                                         numeroRecord++;
                                                         sprintf(nomFichierRecord, "records\\Enregistrement %d.wav", numeroRecord);
                                                         FSOUND_Sample_Free(enregistrement);
                                                         enregistrement = FSOUND_Sample_Alloc(FSOUND_FREE, 13230000, 0, 44100, 128, 128, 255);
                                                         if (enregistrement == NULL)
                                                            fprintf(stderr, "ERR main %d : Le chargement de enregistrement a echoue.\n", __LINE__);
                                                         else
                                                         {
                                                             tempsDernierRecord = SDL_GetTicks();
                                                             FSOUND_Record_StartSample(enregistrement, 0);
                                                             FSOUND_SetPaused(FSOUND_ALL, 0);
                                                         }
                                     }

                    }
                    tempsMaximum = SDL_GetTicks() + musique.longueur - musique.curseur - 5000;

                    sprintf(title, "%s - %s", musique.titre, musique.auteur);
                    if (musique.type == NORMAL)
                       SetWindowText(mainWnd, title);
                    else SetWindowText(mainWnd, musique.titre);

                    while (continuer && SDL_GetTicks() < tempsMaximum && (event.active.gain != 1 || (event.active.state & SDL_APPACTIVE) != SDL_APPACTIVE))  //boucle ultra-simplifiée en attente de la fin de la musique
                    {
                        SDL_PollEvent(&event);

                        //tests concernant les bornes A et B
                        if (((musique.curseur >= tempsPointB && clicGaucheSurLettreB != 1) || (musique.curseur < tempsPointA && clicGaucheSurLettreA != 1)) && modeAB == 2 && musique.type != RADIO && musique.mode != ERREUR && musique.mode != STOP)
                        {
                           if (musique.type != VIDEO)
                           {
                               FSOUND_Stream_SetTime(musique.pointeurStereo, tempsPointA);
                               FSOUND_Stream_SetTime(musique.pointeurMono, tempsPointA);
                           }
                           else
                           {
                               secondsSkipped = tempsPointA / 1000.0;
                               SMPEG_rewind((SMPEG*) musique.pointeurStereo);
                               SMPEG_play((SMPEG*) musique.pointeurStereo);
                               SMPEG_skip((SMPEG*) musique.pointeurStereo, secondsSkipped);
                           }
                        }

                        // tests de communications d'éventuelles applications parallèles
                        for (i = 0 ; i < 500 ; i++)
                        {
                             adressChar = (sharedMemory + sizeof(HWND) + (i * TAILLE_MAX_NOM));
                             if (*adressChar != '\0')
                                break;
                        }

                        if (*adressChar != '\0')
                        {
                                               strcpy(nomFichierListe, adressChar);

                                               strcpy(nomFichierPlaylist, nomFichierListe);
                                               *strstr(nomFichierPlaylist, ".list") = '\0';
                                               strcat(nomFichierPlaylist, ".npl");

                                               creer_playlist(nomFichierListe, nomFichierPlaylist, TITRE);
                                               memset(adressChar, '\0', TAILLE_MAX_NOM);

                                               mettre_a_jour_playlists(nomFichierListePlaylists, nomFichierPlaylistDePlaylists, tableauTotalMusiques);

                                               if (musique.type != VIDEO)
                                               {
                                                   FSOUND_Stream_Stop(musique.pointeurStereo);
                                                   FSOUND_Stream_Stop(musique.pointeurMono);
                                                   FSOUND_Stream_Close(musique.pointeurStereo);
                                                   FSOUND_Stream_Close(musique.pointeurMono);
                                               }
                                               else
                                               {
                                                   SMPEG_stop((SMPEG*) musique.pointeurStereo);
                                                   SMPEG_delete((SMPEG*) musique.pointeurStereo);
                                               }

                                               modeListe = 0;
                                               SDL_FreeSurface(boutonModeListe);
                                               boutonModeListe = IMG_Load("img\\other\\bouton_playlists.jpg");

                                               if ((fichierPlaylist = fopen(nomFichierPlaylist, "r")) == NULL)
                                                  fprintf(stderr, "ERR main %d : Le fichier %s n'a pas pu etre ouvert.\n", __LINE__, nomFichierPlaylist);
                                               else
                                               {
                                                   nombreMusiques = 0;
                                                   while(fgets(temp, TAILLE_MAX_NOM, fichierPlaylist) != NULL)
                                                                         nombreMusiques++;
                                                   while(fclose(fichierPlaylist) == EOF);

                                                   hauteurBarreDefilement = (nombreMusiquesAffichables / (nombreMusiques / 1.0)) * hauteurZoneLecteur;
                                                   SDL_FreeSurface(barreDefilement);
                                                   barreDefilement = SDL_CreateRGBSurface(SDL_HWSURFACE, 26, hauteurBarreDefilement, 32, 0, 0, 0, 0);
                                                   SDL_FreeSurface(rectangle);
                                                   rectangle = SDL_CreateRGBSurface(SDL_HWSURFACE, 1, hauteurBarreDefilement, 32, 0, 0, 0, 0);
                                                   for (i = 0 ; i <= 12 ; i++)
                                                   {
                                                           SDL_FillRect(rectangle, NULL, SDL_MapRGB(ecran->format, (255 * i) / 12, (255 * i) / 12, (255 * i) / 12));
                                                           position.x = i;
                                                           SDL_BlitSurface(rectangle, NULL, barreDefilement, &position);
                                                   }
                                                   for (i = 0 ; i <= 12 ; i++)
                                                   {
                                                           SDL_FillRect(rectangle, NULL, SDL_MapRGB(ecran->format, 255 - (255 * i) / 12, 255 - (255 * i) / 12, 255 - (255 * i) / 12));
                                                           position.x = i + 13;
                                                           SDL_BlitSurface(rectangle, NULL, barreDefilement, &position);
                                                   }
                                                   SDL_FreeSurface(rectangle);
                                                   positionBarreDefilement.x = positionLigneSeparation3.x;
                                                   positionBarreDefilement.y = 0;

                                                   for (j = 0 ; j < TAILLE_MAX_PLAYLIST ; j++)
                                                   {
                                                           tableauTotalMusiques[j] = 0;
                                                           if (j <= 10)
                                                              tableauDernieresMusiques[j] = -1;
                                                   }

                                                   j = 0;

                                                   do
                                                   {
                                                           charger_musique(&musique, configMenu, &affichageParoles, nomFichierListe, compter_lignes(nomFichierListe), positionBoutonReculRapide.x - musique.position.titre.x - 40, &j);
                                                           if (musique.mode == ERREUR)
                                                              tableauTotalMusiques[musique.numero] = -1;
                                                           j++;
                                                   } while (musique.mode == ERREUR && j < configMenu.parametre.nombreChargementsMaxi);

                                                   ligneDebut = 0;
                                                   ligneFin = nombreMusiquesAffichables;

                                                   if (nombreMusiques > nombreMusiquesAffichables  && (musique.numero < ligneDebut || musique.numero > ligneFin))
                                                   {
                                                                          ligneDebut = musique.numero - 1;
                                                                          ligneFin = ligneDebut + nombreMusiquesAffichables;
                                                                          positionBarreDefilement.y = (ligneDebut * (hauteurZoneLecteur - hauteurBarreDefilement)) / ((nombreMusiques - nombreMusiquesAffichables) / 1.0);
                                                   }
                                                   if (musique.mode == ERREUR || musique.mode == STOP)
                                                          mettre_en_stop(&musique, surfaceBoutonPlayPause);
                                                   else
                                                   {
                                                           if (musique.type != VIDEO)
                                                           {
                                                               if (indicStereoMono)
                                                                  FSOUND_Stream_Play(1, musique.pointeurStereo);
                                                               else FSOUND_Stream_Play(1, musique.pointeurMono);
                                                           }
                                                           else SMPEG_play((SMPEG*) musique.pointeurStereo);

                                                           positionDansTableauDernieresMusiques = 0;
                                                           tableauDernieresMusiques[0] = musique.numero;
                                                           tableauTotalMusiques[musique.numero] = 1;

                                                           musique.mode = PLAY;
                                                   }
                                                   volume = (volumeDroit + volumeGauche) / 2;
                                                   if (musique.type != VIDEO)
                                                   {
                                                       FSOUND_SetVolume(1, volume);
                                                       FSOUND_SetPan(1, ceil((255 - volumeGauche + volumeDroit) / 2));
                                                   }
                                                   else SMPEG_setvolume((SMPEG*) musique.pointeurStereo, volume * 100 / 255);

                                                   sprintf(title, "%s - %s", musique.titre, musique.auteur);
                                                   if (musique.type == NORMAL)
                                                       SetWindowText(mainWnd, title);
                                                   else SetWindowText(mainWnd, musique.titre);

                                                   tempsMaximum = SDL_GetTicks() + musique.longueur - musique.curseur - 5000;
                                               }
                        }
                        // fin du test

                        if (test_quit(event, &alt) != 1)
                           continuer = 0;

                        if (passerAuSuivant || passerAuPrecedent || mettreEnPlayOuPause || mettreEnStop)
                           break;

                        SDL_Delay(1000);

                    } //fin de la boucle ultra-simplifiée


                    if (passerAuSuivant || passerAuPrecedent || mettreEnPlayOuPause || mettreEnStop)
                           break;

                    SDL_Delay(100);
                } //fin de la boucle simplifiée

                SDL_FillRect(surfaceVisu, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
                positionBarreVisu2 = 0;
                SetWindowText(mainWnd, "CokieForever");
          }


          switch (event.type)
          {
                 case SDL_VIDEORESIZE:
                      SDL_FreeSurface(ecran);
                      if (event.resize.h > 0)
                      {
                          if (event.resize.h < 525)
                             event.resize.h = 525;
                          ecran = SDL_SetVideoMode(event.resize.w, event.resize.h, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_RESIZABLE);
                      }

                      hauteurZoneLecteur = ecran->h - (hauteurZonePlayPause + 25);
                      hauteurZoneGraphique = hauteurZoneLecteur - hauteurZoneBoutons;

                      SDL_FreeSurface(surfaceZonePlayPause);
                      surfaceZonePlayPause = SDL_CreateRGBSurface(SDL_HWSURFACE, ecran->w, hauteurZonePlayPause, 32, 0, 0, 0, 0);
                      SDL_FillRect(surfaceZonePlayPause, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
                      positionZonePlayPause.y = hauteurZoneLecteur + 25;
                      positionLigneSeparation2.y = hauteurZoneLecteur;
                      positionBoutonPlayPause.x = (ecran->w - surfaceBoutonPlayPause->w) / 2;
                      positionBoutonPlayPause.y = hauteurZoneLecteur + 25 + ((hauteurZonePlayPause - surfaceBoutonPlayPause->h) / 2);
                      positionFlecheLecture.y = hauteurZoneLecteur + 1;
                      ordonneeFlecheLecture = positionFlecheLecture.y;
                      positionBoutonAvanceRapide.x = positionBoutonPlayPause.x + boutonAvanceRapide->w + 30;
                      positionBoutonAvanceRapide.y = hauteurZoneLecteur + 25 + ((hauteurZonePlayPause - boutonAvanceRapide->h) / 2);
                      positionBoutonReculRapide.x = positionBoutonPlayPause.x - boutonReculRapide->w - 30;
                      positionBoutonReculRapide.y = hauteurZoneLecteur + 25 + ((hauteurZonePlayPause - boutonReculRapide->h) / 2);
                      positionBoutonAleatoire.x = positionBoutonAvanceRapide.x + boutonAvanceRapide->w + 50;
                      positionBoutonAleatoire.y = hauteurZoneLecteur + 25 + ((hauteurZonePlayPause - boutonAleatoire->h) / 2);
                      positionBoutonBoucle.x = positionBoutonAleatoire.x + boutonAleatoire->w + 30;
                      positionBoutonBoucle.y = hauteurZoneLecteur + 25 + ((hauteurZonePlayPause - boutonBoucle->h) / 2);
                      positionBoutonActualiser.x = positionBoutonBoucle.x;
                      positionBoutonActualiser.y = hauteurZoneLecteur + 25 + ((hauteurZonePlayPause - boutonActualiser->h) / 2);
                      positionBoutonAB.x = positionBoutonBoucle.x + boutonBoucle->w + 30;
                      positionBoutonAB.y = hauteurZoneLecteur + 25 + ((hauteurZonePlayPause - boutonAB->h) / 2);
                      positionBoutonRecord.x = positionBoutonAB.x;
                      positionBoutonRecord.y = hauteurZoneLecteur + 25 + ((hauteurZonePlayPause - boutonRecord->h) / 2);

                      musique.position.titre.y = hauteurZoneLecteur + 25 + ((hauteurZonePlayPause - (musique.surface.titre->h * 3 + 10)) / 2);
                      musique.position.auteur.y = musique.position.titre.y + musique.surface.titre->h + 5;
                      musique.position.album.y = musique.position.auteur.y + musique.surface.auteur->h + 5;
                      musique.position.image.y = musique.position.titre.y;
                      char temp[TAILLE_MAX_NOM] = " ";
                      strcpy(temp, musique.titre);
                      couper_texte(temp, positionBoutonReculRapide.x - musique.position.titre.x - 40, policeNomMusique);
                      SDL_FreeSurface(musique.surface.titre);
                      musique.surface.titre = TTF_RenderText_Blended(policeNomMusique, temp, couleurBlanc);
                      strcpy(temp, musique.auteur);
                      couper_texte(temp, positionBoutonReculRapide.x - musique.position.auteur.x - 40, policeNomMusique);
                      SDL_FreeSurface(musique.surface.auteur);
                      musique.surface.auteur = TTF_RenderText_Blended(policeNomMusique, temp, couleurBlanc);
                      strcpy(temp, musique.album);
                      couper_texte(temp, positionBoutonReculRapide.x - musique.position.album.x - 40, policeNomMusique);
                      SDL_FreeSurface(musique.surface.album);
                      musique.surface.album = TTF_RenderText_Blended(policeNomMusique, temp, couleurBlanc);

                      positionLigneSeparation3.x = ecran->w - largeurZoneListe;
                      positionBarreDefilement.x = positionLigneSeparation3.x;
                      largeurZoneListe = ecran->w - positionLigneSeparation3.x;
                      largeurZoneGraphique = ecran->w - largeurZoneReglage - largeurZoneListe;
                      positionBoutonChangementVisu.x = positionLigneSeparation3.x - boutonChangementVisu->w - 15;
                      positionBoutonModeGrandEcran.x = positionBoutonChangementVisu.x;
                      positionBoutonSettings.x = positionBoutonChangementVisu.x - boutonSettings->w - 15;
                      positionBoutonResearch.x = positionBoutonSettings.x - boutonResearch->w - 15;
                      positionTempsRafraichissement.x = positionLigneSeparation.x + ligneSeparation->w + 10;

                      SDL_FreeSurface(surfaceVisu);
                      surfaceVisu = SDL_CreateRGBSurface(SDL_HWSURFACE, largeurZoneGraphique, hauteurZoneGraphique, 32, 0, 0, 0, 0);
                      SDL_FillRect(surfaceVisu, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
                      positionBarreVisu2 = 0;

                      SDL_FreeSurface(surfaceCache1);
                      surfaceCache1 = SDL_CreateRGBSurface(SDL_HWSURFACE, largeurZoneGraphique, 100, 32, 0, 0, 0, 0);
                      SDL_FillRect(surfaceCache1, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));

                      SDL_FreeSurface(ligneSeparation4);
                      ligneSeparation4 = SDL_CreateRGBSurface(SDL_HWSURFACE, largeurZoneGraphique, 5, 32, 0, 0, 0, 0);
                      SDL_FillRect(ligneSeparation4, NULL, SDL_MapRGB(ligneSeparation4->format, 0, 0, 0));
                      SDL_BlitSurface(IMG_Load("img\\other\\ligne_separation_4_reduite.jpg"), NULL, ligneSeparation4, &positionZero);

                      positionSurfaceCache2.y = hauteurZoneGraphique - surfaceCache1->h;
                      positionTriangle1.y = hauteurZoneGraphique - triangle1->h;
                      positionPlaylist.x = ecran->w - largeurZoneListe + 30;
                      positionBoutonModeListe.x = ecran->w - largeurZoneListe + 25 + ((largeurZoneListe - 25 - boutonModeListe->w) / 2);
                      positionBoutonModeListe.y = hauteurZoneLecteur - boutonModeListe->h - 10;
                      positionBoutonAlpha.x = ecran->w - largeurZoneListe + 25 + (((positionBoutonModeListe.x - (ecran->w - largeurZoneListe + 25)) - boutonAlpha->w) / 2);
                      positionBoutonAlpha.y = positionBoutonModeListe.y + ((boutonModeListe->h - boutonAlpha->h) / 2);
                      positionBoutonTypePlaylist.x = positionBoutonModeListe.x + boutonModeListe->w + ((ecran->w - positionBoutonModeListe.x - boutonModeListe->w - boutonTypePlaylist->w) / 2);
                      positionBoutonTypePlaylist.y = positionBoutonModeListe.y + ((boutonModeListe->h - boutonTypePlaylist->h) / 2);
                      ligneFin = ((hauteurZoneLecteur - boutonModeListe->h - 20) / (tableauSurfacesPlaylist[0]->h + 5)) + ligneDebut;
                      nombreMusiquesAffichables = ligneFin - ligneDebut;
                      hauteurBarreDefilement = (nombreMusiquesAffichables / (nombreMusiques / 1.0)) * hauteurZoneLecteur;
                      SDL_FreeSurface(barreDefilement);
                      barreDefilement = SDL_CreateRGBSurface(SDL_HWSURFACE, 26, hauteurBarreDefilement, 32, 0, 0, 0, 0);
                      SDL_FreeSurface(rectangle);
                      rectangle = SDL_CreateRGBSurface(SDL_HWSURFACE, 1, hauteurBarreDefilement, 32, 0, 0, 0, 0);
                      for (i = 0 ; i <= 12 ; i++)
                      {
                      SDL_FillRect(rectangle, NULL, SDL_MapRGB(ecran->format, (255 * i) / 12, (255 * i) / 12, (255 * i) / 12));
                      position.x = i;
                      SDL_BlitSurface(rectangle, NULL, barreDefilement, &position);
                      }
                      for (i = 0 ; i <= 12 ; i++)
                      {
                      SDL_FillRect(rectangle, NULL, SDL_MapRGB(ecran->format, 255 - (255 * i) / 12, 255 - (255 * i) / 12, 255 - (255 * i) / 12));
                      position.x = i + 13;
                      SDL_BlitSurface(rectangle, NULL, barreDefilement, &position);
                      }

                      SDL_FreeSurface(editMenu.surfaceBlit);
                      SDL_FreeSurface(rectangle);
                      editMenu.surfaceBlit = SDL_CreateRGBSurface(SDL_HWSURFACE, largeurZoneGraphique, hauteurZoneGraphique, 32, 0, 0, 0, 0);
                      SDL_FillRect(editMenu.surfaceBlit, NULL, SDL_MapRGB(editMenu.surfaceBlit->format, 0, 0, 0));

                      editMenu.hauteur = editMenu.surfaceBlit->h;
                      editMenu.zoneDossier.largeur = (editMenu.surfaceBlit->w - (editMenu.ligneSeparation->w * 3)) / 3;
                      editMenu.zoneMusique.largeur = editMenu.zoneDossier.largeur;
                      editMenu.zonePlaylist.largeur = editMenu.zoneDossier.largeur;
                      editMenu.zoneMusique.abscisse = editMenu.zoneDossier.abscisse + editMenu.zoneDossier.largeur + editMenu.ligneSeparation->w;
                      editMenu.zonePlaylist.abscisse = editMenu.zoneMusique.abscisse + editMenu.zoneMusique.largeur + editMenu.ligneSeparation->w;
                      editMenu.positionBarreRouge.x = editMenu.zonePlaylist.abscisse;
                      editMenu.zoneMusique.positionListe.x = editMenu.zoneMusique.abscisse + 5;
                      editMenu.zonePlaylist.positionListe.x = editMenu.zonePlaylist.abscisse + 5;
                      editMenu.zoneMusique.positionLigneSeparation.x = editMenu.zoneDossier.abscisse + editMenu.zoneDossier.largeur;
                      editMenu.zonePlaylist.positionLigneSeparation.x = editMenu.zoneMusique.abscisse + editMenu.zoneMusique.largeur;

                      editMenu.zoneDossier.nombreAffichables = ((editMenu.hauteur - 5) / (editMenu.zoneDossier.echantillonTexte->h + 5)) - 2;
                      editMenu.zoneDossier.ligneFin = editMenu.zoneDossier.ligneDebut + editMenu.zoneDossier.nombreAffichables;

                      editMenu.zoneMusique.nombreAffichables = (editMenu.hauteur - 5) / (editMenu.zoneMusique.echantillonTexte->h + 5) - 2;
                      editMenu.zoneMusique.ligneFin = editMenu.zoneMusique.ligneDebut + editMenu.zoneMusique.nombreAffichables;

                      editMenu.zonePlaylist.nombreAffichables = (editMenu.hauteur - 5) / (editMenu.zonePlaylist.echantillonTexte->h + 5);
                      if (strstr(editMenu.zonePlaylist.nomFichierPlaylist, ".npl") != NULL)
                         editMenu.zonePlaylist.nombreAffichables -= 2;

                      editMenu.zonePlaylist.ligneFin = editMenu.zonePlaylist.ligneDebut + editMenu.zonePlaylist.nombreAffichables;

                      editMenu.zoneDossier.positionBoutonDossier.x = editMenu.zoneMusique.positionLigneSeparation.x - editMenu.zoneDossier.boutonDossier->w - 10;
                      editMenu.zoneDossier.positionBoutonDossier.y = editMenu.hauteur - 10 - editMenu.zoneDossier.boutonDossier->h;
                      editMenu.zonePlaylist.positionBoutonDossier.x = editMenu.surfaceBlit->w - editMenu.zonePlaylist.boutonDossier->w - 10;
                      editMenu.zonePlaylist.positionBoutonDossier.y = editMenu.zoneDossier.positionBoutonDossier.y;
                      editMenu.positionPosteTravail.x = editMenu.zoneDossier.positionBoutonDossier.x - editMenu.posteTravail->w - 15;
                      editMenu.positionPosteTravail.y = editMenu.zoneDossier.positionBoutonDossier.y;
                      editMenu.positionNouvellePlaylist.x = editMenu.zonePlaylist.positionBoutonDossier.x;
                      editMenu.positionNouvellePlaylist.y = editMenu.zonePlaylist.positionBoutonDossier.y;
                      editMenu.zoneMusique.positionBoutonTypePlaylist.x = editMenu.zoneMusique.positionLigneSeparation.x + editMenu.ligneSeparation->w + 10;
                      editMenu.zoneMusique.positionBoutonTypePlaylist.y = editMenu.zonePlaylist.positionBoutonDossier.y;
                      editMenu.zonePlaylist.positionBoutonTypePlaylist.x = editMenu.zonePlaylist.positionLigneSeparation.x + editMenu.ligneSeparation->w + 10;
                      editMenu.zonePlaylist.positionBoutonTypePlaylist.y = editMenu.zonePlaylist.positionBoutonDossier.y;


                      SDL_FreeSurface(editMenu.surfaceSauvegarde);
                      editMenu.surfaceSauvegarde = SDL_CreateRGBSurface(SDL_HWSURFACE, editMenu.surfaceBlit->w, editMenu.surfaceBlit->h, 32, 0, 0, 0, 0);

                      SDL_FreeSurface(configMenu.surfaceSauvegarde);
                      configMenu.surfaceSauvegarde = SDL_CreateRGBSurface(SDL_HWSURFACE, largeurZoneGraphique - ligneSeparation->w, hauteurZoneGraphique - hauteurZoneBoutons, 32, 0, 0, 0, 0);
                      SDL_FillRect(configMenu.surfaceSauvegarde, NULL, SDL_MapRGB(configMenu.surfaceSauvegarde->format, 0, 0, 0));

                      SDL_FreeSurface(configMenu.surfaceBlit);
                      configMenu.surfaceBlit = SDL_CreateRGBSurface(SDL_HWSURFACE, configMenu.surfaceSauvegarde->w, configMenu.surfaceSauvegarde->h, 32, 0, 0, 0, 0);
                      SDL_FillRect(configMenu.surfaceBlit, NULL, SDL_MapRGB(configMenu.surfaceBlit->format, 0, 0, 0));

                      configMenu.imageDeFond = SDL_CreateRGBSurface (SDL_HWSURFACE, largeurZoneGraphique - 25, hauteurZoneGraphique, 32, 0, 0, 0, 0);
                      SDL_FillRect(configMenu.imageDeFond, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
                      configMenu.positionFond.x = largeurZoneReglage + 25;
                      configMenu.positionFond.y = hauteurZoneBoutons;

                      if (imageDiaporama1 != NULL)
                      {
                          if (imageDiaporama1->w > configMenu.imageDeFond->w || imageDiaporama1->h > configMenu.imageDeFond->h)
                          {
                               SDL_Rect nouvelleDimension;
                               nouvelleDimension.h = configMenu.imageDeFond->h;
                               nouvelleDimension.w = configMenu.imageDeFond->w;
                               RSZ_redimensionner_image(nouvelleDimension, &imageDiaporama1);
                          }
                          positionImageDiaporama1.x = (configMenu.imageDeFond->w - imageDiaporama1->w) / 2;
                          positionImageDiaporama1.y = (configMenu.imageDeFond->h - imageDiaporama1->h) / 2;
                      }

                      if (imageDiaporama2 != NULL)
                      {
                          if (imageDiaporama2->w > configMenu.imageDeFond->w || imageDiaporama2->h > configMenu.imageDeFond->h)
                          {
                               SDL_Rect nouvelleDimension;
                               nouvelleDimension.h = configMenu.imageDeFond->h;
                               nouvelleDimension.w = configMenu.imageDeFond->w;
                               RSZ_redimensionner_image(nouvelleDimension, &imageDiaporama2);
                          }
                          positionImageDiaporama2.x = (configMenu.imageDeFond->w - imageDiaporama2->w) / 2;
                          positionImageDiaporama2.y = (configMenu.imageDeFond->h - imageDiaporama2->h) / 2;
                      }

                      positionBoutonTailleVideo.x = (ecran->w - boutonTailleVideo->w) / 2;
                      positionBoutonTailleVideo.y = ecran->h - boutonTailleVideo->h - 10;
                      positionLigneTailleVideo.x = (ecran->w - ligneTailleVideo->w) / 2;
                      positionLigneTailleVideo.y = (boutonTailleVideo->h - ligneTailleVideo->h) / 2 + positionBoutonTailleVideo.y;
                      positionBoutonTailleVideoMoins.x = positionLigneTailleVideo.x - boutonTailleVideo->w - 10;
                      positionBoutonTailleVideoMoins.y = positionBoutonTailleVideo.y;
                      positionBoutonTailleVideoPlus.x = positionLigneTailleVideo.x + ligneTailleVideo->w + 10;
                      positionBoutonTailleVideoPlus.y = positionBoutonTailleVideo.y;
                      positionBoutonModePetitEcran.x = ecran->w - boutonModePetitEcran->w - 10;
                      positionBoutonModePetitEcran.y = ecran->h - boutonModePetitEcran->h - 10;
                      ordonneeBoutonTailleVideo = positionBoutonTailleVideo.y;

                      affichageParoles.hauteurZone = hauteurZoneGraphique;
                      affichageParoles.largeurZone = largeurZoneGraphique - 60;
                      SDL_FreeSurface(affichageParoles.surfaceParoles);
                      affichageParoles.surfaceParoles = SDL_CreateRGBSurface(SDL_HWSURFACE, affichageParoles.largeurZone, affichageParoles.hauteurZone, 32, 0, 0, 0, 0);

                      if (!bigScreenMode)
                         agrandissement = 1.0;
                      if (musique.type == VIDEO)
                         resize_video((SMPEG*) musique.pointeurStereo);

                      recreer_surface(&editMenu);
                      CFG_recreer_surface(&configMenu);
                      actualiser_paroles(&affichageParoles, &musique);

                      break;
                 case SDL_KEYUP:
                      switch (event.key.keysym.sym)
                      {
                             case SDLK_LCTRL:
                                  ctrl = 0;
                                  break;
                             case SDLK_LSHIFT:
                                  maj = 0;
                                  break;
                            default:
                                break;
                      }

                      if (event.key.keysym.sym == toucheActive)
                         toucheActive = 0;
                      break;
                 case SDL_KEYDOWN:
                      switch (event.key.keysym.sym)
                      {
                             case SDLK_LCTRL:
                                  ctrl = 1;
                                  break;
                             case SDLK_LSHIFT:
                                  maj = 1;
                                  break;
                             case SDLK_F2:
                                    if ((fichierListePlaylists = fopen(nomFichierListePlaylists, "r")) == NULL)
                                       fprintf(stderr, "ERR main %d : Le fichier %s n'a pas pu etre ouvert.\n", __LINE__, nomFichierListePlaylists);
                                    else
                                    {
                                        lister_extension("playlists", nomFichierListePlaylists, ".wpl", "w");
                                        while(fgets(nomPlaylist, TAILLE_MAX_NOM, fichierListePlaylists) != NULL)
                                        {
                                                                 if ((positionEntree = strchr(nomPlaylist, '\n')) != NULL)
                                                                    *positionEntree = '\0';
                                                                 strcpy(nomPlaylist2, nomPlaylist);
                                                                 positionExtension = strstr(nomPlaylist2, ".wpl");
                                                                 if (positionExtension != NULL)
                                                                 {
                                                                    strcpy(positionExtension, ".list");
                                                                    convertir_wpl_en_list(nomPlaylist, nomPlaylist2);

                                                                    strcpy(nomPlaylist3, nomPlaylist2);
                                                                    positionExtension = strstr(nomPlaylist3, ".list");
                                                                    if (positionExtension != NULL)
                                                                    {
                                                                           strcpy(positionExtension, ".npl");
                                                                           creer_playlist(nomPlaylist2, nomPlaylist3, typePlaylist);
                                                                    }
                                                                 }
                                        }

                                        lister_extension("playlists", nomFichierListePlaylists, ".m3u", "w");
                                        while(fgets(nomPlaylist, TAILLE_MAX_NOM, fichierListePlaylists) != NULL)
                                        {
                                                                 if ((positionEntree = strchr(nomPlaylist, '\n')) != NULL)
                                                                    *positionEntree = '\0';
                                                                 strcpy(nomPlaylist2, nomPlaylist);
                                                                 if ((positionExtension = strstr(nomPlaylist2, ".m3u")) != NULL)
                                                                 {
                                                                    strcpy(positionExtension, ".list");
                                                                    convertir_m3u_en_list(nomPlaylist, nomPlaylist2);

                                                                    strcpy(nomPlaylist3, nomPlaylist2);
                                                                    positionExtension = strstr(nomPlaylist3, ".list");
                                                                    if (positionExtension != NULL)
                                                                    {
                                                                           strcpy(positionExtension, ".npl");
                                                                           creer_playlist(nomPlaylist2, nomPlaylist3, typePlaylist);
                                                                    }
                                                                 }
                                        }
                                        while(fclose(fichierListePlaylists) == EOF);
                                        mettre_a_jour_playlists(nomFichierListePlaylists, nomFichierPlaylistDePlaylists, tableauTotalMusiques);
                                        creer_playlist(nomFichierListe, nomFichierPlaylist, typePlaylist);
                                        recreer_surface(&editMenu);
                                    }
                                    break;
                             default:
                                     toucheActive = event.key.keysym.sym;
                                     break;
                      }
                      break;
                 case SDL_MOUSEBUTTONUP:
                      switch (event.button.button)
                      {
                             case SDL_BUTTON_WHEELDOWN:
                                  if (numeroVisu == NOMBRE_VISUS)
                                  {
                                                 event.button.x -= editMenu.positionSurfaceBlit.x;
                                                 event.button.y -= editMenu.positionSurfaceBlit.y;

                                                 if (editMenu.zoneDossier.nombreAffichables < editMenu.zoneDossier.nombreTotal && test_position(event, editMenu.zoneDossier.positionLigneSeparation, editMenu.zoneDossier.largeur, editMenu.hauteur))
                                                 {
                                                                  editMenu.zoneDossier.ligneDebut += 1;
                                                                  if (editMenu.zoneDossier.ligneDebut + editMenu.zoneDossier.nombreAffichables > editMenu.zoneDossier.nombreTotal)
                                                                     editMenu.zoneDossier.ligneDebut = editMenu.zoneDossier.nombreTotal - editMenu.zoneDossier.nombreAffichables;
                                                                  editMenu.zoneDossier.ligneFin = editMenu.zoneDossier.ligneDebut + editMenu.zoneDossier.nombreAffichables;
                                                                  recreer_surface(&editMenu);
                                                 }
                                                 else if (editMenu.zoneMusique.nombreAffichables < editMenu.zoneMusique.nombreTotal && test_position(event, editMenu.zoneMusique.positionLigneSeparation, editMenu.zoneMusique.largeur, editMenu.hauteur))
                                                 {
                                                                  editMenu.zoneMusique.ligneDebut += 1;
                                                                  if (editMenu.zoneMusique.ligneDebut + editMenu.zoneMusique.nombreAffichables > editMenu.zoneMusique.nombreTotal)
                                                                     editMenu.zoneMusique.ligneDebut = editMenu.zoneMusique.nombreTotal - editMenu.zoneMusique.nombreAffichables;
                                                                  editMenu.zoneMusique.ligneFin = editMenu.zoneMusique.ligneDebut + editMenu.zoneMusique.nombreAffichables;
                                                                  recreer_surface(&editMenu);
                                                 }
                                                 else if (editMenu.zonePlaylist.nombreAffichables < editMenu.zonePlaylist.nombreTotal && test_position(event, editMenu.zonePlaylist.positionLigneSeparation, editMenu.zonePlaylist.largeur, editMenu.hauteur))
                                                 {
                                                                  editMenu.zonePlaylist.ligneDebut += 1;
                                                                  if (editMenu.zonePlaylist.ligneDebut + editMenu.zonePlaylist.nombreAffichables > editMenu.zonePlaylist.nombreTotal)
                                                                     editMenu.zonePlaylist.ligneDebut = editMenu.zonePlaylist.nombreTotal - editMenu.zonePlaylist.nombreAffichables;
                                                                  editMenu.zonePlaylist.ligneFin = editMenu.zonePlaylist.ligneDebut + editMenu.zonePlaylist.nombreAffichables;
                                                                  recreer_surface(&editMenu);
                                                 }

                                                 event.button.x += editMenu.positionSurfaceBlit.x;
                                                 event.button.y += editMenu.positionSurfaceBlit.y;
                                  }
                                  else if (numeroVisu == 2 && test_position(event, affichageParoles.positionSurfaceParoles, affichageParoles.largeurZone, affichageParoles.hauteurZone))
                                  {
                                       affichageParoles.positionBarreDefilement.y += 10;
                                       if (affichageParoles.positionBarreDefilement.y + affichageParoles.hauteurBarreDefilement > affichageParoles.hauteurZone + affichageParoles.hauteurDiff)
                                          affichageParoles.positionBarreDefilement.y = affichageParoles.hauteurZone - affichageParoles.hauteurBarreDefilement + affichageParoles.hauteurDiff;
                                       actualiser_paroles(&affichageParoles, &musique);
                                  }

                                  if (nombreMusiquesAffichables < nombreMusiques && test_position(event, positionLigneSeparation3, largeurZoneListe, hauteurZoneLecteur))
                                  {
                                     positionBarreDefilement.y += 10;
                                     if (positionBarreDefilement.y > hauteurZoneLecteur - hauteurBarreDefilement)
                                        positionBarreDefilement.y = hauteurZoneLecteur - hauteurBarreDefilement;
                                     ligneDebut = (positionBarreDefilement.y * (nombreMusiques - nombreMusiquesAffichables)) / ((hauteurZoneLecteur - hauteurBarreDefilement) / 1.0);
                                     ligneFin = ligneDebut + nombreMusiquesAffichables;
                                  }

                                  break;
                             case SDL_BUTTON_WHEELUP:
                                  if (numeroVisu == NOMBRE_VISUS)
                                  {
                                                 event.button.x -= editMenu.positionSurfaceBlit.x;
                                                 event.button.y -= editMenu.positionSurfaceBlit.y;

                                                 if (editMenu.zoneDossier.nombreAffichables < editMenu.zoneDossier.nombreTotal && test_position(event, editMenu.zoneDossier.positionLigneSeparation, editMenu.zoneDossier.largeur, editMenu.hauteur))
                                                 {
                                                                  editMenu.zoneDossier.ligneDebut -= 1;
                                                                  if (editMenu.zoneDossier.ligneDebut < 0)
                                                                     editMenu.zoneDossier.ligneDebut = 0;
                                                                  editMenu.zoneDossier.ligneFin = editMenu.zoneDossier.ligneDebut + editMenu.zoneDossier.nombreAffichables;
                                                                  recreer_surface(&editMenu);
                                                 }
                                                 else if (editMenu.zoneMusique.nombreAffichables < editMenu.zoneMusique.nombreTotal && test_position(event, editMenu.zoneMusique.positionLigneSeparation, editMenu.zoneMusique.largeur, editMenu.hauteur))
                                                 {
                                                                  editMenu.zoneMusique.ligneDebut -= 1;
                                                                  if (editMenu.zoneMusique.ligneDebut < 0)
                                                                     editMenu.zoneMusique.ligneDebut = 0;
                                                                  editMenu.zoneMusique.ligneFin = editMenu.zoneMusique.ligneDebut + editMenu.zoneMusique.nombreAffichables;
                                                                  recreer_surface(&editMenu);
                                                 }
                                                 else if (editMenu.zonePlaylist.nombreAffichables < editMenu.zonePlaylist.nombreTotal && test_position(event, editMenu.zonePlaylist.positionLigneSeparation, editMenu.zonePlaylist.largeur, editMenu.hauteur))
                                                 {
                                                                  editMenu.zonePlaylist.ligneDebut -= 1;
                                                                  if (editMenu.zonePlaylist.ligneDebut < 0)
                                                                     editMenu.zonePlaylist.ligneDebut = 0;
                                                                  editMenu.zonePlaylist.ligneFin = editMenu.zonePlaylist.ligneDebut + editMenu.zonePlaylist.nombreAffichables;
                                                                  recreer_surface(&editMenu);
                                                 }

                                                 event.button.x += editMenu.positionSurfaceBlit.x;
                                                 event.button.y += editMenu.positionSurfaceBlit.y;
                                  }
                                  else if (numeroVisu == 2 && test_position(event, affichageParoles.positionSurfaceParoles, affichageParoles.largeurZone, affichageParoles.hauteurZone))
                                  {
                                       affichageParoles.positionBarreDefilement.y -= 10;
                                       if (affichageParoles.positionBarreDefilement.y < affichageParoles.hauteurDiff)
                                          affichageParoles.positionBarreDefilement.y = affichageParoles.hauteurDiff;
                                       actualiser_paroles(&affichageParoles, &musique);
                                  }

                                  if (nombreMusiquesAffichables < nombreMusiques && test_position(event, positionLigneSeparation3, largeurZoneListe, hauteurZoneLecteur))
                                  {
                                     positionBarreDefilement.y -= 10;
                                     if (positionBarreDefilement.y < 0)
                                        positionBarreDefilement.y = 0;
                                     ligneDebut = (positionBarreDefilement.y * (nombreMusiques - nombreMusiquesAffichables)) / ((hauteurZoneLecteur - hauteurBarreDefilement) / 1.0);
                                     ligneFin = ligneDebut + nombreMusiquesAffichables;
                                  }

                                  break;
                      }
                      break;
          }


          if (musique.mode != PLAY || musique.type == VIDEO)
             tempsRafraichissement = TEMPS_RAFRAICHISSEMENT_LENT;
          else tempsRafraichissement = configMenu.parametre.rafraichissement;


             volume = (volumeDroit + volumeGauche) / 2;
             if (musique.type != VIDEO)
             {
                 FSOUND_SetVolume(1, volume);
                 FSOUND_SetPan(1, ceil((255 - volumeGauche + volumeDroit) / 2));
             }
             else SMPEG_setvolume((SMPEG*) musique.pointeurStereo, volume * 100 / 255);

             if (musique.mode == ERREUR || musique.mode == STOP)
                musique.curseur = 0;
             else
             {
                 SMPEG_Info SInfo;
                 if (musique.type == VIDEO && musique.mode == PLAY)
                 {
                    SMPEG_getinfo((SMPEG*) musique.pointeurStereo, &SInfo);
                    musique.curseur = (SInfo.current_time + secondsSkipped) * 1000;
                 }
                 else if (musique.type != VIDEO)
                 {
                     if (indicStereoMono)
                        musique.curseur = FSOUND_Stream_GetTime(musique.pointeurStereo);
                     else musique.curseur = FSOUND_Stream_GetTime(musique.pointeurMono);
                 }

                 if (clicGaucheSurFlecheLecture)
                    musique.curseur = (positionFlecheLecture.x * musique.longueur) / ecran->w;
             }

             diviser_temps_et_stocker(musique.curseur, NULL, &secondesMusiqueEcoulees, &minutesMusiqueEcoulees, NULL, texteTempsMusiqueEcoule);
             if (musique.type == RADIO)
                strcpy (texteTempsMusiqueEcoule, "RADIO");
             if (musique.mode == STOP)
                strcpy(texteTempsMusiqueEcoule, "STOP");
             if (musique.mode == ERREUR)
                strcpy(texteTempsMusiqueEcoule, "ERREUR");
             SDL_FreeSurface(musique.surface.curseur);
             musique.surface.curseur = TTF_RenderText_Blended(policeTempsMusiqueEcoule, texteTempsMusiqueEcoule, couleurRouge);
             musique.position.curseur.x = (largeurZoneReglage - musique.surface.curseur->w) / 2;

             if (((musique.curseur >= tempsPointB && clicGaucheSurLettreB != 1) || (musique.curseur < tempsPointA && clicGaucheSurLettreA != 1)) && modeAB == 2 && musique.type != RADIO && musique.mode != STOP && musique.mode != ERREUR)
             {
                if (musique.type != VIDEO)
                {
                    FSOUND_Stream_SetTime(musique.pointeurStereo, tempsPointA);
                    FSOUND_Stream_SetTime(musique.pointeurMono, tempsPointA);
                }
                else
                {
                    secondsSkipped = tempsPointA / 1000.0;
                    SMPEG_rewind((SMPEG*) musique.pointeurStereo);
                    SMPEG_play((SMPEG*) musique.pointeurStereo);
                    SMPEG_skip((SMPEG*) musique.pointeurStereo, secondsSkipped);
                }
             }

             if (modeBoucle && musique.type == VIDEO && musique.mode == PLAY && SMPEG_status((SMPEG*) musique.pointeurStereo) != SMPEG_PLAYING)
             {
                 SMPEG_rewind((SMPEG*) musique.pointeurStereo);
                 SMPEG_play((SMPEG*) musique.pointeurStereo);
                 secondsSkipped = 0;
             }

             if (musique.mode != STOP && musique.mode != ERREUR && modeBoucle == 0 && musique.type != RADIO && ((musique.type == VIDEO && musique.mode == PLAY && SMPEG_status((SMPEG*) musique.pointeurStereo) != SMPEG_PLAYING) || (musique.type != VIDEO && musique.curseur >= musique.longueur - 500)))
             {
                                if (musique.type != VIDEO)
                                {
                                    FSOUND_Stream_Stop(musique.pointeurStereo);
                                    FSOUND_Stream_Stop(musique.pointeurMono);
                                }
                                else SMPEG_rewind((SMPEG*) musique.pointeurStereo);

                                i = 0;
                                do
                                {
                                    if (musique.type != VIDEO)
                                    {
                                        FSOUND_Stream_Close(musique.pointeurStereo);
                                        FSOUND_Stream_Close(musique.pointeurMono);
                                    }
                                    else SMPEG_delete((SMPEG*) musique.pointeurStereo);

                                    do
                                    {
                                        if (musique.mode == ERREUR)
                                             tableauTotalMusiques[musique.numero] = -1;
                                        if (modeAleatoire)
                                           charger_musique(&musique, configMenu, &affichageParoles, nomFichierListe, -1, positionBoutonReculRapide.x - musique.position.titre.x - 40, &i);
                                        else
                                        {
                                             musique.numero++;
                                             if (musique.numero > nombreMusiques)
                                                musique.numero = 1;
                                             charger_musique(&musique, configMenu, &affichageParoles, nomFichierListe, musique.numero, positionBoutonReculRapide.x - musique.position.titre.x - 40, &i);
                                        }
                                        i++;
                                    } while (musique.mode == ERREUR && i < configMenu.parametre.nombreChargementsMaxi);
                                    if (i >= configMenu.parametre.nombreChargementsMaxi)
                                    {
                                          for (j = 0 ; j < TAILLE_MAX_PLAYLIST ; j++)
                                          {
                                              tableauTotalMusiques[j] = 0;
                                          }
                                          if (musique.mode != ERREUR)
                                             musique.mode = STOP;
                                    }
                                } while (tableauTotalMusiques[musique.numero] && modeAleatoire && i < configMenu.parametre.nombreChargementsMaxi);


                                if (nombreMusiques > nombreMusiquesAffichables && clicGaucheSurBarreDefilement != 1 && (musique.numero < ligneDebut || musique.numero > ligneFin))
                                {
                                              ligneDebut = musique.numero - 1;
                                              ligneFin = ligneDebut + nombreMusiquesAffichables;
                                              positionBarreDefilement.y = (ligneDebut * (hauteurZoneLecteur - hauteurBarreDefilement)) / ((nombreMusiques - nombreMusiquesAffichables) / 1.0);
                                }

                                if (musique.mode == ERREUR || musique.mode == STOP)
                                   mettre_en_stop(&musique, surfaceBoutonPlayPause);
                                else
                                {
                                    if (musique.type != VIDEO)
                                    {
                                        if (indicStereoMono)
                                           FSOUND_Stream_Play(1, musique.pointeurStereo);
                                        else FSOUND_Stream_Play(1, musique.pointeurMono);
                                    }
                                    else SMPEG_play((SMPEG*) musique.pointeurStereo);

                                    positionDansTableauDernieresMusiques++;
                                    if (positionDansTableauDernieresMusiques > 10)
                                       positionDansTableauDernieresMusiques = 0;
                                    tableauDernieresMusiques[positionDansTableauDernieresMusiques] = musique.numero;
                                    tableauTotalMusiques[musique.numero] = 1;

                                    musique.mode = PLAY;
                                }
             }

             if (clicGaucheSurFlecheLecture != 1 && musique.mode != STOP && musique.mode != ERREUR)
                positionFlecheLecture.x = (musique.curseur * (ecran->w - flecheLecture->w)) / musique.longueur;
             else if (clicGaucheSurFlecheLecture != 1 && (musique.mode == ERREUR || musique.mode == STOP))
                  positionFlecheLecture.x = 0;



             if (test_exist(nomFichierPlaylist) == 0)
                creer_playlist(nomFichierListe, nomFichierPlaylist, typePlaylist);

             if (musique.mode == PLAY)
                surfaceBoutonPlayPause = surfaceBoutonPause;
             else surfaceBoutonPlayPause = surfaceBoutonPlay;

             if (numeroVisu == -1 && musique.mode != ERREUR)
                numeroVisu = 0;
             if (numeroVisu != -1 && musique.mode == ERREUR)
                numeroVisu = -1;


             //rafraichissement des données internet
             int bufferused = 0;

             if (musique.type != NORMAL && musique.type != VIDEO)
             {
                                 FSOUND_Stream_Net_GetStatus(musique.pointeurStereo, &musique.statutInternet, &bufferused, &musique.debit, (unsigned int*)&musique.protocoleInternet);

                                 if ((musique.protocoleInternet & FSOUND_PROTOCOL_SHOUTCAST) == FSOUND_PROTOCOL_SHOUTCAST)
                                    strcpy(musique.album, "Protocole Shoutcast");
                                 else if ((musique.protocoleInternet & FSOUND_PROTOCOL_ICECAST) == FSOUND_PROTOCOL_ICECAST)
                                      strcpy(musique.album, "Protocole Icecast");
                                 else if ((musique.protocoleInternet & FSOUND_PROTOCOL_HTTP) == FSOUND_PROTOCOL_HTTP)
                                      strcpy(musique.album, "Protocole HTTP");
                                 else strcpy(musique.album, "Protocole non reconnu");

                                 switch(musique.statutInternet)
                                 {
                                                                case FSOUND_STREAM_NET_NOTCONNECTED:
                                                                     sprintf(musique.album, "%s : Non connecté", musique.album);
                                                                     break;
                                                                case FSOUND_STREAM_NET_READY:
                                                                     sprintf(musique.album, "%s : Prêt", musique.album);
                                                                     break;
                                                                case FSOUND_STREAM_NET_ERROR:
                                                                     sprintf(musique.album, "%s : Erreur", musique.album);
                                                                     break;
                                                                default :
                                                                     sprintf(musique.album, "%s : Erreur", musique.album);
                                                                     break;
                                 }

                                  if ((musique.protocoleInternet & FSOUND_FORMAT_MPEG) == FSOUND_FORMAT_MPEG)
                                    strcpy(musique.format, "MP3");
                                 else if ((musique.protocoleInternet & FSOUND_FORMAT_OGGVORBIS) == FSOUND_FORMAT_OGGVORBIS)
                                      strcpy(musique.format, "OGG");
                                 else strcpy(musique.format, "...");


                                 char temp[TAILLE_MAX_NOM];
                                 strcpy(temp, musique.album);
                                 couper_texte(temp, positionBoutonReculRapide.x - musique.position.titre.x - 40, policeNomMusique);
                                 if (musique.surface.album != NULL)
                                    SDL_FreeSurface(musique.surface.album);
                                 musique.surface.album = TTF_RenderText_Blended(policeNomMusique, temp, couleurBlanc);

                                 if (musique.surface.format != NULL)
                                    SDL_FreeSurface(musique.surface.format);
                                 musique.surface.format = TTF_RenderText_Blended(policeFormatMusique, musique.format, couleurJaune);
             }

                        // tests de communications d'éventuelles applications parallèles
                        for (i = 0 ; i < 500 ; i++)
                        {
                             adressChar = (sharedMemory + sizeof(HWND) + (i * TAILLE_MAX_NOM));
                             if (*adressChar != '\0')
                                break;
                        }

                        if (*adressChar != '\0')
                        {
                                               strcpy(nomFichierListe, adressChar);

                                               strcpy(nomFichierPlaylist, nomFichierListe);
                                               *strstr(nomFichierPlaylist, ".list") = '\0';
                                               strcat(nomFichierPlaylist, ".npl");

                                               creer_playlist(nomFichierListe, nomFichierPlaylist, TITRE);
                                               memset(adressChar, '\0', TAILLE_MAX_NOM);

                                               mettre_a_jour_playlists(nomFichierListePlaylists, nomFichierPlaylistDePlaylists, tableauTotalMusiques);

                                               if (musique.type != VIDEO)
                                               {
                                                   FSOUND_Stream_Stop(musique.pointeurStereo);
                                                   FSOUND_Stream_Stop(musique.pointeurMono);
                                                   FSOUND_Stream_Close(musique.pointeurStereo);
                                                   FSOUND_Stream_Close(musique.pointeurMono);
                                               }
                                               else
                                               {
                                                    SMPEG_stop((SMPEG*) musique.pointeurStereo);
                                                    SMPEG_delete((SMPEG*) musique.pointeurStereo);
                                               }

                                               modeListe = 0;
                                               SDL_FreeSurface(boutonModeListe);
                                               boutonModeListe = IMG_Load("img\\other\\bouton_playlists.jpg");

                                               if ((fichierPlaylist = fopen(nomFichierPlaylist, "r")) == NULL)
                                                      fprintf(stderr, "ERR main %d : Le fichier %s n'a pas pu etre ouvert.\n", __LINE__, nomFichierPlaylist);
                                               else
                                               {
                                                   nombreMusiques = 0;
                                                   while(fgets(temp, TAILLE_MAX_NOM, fichierPlaylist) != NULL)
                                                                         nombreMusiques++;
                                                   while(fclose(fichierPlaylist) == EOF);

                                                   hauteurBarreDefilement = (nombreMusiquesAffichables / (nombreMusiques / 1.0)) * hauteurZoneLecteur;
                                                   SDL_FreeSurface(barreDefilement);
                                                   barreDefilement = SDL_CreateRGBSurface(SDL_HWSURFACE, 26, hauteurBarreDefilement, 32, 0, 0, 0, 0);
                                                   SDL_FreeSurface(rectangle);
                                                   rectangle = SDL_CreateRGBSurface(SDL_HWSURFACE, 1, hauteurBarreDefilement, 32, 0, 0, 0, 0);
                                                   for (i = 0 ; i <= 12 ; i++)
                                                   {
                                                           SDL_FillRect(rectangle, NULL, SDL_MapRGB(ecran->format, (255 * i) / 12, (255 * i) / 12, (255 * i) / 12));
                                                           position.x = i;
                                                           SDL_BlitSurface(rectangle, NULL, barreDefilement, &position);
                                                   }
                                                   for (i = 0 ; i <= 12 ; i++)
                                                   {
                                                           SDL_FillRect(rectangle, NULL, SDL_MapRGB(ecran->format, 255 - (255 * i) / 12, 255 - (255 * i) / 12, 255 - (255 * i) / 12));
                                                           position.x = i + 13;
                                                           SDL_BlitSurface(rectangle, NULL, barreDefilement, &position);
                                                   }
                                                   SDL_FreeSurface(rectangle);
                                                   positionBarreDefilement.x = positionLigneSeparation3.x;
                                                   positionBarreDefilement.y = 0;

                                                   for (j = 0 ; j < TAILLE_MAX_PLAYLIST ; j++)
                                                   {
                                                           tableauTotalMusiques[j] = 0;
                                                           if (j <= 10)
                                                              tableauDernieresMusiques[j] = -1;
                                                   }

                                                   j = 0;

                                                   do
                                                   {
                                                           charger_musique(&musique, configMenu, &affichageParoles, nomFichierListe, compter_lignes(nomFichierListe), positionBoutonReculRapide.x - musique.position.titre.x - 40, &j);
                                                           if (musique.mode == ERREUR)
                                                              tableauTotalMusiques[musique.numero] = -1;
                                                           j++;
                                                   } while (musique.mode == ERREUR && j < configMenu.parametre.nombreChargementsMaxi);

                                                   ligneDebut = 0;
                                                   ligneFin = nombreMusiquesAffichables;

                                                   if (nombreMusiques > nombreMusiquesAffichables  && (musique.numero < ligneDebut || musique.numero > ligneFin))
                                                   {
                                                                          ligneDebut = musique.numero - 1;
                                                                          ligneFin = ligneDebut + nombreMusiquesAffichables;
                                                                          positionBarreDefilement.y = (ligneDebut * (hauteurZoneLecteur - hauteurBarreDefilement)) / ((nombreMusiques - nombreMusiquesAffichables) / 1.0);
                                                   }
                                                   if (musique.mode == ERREUR || musique.mode == STOP)
                                                          mettre_en_stop(&musique, surfaceBoutonPlayPause);
                                                   else
                                                   {
                                                           if (musique.type != VIDEO)
                                                           {
                                                               if (indicStereoMono)
                                                                  FSOUND_Stream_Play(1, musique.pointeurStereo);
                                                               else FSOUND_Stream_Play(1, musique.pointeurMono);
                                                           }
                                                           else SMPEG_play((SMPEG*) musique.pointeurStereo);

                                                           positionDansTableauDernieresMusiques = 0;
                                                           tableauDernieresMusiques[0] = musique.numero;
                                                           tableauTotalMusiques[musique.numero] = 1;

                                                           musique.mode = PLAY;
                                                   }
                                                   volume = (volumeDroit + volumeGauche) / 2;
                                                   if (musique.type != VIDEO)
                                                   {
                                                       FSOUND_SetVolume(1, volume);
                                                       FSOUND_SetPan(1, ceil((255 - volumeGauche + volumeDroit) / 2));
                                                   }
                                                   else SMPEG_setvolume((SMPEG*) musique.pointeurStereo, volume * 100 / 255);

                                                   sprintf(title, "%s - %s", musique.titre, musique.auteur);
                                                   if (musique.type == NORMAL)
                                                       SetWindowText(mainWnd, title);
                                                   else SetWindowText(mainWnd, musique.titre);
                                               }
                        }
                        // fin du test



             SDL_Flip(ecran);



             //ajout du diaporama
             if (imageDiaporama2 != NULL && musique.type != VIDEO)   //en transition, image2 -> image1
             {
                 SDL_FillRect(configMenu.imageDeFond, NULL, SDL_MapRGB(configMenu.imageDeFond->format, 0, 0, 0));

                 int transparency = (SDL_GetTicks() - tempsDernierEffetDiaporama) * 255 / configMenu.parametre.vitesseTransition;
                 if (transparency > 255)
                    transparency = 255;
                 if (transparency < 0)
                    transparency = 0;

                 SDL_SetAlpha(imageDiaporama1, SDL_SRCALPHA, transparency);
                 SDL_SetAlpha(imageDiaporama2, SDL_SRCALPHA, 255 - transparency);
                 SDL_BlitSurface(imageDiaporama2, NULL, configMenu.imageDeFond, &positionImageDiaporama2);
                 if (imageDiaporama1 != NULL)
                    SDL_BlitSurface(imageDiaporama1, NULL, configMenu.imageDeFond, &positionImageDiaporama1);
                 if (transparency >= 255)   //fin de la transition
                 {
                     SDL_FreeSurface(imageDiaporama2);
                     imageDiaporama2 = NULL;
                     tempsDernierEffetDiaporama = SDL_GetTicks();
                 }
             }
             else if (configMenu.parametre.nombreImagesDiaporama > 0 && musique.type != VIDEO)   //normal
             {
                 if (SDL_GetTicks() - tempsDernierEffetDiaporama >= configMenu.parametre.vitesseDefilement)   //début de transition
                 {
                     char buffer[TAILLE_MAX_NOM] = "\0";

                     configMenu.parametre.numeroImageDiaporama++;
                     if (configMenu.parametre.numeroImageDiaporama > configMenu.parametre.nombreImagesDiaporama)
                        configMenu.parametre.numeroImageDiaporama = 1;
                     lire_ligne(configMenu.parametre.fichierListeDiaporama, configMenu.parametre.numeroImageDiaporama, buffer, TAILLE_MAX_NOM);

                     imageDiaporama2 = imageDiaporama1;
                     imageDiaporama1 = IMG_Load(buffer);
                     if (imageDiaporama1 == NULL)
                     {
                         imageDiaporama1 = imageDiaporama2;
                         imageDiaporama2 = NULL;
                     }
                     else
                     {
                          if (imageDiaporama1->w > configMenu.imageDeFond->w || imageDiaporama1->h > configMenu.imageDeFond->h)
                          {
                               SDL_Rect nouvelleDimension;
                               nouvelleDimension.h = configMenu.imageDeFond->h;
                               nouvelleDimension.w = configMenu.imageDeFond->w;
                               RSZ_redimensionner_image(nouvelleDimension, &imageDiaporama1);
                          }
                          positionImageDiaporama1.x = (configMenu.imageDeFond->w - imageDiaporama1->w) / 2;
                          positionImageDiaporama1.y = (configMenu.imageDeFond->h - imageDiaporama1->h) / 2;

                          if (imageDiaporama2 != NULL && (imageDiaporama2->w > configMenu.imageDeFond->w || imageDiaporama2->h > configMenu.imageDeFond->h))
                          {
                               SDL_Rect nouvelleDimension;
                               nouvelleDimension.h = configMenu.imageDeFond->h;
                               nouvelleDimension.w = configMenu.imageDeFond->w;
                               RSZ_redimensionner_image(nouvelleDimension, &imageDiaporama2);
                          }

                          if (imageDiaporama2 != NULL)
                          {
                              positionImageDiaporama2.x = (configMenu.imageDeFond->w - imageDiaporama2->w) / 2;
                              positionImageDiaporama2.y = (configMenu.imageDeFond->h - imageDiaporama2->h) / 2;
                          }

                          tempsDernierEffetDiaporama = SDL_GetTicks();
                     }
                 }

                 SDL_FillRect(configMenu.imageDeFond, NULL, SDL_MapRGB(configMenu.imageDeFond->format, 0, 0, 0));
                 if (imageDiaporama2 != NULL)
                    SDL_BlitSurface(imageDiaporama2, NULL, configMenu.imageDeFond, &positionImageDiaporama2);
                 else if (imageDiaporama1 != NULL)
                    SDL_BlitSurface(imageDiaporama1, NULL, configMenu.imageDeFond, &positionImageDiaporama1);
             }



             if (numeroVisu < NOMBRE_VISUS)
             {
                if (musique.type != VIDEO)
                {
                   SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
                   SDL_BlitSurface(configMenu.imageDeFond, NULL, ecran, &configMenu.positionFond);
                }
                else
                {
                    SDL_Rect rect;
                    Uint32 color = SDL_MapRGB(ecran->format, 0, 0, 0);

                    rect.x = 0;
                    rect.y = 0;
                    rect.w = positionSurfaceSMPEG.x;
                    rect.h = ecran->h;
                    SDL_FillRect(ecran, &rect, color);

                    rect.x = 0;
                    rect.y = 0;
                    rect.w = ecran->w;
                    rect.h = positionSurfaceSMPEG.y;
                    SDL_FillRect(ecran, &rect, color);

                    rect.x = positionSurfaceSMPEG.x + positionSurfaceSMPEG.w;
                    rect.y = 0;
                    rect.w = ecran->w - rect.x;
                    rect.h = ecran->h;
                    SDL_FillRect(ecran, &rect, color);

                    rect.x = 0;
                    rect.y = positionSurfaceSMPEG.y + positionSurfaceSMPEG.h;
                    rect.w = ecran->w;
                    rect.h = ecran->h - rect.y;
                    SDL_FillRect(ecran, &rect, color);
                }
             }


             if (numeroVisu == 0 && musique.type != VIDEO)
             {
                         int l = 0;
                         for (i = 0 ; i <= largeurZoneGraphique - 50 - 60 ; i += 60)
                         {
                                   int k;
                                   float spectreTotal = 0.0;
                                   for (k = l ; k <= (50 / ((largeurZoneGraphique - 50) / 60)) + l ; k++)
                                   {
                                       if (k <= 510)
                                          spectreTotal = spectreTotal + spectre[k];
                                   }
                                   l = k + 1;
                                   float moyenneSpectre = spectreTotal / k;
                                   if (volume > 0)
                                      moyenneSpectre *= 200 / (volume / 1.0);
                                   int hauteurBarre = moyenneSpectre * 20 * hauteurZoneGraphique;
                                   if (hauteurBarre > hauteurZoneGraphique)
                                      hauteurBarre = hauteurZoneGraphique;
                                   for (j = hauteurZoneGraphique ; j >= hauteurZoneGraphique - hauteurBarre + 30 ; j -= (30 + 5))
                                   {
                                       Uint32 couleurPixel = SDL_MapRGB(ecran->format, ceil((255 * j) / hauteurZoneGraphique), ceil(255 - (((255 * j) / hauteurZoneGraphique) * (i / (largeurZoneGraphique / 1.0)))), 0);
                                       setRectangle(ecran, i + largeurZoneReglage + 50, j - 35 + hauteurZoneBoutons, couleurPixel);
                                   }
                         }
               }
               else if (numeroVisu == 1 && musique.type != VIDEO)
               {
                         float moyenneSpectre = 0;
                         for (i = 0 ; i <= 511 ; i++)
                         {
                             moyenneSpectre += (spectre[i] * 40);
                         }
                         moyenneSpectre /= 512.0;
                         if (volume > 0)
                            moyenneSpectre *= 200 / (volume / 1.0);
                         hauteurBarre = ((hauteurZoneGraphique / 2.0) - 100) * moyenneSpectre;
                         positionBarreNoire.x = positionBarreVisu2;
                         SDL_BlitSurface(barreNoire, NULL, surfaceVisu, &positionBarreNoire);
                         for (i = (hauteurZoneGraphique / 2.0) - hauteurBarre ; i <= hauteurZoneGraphique / 2.0 ; i++)
                         {
                             setPixel2(surfaceVisu, positionBarreVisu2, i, SDL_MapRGB(ecran->format, (255 * (i - 100)) / ((hauteurZoneGraphique / 2.0) - 100), 255, 0));
                         }
                         for (i = hauteurZoneGraphique / 2.0 ; i <= (hauteurZoneGraphique / 2.0) + hauteurBarre ; i++)
                         {
                             setPixel2(surfaceVisu, positionBarreVisu2, i, SDL_MapRGB(ecran->format, 255 - ((255 * (i - (hauteurZoneGraphique / 2.0))) / (hauteurZoneGraphique - 100 - (hauteurZoneGraphique / 2.0))), 255, 0));
                         }
                         positionBarreVisu2++;
                         if (positionBarreVisu2 > largeurZoneGraphique)
                            positionBarreVisu2 = 0;

                         positionTriangle1.x = positionBarreVisu2 - 12;
                         positionTriangle2.x = positionBarreVisu2 - 12;
                         positionBarreInterTriangles.x = positionBarreVisu2;
                         SDL_BlitSurface(surfaceCache1, NULL, surfaceVisu, &positionSurfaceCache1);
                         SDL_BlitSurface(surfaceCache1, NULL, surfaceVisu, &positionSurfaceCache2);
                         SDL_BlitSurface(barreInterTriangles, NULL, surfaceVisu, &positionBarreInterTriangles);
                         SDL_BlitSurface(triangle1, NULL, surfaceVisu, &positionTriangle1);
                         SDL_BlitSurface(triangle2, NULL, surfaceVisu, &positionTriangle2);

                         SDL_SetColorKey(surfaceVisu, SDL_SRCCOLORKEY, SDL_MapRGB(surfaceVisu->format, 0, 0, 0));
                         SDL_BlitSurface(surfaceVisu, NULL, ecran, &positionVisu2);
                         SDL_BlitSurface(ligneSeparation4, NULL, ecran, &positionLigneSeparation4);
             }
             else if (numeroVisu == 2 && musique.type != VIDEO)
             {
                  SDL_BlitSurface(ligneSeparation, NULL, ecran, &positionLigneSeparation);
                  afficher_paroles (&affichageParoles, &musique, event);
             }
             else if (numeroVisu == NOMBRE_VISUS && !bigScreenMode)
             {
                  if (editer_playlist(&editMenu, &musique, configMenu, &affichageParoles, event, positionBoutonReculRapide.x - musique.position.titre.x - 40, &ctrl, &maj) == 2)
                  {
                          numeroVisu++;
                          if (numeroVisu > NOMBRE_VISUS)
                             numeroVisu = 0;
                          SDL_FillRect(surfaceVisu, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
                          positionBarreVisu2 = 0;
                  }
                  else
                  {
                          SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
                          SDL_BlitSurface(ligneSeparation4, NULL, ecran, &positionLigneSeparation4);
                          SDL_BlitSurface(ligneSeparation, NULL, ecran, &positionLigneSeparation);
                          SDL_BlitSurface(editMenu.surfaceBlit, NULL, ecran, &editMenu.positionSurfaceBlit);
                  }
             }
             else if (numeroVisu == NOMBRE_VISUS + 1 && !bigScreenMode)
             {
                  CFG_configurer_lecteur(&configMenu, event);
                  SDL_FillRect(ecran, NULL, SDL_MapRGB(ecran->format, 0, 0, 0));
                  SDL_BlitSurface(configMenu.surfaceBlit, NULL, ecran, &configMenu.positionBlit);
             }
             else if (numeroVisu == -1)
             {
                     bigScreenMode = 0;
                     agrandissement = 1.0;

                     char texteAAfficher[5000] = "ERREUR";
                     if (strcmp(musique.titre, "Loading Error") == 0 && (musique.type == NORMAL || musique.type == VIDEO))
                        sprintf(texteAAfficher, "ERREUR DE CHARGEMENT\n\nLe logiciel a rencontré une erreur : plusieurs chargements ont été effectués sans succès.\nVeuillez vérifier les points suivants :\n\n  1° Si vous utilisez une playlist personnelle, vérifiez sa validité.\n  2° Les musiques ont peut-être été déplacées, renommées ou supprimées.\n  3° Si vous utilisez une playlist wpl, vérifiez qu'elle est enregistrée au format ANSI (voir FAQ).\n\nSi ces points ne donnent aucun résultat, effacez votre playlist et recréez-la.\n\nCokie, CrazyProg, pour vous servir =P");
                     else if (strcmp(musique.titre, "Loading Error") == 0)
                          sprintf(texteAAfficher, "ERREUR DE CHARGEMENT\n\nLe logiciel a rencontré une erreur : plusieurs chargements ont été effectués sans succès.\nVeuillez vérifier les points suivants :\n\n  1° Vous êtes peut-être déconnecté d'Internet.\n  2° L'adresse de la webradio que vous utilisez : %s est peut-être erronée ou non valide (voir FAQ).\n  3° Le site présente peut-être des problèmes de chargement.\n\nPour vérifier simplement votre webradio, transmettez l'adresse à votre navigateur Internet.\n\nCokie, CrazyProg, pour vous servir =P", musique.auteur);
                     else if (strcmp(musique.titre, "Opening Error") == 0)
                          sprintf(texteAAfficher, "ERREUR D'OUVERTURE\n\nLe logiciel a rencontré une erreur : Le fichier %s n'a pas pu etre ouvert.\nCe fichier est indispensable au chargement de vos musiques.\nVeuillez vérifier les points suivants :\n\n  1° Le fichier a peut-être été déplacé, renommé ou supprimé.\n  2° Le fichier est peut-être utilisé par un autre programme. S'il est actuellement ouvert, fermez-le.\n\nSi ces points ne donnent aucun résultat, effacez votre playlist et recréez-la.\n\nCokie, CrazyProg, pour vous servir =P", nomFichierListe);

                     afficher_texte(texteAAfficher, policeDErreur, couleurBlanc, couleurNoir, 1, 5, largeurZoneGraphique - 50, positionTexteErreur, ecran);
             }

             if (!bigScreenMode)
             {
                 if (musique.type == VIDEO && numeroVisu < NOMBRE_VISUS && numeroVisu >= 0)
                    SDL_BlitSurface(boutonModeGrandEcran, NULL, ecran, &positionBoutonModeGrandEcran);
                 else SDL_BlitSurface(boutonChangementVisu, NULL, ecran, &positionBoutonChangementVisu);
                 SDL_BlitSurface(boutonSettings, NULL, ecran, &positionBoutonSettings);
                 SDL_BlitSurface(boutonResearch, NULL, ecran, &positionBoutonResearch);
                 SDL_BlitSurface(surfaceTempsRafraichissement, NULL, ecran, &positionTempsRafraichissement);
                 SDL_BlitSurface(boutonAjustementAudio, NULL, ecran, &positionBoutonAjustementAudio);;
                 SDL_BlitSurface(surfaceLockIndic, NULL, ecran, &positionLockIndic);
                 SDL_BlitSurface(musique.surface.frequence, NULL, ecran, &musique.position.frequence);
                 musique.position.longueur.x = (largeurZoneReglage - musique.surface.longueur->w) / 2;
                 SDL_BlitSurface(musique.surface.longueur, NULL, ecran, &musique.position.longueur);
                 musique.position.poids.x = (largeurZoneReglage - musique.surface.poids->w) / 2;
                 SDL_BlitSurface(musique.surface.poids, NULL, ecran, &musique.position.poids);
                 musique.position.format.x = (largeurZoneReglage - musique.surface.format->w) / 2;
                 SDL_BlitSurface(musique.surface.format, NULL, ecran, &musique.position.format);
                 SDL_BlitSurface(musique.surface.curseur, NULL, ecran, &musique.position.curseur);
                 if (numeroVisu != 2 && numeroVisu != NOMBRE_VISUS)
                    SDL_BlitSurface(ligneSeparation, NULL, ecran, &positionLigneSeparation);
                 SDL_BlitSurface(surfaceIndicStereoMono, NULL, ecran, &positionIndicStereoMono);
                 SDL_BlitSurface(ligneVolume, NULL, ecran, &positionLigneVolumeDroit);
                 SDL_BlitSurface(ligneVolume, NULL, ecran, &positionLigneVolumeGauche);
                 SDL_BlitSurface(boutonVolume, NULL, ecran, &positionBoutonVolumeDroit);
                 SDL_BlitSurface(boutonVolume, NULL, ecran, &positionBoutonVolumeGauche);
                 SDL_BlitSurface(ligneSeparation3, NULL, ecran, &positionLigneSeparation3);
                 if (nombreMusiquesAffichables < nombreMusiques)
                    SDL_BlitSurface(barreDefilement, NULL, ecran, &positionBarreDefilement);
                 if (modeListe == 0)
                    afficher_fichier_texte(nomFichierPlaylist, policePlaylist, couleurBlanc, couleurBlanc, 2, 5, largeurZoneListe - 35, ligneDebut, ligneFin, positionPlaylist, ecran, tableauSurfacesPlaylist, tableauPositionsPlaylist, tableauTextesPlaylist);
                 else afficher_fichier_texte(nomFichierPlaylistDePlaylists, policePlaylist, couleurBlanc, couleurBlanc, 2, 5, largeurZoneListe - 35, ligneDebut, ligneFin, positionPlaylist, ecran, tableauSurfacesPlaylist, tableauPositionsPlaylist, tableauTextesPlaylist);

                 for (i = ligneDebut + 1 ; i <= ligneFin && modeListe == 0; i++)
                 {
                     if (tableauTotalMusiques[i] == -1)
                     {
                        if (tableauSurfacesPlaylist[i - 1] != NULL)
                           SDL_FreeSurface(tableauSurfacesPlaylist[i - 1]);
                        tableauSurfacesPlaylist[i - 1] = TTF_RenderText_Blended(policePlaylist, tableauTextesPlaylist[i - 1], couleurRouge);
                        SDL_BlitSurface(tableauSurfacesPlaylist[i - 1], NULL, ecran, &tableauPositionsPlaylist[i - 1]);
                     }
                 }

                 if (modeListe == 0 && musique.numero > ligneDebut && musique.numero <= ligneFin && musique.mode != ERREUR)
                 {
                                  char chaine[TAILLE_MAX_NOM];
                                  char *positionTiret = NULL;

                                  strcpy(chaine, musique.nom);
                                  if (strchr(chaine, '.') != NULL)
                                     *(strchr(chaine, '.')) = '\0';

                                  switch(typePlaylist)
                                  {
                                              case TITRE:
                                                   strcpy(chaine, musique.titre);
                                                   break;
                                              case AUTEUR:
                                                   strcpy(chaine, musique.auteur);
                                                   break;
                                              case NOM_SIMPLIFIE:
                                                   if ((positionTiret = strstr(chaine, " - ")) != NULL)
                                                      strcpy(chaine, positionTiret + 3);
                                                   break;
                                              case TITRE_ET_AUTEUR:
                                                   sprintf(chaine, "%s - %s", musique.titre, musique.auteur);
                                                   break;
                                            default:
                                                break;
                                  }

                                  char chaine2[TAILLE_MAX_NOM];
                                  strcpy(chaine2, tableauTextesPlaylist[musique.numero - 1]);
                                  if (strchr(chaine2, '.') != NULL)
                                     *(strchr(chaine2, '.')) = '\0';

                                  if (strstr(chaine, chaine2) != NULL)
                                  {
                                      SDL_FreeSurface(tableauSurfacesPlaylist[musique.numero - 1]);
                                      tableauSurfacesPlaylist[musique.numero - 1] = TTF_RenderText_Blended(policePlaylist, tableauTextesPlaylist[musique.numero - 1], couleurVert);
                                      SDL_BlitSurface(tableauSurfacesPlaylist[musique.numero - 1], NULL, ecran, &tableauPositionsPlaylist[musique.numero - 1]);
                                  }
                 }

                 SDL_BlitSurface(boutonModeListe, NULL, ecran, &positionBoutonModeListe);
                 SDL_BlitSurface(boutonAlpha, NULL, ecran, &positionBoutonAlpha);
                 SDL_BlitSurface(boutonTypePlaylist, NULL, ecran, &positionBoutonTypePlaylist);

                 SDL_BlitSurface(surfaceZonePlayPause, NULL, ecran, &positionZonePlayPause);
                 SDL_BlitSurface(musique.surface.titre, NULL, ecran, &musique.position.titre);
                 SDL_BlitSurface(musique.surface.auteur, NULL, ecran, &musique.position.auteur);
                 SDL_BlitSurface(musique.surface.album, NULL, ecran, &musique.position.album);

                 if (musique.mode == ERREUR)
                    SDL_BlitSurface(imageAlbumDefautErreur, NULL, ecran, &musique.position.image);
                 else if (musique.surface.image != NULL)
                      SDL_BlitSurface(musique.surface.image, NULL, ecran, &musique.position.image);
                 else if (musique.type == NORMAL)
                      SDL_BlitSurface(imageAlbumDefautMusique, NULL, ecran, &musique.position.image);
                 else if (musique.type == VIDEO)
                      SDL_BlitSurface(imageAlbumDefautVideo, NULL, ecran, &musique.position.image);
                 else if (musique.type == INTERNET)
                      SDL_BlitSurface(imageAlbumDefautInternet, NULL, ecran, &musique.position.image);
                 else if (musique.type == RADIO)
                      SDL_BlitSurface(imageAlbumDefautRadio, NULL, ecran, &musique.position.image);


                 SDL_BlitSurface(ligneSeparation2, NULL, ecran, &positionLigneSeparation2);
                 SDL_BlitSurface(surfaceBoutonPlayPause, NULL, ecran, &positionBoutonPlayPause);
                 if (musique.type != RADIO)
                    SDL_BlitSurface(flecheLecture, NULL, ecran, &positionFlecheLecture);
                 if (positionBoutonReculRapide.x > 0)
                    SDL_BlitSurface(boutonAvanceRapide, NULL, ecran, &positionBoutonAvanceRapide);
                 if (positionBoutonReculRapide.x > 0)
                    SDL_BlitSurface(boutonReculRapide, NULL, ecran, &positionBoutonReculRapide);
                 SDL_BlitSurface(boutonAleatoire, NULL, ecran, &positionBoutonAleatoire);
                 if (musique.type != RADIO)
                 {
                         SDL_BlitSurface(boutonBoucle, NULL, ecran, &positionBoutonBoucle);
                         SDL_BlitSurface(boutonAB, NULL, ecran, &positionBoutonAB);
                         if (lettreA != NULL)
                            SDL_BlitSurface(lettreA, NULL, ecran, &positionLettreA);
                         if (lettreB != NULL)
                            SDL_BlitSurface(lettreB, NULL, ecran, &positionLettreB);
                 }
                 else
                 {
                     SDL_BlitSurface(boutonActualiser, NULL, ecran, &positionBoutonActualiser);
                     SDL_BlitSurface(boutonRecord, NULL, ecran, &positionBoutonRecord);
                 }

                 SDL_Rect positionListe;
                 positionListe.x = 0;
                 positionListe.y = 0;
                 //afficher_fichier_texte(nomFichierListe, policeNomMusique, couleurBlanc, 0, 2, 5, positionListe, ecran);
             }
             else
             {
                 SDL_BlitSurface(ligneTailleVideo, NULL, ecran, &positionLigneTailleVideo);
                 SDL_BlitSurface(boutonTailleVideo, NULL, ecran, &positionBoutonTailleVideo);
                 SDL_BlitSurface(boutonTailleVideoMoins, NULL, ecran, &positionBoutonTailleVideoMoins);
                 SDL_BlitSurface(boutonTailleVideoPlus, NULL, ecran, &positionBoutonTailleVideoPlus);
                 SDL_BlitSurface(boutonModePetitEcran, NULL, ecran, &positionBoutonModePetitEcran);
             }

             tempsActuel = SDL_GetTicks();
             if (tempsActuel - tempsPrecedent < tempsRafraichissement)
             {
                SDL_Delay(tempsRafraichissement - (tempsActuel - tempsPrecedent));
             }

             sprintf(texteTempsRafraichissement, "%d ms", SDL_GetTicks() - tempsPrecedent);
             SDL_FreeSurface(surfaceTempsRafraichissement);
             surfaceTempsRafraichissement = TTF_RenderText_Solid(policeTempsRafraichissement, texteTempsRafraichissement, couleurBlanc);

             tempsPrecedent = SDL_GetTicks();
    }


    remove("playlists\\Originelle.list");
    remove("playlists\\Originelle.npl");

    if ((saveFile = fopen("saveFile.sf", "w+")) == NULL)
       fprintf(stderr, "ERR main %d : Le fichier de sauvegarde n'a pas pu etre ecrit.\n", __LINE__);
    else
    {
            fprintf(saveFile, "[parameters]\n");
            for (i = 0 ; strcmp(configMenu.tableauTextes[i], "(none)") != 0 && i < 100 ; i++)
            {
                fprintf(saveFile, "%s\n", configMenu.tableauTextesReponses[i]);
            }
            fprintf(saveFile, "[over]\n\n\n");
    }

    if (strcmp(nomFichierListe, "playlists\\Originelle.list") != 0 && configMenu.parametre.sauvegarde)
    {
            if (musique.numero > TAILLE_MAX_PLAYLIST)
            {
               musique.numero = -1;
               musique.curseur = 0;
            }
            fprintf(saveFile, "[saved]\n%d\n%d\n%s\n%s\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n%s", volumeDroit, volumeGauche, nomFichierListe, nomFichierPlaylist, musique.numero, musique.curseur, indicStereoMono, lockIndic, modeAleatoire, modeBoucle, numeroVisu, editMenu.repertoireVirtuel);
    }

    if (saveFile != NULL)
       while(fclose(saveFile) == EOF);

    if (modeRecord)
    {
                FSOUND_Record_Stop();
                if (test_exist(nomFichierRecord) == 0)
                   sauver_en_wav(enregistrement, nomFichierRecord, SDL_GetTicks() - tempsDernierRecord + 500);
                else ajouter_wav(enregistrement, nomFichierRecord, SDL_GetTicks() - tempsDernierRecord + 500);
                FSOUND_Sample_Free(enregistrement);
    }

    SDL_Quit();
    TTF_Quit();
    FSOUND_DSP_SetActive(FSOUND_DSP_GetFFTUnit(), 0);
    FSOUND_Close();
    return EXIT_SUCCESS;
}



/*fonction gentiment donnée par Mateo21... ne pas chercher à comprendre ^^
juste envoyer : la surface à modifier (bloquée), les coordonnées x et y du pixel à modifier et sa couleur.*/
void setPixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;

    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}


void setPixel2(SDL_Surface *surface, int x, int y, Uint32 couleurPixel)
{
     SDL_Surface *surfacePixel = NULL;
     surfacePixel = SDL_CreateRGBSurface (SDL_HWSURFACE, 1, 1, 32, 0, 0, 0, 0);
     SDL_FillRect (surfacePixel, NULL, couleurPixel);
     SDL_Rect positionPixel;
     positionPixel.x = x;
     positionPixel.y = y;
     SDL_BlitSurface (surfacePixel, NULL, surface, &positionPixel);
     SDL_FreeSurface (surfacePixel);
}


void setRectangle(SDL_Surface *surface, int x, int y, Uint32 couleurRectangle)
{
     SDL_Surface *surfaceRectangle = NULL;
     surfaceRectangle = SDL_CreateRGBSurface (SDL_HWSURFACE, 50, 30, 32, 0, 0, 0, 0);
     SDL_FillRect (surfaceRectangle, NULL, couleurRectangle);
     SDL_Rect positionRectangle;
     positionRectangle.x = x;
     positionRectangle.y = y;
     SDL_BlitSurface (surfaceRectangle, NULL, surface, &positionRectangle);
     SDL_FreeSurface (surfaceRectangle);
}



int test_quit (SDL_Event event, int *alt)
{
    int continuer = 1;
    switch (event.type)
           {
                  case SDL_QUIT:
                       continuer = 0;
                       break;
                  case SDL_KEYUP:
                       if (event.key.keysym.sym == SDLK_LALT)
                          *alt = 0;
                       break;
                  case SDL_KEYDOWN:
                       switch(event.key.keysym.sym)
                       {
                            case SDLK_ESCAPE:
                                 continuer = 0;
                                 break;
                            case SDLK_LALT:
                                 *alt = 1;
                                 break;
                            case SDLK_F4:
                                 if (*alt)
                                    continuer = 0;
                                 break;
                            default:
                                break;
                       }
                       break;
           }
     return continuer;
}



void charger_musique(Musique *musique, MenuConfigLecteur configMenu, AffichageParoles *affichageParoles, char nomFichierListe[], int numeroLigne, int cutPosition, int *numeroChargement)
{
                     if (musique->mode == ERREUR)
                        musique->mode = PLAY;

                     TTF_Font *policeNomMusique = NULL;
                     policeNomMusique = TTF_OpenFont ("fonts\\sdigit.ttf", 15);
                     TTF_Font *policeNomAuteur = NULL;
                     policeNomAuteur = TTF_OpenFont ("fonts\\sdigit.ttf", 15);
                     TTF_Font *policeNomAlbum = NULL;
                     policeNomAlbum = TTF_OpenFont ("fonts\\sdigit.ttf", 15);
                     SDL_Color couleurBlanc = {255, 255, 255};

                     TTF_Font *policeIndicMonoStereo = NULL;
                     policeIndicMonoStereo = TTF_OpenFont("fonts\\sdigit.ttf", 20);
                     SDL_Color couleurJaune = {255, 255, 0};

                     TTF_Font *policePoidsFichier = NULL;
                     policePoidsFichier = TTF_OpenFont ("fonts\\sdigit.ttf", 20);
                     SDL_Color couleurBleuClair = {0, 100, 255};

                     TTF_Font *policeLongueurPiste = NULL;
                     policeLongueurPiste = TTF_OpenFont("fonts\\sdigit.ttf", 20);
                     SDL_Color couleurRose = {255, 0, 100};

                     TTF_Font *policeFrequenceCanal = NULL;
                     policeFrequenceCanal = TTF_OpenFont("fonts\\sdigit.ttf", 20);
                     SDL_Color couleurVert = {0, 255, 0};

                     TTF_Font *policeFormat = NULL;
                     policeFormat = TTF_OpenFont("fonts\\sdigit.ttf", 20);

                     SDL_Rect positionInit;
                     positionInit.x = 0;
                     positionInit.y = 0;

                     char temp[TAILLE_MAX_NOM] = " ";

                     SDL_Surface *efface = NULL;
                     efface = SDL_CreateRGBSurface (SDL_HWSURFACE, cutPosition, musique->surface.titre->h, 32, 0, 0, 0, 0);
                     SDL_FillRect(efface, NULL, SDL_MapRGB(efface->format, 0, 0, 0));


                     if (musique->surface.titre != NULL)
                     {
                        if (numeroChargement != NULL)
                           sprintf(temp, "Loading... (%d)", *numeroChargement);
                        else strcpy(temp, "Loading... (0)");
                        couper_texte(temp, cutPosition, policeNomMusique);
                        SDL_BlitSurface(efface, NULL, musique->surface.blit, &musique->position.titre);
                        SDL_FreeSurface(musique->surface.titre);
                        musique->surface.titre = TTF_RenderText_Blended(policeNomMusique, temp, couleurBlanc);
                        SDL_BlitSurface(musique->surface.titre, NULL, musique->surface.blit, &musique->position.titre);
                        SDL_Flip(musique->surface.blit);
                     }

                     if (musique->surface.auteur != NULL)
                     {
                        strcpy(temp, "Please wait");
                        couper_texte(temp, cutPosition, policeNomMusique);
                        SDL_BlitSurface(efface, NULL, musique->surface.blit, &musique->position.auteur);
                        SDL_FreeSurface(musique->surface.auteur);
                        musique->surface.auteur = TTF_RenderText_Blended(policeNomMusique, temp, couleurBlanc);
                        SDL_BlitSurface(musique->surface.auteur, NULL, musique->surface.blit, &musique->position.auteur);
                        SDL_Flip(musique->surface.blit);
                     }


                     char textePoidsFichier[10] = "0.00 Mo ", texteLongueurPiste[10] = "00:00", texteFrequenceCanal[10] = "00.0 kHz";
                     int secondesPiste = 0, minutesPiste = 0;

                     FILE *fichierListe = NULL;
                     char nomNouvelleMusique[TAILLE_MAX_NOM] = "";
                     int randomLoad = 0;
                     int i = 0;

                     musique->pointeurStereo = NULL;
                     musique->pointeurMono = NULL;


                     if ((fichierListe = fopen(nomFichierListe, "r")) == NULL)
                     {
                                       fprintf(stderr, "ERR charger_musique %d : Impossible d'ouvrir le fichier %s.\n", __LINE__, nomFichierListe);
                                       musique->mode = ERREUR;
                                       strcpy(musique->titre, "Opening Error");
                                       sprintf(musique->auteur, "ERR charger_musique %d : Impossible d'ouvrir le fichier %s", __LINE__, nomFichierListe);
                     }
                     else
                     {

                         while (fgets(nomNouvelleMusique, TAILLE_MAX_NOM, fichierListe) != NULL)
                               i++;

                         if (i < numeroLigne)
                            numeroLigne = -1;

                         rewind(fichierListe);
                         if (numeroLigne == -1)
                         {
                                  numeroLigne = (rand() % i) + 1;
                                  randomLoad = 1;
                         }

                         for (i = 1 ; i <= numeroLigne ; i++)
                             fgets(nomNouvelleMusique, TAILLE_MAX_NOM, fichierListe);

                         char *positionEntree = NULL;
                         if ((positionEntree = strchr(nomNouvelleMusique, '\n')) != NULL)
                                  *positionEntree = '\0';

                         strcpy(musique->adresse, nomNouvelleMusique);

                         char extension[20] = "\0";
                         if (strchr(musique->adresse, '.') != NULL)
                            strcpy(extension, strrchr(musique->adresse, '.') + 1);
                         for (i = 0 ; extension[i] != '\0' ; i++)
                             extension[i] = toupper(extension[i]);

                         if (strcmp(extension, "PLS") == 0 || strcmp(extension, "URL") == 0)
                         {
                                  FILE *fichierMusique = NULL;
                                  if ((fichierMusique = fopen(musique->adresse, "r")) == NULL)
                                  {
                                       fprintf(stderr, "ERR charger_musique %d : Impossible d'ouvrir le fichier %s.\n", __LINE__, musique->adresse);
                                       musique->mode = ERREUR;
                                       strcpy(musique->titre, "Opening Error");
                                       sprintf(musique->auteur, "Impossible d'ouvrir le fichier %s", musique->adresse);
                                  }
                                  else
                                  {
                                      char *positionAdresse = NULL;
                                      char chaine[TAILLE_MAX_NOM] = " ";
                                      do
                                      {
                                           fgets(chaine, TAILLE_MAX_NOM, fichierMusique);
                                      } while ((positionAdresse = strstr(chaine, "http")) == NULL && (positionAdresse = strstr(chaine, "http")) == NULL);
                                      char *positionEntree = NULL;
                                      if ((positionEntree = strchr(chaine, '\n')) != NULL)
                                         *positionEntree = '\0';
                                      strcpy(musique->nom, musique->adresse);
                                      strcpy(musique->adresse, positionAdresse);
                                      strcpy(musique->auteur, musique->adresse);
                                      while(fclose(fichierMusique) == EOF);
                                  }
                         }
                         else if (strcmp(extension, "ASX") == 0)
                         {
                                  FILE *fichierMusique = NULL;
                                  if ((fichierMusique = fopen(musique->adresse, "r")) == NULL)
                                  {
                                       fprintf(stderr, "ERR charger_musique %d : Impossible d'ouvrir le fichier %s.\n", __LINE__, musique->adresse);
                                       musique->mode = ERREUR;
                                       strcpy(musique->titre, "Opening Error");
                                       sprintf(musique->auteur, "Impossible d'ouvrir le fichier %s", musique->adresse);
                                  }
                                  else
                                  {
                                      char *positionAdresse = NULL;
                                      char chaine[TAILLE_MAX_NOM] = " ";
                                      do
                                      {
                                           fgets(chaine, TAILLE_MAX_NOM, fichierMusique);
                                      } while ((positionAdresse = strstr(chaine, "http")) == NULL && (positionAdresse = strstr(chaine, "mms:")) == NULL);
                                      char *positionEntree = NULL;
                                      if ((positionEntree = strchr(chaine, '\n')) != NULL)
                                         *positionEntree = '\0';
                                      strcpy(musique->nom, musique->adresse);
                                      strcpy(musique->adresse, positionAdresse);
                                      char *positionGuillemet = NULL;
                                      if ((positionGuillemet = strrchr(musique->adresse, '"')) != NULL);
                                         *positionGuillemet = '\0';
                                      strcpy(musique->auteur, musique->adresse);
                                      while(fclose(fichierMusique) == EOF);
                                  }
                         }

                         while(fclose(fichierListe) == EOF);
                     }

                     if (strstr(nomFichierListe, "playlists\\Radios.list") != NULL)
                            musique->type = RADIO;
                     else if (strstr(musique->adresse, "http:") != NULL || strstr(musique->adresse, "mms:") != NULL)
                              musique->type = INTERNET;
                     else musique->type = NORMAL;

                     char extension[20] = "\0";
                     if (strchr(musique->adresse, '.') != NULL)
                        strcpy(extension, strrchr(musique->adresse, '.') + 1);
                     for (i = 0 ; extension[i] != '\0' ; i++)
                         extension[i] = toupper(extension[i]);

                     if (musique->mode != ERREUR)
                     {

                         if (musique->surface.album != NULL)
                         {
                             sprintf(temp, "%s...", musique->adresse);
                             couper_texte(temp, cutPosition, policeNomMusique);
                             SDL_BlitSurface(efface, NULL, musique->surface.blit, &musique->position.album);
                             SDL_FreeSurface(musique->surface.album);
                             musique->surface.album = TTF_RenderText_Blended(policeNomMusique, temp, couleurBlanc);
                             SDL_BlitSurface(musique->surface.album, NULL, musique->surface.blit, &musique->position.album);
                             SDL_Flip(musique->surface.blit);
                         }

                         SDL_Event event;


                         if (strcmp(extension, "MPG") == 0)
                         {
                            FSOUND_DSP_SetActive(FSOUND_DSP_GetFFTUnit(), 0);

                            musique->type = VIDEO;
                            SMPEG_Info SInfo;
                            musique->pointeurStereo = (FSOUND_STREAM*) SMPEG_new(musique->adresse, &SInfo, 1);
                            musique->pointeurMono = musique->pointeurStereo;

                            if (SMPEG_error((SMPEG*) musique->pointeurStereo))
                            {
                                fprintf(stderr, "ERR charger_musique %d : Le chargement de %s (video) a echoue.\n", __LINE__, musique->adresse);
                                musique->mode = ERREUR;
                                strcpy(musique->titre, "Loading Error");
                                strcpy(musique->auteur, musique->adresse);
                                strcpy(musique->album, "Erreur inconnue.");
                            }
                            else
                            {
                                musique->longueur = SInfo.total_time * 1000;
                                musique->poids = SInfo.total_size * 1.0 / pow(2, 20);
                                secondsSkipped = 0;

                                resize_video((SMPEG*) musique->pointeurStereo);

                                if (surfaceSMPEG == NULL)
                                {
                                    fprintf(stderr, "ERR charger_musique %d : Impossible d'allouer la surface vidéo.\n", __LINE__);
                                    musique->mode = ERREUR;
                                    strcpy(musique->titre, "Playing Error");
                                    strcpy(musique->auteur, musique->adresse);
                                    strcpy(musique->album, "Echec de l'allocation de mémoire.");
                                }
                                else
                                    SMPEG_setdisplay((SMPEG*) musique->pointeurStereo, surfaceSMPEG, NULL, (SMPEG_DisplayCallback) display_callback);
                            }
                         }
                         else
                         {
                             FSOUND_DSP_SetActive(FSOUND_DSP_GetFFTUnit(), 1);

                             if (configMenu.parametre.chargementNonBlocking)
                             {
                                 if (strcmp(extension, "MP3") == 0 && configMenu.parametre.chargementAccurate)
                                        musique->pointeurStereo = FSOUND_Stream_Open(musique->adresse, FSOUND_LOOP_NORMAL | FSOUND_NONBLOCKING | FSOUND_MPEGACCURATE, 0, 0);
                                 else musique->pointeurStereo = FSOUND_Stream_Open(musique->adresse, FSOUND_LOOP_NORMAL | FSOUND_NONBLOCKING, 0, 0);
                             }
                             else
                             {
                                 if (strcmp(extension, "MP3") == 0 && configMenu.parametre.chargementAccurate)
                                        musique->pointeurStereo = FSOUND_Stream_Open(musique->adresse, FSOUND_LOOP_NORMAL | FSOUND_MPEGACCURATE, 0, 0);
                                 else musique->pointeurStereo = FSOUND_Stream_Open(musique->adresse, FSOUND_LOOP_NORMAL, 0, 0);
                             }

                             for (i = SDL_GetTicks() ; FSOUND_Stream_GetOpenState(musique->pointeurStereo) != 0 && FSOUND_Stream_GetOpenState(musique->pointeurStereo) != -3 && SDL_GetTicks() - i < configMenu.parametre.dureeMaxChargement ;)
                             {
                                 SDL_PollEvent (&event);
                                 if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE)
                                 {
                                     fprintf(stdout, "INF charger_musique %d : Le chargement de %s (stereo) a été interrompu.\n", __LINE__, musique->adresse);
                                     musique->mode = ERREUR;
                                     strcpy(musique->titre, "Loading Error");
                                     strcpy(musique->auteur, musique->adresse);
                                     stocker_erreur(&musique->erreur, musique->album);
                                     if (numeroChargement != NULL)
                                        *numeroChargement = configMenu.parametre.nombreChargementsMaxi;
                                     return;
                                 }
                             }

                             if (musique->pointeurStereo == NULL || FSOUND_Stream_GetOpenState(musique->pointeurStereo) != 0)
                             {
                                fprintf(stderr, "ERR charger_musique %d : Le chargement de %s (stereo) a echoue.\n", __LINE__, musique->adresse);
                                musique->mode = ERREUR;
                                strcpy(musique->titre, "Loading Error");
                                strcpy(musique->auteur, musique->adresse);
                                stocker_erreur(&musique->erreur, musique->album);
                             }

                             if (musique->mode != ERREUR)
                             {
                                 if (configMenu.parametre.chargementNonBlocking)
                                 {
                                     if ((strstr(musique->adresse, ".mp3") != NULL || strstr(musique->adresse, ".MP3") != NULL) && configMenu.parametre.chargementAccurate)
                                        musique->pointeurMono = FSOUND_Stream_Open(musique->adresse, FSOUND_LOOP_NORMAL | FSOUND_FORCEMONO | FSOUND_NONBLOCKING | FSOUND_MPEGACCURATE, 0, 0);
                                     else musique->pointeurMono = FSOUND_Stream_Open(musique->adresse, FSOUND_LOOP_NORMAL | FSOUND_FORCEMONO | FSOUND_NONBLOCKING, 0, 0);
                                 }
                                 else
                                 {
                                     if ((strstr(musique->adresse, ".mp3") != NULL || strstr(musique->adresse, ".MP3") != NULL) && configMenu.parametre.chargementAccurate)
                                        musique->pointeurMono = FSOUND_Stream_Open(musique->adresse, FSOUND_LOOP_NORMAL | FSOUND_FORCEMONO | FSOUND_MPEGACCURATE, 0, 0);
                                     else musique->pointeurMono = FSOUND_Stream_Open(musique->adresse, FSOUND_LOOP_NORMAL | FSOUND_FORCEMONO, 0, 0);
                                 }

                                 for (i = SDL_GetTicks() ; FSOUND_Stream_GetOpenState(musique->pointeurMono) != 0 && FSOUND_Stream_GetOpenState(musique->pointeurMono) != -3 && SDL_GetTicks() - i < configMenu.parametre.dureeMaxChargement ;)
                                 {
                                     SDL_PollEvent (&event);
                                     if (event.type == SDL_KEYUP&& event.key.keysym.sym == SDLK_ESCAPE)
                                     {
                                         fprintf(stdout, "INF charger_musique %d : Le chargement de %s (mono) a été interrompu.\n", __LINE__, musique->adresse);
                                         musique->mode = ERREUR;
                                         strcpy(musique->titre, "Loading Error");
                                         strcpy(musique->auteur, musique->adresse);
                                         stocker_erreur(&musique->erreur, musique->album);
                                         if (numeroChargement != NULL)
                                            *numeroChargement = configMenu.parametre.nombreChargementsMaxi;
                                         return;
                                     }
                                 }
                                 if (musique->pointeurMono == NULL || FSOUND_Stream_GetOpenState(musique->pointeurMono) != 0)
                                 {
                                    fprintf(stderr, "ERR charger_musique %d : Le chargement de %s (mono) a echoue.\n", __LINE__, musique->adresse);
                                    musique->mode = ERREUR;
                                    strcpy(musique->titre, "Loading Error");
                                    strcpy(musique->auteur, musique->adresse);
                                    stocker_erreur(&musique->erreur, musique->album);
                                 }
                             }
                         }
                     }


                     if (musique->mode != ERREUR && musique->type != VIDEO)
                     {
                         musique->longueur = FSOUND_Stream_GetLengthMs(musique->pointeurStereo);
                         musique->poids = FSOUND_Stream_GetLength(musique->pointeurStereo) / pow(2, 20);

                     }
                     else if (musique->type != VIDEO)
                     {
                         musique->longueur = 1;
                         musique->poids = 1.0;
                     }

                     sprintf(textePoidsFichier, "%lf", musique->poids);
                     char *positionPoint = strchr(textePoidsFichier, '.');
                     if (positionPoint != NULL)
                     {
                        char *positionFin = positionPoint + 3;
                        if (*positionFin >= '5' && *(positionFin - 1) != '9')
                           *(positionFin - 1) = *(positionFin - 1) + 1;
                        *positionFin = '\0';
                     }

                     sprintf(textePoidsFichier, "%s Mo", textePoidsFichier);
                     if (musique->type == RADIO || musique->mode == ERREUR)
                        strcpy(textePoidsFichier, "--- Mo");
                     if (musique->surface.poids != NULL)
                        SDL_FreeSurface(musique->surface.poids);
                     musique->surface.poids = TTF_RenderText_Blended(policePoidsFichier, textePoidsFichier, couleurBleuClair);


                     diviser_temps_et_stocker(musique->longueur, NULL, &secondesPiste, &minutesPiste, NULL, texteLongueurPiste);
                     if (musique->type == RADIO || musique->mode == ERREUR)
                        strcpy(texteLongueurPiste, "--:--");
                     if (musique->surface.longueur != NULL)
                        SDL_FreeSurface(musique->surface.longueur);
                     musique->surface.longueur = TTF_RenderText_Blended(policeLongueurPiste, texteLongueurPiste, couleurRose);



                     musique->frequence = FSOUND_GetFrequency(1) / 1000.0;
                     sprintf(texteFrequenceCanal, "%lf", musique->frequence);
                     positionPoint = strchr(texteFrequenceCanal, '.');
                     if (positionPoint != NULL)
                     {
                        char *positionFin = positionPoint + 2;
                        if (*positionFin >= '5' && *positionFin != '9')
                           *(positionFin - 1) = *(positionFin - 1) + 1;
                        *positionFin = '\0';
                     }

                     sprintf(texteFrequenceCanal, "%s kHz", texteFrequenceCanal);
                     if (musique->surface.frequence != NULL)
                        SDL_FreeSurface(musique->surface.frequence);
                     musique->surface.frequence = TTF_RenderText_Blended(policeFrequenceCanal, texteFrequenceCanal, couleurVert);



                     if (musique->mode != ERREUR)
                     {
                         if (musique->type == NORMAL || musique->type == VIDEO)
                            strcpy(musique->nom, musique->adresse);
                         char *positionAntiSlash = NULL;
                         if ((positionAntiSlash = strrchr(musique->nom, '\\')) != NULL)
                            strcpy(musique->nom, positionAntiSlash + 1);

                         recuperer_tag(musique, 1, 1);
                     }

                     strcpy(temp, musique->titre);
                     if (musique->surface.titre != NULL)
                            SDL_FreeSurface(musique->surface.titre);
                     couper_texte(temp, cutPosition, policeNomMusique);
                     musique->surface.titre = TTF_RenderText_Blended(policeNomMusique, temp, couleurBlanc);

                     strcpy(temp, musique->auteur);
                     if (musique->surface.auteur != NULL)
                            SDL_FreeSurface(musique->surface.auteur);
                     couper_texte(temp, cutPosition, policeNomMusique);
                     musique->surface.auteur = TTF_RenderText_Blended(policeNomAuteur, temp, couleurBlanc);

                     strcpy(temp, musique->album);
                     if (musique->surface.album != NULL)
                            SDL_FreeSurface(musique->surface.album);
                     couper_texte(temp, cutPosition, policeNomMusique);
                     musique->surface.album = TTF_RenderText_Blended(policeNomAlbum, temp, couleurBlanc);


                     if (musique->type == NORMAL || musique->type == VIDEO)
                        strcpy(musique->format, extension);
                     else strcpy(musique->format, "...");

                     if (musique->mode == ERREUR)
                        strcpy(musique->format, "###");

                     if (musique->surface.format != NULL)
                        SDL_FreeSurface(musique->surface.format);
                     musique->surface.format = TTF_RenderText_Blended(policeFormat, musique->format, couleurJaune);


                     if (numeroLigne == -1)
                            musique->numero = 1;
                     else musique->numero = numeroLigne;

                     actualiser_paroles (affichageParoles, musique);

                     TTF_CloseFont(policeNomMusique);
                     TTF_CloseFont(policeNomAuteur);
                     TTF_CloseFont(policeNomAlbum);
                     TTF_CloseFont (policeIndicMonoStereo);
                     TTF_CloseFont (policePoidsFichier);
                     TTF_CloseFont (policeLongueurPiste);
                     TTF_CloseFont (policeFrequenceCanal);
                     TTF_CloseFont (policeFormat);

                     SDL_FreeSurface(efface);

                     return;
}


int creer_playlist(char nomFichierListe[], char nomFichierSauvegarde2[], TypePlaylist type)
{
    char *ligneLecture = NULL;
    ligneLecture = malloc(sizeof(char) * (TAILLE_MAX_NOM * 2 + 4));

    char *nomFichierSauvegarde = NULL;
    nomFichierSauvegarde = malloc(sizeof(char) * (strlen(nomFichierSauvegarde2) + 1));
    strcpy(nomFichierSauvegarde, nomFichierSauvegarde2);

    if (strcmp(nomFichierSauvegarde, nomFichierListe) == 0)
    {
       if (strrchr(nomFichierSauvegarde, '\\') != NULL)
          strcpy(strrchr(nomFichierSauvegarde, '\\') + 1, "creer_playlist_temp.tmp");
       else strcpy(nomFichierSauvegarde, "creer_playlist_temp.tmp");
    }


    FILE *fichierSauvegarde = NULL;
    if ((fichierSauvegarde = fopen(nomFichierSauvegarde, "w")) == NULL)
    {
                      fprintf(stderr, "ERR creer_playlist %d : Le fichier %s n'a pas pu etre ouvert.\n", __LINE__, nomFichierSauvegarde);
                      free(ligneLecture);
                      free(nomFichierSauvegarde);
                      return 0;
    }


    FILE *fichierListe = NULL;

    if (test_vide(nomFichierListe) == 0)
    {
                                fprintf(stdout, "INF creer_playlist %d : La liste %s est vide.\n", __LINE__, nomFichierListe);
                                while(fclose(fichierSauvegarde) == EOF);
                                free(ligneLecture);
                                free(nomFichierSauvegarde);
                                return 0;
    }

    if ((fichierListe = fopen(nomFichierListe, "r")) == NULL)
    {
                      fprintf(stderr, "ERR creer_playlist %d : Le fichier %s n'a pas pu etre ouvert.\n", __LINE__, nomFichierListe);
                      while(fclose(fichierSauvegarde) == EOF);
                      free(ligneLecture);
                      free(nomFichierSauvegarde);
                      return 0;
    }


    char *positionTiret = NULL;
    char *positionAntiSlash = NULL;
    char *positionExtension = NULL;
    char *positionEntree = NULL;

    Musique musique;
    musique.type = NORMAL;

    while (fgets(ligneLecture, TAILLE_MAX_NOM * 2 + 3, fichierListe) != NULL)
    {
          if ((positionEntree = strrchr(ligneLecture, '\n')) != NULL)
             *positionEntree = '\0';

          if (ligneLecture[0] != '\0')
          {
                              strcpy(musique.adresse, ligneLecture);

                              if ((positionAntiSlash = strrchr(ligneLecture, '\\')) != NULL)
                                 strcpy(ligneLecture, positionAntiSlash + 1);
                              if ((positionExtension = strrchr(ligneLecture, '.')) != NULL)
                                 *positionExtension = '\0';
                              strcpy(musique.nom, ligneLecture);

                              if (type != NOM)
                                 recuperer_tag(&musique, 0, 0);

                              switch(type)
                              {
                                  case TITRE:
                                       strcpy(ligneLecture, musique.titre);
                                       break;
                                  case AUTEUR:
                                       strcpy(ligneLecture, musique.auteur);
                                       break;
                                  case NOM_SIMPLIFIE:
                                       if ((positionTiret = strstr(ligneLecture, " - ")) != NULL)
                                          strcpy(ligneLecture, positionTiret + 3);
                                       break;
                                  case TITRE_ET_AUTEUR:
                                       sprintf(ligneLecture, "%s - %s", musique.titre, musique.auteur);
                                       break;
                                  default:
                                        break;
                              }

                              fprintf(fichierSauvegarde, "%s\n", ligneLecture);
          }
    }

    while(fclose(fichierListe) == EOF);
    while(fclose(fichierSauvegarde) == EOF);
    free(ligneLecture);

    if (strstr(nomFichierSauvegarde, "creer_playlist_temp.tmp") != NULL)
    {
                                     remove(nomFichierListe);
                                     rename(nomFichierSauvegarde, nomFichierListe);
    }

    free(nomFichierSauvegarde);

    return 1;
}



void mettre_en_stop(Musique *musique, SDL_Surface *surfaceBoutonPlayPause)
{
    if (musique->mode != ERREUR)
       musique->mode = STOP;
    if (musique->type != VIDEO)
    {
        FSOUND_Stream_Stop(musique->pointeurStereo);
        FSOUND_Stream_Stop(musique->pointeurMono);
    }
    else SMPEG_rewind((SMPEG*) musique->pointeurStereo);
}


void mettre_a_jour_playlists(char nomFichierListePlaylists[], char nomFichierPlaylistDePlaylists[], int tableauTotalMusiques[TAILLE_MAX_PLAYLIST])
{
     lister_extension("musiques", "playlists\\Dossier musiques.list", ".mp3 ; .wma ; .wav ; .aac ; .ogg ; .url ; .mpg", "w");
     creer_playlist("playlists\\Dossier musiques.list", "playlists\\Dossier musiques.npl", NOM);
     classer_alphabetique_fichier_texte("playlists\\Dossier musiques.npl", TAILLE_MAX_NOM, "playlists\\Dossier musiques.list");

     if (lister_extension("radios", "playlists\\Radios.list", ".pls", "w") == 0)
     {
        FILE *fichierNRJ = NULL;
        if ((fichierNRJ = fopen("radios\\100.3 FM - NRJ.pls", "w")) == NULL)
           fprintf(stderr, "ERR mettre_a_jour_playlists %d : Le fichier NRJ.pls n'a pas pu etre créé.\n", __LINE__);
        else
        {
            fprintf(fichierNRJ, "[playlist]\nnumberofentries=1\nFile1=http://80.236.126.2:708\nTitle1=(#1 - 13/100) NRJ\nLength1=-1\nVersion=2\n");
            while(fclose(fichierNRJ) == EOF);
        }
     }

     lister_extension("radios", "playlists\\Radios.list", ".pls ; .asx ; .url", "w");
     creer_playlist("playlists\\Radios.list", "playlists\\Radios.npl", NOM);
     lister_extension("records", "playlists\\Records.list", ".wav", "w");
     creer_playlist("playlists\\Records.list", "playlists\\Records.npl", NOM);
     lister_extension("playlists", nomFichierListePlaylists, ".npl", "w");
     creer_playlist(nomFichierListePlaylists, nomFichierPlaylistDePlaylists, NOM);

     int i;
     for (i = 0 ; i < TAILLE_MAX_PLAYLIST ; i++)
     {
         if (tableauTotalMusiques[i] == -1)
            tableauTotalMusiques[i] = 0;
     }
}


int recuperer_tag(Musique *musique, int prendreImage, int prendreParoles)
{
                      char *positionExtension = NULL;

                      strcpy(musique->titre, musique->nom);
                      if ((positionExtension = strrchr(musique->titre, '.')) != NULL)
                            *positionExtension = '\0';

                      strcpy(musique->paroles, "Aucune parole enregistrée dans le fichier.");

                      if (musique->type == NORMAL)
                      {
                           strcpy(musique->auteur, "Artiste Inconnu");
                           strcpy(musique->album, "Album Inconnu");
                           strcpy(musique->annee, "Année Inconnue");
                      }
                      else if (musique->type == VIDEO)
                      {
                           strcpy(musique->album, musique->adresse);
                           strcpy(musique->auteur, "Video MPEG");
                           strcpy(musique->annee, "Année Inconnue");
                      }
                      else strcpy(musique->album, "Streaming Internet");

                      char extension[20] = "\0";
                      positionExtension = strrchr(musique->adresse, '.');
                      if (positionExtension != NULL)
                         strcpy(extension, positionExtension + 1);

                      /*int i;
                      for (i = 0 ; extension[i] != '\0' ; i++)
                          extension[i] = toupper(extension[i]);

                      if (strcmp(extension, "MP3") != 0)
                         return 0;*/

                      FILE *fichier = NULL;
                      if ((fichier = fopen(musique->adresse, "rb")) == NULL)
                      {
                                    fprintf(stderr, "ERR Fonction recuperer_tag %d : Le fichier %s n'a pas pu etre ouvert.\n", __LINE__, musique->adresse);
                                    return 0;
                      }

                      char identifieur[TAILLE_MAX_NOM] = "\0";
                      fread(identifieur, 1, 3, fichier);
                      identifieur[3] = '\0';
                      if (strcmp(identifieur, "ID3") != 0)
                      {
                          fprintf(stdout, "INF recuperer_tag %d : Le fichier %s ne contient pas de tag valide.\n", __LINE__, musique->adresse);
                          while(fclose(fichier) == EOF);
                          return 0;
                      }

                      int nombreCaracteres = 0, tailleTag = 0;
                      fseek(fichier, 6, SEEK_SET);
                      fread(&tailleTag, sizeof(int), 1, fichier);
                      tailleTag = inverser_nombre(tailleTag);    //le nombre est stocké à l'envers dans le fichier : nécessité de le réinverser

                      char *tag = malloc(tailleTag), *position = NULL;
                      int bitsLus = 0;

                      if (tag != NULL)
                         bitsLus = fread(tag, 1, tailleTag, fichier);
                      else
                      {
                          fprintf(stderr, "ERR recuperer_tag %d : Erreur d'allocation de mémoire.", __LINE__);
                          while(fclose(fichier) == EOF);
                          return 0;
                      }

                      if ((position = strstr2(tag, "TPE1", bitsLus)) != NULL)
                      {
                         CopyMemory(&nombreCaracteres, position + 4, sizeof(int));
                         nombreCaracteres = inverser_nombre(nombreCaracteres);    //le nombre est stocké à l'envers dans le fichier : nécessité de le réinverser
                         if (nombreCaracteres > 1 && nombreCaracteres <= TAILLE_MAX_NOM)
                         {
                            CopyMemory(musique->auteur, position + 7 + sizeof(int), nombreCaracteres - 1);
                            musique->auteur[nombreCaracteres - 1] = '\0';
                         }
                      }

                      if ((position = strstr2(tag, "TIT2", bitsLus)) != NULL)
                      {
                         CopyMemory(&nombreCaracteres, position + 4, sizeof(int));
                         nombreCaracteres = inverser_nombre(nombreCaracteres);
                         if (nombreCaracteres > 1 && nombreCaracteres <= TAILLE_MAX_NOM)
                         {
                            CopyMemory(musique->titre, position + 7 + sizeof(int), nombreCaracteres - 1);
                            musique->titre[nombreCaracteres - 1] = '\0';
                         }
                      }

                      if ((position = strstr2(tag, "TALB", bitsLus)) != NULL)
                      {
                         CopyMemory(&nombreCaracteres, position + 4, sizeof(int));
                         nombreCaracteres = inverser_nombre(nombreCaracteres);
                         if (nombreCaracteres > 1 && nombreCaracteres <= TAILLE_MAX_NOM)
                         {
                            CopyMemory(musique->album, position + 7 + sizeof(int), nombreCaracteres - 1);
                            musique->album[nombreCaracteres - 1] = '\0';
                         }
                      }

                      if ((position = strstr2(tag, "TYER", bitsLus)) != NULL)
                      {
                         CopyMemory(&nombreCaracteres, position + 4, sizeof(int));
                         nombreCaracteres = inverser_nombre(nombreCaracteres);
                         if (nombreCaracteres > 1 && nombreCaracteres <= TAILLE_MAX_NOM)
                         {
                            CopyMemory(musique->annee, position + 7 + sizeof(int), nombreCaracteres - 1);
                            musique->annee[nombreCaracteres - 1] = '\0';
                         }
                      }

                      if (prendreParoles && (position = strstr2(tag, "USLT", bitsLus)) != NULL)
                      {
                         CopyMemory(&nombreCaracteres, position + 4, sizeof(int));
                         nombreCaracteres = inverser_nombre(nombreCaracteres);
                         if (nombreCaracteres > 1 && nombreCaracteres <= TAILLE_MAX_PAROLES)
                         {
                            int i = 2;

                            position += 7 + sizeof(int);
                            while (*position != '\0')
                            {
                                  position++;
                                  i++;
                            }

                            CopyMemory(musique->paroles, position + 1, nombreCaracteres - i);
                            musique->paroles[nombreCaracteres - i] = '\0';
                         }
                      }

                      free(tag);

                      /*char tag[TAILLE_MAX_NOM] = " ";

                      lire_caracteres(fichier, tag, 3);
                      if (strcmp(tag, "TAG") != 0)
                      {
                                      fprintf(stdout, "INF recuperer_tag %d : Le fichier %s ne contient pas de tag valide.\n", __LINE__, musique->adresse);
                                      while(fclose(fichier) == EOF);
                                      return 0;
                      }

                      musique->auteur[0] = '\0';
                      musique->album[0] = '\0';
                      musique->annee[0] = '\0';

                      lire_caracteres(fichier, musique->titre, 30);
                      lire_caracteres(fichier, musique->auteur, 30);
                      lire_caracteres(fichier, musique->album, 30);
                      lire_caracteres(fichier, musique->annee, 4);

                      if (musique->titre[0] == '\0')
                      {
                          strcpy(musique->titre, musique->nom);
                          char *positionExtension = NULL;
                          if ((positionExtension = strrchr(musique->titre, '.')) != NULL)
                             *positionExtension = '\0';
                      }
                      else (musique->titre[30] = '\0');

                      if (musique->auteur[0] == '\0')
                         strcpy(musique->auteur, "Artiste Inconnu");
                      else (musique->auteur[30] = '\0');

                      if (musique->album[0] == '\0')
                         strcpy(musique->album, "Album Inconnu");
                      else (musique->album[30] = '\0');

                      if (musique->annee[0] == '\0')
                         strcpy(musique->annee, "Année Inconnue");
                      else musique->annee[4] = '\0';

                      fseek(fichier, -1, SEEK_END);
                      int genre = 0;
                      genre = fgetc(fichier);*/

                      while(fclose(fichier) == EOF);

                      if (prendreImage)
                      {
                          if (musique->surface.image != NULL)
                             SDL_FreeSurface(musique->surface.image);
                          musique->surface.image = recuperer_image(musique->adresse);

                          if (musique->surface.image != NULL)
                          {
                              SDL_Rect nouvelleDimension;
                              nouvelleDimension.h = musique->position.album.y + musique->surface.album->h - musique->position.titre.y;
                              nouvelleDimension.w = nouvelleDimension.h;
                              RSZ_redimensionner_image(nouvelleDimension, &(musique->surface.image));
                          }
                      }

                      return 1;
}


void stocker_erreur(int *numeroErreur, char chaineStockage[])
{
     *numeroErreur = FSOUND_GetError();

     switch(*numeroErreur)
     {
                          case FMOD_ERR_NONE:
                               sprintf(chaineStockage, "Aucune erreur trouvée.");
                               break;
                          case FMOD_ERR_FILE_NOTFOUND:
                               sprintf(chaineStockage, "Le fichier est introuvable.");
                               break;
                          case FMOD_ERR_FILE_FORMAT:
                               sprintf(chaineStockage, "Le format n'est pas reconnu.");
                               break;
                          case FMOD_ERR_FILE_BAD:
                               sprintf(chaineStockage, "Le chargement a échoué.");
                               break;
                          case FMOD_ERR_MEMORY:
                               sprintf(chaineStockage, "L'ordinateur ne dispose pas de ressources suffisantes.");
                               break;
                          case FMOD_ERR_VERSION:
                               sprintf(chaineStockage, "Version de fichier non supportée.");
                               break;
                          case FMOD_ERR_MEDIAPLAYER:
                               sprintf(chaineStockage, "Un composant Windows Media Player est introuvable.");
                               break;
     }
}



VOID CALLBACK OsmProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
     if (idEvent != 1 || !copieConfigMenu->parametre.osmoCom)
        return;

     //module de communication avec Osmoled
     FILE *osmoFile = NULL;
     HWND osmoWnd = NULL;
     if (!IsWindow(osmoWnd) && (osmoFile = fopen("osmcom.txt", "r")) == NULL)
        fprintf(stderr, "Erreur d'ouverture du fichier de communication avec Osmoled.\n");
     else
     {
         char adresseFenetreOsmo[TAILLE_MAX_NOM];
         fgets(adresseFenetreOsmo, TAILLE_MAX_NOM, osmoFile);
         osmoWnd = (HWND) strtol(adresseFenetreOsmo, NULL, 10);
         fclose(osmoFile);
     }

     if ((osmoFile = fopen("spectre.tmp", "wb")) != NULL)
     {
         if (spectre == NULL)
            return;

         fwrite(spectre, sizeof(float), 512, osmoFile);
         fclose(osmoFile);
         if (osmoWnd != NULL && IsWindow(osmoWnd))
            SendMessage(osmoWnd, WM_TIMER, 508, 0);   //envoi du spectre au module Osmoled
     }

     return;
}


VOID CALLBACK ReceiveProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
     //reçoit les messages de l'application auxiliaire

     char buffer[TAILLE_MAX_NOM] = "\0";
     strcpy(buffer, (char*) sharedAuxMemory);

     if (strcmp(buffer, "SUIVANT") == 0)
        passerAuSuivant = 1;
     else if (strcmp(buffer, "PRECEDENT") == 0)
          passerAuPrecedent = 1;
     else if (strcmp(buffer, "PLAY/PAUSE") == 0)
          mettreEnPlayOuPause = 1;
     else if (strcmp(buffer, "STOP") == 0)
          mettreEnStop = 1;

     strcpy((char*)sharedAuxMemory, "\0");
     return;
}



void display_callback (SDL_Surface* dst, int x, int y, unsigned int w, unsigned int h)
{
     if (!bigScreenMode)
     {
        positionSurfaceSMPEG.x = copieConfigMenu->positionFond.x + (copieConfigMenu->imageDeFond->w - w) / 2;
        positionSurfaceSMPEG.y = copieConfigMenu->positionFond.y + (copieConfigMenu->imageDeFond->h - h) / 2;
     }
     else
     {
         positionSurfaceSMPEG.x = (copieConfigMenu->ecran->w - w - 20) / 2;
         positionSurfaceSMPEG.y = 10 + (copieConfigMenu->ecran->h - h - 80) / 2;
     }

     if (numeroVisu < NOMBRE_VISUS && numeroVisu >= 0)
     {
         SDL_BlitSurface(surfaceSMPEG, NULL, copieConfigMenu->ecran, &positionSurfaceSMPEG);
         SDL_UpdateRect(copieConfigMenu->ecran, positionSurfaceSMPEG.x, positionSurfaceSMPEG.y, w, h);
     }

     return;
}


void resize_video(SMPEG* smpeg)
{
     if (SDL_GetTicks() - tempsDernierResize < 1500)
        SDL_Delay(1500 - SDL_GetTicks() + tempsDernierResize);

     SMPEG_Info SInfo;
     SMPEG_getinfo(smpeg, &SInfo);
     SDL_Rect traceRect;

     int originalWidth = SInfo.width,
         originalHeight = SInfo.height;
     SInfo.width *= agrandissement;
     SInfo.height *= agrandissement;

     if (!bigScreenMode)
     {
         traceRect.w = copieConfigMenu->imageDeFond->w;
         traceRect.h = copieConfigMenu->imageDeFond->h;
     }
     else
     {
         traceRect.w = copieConfigMenu->ecran->w - 20;
         traceRect.h = copieConfigMenu->ecran->h - 80;
     }

     if (SInfo.width > traceRect.w)
     {
         agrandissement = traceRect.w * 1.0 / originalWidth;
         int newHeight = originalHeight * agrandissement;
         SInfo.height = newHeight;
         SInfo.width = traceRect.w;
     }

     if (SInfo.height > traceRect.h)
     {
         agrandissement = traceRect.h * 1.0 / originalHeight;
         int newWidth = originalWidth * agrandissement;
         SInfo.height = traceRect.h;
         SInfo.width = newWidth;
     }

     if (surfaceSMPEG != NULL)
        SDL_FreeSurface(surfaceSMPEG);
     surfaceSMPEG = SDL_AllocSurface(SDL_HWSURFACE, SInfo.width, SInfo.height, copieConfigMenu->ecran->format->BitsPerPixel, copieConfigMenu->ecran->format->Rmask, copieConfigMenu->ecran->format->Gmask, copieConfigMenu->ecran->format->Bmask, copieConfigMenu->ecran->format->Amask);

     positionSurfaceSMPEG.w = SInfo.width;
     positionSurfaceSMPEG.h = SInfo.height;

     SMPEG_scaleXY(smpeg, SInfo.width, SInfo.height);
     tempsDernierResize = SDL_GetTicks();

     return;
}


char* strstr2 (char string1[], char string2[], int max)
{
      char *buffer = malloc(strlen(string2) + 1);
      if (buffer == NULL)
         return NULL;
      buffer[0] = '\0';

      int i;
      for (i = 0 ; i < max - strlen(string2) ; i++)
      {
          if (string1[i] == string2[0])
          {
              CopyMemory(buffer, string1 + i, strlen(string2));
              buffer[strlen(string2)] = '\0';

              if (strcmp(buffer, string2) == 0)
              {
                 free(buffer);
                 return string1 + i;
              }
          }
      }

      free(buffer);
      return NULL;
}

unsigned int inverser_nombre (unsigned int nombre)
{
    unsigned char decomposition[4];
    CopyMemory(decomposition, &nombre, 4);
    unsigned int resultat;

    resultat = decomposition[3] + (decomposition[2] * pow(2,8)) + (decomposition[1] * pow(2,16)) + (decomposition[0] * pow(2,24));
    return resultat;
}


SDL_Surface* recuperer_image (char nomDuFichier[])
{
    FILE *fichier = fopen(nomDuFichier, "rb");
    if (fichier == NULL)
    {
        fprintf(stderr, "ERR recuperer_image %d : Impossible d'ouvrir le fichier %s.\n", __LINE__, nomDuFichier);
        return NULL;
    }

    char identifieur[TAILLE_MAX_NOM] = "\0";
    fread(identifieur, 1, 3, fichier);
    identifieur[3] = '\0';
    if (strcmp(identifieur, "ID3") != 0)
    {
       fclose(fichier);
       return NULL;
    }

    int tailleTag = 0;
    fseek(fichier, 6, SEEK_SET);
    fread(&tailleTag, 1, sizeof(int), fichier);
    tailleTag = inverser_nombre(tailleTag);

    char *tag = malloc(tailleTag);
    if (tag == NULL)
    {
       fclose(fichier);
       fprintf(stderr, "ERR recuperer_image %d : Erreur d'allocation de mémoire (%d bits demandés).\n", __LINE__, tailleTag);
       return NULL;
    }

    rewind(fichier);
    fread(tag, 1, tailleTag, fichier);
    fclose(fichier);

    char *position = NULL;
    if ((position = strstr2(tag, "APIC", tailleTag)) == NULL)
    {
       fprintf(stdout, "INF recuperer_image %d : Le fichier %s ne semble pas contenir d'image d'album.\n", __LINE__, nomDuFichier);
       free(tag);
       return NULL;
    }

    position += 4;
    unsigned int tailleImage = 0;
    CopyMemory(&tailleImage, position, sizeof(int));
    tailleImage = inverser_nombre(tailleImage);

    position += 7;
    CopyMemory(identifieur, position, 6);
    identifieur[6] = '\0';
    if (strcmp(identifieur, "image/") != 0)
    {
       fprintf(stdout, "INF recuperer_image %d : Le fichier %s ne semble pas contenir d'image d'album valide.\n", __LINE__, nomDuFichier);
       free(tag);
       return NULL;
    }

    position += 6;
    char extension[TAILLE_MAX_NOM] = "\0";
    int i = 0;
    for (position = position ; *position != '\0' ; position++)
    {
        extension[i] = *position;
        i++;
    }
    extension[i] = '\0';

    position += 3;
    void *image = malloc(tailleImage);
    if (image == NULL)
    {
       fprintf(stderr, "ERR recuperer_image %d : Erreur d'allocation de mémoire (%d bits demandés).\n", __LINE__, tailleImage);
       free(tag);
       return NULL;
    }

    CopyMemory(image, position, tailleImage);
    free(tag);

    char nomDeFichierImage[TAILLE_MAX_NOM] = "imageAlbum";
    sprintf(nomDeFichierImage, "%s.%s", nomDeFichierImage, extension);
    if ((fichier = fopen(nomDeFichierImage, "wb")) == NULL)
    {
       fprintf(stderr, "ERR recuperer_image %d : Impossible de créer le fichier %s.\n", __LINE__, nomDeFichierImage);
       free(image);
       return NULL;
    }
    fwrite(image, 1, tailleImage, fichier);
    fclose(fichier);

    SDL_Surface *surfaceImage = IMG_Load(nomDeFichierImage);
    if (surfaceImage == NULL)
       fprintf(stderr, "ERR recuperer_image %d : Impossible de charger l'image %s.\n", __LINE__, nomDeFichierImage);

    free(image);
    return surfaceImage;
}


