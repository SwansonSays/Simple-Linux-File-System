#include "mfs.h"


#define MAXFILEPATH 4096
/*
typedef enum {dir,file,nf} LASTELEMENT;


typedef struct parsedPath {
    int isPath; //0 if not complete path or 1 if is
    LASTELEMENT lastElement; //0 = dir, 1 = file, 2 = not found
    dirEntry* parentDir; //ptr to parent directory
    dirEntry* curDir; //pointer to last directory
    int index; //index of file or dir in parent directory
    char lastElementName[MAXFILENAME]; //name of the last element in path
}parsedPath;
*/
dirEntry* cwd;
char cwdPath[MAXFILEPATH];
int isInit = 0;

int initmfs() {
    printf("----Initializing MFS----\n");
    isInit = 1;
    cwd = malloc(dirSize);
    if (fs_setcwd("/") == 0) {
        printf("----Finished Initializing----\n");
        return 0;
    }
    return 1;
}

void printParsedPath(parsedPath* pPath) {
    printf("----Printing pPath----\n");
    printf("isPath = [%d]\n", pPath->isPath);
    printf("lastElement = [%d]\n", pPath->lastElement);
    printf("parentDir name = [%s]\n", pPath->parentDir[pPath->index].fileName);
    printf("index = [%d]\n", pPath->index);
    printf("Nmae of last element [%s]\n", pPath->lastElementName);
    //printf("curDir adress[%ld]\n", pPath->curDir);
    //printf("parentDir adress[%ld]\n", pPath->parentDir);

    printf("----Finished Printing----\n");
}

void printDir(dirEntry* dir) {
    printf("----Printing Directory----\n");
    for(int i = 0; i < dirEntries; i++) {
        printf("FileName[%s], FIleSize[%d], DateCreated[%ld], DateModified[%ld], location[%d], index[%d]\n",dir[i].fileName,dir[i].fileSize,dir[i].dateCreated,dir[i].dateModified,dir[i].location,i);
    }
    printf("----Finished Printing----\n");
}

char* makeAbsolute(char* path) {
    char newPath[MAXFILEPATH];
    strcpy(newPath, "/");
    printf("in MA\n");

    path = strcat(newPath, path);
    if(strcmp(cwdPath, "/") != 0) {
        path = strcat(cwdPath,path);
    }
    
    printf("MA PATH[%s]", path);
    return path;
}

parsedPath* initpPath() {
    parsedPath* pPath = malloc(sizeof(parsedPath));
    pPath->curDir = malloc(dirSize);
    initDir(pPath->curDir);
    pPath->parentDir = malloc(dirSize);
    initDir(pPath->parentDir);
    return pPath;
}

void freepPath(parsedPath* pPath) {
    //free(pPath->curDir);
    //free(pPath->parentDir);
    free(pPath);
}

parsedPath* parsePath(char* path) {
    //printf("****Started Parsing****\n");
    //printf("***[%s]***\n", path);

    char* token; //token for strtok()
    char* saveptr;
    parsedPath* pPath = initpPath(); //allocate memory for parsedPath struct
    pPath->curDir = loadRoot();

    printf("first char of path[%c]\n",path[0]);
    
    if(path[0] != '/') {
        path = makeAbsolute(path);
    }
    //printf("=====Printing after Loading Root=====\n");
   // printDir(pPath->curDir);
    //printf("=====Printing Finished=====\n");

    token = strtok_r(path, "/", &saveptr); //tokenize path using '/' delim
    while(token) {
        //printf("TOKEN: [%s]\n", token);
        pPath->lastElement = nf; //set last element to not found
        for(int i = 0; i < dirEntries; i++) {
            if(strcmp(token, pPath->curDir[i].fileName) == 0) {
                pPath->index = i;// set index of found file in parent dir
                if(pPath->curDir[i].isDir) {
                    pPath->lastElement = dir; // mark it as a directory
                    pPath->curDir = loadDir(pPath->curDir[i].location, pPath->curDir[i].fileSize); // load found directory as current directory before testing next token in path
                } else {
                    pPath->lastElement = file; // mark is as file
                }
                break;
            } else {
                pPath->lastElement = nf;
            }
        }
        strcpy(pPath->lastElementName, token); //set name of last element in path to token
        //printf("Last element Name [%s] | token [%s]\n", pPath->lastElementName,token);
        token = strtok_r(NULL, "/", &saveptr);
    }
    pPath->parentDir = loadDir(pPath->curDir[1].location, pPath->curDir[1].fileSize);
    if(pPath->lastElement == 2) {
        pPath->isPath = 0; //if last element in path was not found set isPath to 0
    } else {
        pPath->isPath = 1; //if file or dir set to 1;
    }
    //printf("=====Printing after ParsePath=====\n");
    printDir(pPath->curDir);
   // printf("=====Printing Finished=====\n");
    //printf("PATH[%s]\n", path);
    return pPath;
}

//add checking to make sure cwdPath is absolute
//returns the name of the current working directory
char* fs_getcwd(char *buf, size_t size) {
    if(strlen(cwdPath) > size) {
        return NULL;
    }
    //printf("PATH [%s]\n",cwdPath);
    return strcpy(buf, cwdPath);
}

