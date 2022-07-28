#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include "fsLow.h"

#define MAXFILENAME 32

typedef struct dirEntry{
	time_t dateCreated;
	time_t dateModified;
	int fileSize;
	int location;
	int isDir;
	int inUse;
	char fileName[MAXFILENAME];
}dirEntry;

typedef struct fileInfo {
	int fileSize;
	int location;
	char fileName[MAXFILENAME];
}fileInfo;

dirEntry* root;
int dirEntries, dirSize, rootLocation;

dirEntry* loadDir(int location, int fileSize);
dirEntry* loadRoot();
void initDir(dirEntry* newDir);
int removeDir(dirEntry* parent, int index);
int isEmpty(dirEntry* dir);