#include "directory.h"

dirEntry* loadDir(dirEntry dir) {
    dirEntry* lDir = malloc(dirSize);
    lDir->dateCreated = dir.dateCreated;
    lDir->dateModified = dir.dateModified;
    lDir->fileSize = dir.fileSize;
    lDir->location = dir.location;
    lDir->isDir = dir.isDir;
    lDir->inUse = dir.inUse;
    strcpy(lDir->fileName, dir.fileName);
    
    return lDir;
}