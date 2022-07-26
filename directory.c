#include "directory.h"

dirEntry* loadDir(dirEntry dir) {
    dirEntry* lDir = malloc(dirSize);
    LBAread(lDir,(dir.fileSize / 512), dir.location);
    return lDir;
}

dirEntry* loadRoot() {
    dirEntry* dir = malloc(dirSize);
    LBAread(dir,(dirSize / 512), rootLocation);
    printf("blocks to read[%d]\n",dirSize/512);
    return dir;
}

// Sets new directory to initialized state
void initDir(dirEntry* newDir) {
    char* blank = "";
    for(int i = 0; i < dirEntries; i++) {
        strcpy(newDir[i].fileName, blank);
        newDir[i].fileSize = 0;
        newDir[i].dateCreated = 0;
        newDir[i].dateModified = 0;
        newDir[i].location = -1;
        newDir[i].isDir = -1;
        newDir[i].inUse = 0;
    }
}