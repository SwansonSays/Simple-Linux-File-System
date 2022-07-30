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
	int fileSize;
	int location;
	int isDir;
	int inUse;
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
	int fileSize;
	int location;
	char fileName[MAXFILENAME];
	parsedPath* pPath;
}fileInfo;


dirEntry* root;
int dirEntries, dirSize, rootLocation;

dirEntry* loadDir(int location, int fileSize);
dirEntry* loadRoot();
void initDir(dirEntry* newDir);
int removeDir(dirEntry* parent, int index);
int isEmpty(dirEntry* dir);