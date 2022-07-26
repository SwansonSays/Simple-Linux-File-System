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

dirEntry* root;
int dirEntries, dirSize, rootLocation;

dirEntry* loadDir(dirEntry);
dirEntry* loadRoot();
void initDir(dirEntry* newDir);