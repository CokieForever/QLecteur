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

#include "fonctions_wav.h"


void sauver_en_wav(FSOUND_SAMPLE *samp, char fileName[], int longueur)

{
    FILE *fp;
    int lenbytes, channels, bits, rate;
    void *ptr1, *ptr2;
    unsigned int len1, len2;
    unsigned int mode;

    if (!samp)
    {
        return;
    }

    mode     = FSOUND_Sample_GetMode(samp);
    bits     = (mode & FSOUND_16BITS) ? 16 : 8;
    channels = (mode & FSOUND_STEREO) ? 2  : 1;
    FSOUND_Sample_GetDefaults(samp, &rate, 0, 0, 0);
    if (longueur == -1)
       lenbytes = FSOUND_Sample_GetLength(samp) * channels * bits / 8;
    else lenbytes = longueur * (rate / 1000.0) * channels * bits / 8;


    {
        #if defined(WIN32) || defined(_WIN64) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
        #pragma pack(1)
        #endif

        /*
            WAV Structures
        */
        typedef struct
        {
	        signed char id[4];
	        int 		size;
        } RiffChunk;

        struct
        {
            RiffChunk       chunk           ;
            unsigned short	wFormatTag      ;    /* format type  */
            unsigned short	nChannels       ;    /* number of channels (i.e. mono, stereo...)  */
            unsigned int	nSamplesPerSec  ;    /* sample rate  */
            unsigned int	nAvgBytesPerSec ;    /* for buffer estimation  */
            unsigned short	nBlockAlign     ;    /* block size of data  */
            unsigned short	wBitsPerSample  ;    /* number of bits per sample of mono data */
        } FmtChunk  = { {{'f','m','t',' '}, sizeof(FmtChunk) - sizeof(RiffChunk) }, 1, channels, rate, rate * channels * bits / 8, 1 * channels * bits / 8, bits };

        struct
        {
            RiffChunk   chunk;
        } DataChunk = { {{'d','a','t','a'}, lenbytes } };

        struct
        {
            RiffChunk   chunk;
	        signed char rifftype[4];
        } WavHeader = { {{'R','I','F','F'}, sizeof(FmtChunk) + sizeof(RiffChunk) + lenbytes }, {'W','A','V','E'} };

        #if defined(WIN32) || defined(_WIN64) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
        #pragma pack()
        #endif

        fp = fopen(fileName, "wb");

        /*
            Write out the WAV header.
        */
        fwrite(&WavHeader, sizeof(WavHeader), 1, fp);
        fwrite(&FmtChunk, sizeof(FmtChunk), 1, fp);
        fwrite(&DataChunk, sizeof(DataChunk), 1, fp);

        /*
            Lock the sample to get acces to the data
        */
        FSOUND_Sample_Lock(samp, 0, lenbytes, &ptr1, &ptr2, &len1, &len2);

        fwrite(ptr1, len1, 1, fp);

        FSOUND_Sample_Unlock(samp, ptr1, ptr2, len1, len2);

        fclose(fp);
    }
}



void ajouter_wav(FSOUND_SAMPLE *samp, char fileName[], int longueur)

{
    FSOUND_SAMPLE *samp2 = NULL;
    samp2 = FSOUND_Sample_Load(FSOUND_FREE, fileName, 0, 0, 0);

    FILE *fp;
    int lenbytes, lenbytesSamp2, channels, bits, rate;
    void *ptr1, *ptr2;
    unsigned int len1, len2;
    unsigned int mode;

    if (!samp)
    {
        return;
    }

    mode     = FSOUND_Sample_GetMode(samp);
    bits     = (mode & FSOUND_16BITS) ? 16 : 8;
    channels = (mode & FSOUND_STEREO) ? 2  : 1;
    FSOUND_Sample_GetDefaults(samp, &rate, 0, 0, 0);
    if (longueur == -1)
       lenbytes = FSOUND_Sample_GetLength(samp) * channels * bits / 8;
    else lenbytes = longueur * rate * channels * bits / 8;
    lenbytesSamp2 = FSOUND_Sample_GetLength(samp2) * channels * bits / 8;


    {
        #if defined(WIN32) || defined(_WIN64) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
        #pragma pack(1)
        #endif

        /*
            WAV Structures
        */
        typedef struct
        {
	        signed char id[4];
	        int 		size;
        } RiffChunk;

        struct
        {
            RiffChunk       chunk           ;
            unsigned short	wFormatTag      ;    /* format type  */
            unsigned short	nChannels       ;    /* number of channels (i.e. mono, stereo...)  */
            unsigned int	nSamplesPerSec  ;    /* sample rate  */
            unsigned int	nAvgBytesPerSec ;    /* for buffer estimation  */
            unsigned short	nBlockAlign     ;    /* block size of data  */
            unsigned short	wBitsPerSample  ;    /* number of bits per sample of mono data */
        } FmtChunk  = { {{'f','m','t',' '}, sizeof(FmtChunk) - sizeof(RiffChunk) }, 1, channels, rate, rate * channels * bits / 8, 1 * channels * bits / 8, bits };

        struct
        {
            RiffChunk   chunk;
        } DataChunk = { {{'d','a','t','a'}, lenbytes + lenbytesSamp2 } };

        struct
        {
            RiffChunk   chunk;
	        signed char rifftype[4];
        } WavHeader = { {{'R','I','F','F'}, sizeof(FmtChunk) + sizeof(RiffChunk) + lenbytes + lenbytesSamp2 }, {'W','A','V','E'} };

        #if defined(WIN32) || defined(_WIN64) || defined(__WATCOMC__) || defined(_WIN32) || defined(__WIN32__)
        #pragma pack()
        #endif

        fp = fopen(fileName, "wb");

        /*
            Write out the WAV header.
        */

        fwrite(&WavHeader, sizeof(WavHeader), 1, fp);
        fwrite(&FmtChunk, sizeof(FmtChunk), 1, fp);
        fwrite(&DataChunk, sizeof(DataChunk), 1, fp);

        /*
            Lock the sample to get acces to the data
        */
        FSOUND_Sample_Lock(samp, 0, lenbytes, &ptr1, &ptr2, &len1, &len2);
        fwrite(ptr1, len1, 1, fp);
        FSOUND_Sample_Unlock(samp, ptr1, ptr2, len1, len2);


        FSOUND_Sample_Lock(samp2, 0, lenbytesSamp2, &ptr1, &ptr2, &len1, &len2);
        fwrite(ptr1, len1, 1, fp);
        FSOUND_Sample_Unlock(samp2, ptr1, ptr2, len1, len2);

        fclose(fp);
    }
}
