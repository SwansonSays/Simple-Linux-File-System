#include "directory.h"

//loads directory at location with fileSize
dirEntry* loadDir(int location, int fileSize) {
    dirEntry* lDir = malloc(dirSize);
    LBAread(lDir,(fileSize / dir_blockSize), location);
    return lDir;
}
//loads root directory to memory
dirEntry* loadRoot() {
    dirEntry* dir = malloc(dirSize);
    LBAread(dir,(dirSize / dir_blockSize), rootLocation);
    //printf("blocks to read[%d]\n",dirSize/512);
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

//remloves directory
int removeDir(dirEntry* parent, int index) {
    char* blank = "";
    //printf ("inRemoveDir parent name [%s]\n", parent[index].fileName);
    //printf ("inRemoveDir parent location [%d]\n", parent[index].location);
    //printf ("inRemoveDir index in parent [%d]\n", index);

    //sets directory entry in parent to unused state
    strcpy(parent[index].fileName, blank);
    parent[index].fileSize = 0;
    parent[index].dateCreated = 0;
    parent[index].dateModified = 0;
    parent[index].location = -1;
    parent[index].isDir = -1;
    parent[index].inUse = 0;

    //write parent dir to disk
    char* writeDir = (char*) parent;
    //printf("LOCATION TO WRITE [%d]\n", parent[0].location);
    LBAwrite(writeDir,dirSize/dir_blockSize, parent[0].location);
    return 1;
}

//checks if dir is empty excludeing '.' and '..'
int isEmpty(dirEntry* dir) {
    //iterate through entries skipping '.' and '..'
    for(int i = 2; i < dirEntries; i++) {
        if(dir[i].inUse == 1) {
            return 0; // return 0 if directory entry is in use
        }
    }
    return 1; // return 1 if empty besides '.' and '..'
}


