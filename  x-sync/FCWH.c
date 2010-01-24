#include "FCWH.h"

/*---------------------------------------------------------------------------*/

int main(int argc, char *argv[]){

/*==========================STEP 1===============================*/

  contiSemiHash01();
  heapSortBufPtrSemiHashValue();
  calcHashValueRepetition();

/*==========================STEP 2===============================*/

//  generateLandmarks();

/*==========================STEP 3===============================*/

//  generateSSS();

/*==========================STEP 4===============================*/

// generateScraps();

/*==========================STEP 5===============================*/

//  generateTT();
//  outputDeltaSize();
/*==========================STEP 6===============================*/

//  recoverFileNew();

//  test();

  freeMemoryAllocated();
  return 0;
}

/*---------------------FUNCTIONS--------------------------------*/

/*====================AUXILIARY FUNCTIONS========================*/

unsigned short int isBigEndian(){
  unsigned short int retVal;
  unsigned long int li;

  unsigned char * ptrChar;

  ptrChar = (unsigned char *)(&li);
  (*ptrChar) = 0x01;
  ++ptrChar;
  (*ptrChar) = 0x02;
  ++ptrChar;
  (*ptrChar) = 0x03;
  ++ptrChar;
  (*ptrChar) = 0x04;

  if(li == 0x01020304){
    retVal = 1;
  }else{
    retVal = 0;
  }

  return retVal;
}

void outputFileInfo(unsigned short int fileNo){
  if(fileNo == 1){
    printf("file size: %ld\n",(unsigned long int)fileSize);
  }else{
    printf("file size: %ld\n",(unsigned long int)fileSizeNew);
  }
  printf("file name: %s\n",testedFile[fileNo-1]);
  return;
}

void swapTwo8Bytes(\
		   void * ptr1,\
		   void * ptr2){
  unsigned long int li1, li2;
  unsigned long int * ptrLi1;
  unsigned long int * ptrLi2;
  ptrLi1 =(unsigned long int *)ptr1;
  ptrLi2 =(unsigned long int *)ptr2;
  li1 = (*ptrLi1);
  li2 = (*(ptrLi1+1));
  (*ptrLi1) = (*ptrLi2);
  (*(ptrLi1 + 1)) = (*(ptrLi2 + 1));
  (*ptrLi2) = li1;
  (*(ptrLi2 + 1)) = li2;
  return;
}

// N.B.: startSubscript counts from 0.
void calcCurrentSHV(\
		    char * ptrWhichFile,\
		    unsigned long int startSubscript,\
		    unsigned long int * retSHV){

  char * curPtr = ptrWhichFile + startSubscript;
  unsigned long int * ptrBufFile4Bytes = (unsigned long int *)curPtr;

  register unsigned long int hashValueCurrent = 0;

  unsigned long int loop;
  for(loop = 0; loop < BLOCKSIZEQUARTER; loop++){
    hashValueCurrent ^= (*ptrBufFile4Bytes);
    ++ptrBufFile4Bytes;
  }
  (*retSHV) = hashValueCurrent;

  return;
}

void calcNextSHV(\
		 char * ptrWhichFile,\
		 unsigned long int startSubscript,\
		 unsigned long int * retSHV){

  // isBigEndian()==0;
  // i.e., ascending buffer 01-02-03-04 will be interpretted as 
  // unsigned long int 0x04030201.

  unsigned long int shvTail,shvHead;

  shvTail = shvHead = (* retSHV);

  // remove the first byte of the old SHV,
  // and obtain the first 3 bytes of the new SHV
  // by means of shift.
  shvTail >>= 8;

  // obtain the last byte of the new SHV,
  // whose position is at the 1st byte of unsigned long int.
  shvHead <<= 24;
  shvHead >>=24;
  unsigned char charHV;
  charHV = (unsigned char)shvHead;
  charHV ^= (*(ptrWhichFile + startSubscript));
  charHV ^= (*(ptrWhichFile + startSubscript + BLOCKSIZE));
  shvHead = (unsigned long int)charHV;
  shvHead <<= 24;

  (*retSHV) = shvHead | shvTail;

  return;
}

/*====================STEP 1=====================================*/
/*GENERATE THE CONTINUOUS SEMI-HASH VALUES OF THE REFERENCE FILE.*/

// open the file of reference version
void openFileReference(){

  fd = open(testedFile[0],O_RDONLY);
  fstat(fd,&statFile);
  fileSize = statFile.st_size;
  bufPtrFile = malloc(fileSize);
  read(fd,bufPtrFile,fileSize);

  return;
}

unsigned int calcCSHVNumber(){
  return (fileSize-BLOCKSIZE+SPANLENGTH)/SPANLENGTH;
}

int mallocBufPtrSemiHashValue(){
  if(fileSize < BLOCKSIZE){
    return -1;
  }
  CSHVnumber =  calcCSHVNumber();

  bufPtrSemiHashValueSize =\
    (size_t)(CSHVnumber * (HASHVALUELENGTH + SUBSCRIPTLENGTH));
  bufPtrSemiHashValue = malloc(bufPtrSemiHashValueSize);
 
  return 0;
}

// close the file of reference version
void closeFileReference(){

  close(fd);

  return;
}

void contiSHV(\
	      char * ptrBufFileStartingPoint,\
	      char * ptrBufSHVsStartingPoint,\
	      unsigned long int startSubscript,\
	      unsigned long int originalSHV,\
	      unsigned long int iterationNumber){

  unsigned long int * ptrLiFile;
  unsigned long int * ptrLiSHVs;
  unsigned long int subscript;
  register unsigned long int currentSHV;
  unsigned long int loop;

  ptrLiFile = (unsigned long int *)ptrBufFileStartingPoint;
  ptrLiSHVs = (unsigned long int *)ptrBufSHVsStartingPoint;
  currentSHV = originalSHV;
  subscript = startSubscript;

  (*ptrLiSHVs) = currentSHV;
  ++ptrLiSHVs;
  (*ptrLiSHVs) = subscript;

  for(loop = 1; loop < iterationNumber; ++loop){
    ++ptrLiSHVs;
    currentSHV ^= (*ptrLiFile)^(*(ptrLiFile + BLOCKSIZEQUARTER));
    (*ptrLiSHVs) = currentSHV;
    ++ptrLiSHVs;
    subscript += 4;
    (*ptrLiSHVs) = subscript;
    ++ptrLiFile;
  }

  return;
}

