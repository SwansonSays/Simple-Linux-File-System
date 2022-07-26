#include "mfs.h"


#define MAXFILEPATH 4096

typedef enum {dir,file,nf} LASTELEMENT;

typedef struct parsedPath {
    int isPath; //0 if not complete path or 1 if is
    LASTELEMENT lastElement; //0 = dir, 1 = file, 2 = not found
    dirEntry* parentDir; //ptr to parent directory
    dirEntry* curDir; //pointer to last directory
    int index; //index of file or dir in parent directory
    char lastElementName[MAXFILENAME]; //name of the last element in path
}parsedPath;

dirEntry* cwd;
char cwdPath[MAXFILEPATH];
int isInit = 0;

int initmfs() {
    printf("----Initializing MFS----\n");
    isInit = 1;
    cwd = malloc(dirSize);
    if (fs_setcwd(".") == 0) {
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
    printf("****Started Parsing****\n");
    printf("***[%s]***\n", path);

    char* token; //token for strtok()
    parsedPath* pPath = initpPath(); //allocate memory for parsedPath struct
    pPath->curDir = loadRoot();

    printf("=====Printing after Loading Root=====\n");
    printDir(pPath->curDir);
    printf("=====Printing Finished=====\n");

    token = strtok(path, "/"); //tokenize path using '/' delim
    while(token) {
        printf("TOKEN: [%s]\n", token);
        pPath->lastElement = nf; //set last element to not found
        pPath->parentDir = pPath->curDir;
        // loop through each directory in curDir
        for(int i = 0; i < dirEntries; i++) {
            //printf("i[%d] FILENAME [%s]\n", i, pPath->curDir[i].fileName);
            // if token mathches name of file in curDir
            printf("++++token =[%s] curDir = [%s]\n", token, pPath->curDir[i].fileName);
            if(strcmp(token, pPath->curDir[i].fileName) == 0) {
                printf("TOKEN[%s] Matches FILENAME[%s]\n",token, pPath->curDir[i].fileName);
                pPath->index = i;// set index of found file in parent dir
                // if found file is a directory
                if(pPath->curDir[i].isDir) {
                    strcpy(pPath->lastElementName, token); //set name of last element in path to token
                    pPath->lastElement = dir; // mark it as a directory
                    //printf("printing parent dir in parsePath before assigning\n");
                    //printDir(pPath->parentDir);
                    pPath->parentDir = &pPath->curDir[i]; //sets parent directory to current directory before loading found directory
                    //printf("printing parent dir in parsePath\n");
                    //printDir(pPath->parentDir);
                    pPath->curDir = loadDir(pPath->curDir[i]); // load found directory as current directory before testing next token in path
                    printf("=====Printing after Loading Dir=====\n");
                    printDir(pPath->curDir);
                    printf("=====Printing Finished=====\n");
                    
                } else {
                    pPath->lastElement = file; // mark is as file
                    strcpy(pPath->lastElementName, token); //set name of last element in path to token
                }
                break;
            }
        }
        strcpy(pPath->lastElementName, token); //set name of last element in path to token
        token = strtok(NULL, "/");
    }
    if(pPath->lastElement == 2) {
        pPath->isPath = 0; //if last element in path was not found set isPath to 0
    } else {
        pPath->isPath = 1; //if file or dir set to 1;
    }
    printf("****Finished Parsing****\n");
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
    printf("fs_setcwd() called\n");
    printf("attmepting to move to [%s]\n", buf);
    parsedPath* pPath = parsePath(buf);
    printParsedPath(pPath);
    //if the last element of the path is a directory move cwd to that directory
    if(pPath->lastElement == dir) {
        strcpy(cwdPath, buf);
        cwd = pPath->curDir;
        printf("Moved to [%s]\n",cwdPath);
        freepPath(pPath);
        return 0;
    }
    printf("move failed\n");
    freepPath(pPath);
    return -1;
}

int fs_mkdir(const char *pathname, mode_t mode) {
    int location = 0;
    char *path[MAXFILENAME];
    //strcpy(path, pathname);
    parsedPath* pPath = parsePath((char *)pathname);
    printf("FINSIHED PARSING IN MKDIR\n");
    printf("Printing parent dir after parsing\n");
    printDir(pPath->parentDir);
    //printParsedPath(pPath);
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
        printf("ERROR CHECK: [%d]\n",pPath->parentDir[0].fileSize);
	    newDir[1].fileSize = pPath->parentDir[0].fileSize;
	    newDir[1].dateCreated = pPath->parentDir[0].dateCreated;
	    newDir[1].dateModified = pPath->parentDir[0].dateModified;
	    newDir[1].location = pPath->parentDir[0].location;
	    newDir[1].isDir = pPath->parentDir[0].isDir;
	    newDir[1].inUse = pPath->parentDir[0].inUse;

        //creates new directory in parent directory if index is not in use
        for(int i = 0; i < dirEntries; i++) {
            if(pPath->parentDir[i].inUse == 0) {
                strcpy(pPath->parentDir[i].fileName, pPath->lastElementName);
                pPath->parentDir[i].fileSize = dirSize;
                pPath->parentDir[i].dateCreated = time(0);
                pPath->parentDir[i].dateModified = time(0);
                pPath->parentDir[i].location = location;
                pPath->parentDir[i].isDir = 1;
                pPath->parentDir[i].inUse = 1;
                break;
            }
        }
        printDir(newDir);
        printDir(pPath->parentDir);

        char* writeDir = (char*) newDir;
        LBAwrite(writeDir,dirSize/fs_blockSize, location);
        writeDir = (char*) pPath->parentDir;
        LBAwrite(writeDir,dirSize/fs_blockSize, newDir[1].location);
    }
    freepPath(pPath);
}

int fs_isFile(char* path) {
    parsedPath* pPath = parsePath(path);
    if(pPath->lastElement == 1) {
        freepPath(pPath);
        return 1;
    }
    freepPath(pPath);
    return 0;
}

int fs_isDir(char* path) {
    parsedPath* pPath = parsePath(path);
    if(pPath->lastElement == 0) {
        freepPath(pPath);
        return 1;
    }
    freepPath(pPath);
    return 0;
}