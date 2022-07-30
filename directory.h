#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include "fsLow.h"

#define MAXFILENAME 32

typedef enum {dir,file,nf} LASTELEMENT;

typedef struct dirEntry{
	time_t dateCreated;
	time_t dateModified;
	int fileSize; //size in bytes
	int location; //block location
	int isDir; //1 if dir, 0 if file
	int inUse; //1 if in use
	char fileName[MAXFILENAME];
}dirEntry;

typedef struct parsedPath {
    int isPath; //0 if not complete path or 1 if is
    LASTELEMENT lastElement; //0 = dir, 1 = file, 2 = not found
    dirEntry* parentDir; //ptr to parent directory
    dirEntry* curDir; //pointer to last directory
    int index; //index of file or dir in parent directory
    char lastElementName[MAXFILENAME]; //name of the last element in path
}parsedPath;

typedef struct fileInfo {
	int fileSize; //Filesize
	int location; //block location
	char fileName[MAXFILENAME]; //files name
	parsedPath* pPath; //path info for file returned from parsePath()
}fileInfo;


dirEntry* root; //root directory

//amount of spots in a dirEntry, size in bytes of dir entry,
//block location of root, size of blocks
int dirEntries, dirSize, rootLocation, dir_blockSize;

dirEntry* loadDir(int location, int fileSize);
dirEntry* loadRoot();
void initDir(dirEntry* newDir);
int removeDir(dirEntry* parent, int index);
int isEmpty(dirEntry* dir);