// continuous semi-hash for the reference file
int contiSemiHash01(){

  openFileReference();
  closeFileReference();
  /*
  outputFileInfo(1);
  */
  if(mallocBufPtrSemiHashValue()==-1){
    return -1;
  }

  char * ptrCharFile = bufPtrFile;
  unsigned long int SHV0;
  calcCurrentSHV(bufPtrFile, 0, &SHV0);
  unsigned long int SHV1 = SHV0;
  calcNextSHV(ptrCharFile, 0, &SHV1);
  unsigned long int SHV2 = SHV1;
  calcNextSHV(ptrCharFile, 1, &SHV2);
  unsigned long int SHV3 = SHV2;
  calcNextSHV(ptrCharFile, 2, &SHV3);

  char * ptrBufFileStartingPoint;
  char * ptrBufSHVsStartingPoint;
  unsigned long int iterationNumber;
  unsigned long int startSubscript;

  ptrBufFileStartingPoint = bufPtrFile;
  ptrBufSHVsStartingPoint = bufPtrSemiHashValue;
  iterationNumber = (fileSize - BLOCKSIZE + HASHVALUELENGTH)\
    / HASHVALUELENGTH;
  startSubscript = 0;
  
  contiSHV(ptrBufFileStartingPoint, ptrBufSHVsStartingPoint,\
	   startSubscript, SHV0, iterationNumber);

  ptrBufFileStartingPoint = bufPtrFile + 1;
  ptrBufSHVsStartingPoint += iterationNumber * 8;
  iterationNumber = (fileSize - 1 - BLOCKSIZE + HASHVALUELENGTH)\
    / HASHVALUELENGTH;
  startSubscript = 1;
  
  contiSHV(ptrBufFileStartingPoint, ptrBufSHVsStartingPoint,\
	   startSubscript, SHV1, iterationNumber);

  ptrBufFileStartingPoint = bufPtrFile + 2;
  ptrBufSHVsStartingPoint += iterationNumber * 8;
  iterationNumber = (fileSize - 2 - BLOCKSIZE + HASHVALUELENGTH)\
    / HASHVALUELENGTH;
  startSubscript = 2;
  
  contiSHV(ptrBufFileStartingPoint, ptrBufSHVsStartingPoint,\
	   startSubscript, SHV2, iterationNumber);

  ptrBufFileStartingPoint = bufPtrFile + 3;
  ptrBufSHVsStartingPoint += iterationNumber * 8;
  iterationNumber = (fileSize - 3 - BLOCKSIZE + HASHVALUELENGTH)\
    / HASHVALUELENGTH;
  startSubscript = 3;
  
  contiSHV(ptrBufFileStartingPoint, ptrBufSHVsStartingPoint,\
	   startSubscript, SHV3, iterationNumber);

  return 0;
}

// pos : floor(n/2)...1. 
// here n is the global variable, CSHVnumber.
// To output result ascendingly,
// it's needed to build a max heap.
void minHeapSieve(unsigned long int pos,unsigned long int posEnd){

  unsigned long int * ptrLiBufHead = (unsigned long int * )bufPtrSemiHashValue;
  unsigned long int curValue = (*(ptrLiBufHead+2*(pos-1)));
  unsigned long int curSubscript = (*(ptrLiBufHead + 2*(pos-1) + 1));

  unsigned long int insertedPos = pos;
  unsigned long int posLeftChild = insertedPos * 2;
  unsigned long int posRightChild;
  unsigned long int posChild;

  unsigned int finished = 0;
  while((finished == 0)&&(posLeftChild <= posEnd)){

    posChild = posLeftChild;
    posRightChild = posLeftChild + 1;
    if((posRightChild <= posEnd) && \
       ((*(ptrLiBufHead + 2*(posRightChild - 1))) > (*(ptrLiBufHead + 2*(posLeftChild -1))))){
      posChild = posRightChild;
    }

    if(curValue >= (*(ptrLiBufHead + 2*(posChild -1)))){
      finished = 1;
    }else{
      (*(ptrLiBufHead + 2*(insertedPos - 1))) = (*(ptrLiBufHead + 2*(posChild -1)));
      (*(ptrLiBufHead + 2*(insertedPos - 1) + 1)) = (*(ptrLiBufHead + 2*(posChild -1) + 1));
      insertedPos = posChild;
      posLeftChild = insertedPos * 2;
    }
  }

  (*(ptrLiBufHead + 2*(insertedPos -1))) = curValue;
  (*(ptrLiBufHead + 2*(insertedPos -1) + 1)) =  curSubscript;

  return;
}

void heapSortBufPtrSemiHashValue(){
  unsigned long int * ptrLiBufHead = (unsigned long int * )bufPtrSemiHashValue;
  unsigned long int pos;
  pos = CSHVnumber/2;
  for(;pos >= 1; --pos){
    minHeapSieve(pos,CSHVnumber);
  }
  for(pos = CSHVnumber; pos >= 2; --pos){
    swapTwo8Bytes(ptrLiBufHead, ptrLiBufHead + 2*(pos - 1));
    minHeapSieve(1,pos -1);
  }

  return;
}

// change the occrences of frequency 1, 2, 3, 4, 5.
void modifyLeastOccurence(\
			  unsigned long int frequency){
  switch(frequency){
  case 1:
    ++occurrenceOnce;
    break;
  case 2:
    ++occurrenceTwice;
    break;
  case 3:
    ++occurrence3Times;
    break;
  case 4:
    ++occurrence4Times;
    break;
  case 5:
    ++occurrence5Times;
    break;
  default:
    break;
  }
  return;
}

// change the top 5 of frequentest SHVs and related frequencies.
void shiftFrequentestSHV(unsigned short int shiftPos){
  switch(shiftPos){
  case 1:
    frequencyTop5 = frequencyTop4;
    shvFrequentest5 = shvFrequentest4;
    frequencyTop4 = frequencyTop3;
    shvFrequentest4 = shvFrequentest3;
    frequencyTop3 = frequencyTop2;
    shvFrequentest3 = shvFrequentest2;
    frequencyTop2 = frequencyTop1;
    shvFrequentest2 = shvFrequentest1;
    break;
  case 2:
    frequencyTop5 = frequencyTop4;
    shvFrequentest5 = shvFrequentest4;
    frequencyTop4 = frequencyTop3;
    shvFrequentest4 = shvFrequentest3;
    frequencyTop3 = frequencyTop2;
    shvFrequentest3 = shvFrequentest2;
    break;
  case 3:
    frequencyTop5 = frequencyTop4;
    shvFrequentest5 = shvFrequentest4;
    frequencyTop4 = frequencyTop3;
    shvFrequentest4 = shvFrequentest3;
    break;
  case 4:
    frequencyTop5 = frequencyTop4;
    shvFrequentest5 = shvFrequentest4;
    break;
  default:
    break;
  }
  return;
}

