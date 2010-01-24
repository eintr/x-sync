               /*******************************************
                *          Statistics of FCWH
                *          Author: Zeming Xu
                *          Date:   Jan, 24, 2010
                *******************************************/

#include "statFCWH.h"

/*---------------------------------------------------------------------------*/

int main(int argc, char *argv[]){

  selectRefineDeg();

  calcFCWH();
  sortSubscript();
  compressSubscript();
  calcFrequency();
  statisticsFCWH();

  freeMemoryAllocated();
  return 0;
}

/*---------------------FUNCTIONS--------------------------------*/

void outputReferenceFileInfo(){
  printf("file size: %ld\n",(unsigned long int)fileSize);
  printf("file name: %s\n",referenceFile);
  return;
}

void selectRefineDeg(){
  //unsigned long int REFINEDEGREE = 1;
  // 1,2,3,4,5,6,7,8

  HASHVECTORLENGTH = 1 << (REFINEDEGREE-1);
  // 1,2,4,8,16,32,64,128

  SUBBLOCKSIZE = BLOCKSIZE / HASHVECTORLENGTH;
  // 1024,512,256,128,64,32,16,8

  return;
}

void openFileReference(){
  fd = open(referenceFile,O_RDONLY);
  fstat(fd,&statFile);
  fileSize = statFile.st_size;
  bufPtrFile = (unsigned char *)malloc(fileSize);
  read(fd,bufPtrFile,fileSize);
  return;
}

void closeFileReference(){
  close(fd);
  return;
}

void calcFCWH(){
  openFileReference();
  closeFileReference();

  RAWFCWHNUMBER = fileSize - SUBBLOCKSIZE + 1;
  bufPtrRawFCWH = (unsigned char *)malloc(RAWFCWHNUMBER);

  unsigned char * headPtr = bufPtrFile;
  unsigned char * tailPtr = bufPtrFile;
  unsigned char * ptrFCWH = bufPtrRawFCWH;
  register unsigned char hashValueCurrent = 0;

  unsigned long int loop;
  for(loop=0; loop < SUBBLOCKSIZE; loop++){
    hashValueCurrent ^= (*tailPtr);
    ++tailPtr;
  }

  (*ptrFCWH) = hashValueCurrent;
  ++ptrFCWH;

  for(loop=1; loop < RAWFCWHNUMBER; loop++){
    hashValueCurrent ^= (*tailPtr);
    hashValueCurrent ^= (*headPtr);
    (*ptrFCWH) = hashValueCurrent;
    ++tailPtr;
    ++headPtr;
    ++ptrFCWH;
  }

  return;
}

unsigned short int ascendant(\
			     unsigned long int curCmp,\
			     unsigned long int cur){

  unsigned char * ptrCmp;
  unsigned char * ptr;
  ptrCmp = ptr = bufPtrRawFCWH;
  ptrCmp += curCmp;
  ptr += cur;

  unsigned long int loop;
  for(loop = 0;loop < HASHVECTORLENGTH; ++loop){
    if((*ptrCmp)>(*ptr)){
      return 0;
    }else{
      if((*ptrCmp) < (*ptr)){
	return 1;
      }else{
	ptrCmp += SUBBLOCKSIZE;
	ptr += SUBBLOCKSIZE;
      }
    }
  }
  return 1;
}

void outputFCWHvec(unsigned long int cur){
  unsigned long int loop;
  unsigned char * curPtr = bufPtrRawFCWH + cur;
  for(loop=0;loop<HASHVECTORLENGTH;++loop){
    printf("%2x ",(*curPtr));
    curPtr += SUBBLOCKSIZE;
  }
  printf("\n");
  return;
}

