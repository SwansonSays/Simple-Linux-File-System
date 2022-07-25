#include "mfs.h"
#include "directory.h"
typedef enum {dir,file,nf} LASTELEMENT;

typedef struct parsedPath {
    int isPath;
    LASTELEMENT lastElement;
    dirEntry parentDir;
    int index;
}parsedPath;

parsedPath* parsePath(char* path) {
    char* token;
    parsedPath* pPath = malloc(sizeof(parsedPath));
    dirEntry* curDir = root;

    token = strtok(path, "/");
    while(token) {
        pPath->lastElement = nf;
        for(int i = 0; i < dirEntries; i++) {
            if(strcmp(token, curDir[i].fileName)) {
                pPath->index = i;
                if(curDir[i].isDir) {
                    pPath->lastElement = dir;
                    curDir = loadDir(root[i]);
                } else {
                    pPath->lastElement = file;
                }
            }
        }
    }
    return pPath;
}
