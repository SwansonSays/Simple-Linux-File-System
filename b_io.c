/**************************************************************
* Class:  CSC-415-0# Fall 2021
* Names: Robert Swanson, Kevin Islas Orgaz
* Student IDs:917106793, 921260116
* GitHub Name:csc415-filesystem-SwansonSays
* Group Name:We didn't get that far
* Project: Basic File System
*
* File: b_io.c
*
* Description: Basic File System - Key File I/O Operations
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"
#include "mfs.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512

typedef struct b_fcb
	{
	/** TODO add al the information you need in the file control block **/
	char * buf;		//holds the open file buffer
	int index;		//holds the current position in the buffer
	int buflen;		//holds how many valid bytes are in the buffer
	fileInfo* fi;	//holds file info
	int flags;		//holds flags ie RD_ONLY, WR_ONLY
	int fileOffset;	//holds the current position in the file
	int currentBlk;	//holds current block
	int numBlocks; //hold the number of blocks in the file
	} b_fcb;
	
b_fcb fcbArray[MAXFCBS];

int startup = 0;	//Indicates that this has not been initialized

//Method to initialize our file system
void b_init ()
	{
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{
		fcbArray[i].buf = NULL; //indicates a free fcbArray
		}
		
	startup = 1;
	}

//Method to get a free FCB element
b_io_fd b_getFCB ()
	{
	for (int i = 0; i < MAXFCBS; i++)
		{
		if (fcbArray[i].buf == NULL)
			{
			return i;		//Not thread safe (But do not worry about it for this assignment)
			}
		}
	return (-1);  //all in use
	}
	
// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
b_io_fd b_open (char * filename, int flags)
	{
	b_io_fd returnFd;
	fileInfo* fi;
	//*** TODO ***:  Modify to save or set any information needed
	//
	//

	if (startup == 0) b_init();  //Initialize our system

	returnFd = b_getFCB();				// get our own file descriptor
	if(returnFd == -1) {				// check for error - all used FCB's
		return -1;
	}		
	fi = getFileInfo(filename, flags);
	if(fi == NULL) {
		return -2;
	}					
	
	fcbArray[returnFd].fi = fi;
	fcbArray[returnFd].buf = malloc(fs_blockSize);
	fcbArray[returnFd].buflen = 0;
	fcbArray[returnFd].index = 0;
	fcbArray[returnFd].flags = flags;
	fcbArray[returnFd].fileOffset = 0;
	fcbArray[returnFd].currentBlk = 0;
	fcbArray[returnFd].numBlocks = (fi->fileSize + (fs_blockSize - 1)) / fs_blockSize;

	return (returnFd);						// all set
	}


// Interface to seek function	
int b_seek (b_io_fd fd, off_t offset, int whence)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
		
	if(whence == SEEK_SET) {
		fcbArray[fd].fileOffset = offset;
	} else if(whence == SEEK_CUR) {
		fcbArray[fd].fileOffset += offset;
	} else if(whence == SEEK_END) {
		fcbArray[fd].fileOffset = fcbArray[fd].fi->fileSize + offset;
	} else {
		return -1;
	}	
	return (fcbArray[fd].fileOffset); //Change this
	}



// Interface to write function	
int b_write (b_io_fd fd, char * buffer, int count)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
	if(fcbArray[fd].flags == 0) {
		printf("Cannot write to read only file\n");
		return -1;
	}

	int bytesWritten = 0;

	//write full blocks to disk
	while(count > fs_blockSize) {
		memcpy(fcbArray[fd].buf, buffer + fcbArray[fd].fileOffset, fs_blockSize);
		LBAwrite(fcbArray[fd].buf, 1, fcbArray[fd].fi->location + (bytesWritten / fs_blockSize));
		fcbArray[fd].fileOffset += fs_blockSize;
		count = count - fcbArray[fd].fileOffset;
		bytesWritten += fs_blockSize;
	}

	bytesWritten += count;
	//write leftover blocks to disk
	memcpy(fcbArray[fd].buf, buffer + fcbArray[fd].fileOffset, count);
	LBAwrite(fcbArray[fd].buf,1,fcbArray[fd].fi->location + (bytesWritten / fs_blockSize));
		
	setFileSize(fcbArray[fd].fi, bytesWritten);

	return (bytesWritten); //Change this
	}



// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill 
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read (b_io_fd fd, char * buffer, int count)
	{
	int bytesRead;
	int bytesReturned;
	int part1, part2, part3;
	int numberOfBlocksToCopy;
	int remainingBytesInMyBuffer;

	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}
	
	if (fcbArray[fd].fi == NULL) {
		return -1;
	}

	if(fcbArray[fd].flags & (1 << 0)) {
		printf("Cannot write to read only file\n");
		return -1;
	}

	remainingBytesInMyBuffer = fcbArray[fd].buflen - fcbArray[fd].index;	
	int amountDelivered = (fcbArray[fd].currentBlk * fs_blockSize) - remainingBytesInMyBuffer;
	
	if((count + amountDelivered) > fcbArray[fd].fi->fileSize) {
		count = fcbArray[fd].fi->fileSize - amountDelivered;

		if(count < 0) {
			return -1;
		}
	}

	//Part 1
	if(remainingBytesInMyBuffer >= count) {
		part1 = count;
		part2 = 0;
		part3 = 0;
	} else {
		part1 = remainingBytesInMyBuffer;

		part3 = count - remainingBytesInMyBuffer;

		numberOfBlocksToCopy = part3 / fs_blockSize;
		part2 = numberOfBlocksToCopy * fs_blockSize;

		part3 = part3 - part2;
	}

	if (part1 > 0) {
		memcpy(buffer, fcbArray[fd].buf + fcbArray[fd].index, part1);
		fcbArray[fd].index += part1;
	}

	if(part2 > 0) {
		bytesRead = LBAread(buffer+part1, numberOfBlocksToCopy, fcbArray[fd].currentBlk + fcbArray[fd].fi->location) * fs_blockSize;
		fcbArray[fd].currentBlk += numberOfBlocksToCopy;
		part2 = bytesRead;
	}

	if(part3 > 0) {
		bytesRead = LBAread(fcbArray[fd].buf, 1, fcbArray[fd].currentBlk + fcbArray[fd].fi->location) * fs_blockSize;
		fcbArray[fd].currentBlk += 1;

		fcbArray[fd].index = 0;
		fcbArray[fd].buflen = bytesRead;

		if(bytesRead < part3) {
			part3 = bytesRead;
		}
		if(part3 > 0) {
			memcpy(buffer + part1 + part2, fcbArray[fd].buf + fcbArray[fd].index, part3);
			fcbArray[fd].index = fcbArray[fd].index + part3;
		}
	}
	bytesReturned = part1 + part2 + part3;

	return (bytesReturned);
	}
	
// Interface to Close the file	
int b_close (b_io_fd fd)
	{
		free(fcbArray[fd].fi);
		free(fcbArray[fd].buf);
		fcbArray[fd].buf = NULL;
	}