void sortSubscript(){
  /*
   * NB:
   * SORTEDSUBSCRIPTNUMBER is NOT RAWFCWHNUMBER !!!
   *
   */
  SORTEDSUBSCRIPTNUMBER =  fileSize - BLOCKSIZE + 1;
  bufPtrSortedSubscript = (unsigned long int *)malloc(SORTEDSUBSCRIPTNUMBER * 4);

  unsigned long int loop;
  unsigned long int * curPtr = bufPtrSortedSubscript;
  for(loop=0; loop<SORTEDSUBSCRIPTNUMBER; ++loop){
    (*curPtr) = loop;
    ++curPtr;
  }

  /*
   * bubble sort
   */
  unsigned long int curCmp, cur;
  for(cur=SORTEDSUBSCRIPTNUMBER - 1;cur > 0; --cur){
    for(curCmp = 0;curCmp < cur; ++curCmp){
      unsigned long int subPre = bufPtrSortedSubscript[curCmp];
      unsigned long int subNext = bufPtrSortedSubscript[curCmp + 1];
      if(ascendant(subPre, subNext) == 0){
	bufPtrSortedSubscript[curCmp] = subNext;
	bufPtrSortedSubscript[curCmp + 1] = subPre;
      }
    }
  }

  /*
   * debug: output the sorted raw FCWH vectors
   */
  /*
  curPtr = bufPtrSortedSubscript;
  for(loop=0; loop<SORTEDSUBSCRIPTNUMBER; ++loop){
    printf("%ld\t",loop);
    printf("%ld\t",(*curPtr)); 
    outputFCWHvec(*curPtr);
    ++curPtr;
  }
  */

  return;
}

unsigned short int identical(\
			     unsigned long int curCmp,\
			     unsigned long int cur){
  unsigned long int loop;
  unsigned char * ptrCmp = (unsigned char *)(bufPtrRawFCWH+curCmp);
  unsigned char * ptr = (unsigned char *)(bufPtrRawFCWH+cur);
  for(loop = 0;loop < HASHVECTORLENGTH; ++loop){
    if((*ptrCmp) != (*ptr)){
      return 0;
    }
    ptrCmp += SUBBLOCKSIZE;
    ptr += SUBBLOCKSIZE;
  }
  return 1;
}

unsigned short int identicalSS(\
			     unsigned long int curCmp,\
			     unsigned long int cur){
  unsigned long int loop;
  unsigned char * ptrCmp = (unsigned char *)(bufPtrFile+curCmp);
  unsigned char * ptr = (unsigned char *)(bufPtrFile+cur);
  for(loop = 0;loop < BLOCKSIZE; ++loop){
    if((*ptrCmp) != (*ptr)){
      return 0;
    }
    ++ptrCmp;
    ++ptr;
  }
  return 1;
}

void compressSubscript(){
  bufPtrCompressedSubscript = (unsigned long int *)malloc(SORTEDSUBSCRIPTNUMBER * 4);

  COMPRESSEDSUBSCRIPTNUMBER = 0;
  unsigned long int curSorted = 0;

  bufPtrCompressedSubscript[COMPRESSEDSUBSCRIPTNUMBER] = bufPtrSortedSubscript[curSorted];
  unsigned long int headCompressed = COMPRESSEDSUBSCRIPTNUMBER;
  unsigned long int testCompressed = headCompressed;
  ++COMPRESSEDSUBSCRIPTNUMBER;
  ++curSorted;

  while(curSorted<SORTEDSUBSCRIPTNUMBER){
    if(identical(bufPtrCompressedSubscript[headCompressed],bufPtrSortedSubscript[curSorted]) ==0){
      bufPtrCompressedSubscript[COMPRESSEDSUBSCRIPTNUMBER] = bufPtrSortedSubscript[curSorted];
      headCompressed = COMPRESSEDSUBSCRIPTNUMBER;
      testCompressed = headCompressed;
      ++COMPRESSEDSUBSCRIPTNUMBER;
      ++curSorted;
    }else{
      while(testCompressed < COMPRESSEDSUBSCRIPTNUMBER){
	if(identicalSS(bufPtrCompressedSubscript[testCompressed],bufPtrSortedSubscript[curSorted]) == 1){
	  ++curSorted;
	  testCompressed = headCompressed;
	  break;
	}else{
	  ++testCompressed;
	}
      }
      if(testCompressed==COMPRESSEDSUBSCRIPTNUMBER){
	bufPtrCompressedSubscript[COMPRESSEDSUBSCRIPTNUMBER] = bufPtrSortedSubscript[curSorted];
	testCompressed = headCompressed;
	++COMPRESSEDSUBSCRIPTNUMBER;
	++curSorted;
      }
    }
  }

  /*
   *                                NOTICE:
   *
   *         The resultant COMPRESSEDSUBSCRIPTNUMBER is independent of
   *         REFINEDEGREE !!! COMPRESSEDSUBSCRIPTNUMBER is exactly the
   *         number of different substrings of BLOCKSIZE bytes in
   *         reference file. And this can be used to debug this function.
   *         You can also use the brute-force algorithm to calculate the
   *         number of different substrings of BLOCKSIZE bytes in
   *         reference file.
   *
   */

  /*
   * debug
   */
  /*
  unsigned long int * curPtr = bufPtrCompressedSubscript;
  unsigned long int loop;
  for(loop=0; loop<COMPRESSEDSUBSCRIPTNUMBER; ++loop){
    printf("%ld\t",loop);
    printf("%ld\t",(*curPtr)); 
    outputFCWHvec(*curPtr);
    ++curPtr;
  }
  */
  return;
}

