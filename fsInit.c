/**************************************************************
* Class:  CSC-415-01 Summer 2022
* Names: Robert Swanson, Kevin Islas Orgaz
* Student IDs:917106793, 921260116
* GitHub Name: csc415-filesystem-SwansonSays
* Group Name:We didn't get that far
* Project: Basic File System
*
* File: fsInit.c
*
* Description: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/


#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"

#define MAXFILENAME 32
#define MINDIRENTRIES 50

typedef struct volumeControlBlock{
int blockSize; //Size of the blocks
int totalBlockCount;   //Total volume
int freeBlocks; // Number of free blocks
int bitMapLocation; //Location to the bitmap
int bitMapBlocks;        // Number of blocks within the Bitmap
int RootDirectory;        // Location of the Root Directory
long Signature;              // Checks to see if the Volume Control Block is valid

}volumeControlBlock;

typedef struct dirEntry{
	time_t dateCreated;
	time_t dateModified;
	int fileSize;
	int location;
	int isDir;
	int inUse;
	char fileName[MAXFILENAME];
}dirEntry;

int freeSpaceSize; //size of bitMap
unsigned char* freeSpaceMap; 
volumeControlBlock * vcb;

void setBit(unsigned char* map, int i)
	{
		map[i/8] |= 1 << (i % 8);
	}

int getBit(unsigned char* map, int i) {
	return map[i/8] & (1 << (i % 8)) != 0;
}

int getFree(int blocksNeeded) {
	int freeCount, firstFree = 0;
	for(int i = 0; i < vcb->totalBlockCount; i++) {
		if(getBit(freeSpaceMap, i) == 0) {
			if(freeCount == 0) {
				firstFree = i;
			}
			freeCount++;
			if(freeCount == blocksNeeded) {
				return firstFree;
			}
		}
	}
}


int initBitMap(uint64_t numberOfBlocks, uint64_t blockSize)
	{
	//number of blocks needed for freespacemap
	freeSpaceSize = ((numberOfBlocks / 8) / blockSize) + 1;
    freeSpaceMap = (unsigned char*) malloc(freeSpaceSize * blockSize);
    
	//set all bitmap values to 0
	for(int i = 0; i < freeSpaceSize; i++)
		{
        
		freeSpaceMap[i] = 0;
        //printf("IN INIT MAP AT POS [%d] = [%d]\n", i, freeSpaceMap[i]);
		}
    
	//set bits as used for VCB and Free Space Managment
	for(int i = 0; i < freeSpaceSize + 1; i++)
		{
		setBit(freeSpaceMap, i);
		}

	LBAwrite(freeSpaceMap, freeSpaceSize, 1);
	return freeSpaceSize + 1;
	}

int initDir(uint64_t blockSize) {
	int blocksNeeded, bytesNeeded, bytesLeftOver, dirLeftOver, dirEntries = 0;
	int rootLocation = 0;
	

	bytesNeeded = (MINDIRENTRIES * sizeof(dirEntry));
	printf("BYTES NEEDED: [%d]\n", bytesNeeded);
	bytesLeftOver = bytesNeeded % blockSize;
	printf("BYTES LEFTOVER: [%d]\n", bytesLeftOver);
	dirLeftOver = bytesLeftOver / sizeof(dirEntry);
	dirEntries = MINDIRENTRIES + dirLeftOver;
	printf("DIR LEFTOVER: [%d]\n", dirLeftOver);
	printf("DIR ENTRY COUNT: [%d]\n", dirEntries);
	bytesNeeded += dirLeftOver * sizeof(dirEntry);
	blocksNeeded = (bytesNeeded / blockSize) + 1;
	printf("BLOCKS NEEDED: [%d]\n", blocksNeeded);
	/*
	dirEntry* root[blocksNeeded * blockSize];
	for (int i = 0; i < sizeof(root)/sizeof(dirEntry); i++) {
		root[i] = malloc(sizeof(dirEntry));
	}
	*/
	dirEntry* root = malloc(blocksNeeded * blockSize);

	rootLocation = getFree(blocksNeeded);
	for(int i = 0; i < blocksNeeded; i++) {
		setBit(freeSpaceMap, i + rootLocation);
	}
	
	LBAwrite(freeSpaceMap, freeSpaceSize, 1);

	strcpy(root[0].fileName, ".");
	root[0].fileSize = blocksNeeded * blockSize;
	root[0].dateCreated = time(0);
	root[0].dateModified = time(0);
	root[0].location = rootLocation;
	root[0].isDir = 1;
	root[0].inUse = 1;
	strcpy(root[1].fileName, "..");
	root[1].fileSize = blocksNeeded * blockSize;
	root[1].dateCreated = time(0);
	root[1].dateModified = time(0);
	root[1].location = rootLocation;
	root[1].isDir = 1;
	root[1].inUse = 1;
	printf("FileName[%s], FIleSize[%d], DateCreated[%ld], DateModified[%ld], location[%d]\n",root[0].fileName,root[0].fileSize,root[0].dateCreated,root[0].dateModified,root[0].location);
	printf("FileName[%s], FIleSize[%d], DateCreated[%ld], DateModified[%ld], location[%d]\n",root[1].fileName,root[1].fileSize,root[1].dateCreated,root[1].dateModified,root[1].location);
	printf("DIR ENTRY SIZE[%ld]\n", sizeof(dirEntry));
	char* writeRoot = (char*) root;

	LBAwrite(writeRoot, blocksNeeded, rootLocation); 
	//printf("name %ld\n", sizeof(root[0]->fileName));
	return rootLocation;
}

void initVCB(uint64_t numberOfBlocks, uint64_t blockSize){
	vcb = malloc(sizeof(volumeControlBlock));
	vcb->blockSize = 512; //Size of the blocks
	vcb->totalBlockCount = 19531;   //Total volume
	vcb->freeBlocks= vcb->totalBlockCount - initBitMap(numberOfBlocks, blockSize); // Number of free blocks
	vcb->bitMapLocation = 1; //Location to the bitmap
	vcb->bitMapBlocks = freeSpaceSize;       // Number of blocks within the Bitmap
	vcb->RootDirectory = initDir(blockSize);      // Location of the Root Directory
	vcb->Signature = 0x6e6f74666172;
	//LBAread(vcb,1,0);
	LBAwrite(vcb,1,0);

}

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */

	initVCB(numberOfBlocks,blockSize);
	return 0;
	}
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	}