//sets cwd
int fs_setcwd(char *buf) {
    //printf("fs_setcwd() called\n");
    //printf("attmepting to move to [%s]\n", buf);
    char* copy = (char*)malloc(strlen(buf) + 1);
    strcpy(copy, buf);
    parsedPath* pPath = parsePath(copy);
    free(copy);

    //printParsedPath(pPath);
    //if the last element of the path is a directory move cwd to that directory
    if(pPath->lastElement == dir) {
        if(buf[0] != '/') {
        buf = makeAbsolute(buf);
        }
        strcpy(cwdPath, buf);
        cwd = pPath->curDir;
        //printf("Moved to [%s]\n",cwdPath);
        freepPath(pPath);
        return 0;
    }
    printf("move failed\n");
    freepPath(pPath);
    return -1;
}

int fs_mkdir(const char *pathname, mode_t mode) {
    int location = 0;
    char* copy = (char*)malloc(strlen(pathname) + 1);
    strcpy(copy, pathname);
    parsedPath* pPath = parsePath(copy);
    free(copy);
     //printParsedPath(pPath);
    /*
    printf("FINSIHED PARSING IN MKDIR\n");
    printf("Printing parent dir after parsing\n");
    printDir(pPath->parentDir);
    //printParsedPath(pPath);
    */
    if(pPath->lastElement == nf) {
        dirEntry* newDir = malloc(dirSize);
        initDir(newDir); // initilizing each entry in new directory to init state
        
        location = getFree(dirSize / fs_blockSize);
        for(int i = 0; i < dirSize / fs_blockSize; i++) {
            setBit(freeSpaceMap, i + location);
        }
        LBAwrite(freeSpaceMap, freeSpaceSize, 1);
        
       //sets first entry of new directory to . and sets info
        strcpy(newDir[0].fileName, ".");
        newDir[0].fileSize = dirSize;
	    newDir[0].dateCreated = time(0);
	    newDir[0].dateModified = time(0);
	    newDir[0].location = location;
	    newDir[0].isDir = 1;
	    newDir[0].inUse = 1;

        //sets second entry of new directory to parent directorys info for ..
        strcpy(newDir[1].fileName, "..");

	    newDir[1].fileSize = pPath->curDir[0].fileSize;
	    newDir[1].dateCreated = pPath->curDir[0].dateCreated;
	    newDir[1].dateModified = pPath->curDir[0].dateModified;
	    newDir[1].location = pPath->curDir[0].location;
	    newDir[1].isDir = pPath->curDir[0].isDir;
	    newDir[1].inUse = pPath->curDir[0].inUse;

        //creates new directory in parent directory if index is not in use
        for(int i = 0; i < dirEntries; i++) {
            if(pPath->curDir[i].inUse == 0) {
                strcpy(pPath->curDir[i].fileName, pPath->lastElementName);
                pPath->curDir[i].fileSize = dirSize;
                pPath->curDir[i].dateCreated = time(0);
                pPath->curDir[i].dateModified = time(0);
                pPath->curDir[i].location = location;
                pPath->curDir[i].isDir = 1;
                pPath->curDir[i].inUse = 1;
                break;
            }
        }
        /*
        printDir(newDir);
        printDir(pPath->parentDir);
        */
        //writes new dir to disk
        char* writeDir = (char*) newDir;
        LBAwrite(writeDir,dirSize/fs_blockSize, location);
        //writes parent dir to disk
        writeDir = (char*) pPath->curDir;
        LBAwrite(writeDir,dirSize/fs_blockSize, newDir[1].location);
    }
    freepPath(pPath);
}

int fs_isFile(char* path) {
    char* copy = (char*)malloc(strlen(path) + 1);
    strcpy(copy, path);
    parsedPath* pPath = parsePath(copy);
    free(copy);
    if(pPath->lastElement == 1) {
        freepPath(pPath);
        return 1;
    }
    freepPath(pPath);
    return 0;
}

int fs_isDir(char* path) {
    char* copy = (char*)malloc(strlen(path) + 1);
    strcpy(copy, path);
    parsedPath* pPath = parsePath(copy);
    free(copy);
    if(pPath->lastElement == 0) {
        freepPath(pPath);
        return 1;
    }
    freepPath(pPath);
    return 0;
}

int fs_rmdir(const char *pathname) {
    //printf("in rmdir\n");
    char* copy = (char*)malloc(strlen(pathname) + 1);
    strcpy(copy, pathname);
    parsedPath* pPath = parsePath(copy);
    free(copy);

    //printParsedPath(pPath);

    if(pPath->lastElement == 0) {
        if(isEmpty(pPath->curDir)) {
            removeDir(pPath->parentDir, pPath->index);
            for(int i = 0; i < dirSize / fs_blockSize; i++) {
                clearBit(freeSpaceMap, i + pPath->curDir[0].location);
            }
            LBAwrite(freeSpaceMap, freeSpaceSize, 1);
        } else {
            printf("Directory %s is not empty. Directory must be empty to be deleted.\n", pPath->lastElementName);
            return -1;
        }
    }
    return 1;
}