void changeFrequentestSHV(\
			  unsigned long int frequency,\
			  unsigned long int semiHashValue){
  if(frequency >= frequencyTop1){
    shiftFrequentestSHV(1);
    frequencyTop1 = frequency;
    shvFrequentest1 = semiHashValue;
  }else{
    if(frequency >= frequencyTop2){
      shiftFrequentestSHV(2);
      frequencyTop2 = frequency;
      shvFrequentest2 = semiHashValue;
    }else{
      if(frequency >= frequencyTop3){
	shiftFrequentestSHV(3);
	frequencyTop3 = frequency;
	shvFrequentest3 = semiHashValue;
      }else{
	if(frequency >= frequencyTop4){
	  shiftFrequentestSHV(4);
	  frequencyTop4 = frequency;
	  shvFrequentest4 = semiHashValue;
	}else{
	  if(frequency > frequencyTop5){
	    frequencyTop5 = frequency;
	    shvFrequentest5 = semiHashValue;
	  }
	}
      }
    }
  }
  return;
}

void statisticsSHV(\
		   unsigned long int frequency,\
		   unsigned long int semiHashValue){
  modifyLeastOccurence(frequency);
  changeFrequentestSHV(frequency,semiHashValue);

  return;
}

void outputSHVstatistics(){
  printf("****************************************\n\n");

  printf("There are %ld SHVs, including identical SHVs.\n",CSHVnumber);
  printf("There are %ld SHVs repetitions.\n",hashValueRepetitionCounter);
  printf("There are %ld distinct SHVs.\n\n",cshvNumberDiff);

  printf("Top 5 frequentest SHVs and respective frequencies:\n");
  printf("\tNo. 1 :\t%lx occurs %ld times.\n",shvFrequentest1,frequencyTop1);
  printf("\tNo. 2 :\t%lx occurs %ld times.\n",shvFrequentest2,frequencyTop2);
  printf("\tNo. 3 :\t%lx occurs %ld times.\n",shvFrequentest3,frequencyTop3);
  printf("\tNo. 4 :\t%lx occurs %ld times.\n",shvFrequentest4,frequencyTop4);
  printf("\tNo. 5 :\t%lx occurs %ld times.\n\n",shvFrequentest5,frequencyTop5);

  printf("Occurence of SHVs that repeats 1~5 times:\n");
  printf("\tRepetition of 1 occurs %ld times.\n",occurrenceOnce);
  printf("\tRepetition of 2 occurs %ld times.\n",occurrenceTwice);
  printf("\tRepetition of 3 occurs %ld times.\n",occurrence3Times);
  printf("\tRepetition of 4 occurs %ld times.\n",occurrence4Times);
  printf("\tRepetition of 5 occurs %ld times.\n\n",occurrence5Times);

  printf("****************************************\n");

  return;
}

int calcHashValueRepetition(){
  if(bufPtrSemiHashValue == NULL){
    return -1;
  }

  occurrenceOnce = 0;
  occurrenceTwice = 0;
  occurrence3Times = 0;
  occurrence4Times = 0;
  occurrence5Times = 0;

  shvFrequentest1 = 0;
  shvFrequentest2 = 0;
  shvFrequentest3 = 0;
  shvFrequentest4 = 0;
  shvFrequentest5 = 0;

  frequencyTop1 = 0;
  frequencyTop2 = 0;
  frequencyTop3 = 0;
  frequencyTop4 = 0;
  frequencyTop5 = 0;

  hashValueRepetitionCounter = 0;
  cshvNumberDiff = 1;

  unsigned long int * ptrLiBufFirst = (unsigned long int *)bufPtrSemiHashValue;
  unsigned long int * ptrLiBufNext;
  ptrLiBufNext = ptrLiBufFirst + 2;

  unsigned long int curRepetition = 1;

  unsigned long int loop;
  for(loop = 1; loop < CSHVnumber; ++loop){
    if((*ptrLiBufFirst)==(*ptrLiBufNext)){
      ++hashValueRepetitionCounter;
      ++curRepetition;
    }else{
      statisticsSHV(curRepetition,(*ptrLiBufFirst));

      // printf("%lx\toccurs %ld times.\n",(*ptrLiBufFirst), curRepetition);
      curRepetition = 1;
      if(loop < CSHVnumber){
	++cshvNumberDiff;
      }
    }
    ptrLiBufFirst += 2;
    ptrLiBufNext += 2;
  }
  
  //outputSHVstatistics();
  return 0;
}

/*==========================STEP 2===============================*/
/*======================OBTAIN LANDMARKS==========================*/

void determineLandmarkSHVoccurence(){
  unsigned long int halfCSHVnumber = (unsigned long int)(cshvNumberDiff / 2) ;
  unsigned long int totalCandidateCSHVs = 0;
  LandmarkNumber = 0;
  totalCandidateCSHVs += occurrenceOnce;
  LandmarkNumber += occurrenceOnce;
  if(totalCandidateCSHVs >= halfCSHVnumber){
    landmarkSHVoccurence = 1;
    return;
  }
  totalCandidateCSHVs += occurrenceTwice;
  LandmarkNumber += 2*occurrenceTwice;
  if(totalCandidateCSHVs >= halfCSHVnumber){
    landmarkSHVoccurence = 2;
    return;
  }
  totalCandidateCSHVs += occurrence3Times + occurrence4Times+occurrence5Times;
  LandmarkNumber += 3*occurrence3Times;
  LandmarkNumber += 4*occurrence4Times;
  LandmarkNumber += 5*occurrence5Times;
  if(totalCandidateCSHVs >= halfCSHVnumber){
    landmarkSHVoccurence = 3;
    return;
  }
  landmarkSHVoccurence = 4;    
  return;
}

