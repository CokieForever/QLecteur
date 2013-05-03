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

#include "fonctions_registre.h"

void assoc_extension(char extension[], char fileType0[], char iconFile[], char openCommand[])
{
    HKEY key = NULL;
    char buffer[MAX_CHAINE],
         fileType[MAX_CHAINE],
         fileType2[MAX_CHAINE] = "\0",
         *position = NULL;
    strncpy(fileType, fileType0, MAX_CHAINE);

    if ((position = strchr(fileType, '\n')) != NULL)
    {
        strcpy(fileType2, position + 1);
        *position = '\0';
    }


    RegCreateKeyEx(
            HKEY_CLASSES_ROOT,
            extension,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE | DELETE,
            NULL,
            &key,
            NULL
       );
    RegSetValueEx(key, NULL, 0, REG_SZ, (const BYTE*)fileType, strlen(fileType) + 1);
    RegCloseKey(key);

    RegCreateKeyEx(
            HKEY_CLASSES_ROOT,
            fileType,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE | DELETE,
            NULL,
            &key,
            NULL
       );
    if (fileType2[0] != '\0')
       RegSetValueEx(key, NULL, 0, REG_SZ, (const BYTE*)fileType2, strlen(fileType2) + 1);
    RegCloseKey(key);

    sprintf(buffer, "%s\\DefaultIcon", fileType);
    RegCreateKeyEx(
            HKEY_CLASSES_ROOT,
            buffer,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE | DELETE,
            NULL,
            &key,
            NULL
       );
    RegSetValueEx(key, NULL, 0, REG_SZ, (const BYTE*)iconFile, strlen(iconFile) + 1);
    RegCloseKey(key);

    sprintf(buffer, "%s\\shell", fileType);
    RegCreateKeyEx(
            HKEY_CLASSES_ROOT,
            buffer,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE | DELETE,
            NULL,
            &key,
            NULL
       );
    RegCloseKey(key);

    strcat(buffer, "\\open");
    RegCreateKeyEx(
            HKEY_CLASSES_ROOT,
            buffer,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE | DELETE,
            NULL,
            &key,
            NULL
       );
    RegCloseKey(key);

    strcat(buffer, "\\command");
    RegCreateKeyEx(
            HKEY_CLASSES_ROOT,
            buffer,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE | DELETE,
            NULL,
            &key,
            NULL
       );
    RegSetValueEx(key, NULL, 0, REG_SZ, (const BYTE*)openCommand, strlen(openCommand) + 1);
    RegCloseKey(key);

    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

    return;
}

int create_autostart(char programm[])
{
    if (strrchr(programm, '\\') == NULL)
       return 0;

    //ouverture de la clé
    HKEY key = NULL;
    int value = RegCreateKeyEx(
            HKEY_CURRENT_USER,
            "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE | DELETE,
            NULL,
            &key,
            NULL
       );

    if (value == ERROR_SUCCESS)
    {
       if (RegSetValueEx(key, strrchr(programm, '\\') + 1, 0, REG_SZ, (const BYTE*)programm, strlen(programm) + 1) != ERROR_SUCCESS)
          return 0;
       return 1;
    }
    else return 0;
}

