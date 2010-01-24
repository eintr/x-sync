               /*******************************************
                *          Statistics of FCWH
                *          Author: Zeming Xu
                *          Date:   Jan, 24, 2010
                *******************************************/

/*-----------------------INCLUDES--------------------------------------------*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*-----------------------GLOBAL VARIABLES & MACROS---------------------------*/

#define BLOCKSIZE 1024
#define REFINEDEGREE 11
// 1,2,3,4,5,6,7,8,9,10,11
unsigned long int SUBBLOCKSIZE;
// 1024,512,256,128,64,32,16,8,4,2,1
unsigned long int HASHVECTORLENGTH;
// 1,2,4,8,16,32,64,128,256,512,1024
/* 
 * h=8 bits(1 Byte)
 * blockSize is the size(in bytes) of the block to generate FCWH - d
 * BLOCKSIZE d=1024=1K bytes
 * the refining degree: 1,    2,   3,  4,   5,  6
 * i = refinedegree -1;
 * subblocksize      : 1024, 512,256, 128, 64, 32
 * subblocksize = d/(2^(refinedegree - 1))
 * vecetor length of hash values: 
 * hashvectorlength = 2^(refinedegree -1) bytes
 */

// "./bibliographie.doc" is the reference file;
char * referenceFile = "./bibliographie.doc";
// file descriptor for reference file & its file size
int fd;
struct stat statFile;
off_t fileSize;

/*
 * Buffers needed in the program:
 */

// buffer to store the original file
unsigned char * bufPtrFile = NULL;
size_t bufPtrFileSize = 0;

// buffer to store the raw FCWH
unsigned char * bufPtrRawFCWH = NULL;
size_t RAWFCWHNUMBER = 0;

// buffer to store the sorted subscripts
unsigned long int * bufPtrSortedSubscript = NULL;
size_t SORTEDSUBSCRIPTNUMBER = 0;

// buffer to store the compressed subscripts
unsigned long int * bufPtrCompressedSubscript = NULL;
size_t COMPRESSEDSUBSCRIPTNUMBER = 0;

/* buffer to store the frequency.
 * The frequency is the FCWH of non-identical substrings.
 */
unsigned long int * bufPtrFrequency = NULL;
size_t FREQUENCYNUMBER = 0;
unsigned long int MAXFREQUENCY;

/* buffer to store the statistics result.
 * the statistics for the frequencies from 0 to the maximum frequency,
 * Hence, the resulting output data can be plotted with matlab command
 * stem.
 */
unsigned long int * bufPtrFcwhStatistics = NULL;
size_t FCWHSTATISTICSNUMBER = 0;

/*------------------------- FUNCTIONS----------------------------------------*/

/*
 * output the reference file information
 */
void outputReferenceFileInfo();

/*
 * select refine degree
 */
void selectRefineDeg();

/* 
 * open the file of reference version
 */
void openFileReference();

/* 
 * close the file of reference version
 */
void closeFileReference();

/*
 * calculate all the raw FCWHs of the reference file
 */
void calcFCWH();

/*
 * using bubble method to sort the FCWH vectors.
 * compare the two FCWH vectors.
 * if vecHashVal(curCmp)<vecHashVal(cur),
 * return 1;
 * else return 0.
 */
unsigned short int ascendant(\
			     unsigned long int curCmp,\
			     unsigned long int cur);

/*
 * output the FCWH vector 
 * in the raw hash buffer
 * starting at subscript cur.
 */
void outputFCWHvec(unsigned long int cur);

/*
 * sort the subscripts of the FCWHs
 */
void sortSubscript();

/*
 * compare the two FCWH vectors.
 * if vecHashVal(curCmp)==vecHashVal(cur),
 * return 1;
 * else return 0.
 */
unsigned short int identical(\
			     unsigned long int curCmp,\
			     unsigned long int cur);

/*
 * compare the two substrings of the reference file with BLOCKSIZE bytes.
 * if substring(curCmp)==substring(cur),
 * return 1;
 * else return 0.
 */
unsigned short int identicalSS(\
			     unsigned long int curCmp,\
			     unsigned long int cur);

/*
 * compress the subscripts of the FCWHs
 */
void compressSubscript();

/*
 * calculate frequency:
 * occurs for 1st, 2nd, 3rd,... times
 */
void calcFrequency();

/*
 * statistics for each frequence.
 * the output is a ***.txt file,
 * as an input for matlab.
 */
void statisticsFCWH();

/*
 * free memory allocated
 */
void freeMemoryAllocated();

/*------------------------------------END-----------------------------------*/