void mallocBufPtrLandmarks(){
  bufPtrLandmarksSize = LandmarkNumber * (HASHVALUELENGTH + SUBSCRIPTLENGTH);
  if(bufPtrLandmarks == NULL){
    free(bufPtrLandmarks);
    bufPtrLandmarks = NULL;
  }

  bufPtrLandmarks = (char *)malloc(bufPtrLandmarksSize);
  return;
}

void genLandmarks1(){
  mallocBufPtrLandmarks();

  unsigned long int * ptrLiBufFirst = (unsigned long int *)bufPtrSemiHashValue;
  unsigned long int * ptrLiBufNext;
  ptrLiBufNext = ptrLiBufFirst + 2;
  unsigned long int * ptrLiDrawback;

  unsigned long int * ptrLiBufLandMark = (unsigned long int * )bufPtrLandmarks;

  unsigned long int curRepetition = 1;

  unsigned long int loop;
  for(loop = 1; loop < CSHVnumber; ++loop){
    if((*ptrLiBufFirst)==(*ptrLiBufNext)){
      ++hashValueRepetitionCounter;
      ++curRepetition;
    }else{
      if(curRepetition == 1){
	ptrLiDrawback = ptrLiBufFirst;
	(*ptrLiBufLandMark) = (*ptrLiDrawback);
	++ptrLiBufLandMark;
	++ptrLiDrawback;
	(*ptrLiBufLandMark) = (*ptrLiDrawback);
	++ptrLiBufLandMark;
      }else{
	curRepetition = 1;
      }
    }
    ptrLiBufFirst += 2;
    ptrLiBufNext += 2;
  }
  return;
}

void genLandmarks2(){
  mallocBufPtrLandmarks();
  unsigned long int * ptrLiBufFirst = (unsigned long int *)bufPtrSemiHashValue;
  unsigned long int * ptrLiBufNext;
  ptrLiBufNext = ptrLiBufFirst + 2;
  unsigned long int * ptrLiDrawback;

  unsigned long int * ptrLiBufLandMark = (unsigned long int * )bufPtrLandmarks;

  unsigned long int curRepetition = 1;

  unsigned long int loop;
  unsigned short int innerLoop;
  for(loop = 1; loop < CSHVnumber; ++loop){
    if((*ptrLiBufFirst)==(*ptrLiBufNext)){
      ++hashValueRepetitionCounter;
      ++curRepetition;
    }else{
      switch(curRepetition){
      case 1:
	ptrLiDrawback = ptrLiBufFirst;
	(*ptrLiBufLandMark) = (*ptrLiDrawback);
	++ptrLiBufLandMark;
	++ptrLiDrawback;
	(*ptrLiBufLandMark) = (*ptrLiDrawback);
	++ptrLiBufLandMark;
	break;
      case 2:
	ptrLiDrawback = ptrLiBufFirst;
	ptrLiDrawback -= 2;
	for(innerLoop = 0; innerLoop < 2; ++innerLoop){
	  (*ptrLiBufLandMark) = (*ptrLiDrawback);
	  ++ptrLiBufLandMark;
	  ++ptrLiDrawback;
	  (*ptrLiBufLandMark) = (*ptrLiDrawback);
	  ++ptrLiBufLandMark;
	  ++ptrLiDrawback;
	}
	break;
      default:
	curRepetition = 1;
	break;
      }
    }
    ptrLiBufFirst += 2;
    ptrLiBufNext += 2;
  }
  return;
}

void genLandmarks3(){
  mallocBufPtrLandmarks();
  unsigned long int * ptrLiBufFirst = (unsigned long int *)bufPtrSemiHashValue;
  unsigned long int * ptrLiBufNext;
  ptrLiBufNext = ptrLiBufFirst + 2;
  unsigned long int * ptrLiDrawback;

  unsigned long int * ptrLiBufLandMark = (unsigned long int * )bufPtrLandmarks;

  unsigned long int curRepetition = 1;

  unsigned long int loop;
  unsigned short int innerLoop;
  for(loop = 1; loop < CSHVnumber; ++loop){
    if((*ptrLiBufFirst)==(*ptrLiBufNext)){
      ++hashValueRepetitionCounter;
      ++curRepetition;
    }else{
      if(curRepetition <= 5){
	ptrLiDrawback = ptrLiBufFirst;
	ptrLiDrawback -= (curRepetition - 1)*2;
	for(innerLoop = 0; innerLoop < curRepetition; ++innerLoop){
	  (*ptrLiBufLandMark) = (*ptrLiDrawback);
	  ++ptrLiBufLandMark;
	  ++ptrLiDrawback;
	  (*ptrLiBufLandMark) = (*ptrLiDrawback);
	  ++ptrLiBufLandMark;
	  ++ptrLiDrawback;
	}
      }
      curRepetition = 1;
    }
    ptrLiBufFirst += 2;
    ptrLiBufNext += 2;
  }
  return;
}

void genLandmarks4(){

  bufPtrLandmarks = bufPtrSemiHashValue;
  bufPtrLandmarksSize = bufPtrSemiHashValueSize;
  return;
}

void substitueLandmarksForBufSHV(){
  if((landmarkSHVoccurence >=1) && (landmarkSHVoccurence <= 3)){
    if(bufPtrSemiHashValue != NULL){
      free(bufPtrSemiHashValue);
      bufPtrSemiHashValue = NULL;
    }
  }
  return;
}

void generateLandmarks(){
  LandmarkNumber = 0;
  determineLandmarkSHVoccurence();
  switch(landmarkSHVoccurence){
  case 1:
    genLandmarks1();
    break;
  case 2:
    genLandmarks2();
    break;
  case 3:
    genLandmarks3();
    break;
  case 4:
    genLandmarks4();
    break;
  default:
    break;
  }
  substitueLandmarksForBufSHV();

  return;
}

/*==========================STEP 3===============================*/
/*====================OBTAIN SAME SUBSTRINGS=====================*/

// open the file of new version
void openFileNew(){
  fdNew = open(testedFile[1],O_RDONLY);
  fstat(fdNew,&statFile);
  fileSizeNew = statFile.st_size;
  bufPtrFileNew = malloc(fileSizeNew);
  bufPtrFileNewSize = fileSizeNew;
  read(fdNew,bufPtrFileNew,fileSizeNew);
  return;
}

