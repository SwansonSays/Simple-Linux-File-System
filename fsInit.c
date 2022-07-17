/**************************************************************
* Class:  CSC-415-0# Fall 2021
* Names: 
* Student IDs:
* GitHub Name:
* Group Name:
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

typedef struct volumeControlBlock{
int blockSize; //Size of the blocks
int totalBlockCount;   //Total volume
int freeBlocks; // Number of free blocks
int bitMapLocation; //Location to the bitmap
int bitMapBlocks;        // Number of blocks within the Bitmap
int RootDirectory;        // Location of the Root Directory
long Signature;              // Checks to see if the Volume Control Block is valid

}volumeControlBlock;
int freeSpaceSize; // put in VCB struct
unsigned char* freeSpaceMap; //put in VCB struct


void initVCB(){
	volumeControlBlock * vcb = malloc(sizeof(volumeControlBlock));
	printf("this is sparta : %ld", sizeof(volumeControlBlock));
	vcb->blockSize = 512; //Size of the blocks
	vcb->totalBlockCount = 19531;   //Total volume
	vcb->freeBlocks= freeSpaceSize; // Number of free blocks
	vcb->bitMapLocation = 1; //Location to the bitmap
	vcb->bitMapBlocks = 1;       // Number of blocks within the Bitmap
	vcb->RootDirectory = 0;      // Location of the Root Directory
	vcb->Signature = 0x416C6C69736F6E41;
	LBAread(vcb,1,0);
	LBAwrite(vcb,1,0);

}


void setBit(unsigned char* map, int i)
	{
		map[i/8] |= 1 << (i % 8);
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
	for(int i = 0; i < freeSpaceSize; i++)
		{
		setBit(freeSpaceMap, i);
		}

	LBAwrite(freeSpaceMap, freeSpaceSize, 1);
	return freeSpaceSize + 1;
	}

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */

	int firstFreeBlock = initBitMap(numberOfBlocks,blockSize); // put in VCB init function
	return 0;
	}
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	}


