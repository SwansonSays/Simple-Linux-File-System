#include "directory.h"

dirEntry* loadDir(int location, int fileSize) {
    dirEntry* lDir = malloc(dirSize);
    LBAread(lDir,(fileSize / 512), location);
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

int removeDir(dirEntry* parent, int index) {
    char* blank = "";
    printf ("inRemoveDir parent name [%s]\n", parent->fileName);
    printf ("inRemoveDir parent location [%d]\n", parent->location);
    strcpy(parent[index].fileName, blank);
    parent[index].fileSize = 0;
    parent[index].dateCreated = 0;
    parent[index].dateModified = 0;
    parent[index].location = -1;
    parent[index].isDir = -1;
    parent[index].inUse = 0;
    // free bits
    char* writeDir = (char*) parent;
    printf("LOCATION TO WRITE [%d]\n", parent[0].location);
    LBAwrite(writeDir,dirSize/512, parent[0].location);
    return 1;
}