// close the file of new version
void closeFileNew(){
  close(fdNew);
  return;
}

void cmpFilesBiDirection(){

  headSubscriptRefFile = 0;
  tailSubscriptRefFile = (unsigned long int)(fileSize -1);
  headSubscriptNewFile = 0;
  tailSubscriptNewFile = (unsigned long int)(fileSizeNew -1);

  headSSSlength = 0;
  tailSSSlength = 0;

  while((headSubscriptRefFile <= tailSubscriptRefFile)\
     && (headSubscriptNewFile <= tailSubscriptNewFile)){
     if(bufPtrFile[headSubscriptRefFile] \
	== bufPtrFileNew[headSubscriptNewFile]){
       ++headSubscriptRefFile;
       ++headSubscriptNewFile;
       ++headSSSlength;
     }else{
       break;
     }
  }

  while((headSubscriptRefFile <= tailSubscriptRefFile)\
     && (headSubscriptNewFile <= tailSubscriptNewFile)){
     if(bufPtrFile[tailSubscriptRefFile] \
	== bufPtrFileNew[tailSubscriptNewFile]){
       --tailSubscriptRefFile;
       --tailSubscriptNewFile;
       ++tailSSSlength;
     }else{
       break;
     }
  }

  return;
}

void appendListSSS(\
		   struct sameSubString sss){

  unsigned long int oldSize = bufPtrSameSubStringSize;
  bufPtrSameSubStringSize += sizeof(struct sameSubString);

  if(bufPtrSameSubString == NULL){
    bufPtrSameSubString = (char *)malloc(bufPtrSameSubStringSize);
  }else{
    bufPtrSameSubString = (char *)realloc\
      (bufPtrSameSubString,bufPtrSameSubStringSize);
  }

  unsigned long int * ptrLi;
  ptrLi = (unsigned long int *)(bufPtrSameSubString + oldSize);

  (*ptrLi) = sss.compressedVersionPosition;
  ++ptrLi;
  (*ptrLi) = sss.referenceVersionPosition;
  ++ptrLi;
  (*ptrLi) = sss.strLength;

  ++sssNumber;

  return;
}

void insertFirstSSS(){
  if(headSSSlength > 0){
    struct sameSubString sss;
    sss.compressedVersionPosition = 0;
    sss.referenceVersionPosition = 0;
    sss.strLength = headSSSlength;
    appendListSSS(sss);
  }
  return;
}

void searchSHVbackward(\
		       unsigned long int shv,\
		       unsigned long int * ptrLi){

  unsigned long int * headPtrLi;
  headPtrLi = (unsigned long int *)bufPtrLandmarks;

  unsigned long int * aPtrLi;
  aPtrLi = ptrLi;
  while(sameSHVnumber < 5){
    if(aPtrLi == headPtrLi){
      return;
    }
    aPtrLi -= 2;
    if((*aPtrLi)==shv){
      sameSHVsubscript[sameSHVnumber] = *(ptrLi+1);
      ++sameSHVnumber;
      if(aPtrLi == headPtrLi){
	break;
      }
    }else{
      break;
    }
  }
  return;
}

void searchSHVforward(\
		      unsigned long int shv,\
		      unsigned long int * ptrLi){
  unsigned long int * tailPtrLi;
  tailPtrLi = (unsigned long int *)\
    (bufPtrLandmarks + bufPtrLandmarksSize\
     - HASHVALUELENGTH - SUBSCRIPTLENGTH);

  unsigned long int * aPtrLi;
  aPtrLi = ptrLi;
  while(sameSHVnumber < 5){
    aPtrLi += 2;
    if(aPtrLi == tailPtrLi){
      return;
    }
    if((*aPtrLi)==shv){
      sameSHVsubscript[sameSHVnumber] = *(ptrLi+1);
      ++sameSHVnumber;
      if(aPtrLi == tailPtrLi){
	break;
      }
    }else{
      break;
    }
  }
  return;
}

unsigned short int binarySearchSHV(\
		     unsigned long int shv){

  unsigned short int retVal = 0;
  sameSHVnumber = 0;
  if((landmarkSHVoccurence >= 1) \
     && (landmarkSHVoccurence <= 3)\
     && (cshvNumberDiff >= 10)){
    if((shv == shvFrequentest1)||\
       (shv == shvFrequentest2)||\
       (shv == shvFrequentest3)||\
       (shv == shvFrequentest4)||\
       (shv == shvFrequentest5)){
      return retVal; 
    }
  }

  unsigned long int leftSubscript;
  unsigned long int rightSubscript;
  unsigned long int midSubscript;
  unsigned long int * ptrLi;
  unsigned long int * basePtrLi;
  basePtrLi = (unsigned long int *)bufPtrLandmarks;

  // search the same semi-hash value.
  leftSubscript = 0;
  rightSubscript = LandmarkNumber - 1;
  while(leftSubscript <= rightSubscript){
    midSubscript = (leftSubscript + rightSubscript)/2;
    ptrLi = basePtrLi +  2*midSubscript;

    if((*ptrLi)==shv){
      sameSHVsubscript[sameSHVnumber] = *(ptrLi+1);
      retVal = 1;
      ++sameSHVnumber;
      break;
    }else{
      if((*ptrLi) < shv){
	if(midSubscript < rightSubscript){// condition of right movement!
	  leftSubscript = midSubscript + 1;
	}else{
	  break;
	}
      }else{
	if(leftSubscript < midSubscript){// condition of left movement!
	  rightSubscript = midSubscript - 1;
	}else{
	  break;
	}
      }
    }
  }

  if((sameSHVnumber > 0) && (landmarkSHVoccurence != 1)){
    searchSHVbackward(shv,ptrLi);
    searchSHVforward(shv,ptrLi);
  }

  return retVal;
}

unsigned short int identicalSubStrings(\
				       unsigned char * ptrRef,\
				       unsigned char * ptrCmpr){
  unsigned long int loop;
  for(loop = 0; loop < BLOCKSIZE; ++loop){
    if((*ptrRef) == (*ptrCmpr)){
      ++ptrRef;
      ++ptrCmpr;
    }else{
      return 0;
    }
  }
  return 1;
}

