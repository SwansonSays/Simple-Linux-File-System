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


int freeSpaceSize; // put in VCB struct
unsigned char* freeSpaceMap; //put in VCB struct

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


