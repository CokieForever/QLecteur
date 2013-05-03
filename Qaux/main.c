/*
QAux - Extension for the QLecteur multimedia player
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


#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <psapi.h>
#include <string.h>
#define MAX_CHAINE 5000
#define ICON_ID 4
#define STOP 1
#define PLAYPAUSE 2
#define CLOSE 3
#define NEXT 5
#define PREVIOUS 6
#define HIDESHOW 7
#define TIMER 1



LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
int print_icon_menu();
int test_exist_process(const char* monProcess);


int visible = 1;

HICON iconTaskbar = NULL;

HWND mainWnd = NULL,
     QLecteurWnd = NULL;

void *sharedMemory = NULL;


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow
                     )

{
    MSG message;

    HANDLE handle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "qauxshared");
    if (handle == NULL)
    {
               MessageBox(NULL, "L'application QLecteur n'est pas démarrée ou n'a pas partagé sa mémoire.", "Attention !", MB_ICONWARNING | MB_OK);
               exit(EXIT_FAILURE);
    }
    sharedMemory = MapViewOfFile(handle, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(HWND) + 1000);
    if (sharedMemory != NULL)
       CopyMemory(&QLecteurWnd, sharedMemory, sizeof(HWND));

    if (!IsWindow(QLecteurWnd) || sharedMemory == NULL)
    {
        MessageBox(NULL, "Les données récupérées sur Qlecteur sont erronées.\nImpossible de continuer.", "Erreur !", MB_OK | MB_ICONERROR);
        exit(EXIT_FAILURE);
    }

    sharedMemory += sizeof(HWND);


    iconTaskbar = (HICON) LoadImage(hInstance, "iconTaskbar", IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR);

    WNDCLASSEX principale;
    principale.cbSize = sizeof(WNDCLASSEX);
    principale.style = CS_HREDRAW|CS_VREDRAW;
    principale.lpfnWndProc = WindowProcedure;
    principale.cbClsExtra = 0;
    principale.cbWndExtra = 0;
    principale.hInstance = hInstance;
    principale.hIcon = iconTaskbar;
    principale.hCursor = LoadCursor(NULL, IDC_ARROW);
    principale.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
    principale.lpszMenuName = NULL;
    principale.lpszClassName = "std";
    principale.hIconSm = iconTaskbar;
    RegisterClassEx(&principale);

    mainWnd = CreateWindowEx(
          WS_EX_CLIENTEDGE,
          "std",
          "Notre fenêtre",
          WS_OVERLAPPEDWINDOW,
          CW_USEDEFAULT,
          CW_USEDEFAULT,
          CW_USEDEFAULT,
          CW_USEDEFAULT,
          NULL,
          NULL,
          hInstance,
          NULL
       );
    ShowWindow(mainWnd, SW_HIDE);

    //mise en place de l'icone
    NOTIFYICONDATA nid;
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = mainWnd;
    nid.uID = ICON_ID;
    nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    nid.uCallbackMessage = WM_COMMAND;
    nid.hIcon = iconTaskbar;
    strcpy(nid.szTip, "Qaux pour QLecteur");

    Shell_NotifyIcon(NIM_ADD, &nid);
    SetTimer(mainWnd, TIMER, 1000, NULL);


    while (GetMessage (&message, NULL, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = mainWnd;
    nid.uID = ICON_ID;
    nid.uFlags = 0;

    Shell_NotifyIcon(NIM_DELETE, &nid);

    CloseHandle(handle);
    return message.wParam;
}




LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    NOTIFYICONDATA nid;
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = mainWnd;
    nid.uID = ICON_ID;
    nid.uFlags = 0;

    int Select, notification;

    switch (message)
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_TIMER:
        {
             if (!IsWindow(QLecteurWnd))
             {
                       KillTimer(hwnd, TIMER);
                       MessageBox(NULL, "L'application QLecteur a été arrêtée.\n Fermeture du programme.", "Information Qaux", MB_ICONINFORMATION | MB_OK);
                       PostQuitMessage(1);
             }

             return FALSE;
        }
        case WM_COMMAND:
             Select = LOWORD(wParam);
             switch (Select)
             {
                    case ICON_ID:
                        notification = lParam;

                        if (notification == WM_RBUTTONDOWN)
                            print_icon_menu();
                        return FALSE;

                    case PLAYPAUSE:
                         strcpy((char*)sharedMemory, "PLAY/PAUSE");
                         return FALSE;
                    case STOP:
                         strcpy((char*)sharedMemory, "STOP");
                         return FALSE;
                    case NEXT:
                         strcpy((char*)sharedMemory, "SUIVANT");
                         return FALSE;
                    case PREVIOUS:
                         strcpy((char*)sharedMemory, "PRECEDENT");
                         return FALSE;
                    case HIDESHOW:
                         if (QLecteurWnd != NULL && IsWindowVisible(QLecteurWnd))
                            ShowWindow(QLecteurWnd, SW_HIDE);
                         else if (QLecteurWnd != NULL && IsWindow(QLecteurWnd))
                              ShowWindow(QLecteurWnd, SW_SHOWMINIMIZED);
                         return FALSE;
                    case CLOSE:
                         if (QLecteurWnd != NULL && !IsWindowVisible(QLecteurWnd))
                            SendMessage(hwnd, WM_COMMAND, HIDESHOW, 0);
                         PostQuitMessage(1);
                         return FALSE;
             }

             return FALSE;
        default:
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}




int print_icon_menu()
{
      HMENU hMenu;
      POINT pt;
      GetCursorPos(&pt);

      hMenu = CreatePopupMenu();
      AppendMenu(hMenu, MF_STRING, PLAYPAUSE, "Play / Pause");
      AppendMenu(hMenu, MF_STRING, STOP, "Stop");
      AppendMenu(hMenu, MF_STRING, NEXT, "Suivant");
      AppendMenu(hMenu, MF_STRING, PREVIOUS, "Précédent");
      if (QLecteurWnd == NULL || !IsWindow(QLecteurWnd))
         AppendMenu(hMenu, MF_STRING | MF_GRAYED, HIDESHOW, "Montrer");
      else if (IsWindowVisible(QLecteurWnd))
         AppendMenu(hMenu, MF_STRING, HIDESHOW, "Cacher");
      else AppendMenu(hMenu, MF_STRING, HIDESHOW, "Montrer");
      AppendMenu(hMenu, MF_STRING | MF_SEPARATOR, 0, NULL);
      AppendMenu(hMenu, MF_STRING, CLOSE, "Fermer");


      if (TrackPopupMenu(hMenu, 0, pt.x, pt.y, 0, mainWnd, NULL))
         return 1;
      else return 0;
}