unsigned char identicalPlateau(\
			       unsigned long int compressedVersionPosition,\
			       unsigned long int referenceVersionPosition,\
			       unsigned long int strLength){
  unsigned char sssOK = 1;

  unsigned char * ptrCharRef;
  unsigned char * ptrCharCmpr;
  unsigned long loop;

  ptrCharCmpr = (unsigned char *)(bufPtrFileNew + compressedVersionPosition);
  ptrCharRef = (unsigned char *)(bufPtrFile + referenceVersionPosition);

  for(loop = 0; loop < strLength; ++loop){
    if((*ptrCharRef)==(*ptrCharCmpr)){
      ++ptrCharRef;
      ++ptrCharCmpr;
    }else{
      sssOK = 0;
      break;
    }
  }

  return sssOK;
}

void expandSSS(\
	       struct sameSubString * sss){

  char * ptrBufRefCmp;
  char * ptrBufCmprCmp;

  // NOTICE: the following 2 variables are signed!!!
  long int expandRefLoop;
  long int expandCmprLoop;

  // backward expansion
  unsigned long int lengthBackward = 0;
  ptrBufRefCmp = bufPtrFile + \
    sss->referenceVersionPosition;
  ptrBufCmprCmp = bufPtrFileNew + \
    sss->compressedVersionPosition;

  for(expandRefLoop = sss->referenceVersionPosition,\
	expandCmprLoop = sss->compressedVersionPosition;
	(expandRefLoop > 0) && (expandCmprLoop > 0);\
	--expandRefLoop, --expandCmprLoop){
    --ptrBufRefCmp;
    --ptrBufCmprCmp;
    if((*ptrBufRefCmp) == (*ptrBufCmprCmp)){
      ++lengthBackward;
    }else{
      break;
    }
  }

  // forward expansion
  unsigned long int lengthForward = 0;
  ptrBufRefCmp = bufPtrFile + \
    sss->referenceVersionPosition + BLOCKSIZE - 1;
  ptrBufCmprCmp = bufPtrFileNew + \
    sss->compressedVersionPosition + BLOCKSIZE - 1;

  for(expandRefLoop = sss->referenceVersionPosition + BLOCKSIZE - 1,
	expandCmprLoop = sss->compressedVersionPosition + BLOCKSIZE - 1;
	(expandRefLoop < fileSize - 1) && (expandCmprLoop < fileSizeNew - 1);\
	++expandRefLoop, ++expandCmprLoop){
    ++ptrBufRefCmp;
    ++ptrBufCmprCmp;
    if((*ptrBufRefCmp) == (*ptrBufCmprCmp)){
      ++lengthForward;
    }else{
      break;
    }
  }

  sss->compressedVersionPosition -= lengthBackward;
  sss->referenceVersionPosition -= lengthBackward;
  sss->strLength += lengthForward + lengthBackward;

  return;
}

void tackleBorderOverlapping(){

  if(sssOverlappingNumber > 0){
    unsigned short int loop;
    unsigned short int foremostTag, hindermostTag;
    unsigned long int foremostCompressedVersionPosition;
    unsigned long int hindermostCompressedVersionPosition;
    unsigned long int foremostPositionNew;
    unsigned long int hindermostPositionNew;

    foremostCompressedVersionPosition\
      = sssOverlapping[0].compressedVersionPosition;
    hindermostCompressedVersionPosition\
      = sssOverlapping[0].compressedVersionPosition\
       + sssOverlapping[0].strLength - 1;
    foremostTag = 0;
    hindermostTag = 0;

    for(loop = 1; loop < sssOverlappingNumber; ++loop){
      foremostPositionNew\
	= sssOverlapping[loop].compressedVersionPosition;
      if(foremostCompressedVersionPosition > foremostPositionNew){
	foremostCompressedVersionPosition = foremostPositionNew;
	foremostTag = loop;
      }
      hindermostPositionNew\
	= sssOverlapping[loop].compressedVersionPosition\
	+ sssOverlapping[loop].strLength - 1;
      if(hindermostCompressedVersionPosition < hindermostPositionNew){
	hindermostCompressedVersionPosition = hindermostPositionNew;
	hindermostTag = loop;
      }
    }

    forwardBufPtrFileNewSubscript = hindermostCompressedVersionPosition + 1;

    appendListSSS(sssOverlapping[foremostTag]);
    if(foremostTag != hindermostTag){
      unsigned long int hindermostShiftLength;
      struct sameSubString newHinderSSS;

      hindermostShiftLength = \
	(sssOverlapping[hindermostTag].compressedVersionPosition\
	+ sssOverlapping[hindermostTag].strLength)\
	- (sssOverlapping[foremostTag].compressedVersionPosition\
	+ sssOverlapping[foremostTag].strLength);

      newHinderSSS.compressedVersionPosition = \
	sssOverlapping[foremostTag].compressedVersionPosition\
	+ sssOverlapping[foremostTag].strLength;
      newHinderSSS.referenceVersionPosition = \
	sssOverlapping[hindermostTag].referenceVersionPosition\
	+ hindermostShiftLength;
      newHinderSSS.strLength = hindermostShiftLength;

      appendListSSS(newHinderSSS);
    }
  }

  return;
}

void checkSSSs(){

  if(sameSHVnumber > 0){

    unsigned short int loop;
    unsigned char * ptrRef;
    unsigned char * ptrCmpr;
    ptrCmpr = (unsigned char *)(bufPtrFileNew + forwardBufPtrFileNewSubscript);

    sssOverlappingNumber = 0;
    for(loop = 0; loop < sameSHVnumber; ++loop){
      ptrRef =  (unsigned char *)(bufPtrFile +  sameSHVsubscript[loop]);
      if(identicalSubStrings(ptrRef,ptrCmpr) == 1){

	struct sameSubString sss;
	sss.compressedVersionPosition = forwardBufPtrFileNewSubscript;
	sss.referenceVersionPosition = sameSHVsubscript[loop];
	sss.strLength = BLOCKSIZE;
	expandSSS(&sss);
	sssOverlapping[sssOverlappingNumber].compressedVersionPosition\
	  = sss.compressedVersionPosition;
	sssOverlapping[sssOverlappingNumber].referenceVersionPosition\
	  = sss.referenceVersionPosition;
	sssOverlapping[sssOverlappingNumber].strLength\
	  = sss.strLength;
	++sssOverlappingNumber;
      }
    }
    tackleBorderOverlapping();
  }

  return;
}

