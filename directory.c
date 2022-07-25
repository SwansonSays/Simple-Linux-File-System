#include "directory.h"

dirEntry* loadDir(dirEntry dir) {
    
    dirEntry* lDir = malloc(dirSize);
    LBAread(lDir,(dir.fileSize % 512) + 1, dir.location);
    /*
    for(int i = 0; i < dirEntries; i++) {
        printf("IN DIR.FILENAME[%s] NAME: [%s]\n", dir.fileName,lDir[i].fileName);
    }
    */
    /*
    for(int i = 0; i < dirEntries; i++) {
        lDir[i].dateCreated = dir.dateCreated;
        lDir[i].dateModified = dir.dateModified;
        lDir[i].fileSize = dir.fileSize;
        lDir[i].location = dir.location;
        lDir[i].isDir = dir.isDir;
        lDir[i].inUse = dir.inUse;
        strcpy(lDir[i].fileName, dir.fileName);
    }
    */
    
    return lDir;
}