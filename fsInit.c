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
#include <time.h>

#include "fsLow.h"
#include "directory.h"
#include "freeSpace.h"

#define MAXFILENAME 32
#define MINDIRENTRIES 50
#define SIGNATURE 0x6e6f74666172

typedef struct volumeControlBlock{
int blockSize; //Size of the blocks
int totalBlockCount;   //Total volume
int freeBlocks; // Number of free blocks
int bitMapLocation; //Location to the bitmap
int bitMapBlocks;        // Number of blocks within the Bitmap
int RootDirectory;        // Location of the Root Directory
long Signature;              // Checks to see if the Volume Control Block is valid

}volumeControlBlock;

volumeControlBlock * vcb;

int initBitMap(uint64_t numberOfBlocks, uint64_t blockSize)
	{
	//number of blocks needed for freespacemap
	freeSpaceSize = ((numberOfBlocks / 8) / blockSize) + 1;
    freeSpaceMap = (unsigned char*) malloc(freeSpaceSize * blockSize);
    totalBlockCount = vcb->totalBlockCount;
	fs_blockSize = blockSize;
	//set all bitmap values to 0
	for(int i = 0; i < freeSpaceSize; i++)
		{
		freeSpaceMap[i] = 0;
		}
    
	//set bits as used for VCB and Free Space Managment
	for(int i = 0; i < freeSpaceSize + 1; i++)
		{
		setBit(freeSpaceMap, i);
		}

	LBAwrite(freeSpaceMap, freeSpaceSize, 1);
	return freeSpaceSize + 1;
	}

int initRoot(uint64_t blockSize) {
	int blocksNeeded = 0, bytesNeeded = 0, bytesLeftOver = 0, dirLeftOver = 0;
	dirEntries = 0, dirSize = 0, rootLocation = 0, dir_blockSize = 0;

	//Calculates amount of entries in each directory based on blockSize of system
	bytesNeeded = (MINDIRENTRIES * sizeof(dirEntry));
	bytesLeftOver = bytesNeeded % blockSize;
	dirLeftOver = bytesLeftOver / sizeof(dirEntry);
	dirEntries = MINDIRENTRIES + dirLeftOver;
	bytesNeeded += dirLeftOver * sizeof(dirEntry);
	blocksNeeded = (bytesNeeded / blockSize) + 1;
	dir_blockSize = blockSize;
	dirSize = blocksNeeded * blockSize;

	root = malloc(dirSize);
	const char* blank = "";
	//sets each directory space in root to initilised state
	for(int i = 0; i < dirEntries; i++) {
		strcpy(root[i].fileName, blank);
		root[i].fileSize = 0;
		root[i].dateCreated = 0;
		root[i].dateModified = 0;
		root[i].location = -1;
		root[i].isDir = -1;
		root[i].inUse = 0;
	}

	//gets free location for root from freepsace management
	//and sets blocks needed as used
	rootLocation = getFree(blocksNeeded);
	for(int i = 0; i < blocksNeeded; i++) {
		setBit(freeSpaceMap, i + rootLocation);
	}
	LBAwrite(freeSpaceMap, freeSpaceSize, 1);

	//initializes '.' and '..' entry for root
	strcpy(root[0].fileName, ".");
	root[0].fileSize = dirSize;
	root[0].dateCreated = time(0);
	root[0].dateModified = time(0);
	root[0].location = rootLocation;
	root[0].isDir = 1;
	root[0].inUse = 1;
	strcpy(root[1].fileName, "..");
	root[1].fileSize = dirSize;
	root[1].dateCreated = time(0);
	root[1].dateModified = time(0);
	root[1].location = rootLocation;
	root[1].isDir = 1;
	root[1].inUse = 1;
	
	//wrties root to disk
	char* writeRoot = (char*) root;
	LBAwrite(writeRoot, blocksNeeded, rootLocation); 
	return rootLocation;
}

void initVCB(uint64_t numberOfBlocks, uint64_t blockSize){
	vcb = malloc(blockSize);
	vcb->blockSize = blockSize; //Size of the blocks
	vcb->totalBlockCount = numberOfBlocks;   //Total volume
	vcb->freeBlocks= vcb->totalBlockCount - initBitMap(numberOfBlocks, blockSize); // Number of free blocks
	vcb->bitMapLocation = 1; //Location to the bitmap
	vcb->bitMapBlocks = freeSpaceSize;       // Number of blocks within the Bitmap
	vcb->RootDirectory = initRoot(blockSize);      // Location of the Root Directory
	vcb->Signature = SIGNATURE;

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