void findInsertSSSs(){

  if(fileSizeNew < BLOCKSIZE){
    return;
  }

  unsigned short int foundSHV = 0;
  unsigned short int foundIdenticalString = 0;
  unsigned long int sssStartPoint;
  unsigned long int sssLength;

  unsigned long int lowerBound = 0;
  unsigned long int upperBound = fileSizeNew - BLOCKSIZE;
  unsigned long int currentSHV;

  unsigned char * ptrRef;
  unsigned char * ptrCmpr;
  struct sameSubString sss;

  calcCurrentSHV(bufPtrFileNew,lowerBound,&currentSHV);

  for(; lowerBound <= upperBound; ){
    foundSHV = binarySearchSHV(currentSHV);
    if(foundSHV == 0){
      if(lowerBound + 1 <= upperBound){
	calcNextSHV(bufPtrFileNew, lowerBound, &currentSHV);
	++lowerBound;
      }else{
	return;
      }
    }else{
      ptrRef = (unsigned char *)(bufPtrFile + sameSHVsubscript[0]);
      ptrCmpr = (unsigned char *)(bufPtrFileNew + lowerBound);
      foundIdenticalString = identicalSubStrings(ptrRef,ptrCmpr);
      if(foundIdenticalString == 0){
	if(lowerBound + 1  <= upperBound){
	  calcNextSHV(bufPtrFileNew, lowerBound, &currentSHV);
	  ++lowerBound;
	}else{
	  return;
	}
      }else{
	sss.compressedVersionPosition = lowerBound;
	sss.referenceVersionPosition = sameSHVsubscript[0];
	sss.strLength = BLOCKSIZE;
	expandSSS(&sss);
	appendListSSS(sss);
	sssStartPoint = sss.compressedVersionPosition;
	sssLength = sss.strLength;

	lowerBound = sssStartPoint + sssLength;
	if(lowerBound <= upperBound){
	  calcCurrentSHV(bufPtrFileNew,lowerBound,&currentSHV);
	}else{
	  return;
	}
      }
    }
  }
  return;
}

void insertLastSSS(){
  if(tailSSSlength > 0){
    struct sameSubString sss;
    sss.compressedVersionPosition = tailSubscriptNewFile;
    sss.referenceVersionPosition = tailSubscriptRefFile;
    sss.strLength = tailSSSlength;
    appendListSSS(sss);
  }
  return;
}

// generate buffer of same sub-strings
void generateSSS(){
  sssNumber = 0;
  bufPtrSameSubString = NULL;
  bufPtrSameSubStringSize = 0;
  propellingSubscript = 0;

  openFileNew();
  closeFileNew();
  /*
  outputFileInfo(2);
  */

  // cmpFilesBiDirection();

  //  insertFirstSSS();

  findInsertSSSs();

  //  insertLastSSS();

  return;
}

/*==========================STEP 4===============================*/
/*====================GENERATE SCRAPS============================*/

// generate scraps
void generateScraps(){
  return;
}

/*==========================STEP 5===============================*/
/*====================OBTAIN TRANSCRIPT TEMPLATE=================*/

void appendTTidentical(\
		       unsigned long int strLength,\
		       unsigned long int refStartSubscript){

  unsigned long int oldSize = bufPtrTranscriptTemplateSize;
  bufPtrTranscriptTemplateSize += sizeof(unsigned char);
  bufPtrTranscriptTemplateSize += sizeof(unsigned long int) * 2;

  if(bufPtrTranscriptTemplate == NULL){
    bufPtrTranscriptTemplate = (char *)malloc(bufPtrTranscriptTemplateSize);
  }else{
    bufPtrTranscriptTemplate = (char *)realloc\
      (bufPtrTranscriptTemplate,bufPtrTranscriptTemplateSize);
  }

  unsigned char * ptrChar;
  unsigned long int * ptrLi;

  ptrChar = (unsigned char *)(bufPtrTranscriptTemplate + oldSize);
  (*ptrChar) = 0;

  ptrLi = (unsigned long int *)(ptrChar + 1);
  (*ptrLi) = strLength;
  ++ptrLi;
  (*ptrLi) = refStartSubscript;

  ++ttNumber;

  return;
}

void appendTTpeculiar(\
		      unsigned long int strLength,\
		      unsigned long int cmprStartSubscript){

  unsigned long int oldSize = bufPtrTranscriptTemplateSize;
  bufPtrTranscriptTemplateSize += sizeof(unsigned char);
  bufPtrTranscriptTemplateSize += sizeof(unsigned long int);
  bufPtrTranscriptTemplateSize += strLength;

  if(bufPtrTranscriptTemplate == NULL){
    bufPtrTranscriptTemplate = (char *)malloc(bufPtrTranscriptTemplateSize);
  }else{
    bufPtrTranscriptTemplate = (char *)realloc\
      (bufPtrTranscriptTemplate,bufPtrTranscriptTemplateSize);
  }

  unsigned char * ptrChar;
  unsigned char * ptrCharCmpr = (unsigned char *)(bufPtrFileNew + cmprStartSubscript);
  unsigned long int * ptrLi;

  ptrChar = (unsigned char *)(bufPtrTranscriptTemplate + oldSize);
  (*ptrChar) = 1;

  ptrLi = (unsigned long int *)(ptrChar + 1);
  (*ptrLi) = strLength;

  ptrChar += 5;
  unsigned long int loopCp;
  for(loopCp = 0; loopCp < strLength; ++loopCp){
    (*ptrChar) = (*ptrCharCmpr);
    ++ptrChar;
    ++ptrCharCmpr;
  }

  ++ttNumber;

  return;
}