int fs_delete(char* filename) {
    char* copy = (char*)malloc(strlen(filename) + 1);
    strcpy(copy, filename);
    parsedPath* pPath = parsePath(copy);
    free(copy);

    if(pPath->lastElement == 1) {
        removeDir(pPath->parentDir, pPath->index);
        for(int i = 0; i < dirSize / fs_blockSize; i++) {
            clearBit(freeSpaceMap, i + pPath->curDir[0].location);
        }
        LBAwrite(freeSpaceMap, freeSpaceSize, 1);
    }
    return 1;
}

fileInfo* makeFile(parsedPath* pPath) {
    fileInfo* fi;
    int location = 0;
    int index = 0;

    fi = malloc(sizeof(fileInfo));

    location = getFree(1);
    setBit(freeSpaceMap, location);
    LBAwrite(freeSpaceMap, freeSpaceSize, 1);

    printf("IN MAKEFILE\n");
    //printParsedPath(pPath);

    //Finds free entry in parent dir
    for(int i = 0; i < dirEntries; i++) {
        if(pPath->curDir[i].inUse == 0) {
            //set info in parent dir
            strcpy(pPath->curDir[i].fileName, pPath->lastElementName);
            pPath->curDir[i].fileSize = 0;
            pPath->curDir[i].dateCreated = time(0);
            pPath->curDir[i].dateModified = time(0);
            pPath->curDir[i].location = location;
            pPath->curDir[i].isDir = 0;
            pPath->curDir[i].inUse = 1;

            pPath->index = i;
            break;
        }
    }
    //sets fileInfo
    strcpy(fi->fileName,pPath->lastElementName);
    fi->fileSize = 0;
    fi->location = location;
    fi->pPath = pPath;
    //writes file to disk
    /*
    char* writeFile = (char*) file;
    LBAwrite(writeFile, 1, location);
    */

    //write parent directory to disk
    char* writeFile = (char*) pPath->curDir;
    LBAwrite(writeFile, dirSize/fs_blockSize,  pPath->curDir[0].location);

    return fi;
}

fileInfo* getFileInfo(char* path, int flags) {
    fileInfo* fi;
    char* copy = (char*)malloc(strlen(path) + 1);
    strcpy(copy, path);
    parsedPath* pPath = parsePath(copy);
    free(copy);

    printParsedPath(pPath);

    if(pPath->lastElement == file) {
        fi = malloc(sizeof(fileInfo));
        strcpy(fi->fileName,pPath->lastElementName);
        fi->fileSize = pPath->curDir[pPath->index].fileSize;
        fi->location = pPath->curDir[pPath->index].location;
    } else if(pPath->lastElement == file && (flags & (1 << 9))) { //truncates file if found
        fi = malloc(sizeof(fileInfo));
        strcpy(fi->fileName,pPath->lastElementName);
        fi->fileSize = 0;
        fi->location = pPath->curDir[pPath->index].location;
    } else if(pPath->lastElement == nf && (flags & (1 << 6))) { //creates file if not found
        fi = makeFile(pPath);
        printf("Finished makeFile()\n");
    } else {
        fi = NULL;
    }

    return fi;
}

void setFileSize(fileInfo* fi, int size) {
    printf("set file info called size[%d]\n",size);
    fi->pPath->curDir[fi->pPath->index].fileSize = size;

    char* writeFile = (char*) fi->pPath->curDir;
    LBAwrite(writeFile, dirSize/fs_blockSize, fi->pPath->curDir[0].location);
}

int moveDirEntry(char* src, char* dest) {
    char* copy = (char*)malloc(MAXFILEPATH);
    strcpy(copy, src);
    parsedPath* pPath = parsePath(copy);
    strcpy(copy,dest);
    parsedPath* pPathDest = parsePath(copy);
    free(copy);

    printf("SRC\n");
    printParsedPath(pPath);
    printf("DEST\n");
    printParsedPath(pPathDest);
    for(int i = 0; i < dirEntries; i++) {
        if(pPath->parentDir[i].inUse == 0) {
            pPathDest->parentDir[i].dateCreated = pPath->curDir[pPath->index].dateCreated;
            pPathDest->parentDir[i].dateModified = time(0);
            pPathDest->parentDir[i].fileSize = pPath->curDir[pPath->index].fileSize;
            pPathDest->parentDir[i].location = pPath->curDir[pPath->index].location;
            pPathDest->parentDir[i].isDir = pPath->curDir[pPath->index].isDir;
            pPathDest->parentDir[i].inUse = pPath->curDir[pPath->index].inUse;
            strcpy(pPathDest->parentDir[i].fileName, pPath->curDir[pPath->index].fileName);
            break;
        }
    }

    //delete src write but dont freespace manage
    removeDir(pPath->parentDir, pPath->index);
    char* writeFile = (char*) pPath->parentDir;
    LBAwrite(writeFile,dirSize/fs_blockSize, pPath->parentDir[0].location);
    writeFile = (char*) pPathDest->parentDir;
    LBAwrite(writeFile,dirSize/fs_blockSize, pPathDest->parentDir[0].location);

    return 1;
}
