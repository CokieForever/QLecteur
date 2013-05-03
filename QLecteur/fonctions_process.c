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

#include "fonctions_process.h"
#define TAILLE_MAX_CHAINE 1000
#define TAILLE_MAX_TAB 5000


// Determine le nombre de processus du nom de "monProcess" en cours d'éxécution
int test_exist_process(const char* monProcess)
{
	int nombreTrouves = 0;
    DWORD processes[TAILLE_MAX_TAB], nb_processes;
	char process_name[TAILLE_MAX_CHAINE];
	int i;
	HANDLE hprocess;

	EnumProcesses(processes, sizeof(processes), &nb_processes);
    //On teste tous les processus pour voir si leur nom correspond

	for(i = 0  ; i < nb_processes / sizeof(DWORD) ; i++)
  	{
		hprocess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
		//Prend le nom du processus et le met dans process_name
		GetModuleBaseName(hprocess, NULL, process_name, sizeof(process_name));
		CloseHandle(hprocess);

		if(strcmp(process_name, monProcess) == 0)
        {
			nombreTrouves++;
		}
	}
	return nombreTrouves;
}