// generate transcript template
void generateTT(){

  bufPtrTranscriptTemplate = NULL;
  bufPtrTranscriptTemplateSize = 0;
  ttNumber = 0;

  unsigned long int sssNumberUsed = 0;
  unsigned long int currentSubscript = 0;

  unsigned long int cmprPosition;
  unsigned long int refPosition;
  unsigned long int sssStrLength;
  unsigned long int peculiarStrLength;

  unsigned long int * ptrLi =(unsigned long int *)bufPtrSameSubString;

  for(; currentSubscript < bufPtrFileNewSize; ++sssNumberUsed){

    if(sssNumberUsed < sssNumber){

      cmprPosition = (*ptrLi);
      ++ptrLi;
      refPosition = (*ptrLi);
      ++ptrLi;
      sssStrLength = (*ptrLi);
      ++ptrLi;

      if(currentSubscript < cmprPosition){
	peculiarStrLength = cmprPosition - currentSubscript;
	appendTTpeculiar(peculiarStrLength,currentSubscript);
	currentSubscript += peculiarStrLength;
      }

      appendTTidentical(sssStrLength, refPosition);
      currentSubscript += sssStrLength;

    }else{
      break;
    }
  }

  if(currentSubscript < bufPtrFileNewSize){
    peculiarStrLength = bufPtrFileNewSize - currentSubscript;
    appendTTpeculiar(peculiarStrLength,currentSubscript);
    currentSubscript += peculiarStrLength;
  }

  return;
}

void outputDeltaSize(){

  printf("The size of the new version is %ld.\n",\
	 (unsigned long int)bufPtrFileNewSize);
  printf("The size of the delta file is %ld.\n",\
	 (unsigned long int)bufPtrTranscriptTemplateSize);
}

/*==========================STEP 6===============================*/
/*====================RECOVER NEW FILE===========================*/

unsigned long int obtainFileLength(){

  unsigned long int retVal = 0;
  unsigned long int loop;
  unsigned char * ptrChar;
  unsigned long int * ptrLi;
  unsigned char ttTag;

  ptrChar = (unsigned char *)bufPtrTranscriptTemplate;
  for(loop = 0; loop < ttNumber; ++loop){
    ttTag = (*ptrChar);
    ptrLi = (unsigned long int *)(ptrChar + 1);
    retVal += (*ptrLi);
    if(ttTag == 0){
      ptrChar += sizeof(unsigned char);
      ptrChar += sizeof(unsigned long int) * 2;
    }else{
      ptrChar += sizeof(unsigned char);
      ptrChar += sizeof(unsigned long int);
      ptrChar += (*ptrLi);
    }
  }

  return retVal; 
}

void writeBufRecovFileIdentical(\
				unsigned long int curBufRecovFileSubscript,\
				unsigned long int refStartSubscript,\
				unsigned long int strLength){

  char * ptrRef;
  char * ptrRecov;
  ptrRef = bufPtrFile + refStartSubscript;
  ptrRecov = bufPtrFileRecovered + curBufRecovFileSubscript;

  unsigned long int loop;
  for(loop = 0; loop < strLength; ++loop){
    (*ptrRecov) = (*ptrRef);
    ++ptrRecov;
    ++ptrRef;
  }

  return;
}

void writeBufRecovFilePeculiar(\
			       unsigned long int curBufRecovFileSubscript,\
			       unsigned long int fromTTSubscript,\
			       unsigned long int strLength){

  char * ptrTT;
  char * ptrRecov;

  ptrTT = bufPtrTranscriptTemplate + fromTTSubscript;

  ptrRecov = bufPtrFileRecovered + curBufRecovFileSubscript;

  unsigned long int loop;
  for(loop = 0; loop < strLength; ++loop){
    (*ptrRecov) = (*ptrTT);
    ++ptrRecov;
    ++ptrTT;
  }

  return;
}

void recoverFileNew(){

  bufPtrFileRecoveredSize = fileSizeRecovered = obtainFileLength();
  bufPtrFileRecovered = (char *)malloc(bufPtrFileRecoveredSize);

  char * ptrCharTT = bufPtrTranscriptTemplate;

  unsigned long int * ptrLi;

  unsigned long int curBufRecovFileSubscript = 0;
  unsigned long int refStartSubscript;
  unsigned long int fromTTSubscript = 0;
  unsigned long int strLength;
  unsigned char ttTag;

  unsigned long int loopAppend;
  for(loopAppend = 0; loopAppend < ttNumber; ++loopAppend){

    ttTag = (*ptrCharTT);
    ptrCharTT += sizeof(unsigned char);
    fromTTSubscript += sizeof(unsigned char);
    ptrLi = (unsigned long int *)(ptrCharTT);
    strLength = (*ptrLi);
    fromTTSubscript += sizeof(unsigned long int);
    ptrCharTT += sizeof(unsigned long int);

    if(ttTag == 0){// process identical part

      ++ptrLi;
      refStartSubscript = (*ptrLi);
      writeBufRecovFileIdentical(curBufRecovFileSubscript, \
				 refStartSubscript, strLength);
      curBufRecovFileSubscript += strLength;
      ptrCharTT += sizeof(unsigned long int);
      fromTTSubscript += sizeof(unsigned long int);

    }else{// process peculiar part

      writeBufRecovFilePeculiar(curBufRecovFileSubscript,\
				fromTTSubscript, strLength);

      curBufRecovFileSubscript += strLength;
      ptrCharTT += strLength;
      fromTTSubscript += strLength;
    }
  }

  char fileRECOVname[64];
  strcpy(fileRECOVname,testedFile[1]);
  strcat(fileRECOVname,".RECOV");
  int fdRECOV;
  fdRECOV = open(fileRECOVname,\
			 O_CREAT|O_RDWR|O_TRUNC);
  write(fdRECOV,bufPtrFileRecovered,bufPtrFileRecoveredSize);
  close(fdRECOV);

  return;
}

void freeMemoryAllocated(){
  if(bufPtrFile != NULL){
    free(bufPtrFile);
    bufPtrFile = NULL;
  }
  if(bufPtrFileNew != NULL){
    free(bufPtrFileNew);
    bufPtrFileNew = NULL;
  }
  if(bufPtrSemiHashValue != NULL){
    free(bufPtrSemiHashValue);
    bufPtrSemiHashValue = NULL;
  }
  if(bufPtrLandmarks != NULL){
    free(bufPtrLandmarks);
    bufPtrLandmarks = NULL;
  }
  if(bufPtrSameSubString != NULL){
    free(bufPtrSameSubString);
    bufPtrSameSubString = NULL;
  }
  if(bufPtrTranscriptTemplate != NULL){
    free(bufPtrTranscriptTemplate);
    bufPtrTranscriptTemplate = NULL;
  }
  if(bufPtrFileRecovered != NULL){
    free(bufPtrFileRecovered);
    bufPtrFileRecovered = NULL;
  }

  return;
}

/*------------------------------------END-----------------------------------*/

void test(){
  return;
}

/*------------------------------------OK-------------------------------------*/