void calcFrequency(){
  FREQUENCYNUMBER = COMPRESSEDSUBSCRIPTNUMBER;
  bufPtrFrequency = (unsigned long int *)malloc(FREQUENCYNUMBER * 4);

  unsigned long int loop;
  unsigned long int * curPtr;

  // 1st traverse: from first to last, initialize to 0
  curPtr = bufPtrFrequency;
  for(loop=0;loop<FREQUENCYNUMBER;++loop){
    (*curPtr) = 0;
    ++curPtr;
  }

  // 2nd traverse: from first to last, initialize to 0
  curPtr = bufPtrFrequency;
  ++curPtr;
  unsigned long int valPre, valNext;
  for(loop=1;loop<FREQUENCYNUMBER;++loop){
    valPre = bufPtrCompressedSubscript[loop-1];
    valNext = bufPtrCompressedSubscript[loop];
    if(identical(valPre,valNext) == 1){
      (*curPtr)=(*(curPtr-1))+1;
    }

    ++curPtr;
  }

  // 3rd traverse: from last to first to determine the maximum frequency
  MAXFREQUENCY = 0;
  loop = FREQUENCYNUMBER - 1;
  curPtr = bufPtrFrequency + loop;;
  for(;loop > 0; --loop){
    unsigned long frequency = (* curPtr);
    if(frequency > MAXFREQUENCY){
      MAXFREQUENCY = frequency;
    }
    --curPtr;
  }

  /*
   * debug
   */
  /*
  curPtr = bufPtrFrequency;
  for(loop=0;loop<FREQUENCYNUMBER;++loop){
    printf("%ld\t%ld\n",loop,(*curPtr));
    ++curPtr;
  }
  printf("maximum frequency = %ld\n",MAXFREQUENCY);
  */

  return;
}

void statisticsFCWH(){
  FCWHSTATISTICSNUMBER = MAXFREQUENCY + 1;
  bufPtrFcwhStatistics = (unsigned long int *)malloc(FCWHSTATISTICSNUMBER * 4);

  unsigned long int loop;
  unsigned long int * curPtr;

  // traverse bufPtrFcwhStatistics: from first to last, initialize to 0
  curPtr = bufPtrFcwhStatistics;
  for(loop=0;loop<FCWHSTATISTICSNUMBER;++loop){
    (*curPtr) = 0;
    ++curPtr;
  }

  curPtr = bufPtrFrequency;
  for(loop=0;loop<FREQUENCYNUMBER - 1;++loop){
    if(bufPtrFrequency[loop + 1] == 0){
      bufPtrFcwhStatistics[bufPtrFrequency[loop]] += bufPtrFrequency[loop] + 1;
    }
  }
  bufPtrFcwhStatistics[bufPtrFrequency[loop]] += bufPtrFrequency[loop] + 1;
  
  /*
   * output for matlab.
   */
  curPtr = bufPtrFcwhStatistics;
  for(loop=0;loop<FCWHSTATISTICSNUMBER;++loop){
    printf("%ld ",(*curPtr));
    ++curPtr;
  }

  return;
}

void freeMemoryAllocated(){
  if(bufPtrFile != NULL){
    free(bufPtrFile);
    bufPtrFile = NULL;
  }
  if(bufPtrRawFCWH != NULL){
    free(bufPtrRawFCWH);
    bufPtrRawFCWH = NULL;
  }
  if(bufPtrSortedSubscript != NULL){
    free(bufPtrSortedSubscript);
    bufPtrSortedSubscript = NULL;
  }
  if(bufPtrCompressedSubscript != NULL){
    free(bufPtrCompressedSubscript);
    bufPtrCompressedSubscript = NULL;
  }
  if(bufPtrFrequency != NULL){
    free(bufPtrFrequency);
    bufPtrFrequency = NULL;
  }
  if(bufPtrFcwhStatistics != NULL){
    free(bufPtrFcwhStatistics);
    bufPtrFcwhStatistics = NULL;
  }

  return;
}

/*------------------------------------END-----------------------------------*/
