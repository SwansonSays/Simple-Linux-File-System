#include "mfs.h"
#include "directory.h"

#define MAXFILEPATH 4096

typedef enum {dir,file,nf} LASTELEMENT;

typedef struct parsedPath {
    int isPath;
    LASTELEMENT lastElement;
    dirEntry* parentDir;
    dirEntry* curDir;
    int index;
}parsedPath;

dirEntry* cwd;
char cwdPath[MAXFILEPATH];

int initmfs() {
    if (fs_setcwd("/.") == 0) {
        return 0;
    }
    return 1;
}

parsedPath* parsePath(char* path) {
    char* token;
    parsedPath* pPath = malloc(sizeof(parsedPath));
    pPath->curDir = root;

    token = strtok(path, "/");
    while(token) {
        //printf("TOKEN: [%s]\n", token);
        pPath->lastElement = nf;
        for(int i = 0; i < dirEntries; i++) {
            //printf("i[%d] FILENAME [%s]\n", i, pPath->curDir[i].fileName);
            if(strcmp(token, pPath->curDir[i].fileName) == 0) {
                //printf("TOKEN[%s] Matches FILENAME[%s]\n",token, pPath->curDir[i].fileName);
                pPath->index = i;
                if(pPath->curDir[i].isDir) {
                    pPath->lastElement = dir;
                    pPath->curDir = loadDir(pPath->curDir[i]);
                } else {
                    pPath->lastElement = file;
                }
            }
        }
        token = strtok(NULL, "/");
    }
    return pPath;
}
//add checking to make sure cwdPath is absolute
char* fs_getcwd(char *buf, size_t size) {
    if(strlen(cwdPath) > size) {
        return NULL;
    }
    //printf("PATH [%s]\n",cwdPath);
    return strcpy(buf, cwdPath);
}

int fs_setcwd(char *buf) {
    parsedPath* pPath = parsePath(buf);
    if(pPath->lastElement == dir) {
        strcpy(cwdPath, buf);
        cwd = pPath->curDir;
        printf("Moved to [%s]\n",cwdPath);
        return 0;
    }
    printf("move failed\n");
    return -1;
}