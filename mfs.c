#include "mfs.h"
#define MAXFILEPATH 4096

dirEntry* cwd; //ptr to current working directory
char cwdPath[MAXFILEPATH]; //path of current working dirctory
int isInit = 0;

//init MFS and sets cwd to root
int initmfs() {
    isInit = 1;
    cwd = malloc(dirSize);
    if (fs_setcwd("/") == 0) {
        return 0;
    }
    return 1;
}

//checks if path is abosulte and adds / to make absolute if not
char* makeAbsolute(char* path) {
    char newPath[MAXFILEPATH];
    strcpy(newPath, "/");

    path = strcat(newPath, path);
    if(strcmp(cwdPath, "/") != 0) {
        path = strcat(cwdPath,path);
    }
    
    return path;
}

//inits parsedPath structure returned my parsePath and mallocs dirEntry structs it holds
parsedPath* initpPath() {
    parsedPath* pPath = malloc(sizeof(parsedPath));
    pPath->curDir = malloc(dirSize);
    initDir(pPath->curDir);
    pPath->parentDir = malloc(dirSize);
    initDir(pPath->parentDir);
    return pPath;
}

void freepPath(parsedPath* pPath) {
    free(pPath);
}

//parses path and returns pPath for data used by other functions
parsedPath* parsePath(char* path) {
    char* token; //token for strtok()
    char* saveptr;
    parsedPath* pPath = initpPath(); //allocate memory for parsedPath struct
    pPath->curDir = loadRoot();

    if(path[0] != '/') {
        path = makeAbsolute(path);
    }

    token = strtok_r(path, "/", &saveptr); //tokenize path using '/' delim
    while(token) {
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
        token = strtok_r(NULL, "/", &saveptr);
    }
    pPath->parentDir = loadDir(pPath->curDir[1].location, pPath->curDir[1].fileSize);
    if(pPath->lastElement == 2) {
        pPath->isPath = 0; //if last element in path was not found set isPath to 0
    } else {
        pPath->isPath = 1; //if file or dir set to 1;
    }

    return pPath;
}


//returns the name of the current working directory
char* fs_getcwd(char *buf, size_t size) {
    if(strlen(cwdPath) > size) {
        return NULL;
    }
    return strcpy(buf, cwdPath);
}

//sets cwd
int fs_setcwd(char *buf) {
    char* copy = (char*)malloc(strlen(buf) + 1);
    strcpy(copy, buf);
    parsedPath* pPath = parsePath(copy);
    free(copy);

    if(strcmp(buf, "..") == 0) {
        cwd = pPath->parentDir;
        strcpy(cwdPath, pPath->parentDir[pPath->index].fileName);
        free(pPath);
        return 0;
    }

    //if the last element of the path is a directory move cwd to that directory
    if(pPath->lastElement == dir) {
        if(buf[0] != '/') {
        buf = makeAbsolute(buf);
        }
        strcpy(cwdPath, buf);
        cwd = pPath->curDir;

        freepPath(pPath);
        return 0;
    }
    printf("move failed\n");
    freepPath(pPath);
    return -1;
}

//makes new directory at specified path
int fs_mkdir(const char *pathname, mode_t mode) {
    int location = 0;
    char* copy = (char*)malloc(strlen(pathname) + 1);
    strcpy(copy, pathname);
    parsedPath* pPath = parsePath(copy);
    free(copy);

    //if the last element of the path does not exist,
    //make a new directory with the name of the last element   
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

        //writes new dir to disk
        char* writeDir = (char*) newDir;
        LBAwrite(writeDir,dirSize/fs_blockSize, location);
        //writes parent dir to disk
        writeDir = (char*) pPath->curDir;
        LBAwrite(writeDir,dirSize/fs_blockSize, newDir[1].location);
    }
    freepPath(pPath);
}

//if the last element of the path is file
//returns 1 if not returns 0
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

//if the last element of the path is directory
//returns 1 if not returns 0
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

//removes directory at pathname if empty
int fs_rmdir(const char *pathname) {
    char* copy = (char*)malloc(strlen(pathname) + 1);
    strcpy(copy, pathname);
    parsedPath* pPath = parsePath(copy);
    free(copy);

    //checks if last element is directory
    if(pPath->lastElement == 0) {
        if(isEmpty(pPath->curDir)) { //checks that directory is empty execpt for '.' and '..'
            //removes directory at index in its parents directory and clears the bits
            //on the freespace map
            removeDir(pPath->parentDir, pPath->index); 
            for(int i = 0; i < dirSize / fs_blockSize; i++) {
                clearBit(freeSpaceMap, i + pPath->curDir[0].location);
            }
            LBAwrite(freeSpaceMap, freeSpaceSize, 1); //write map to disk
        //if directory is not empty throw error    
        } else {
            printf("Directory %s is not empty. Directory must be empty to be deleted.\n", pPath->lastElementName);
            free(pPath);
            return -1;
        }
    }
    free(pPath);
    return 1;
}

//deletes specified file
int fs_delete(char* filename) {
    char* copy = (char*)malloc(strlen(filename) + 1);
    strcpy(copy, filename);
    parsedPath* pPath = parsePath(copy);
    free(copy);

    //if last element of path is file remove it
    if(pPath->lastElement == 1) {
        removeDir(pPath->parentDir, pPath->index);
        for(int i = 0; i < dirSize / fs_blockSize; i++) {
            clearBit(freeSpaceMap, i + pPath->curDir[0].location);
        }
        LBAwrite(freeSpaceMap, freeSpaceSize, 1);
    }
    free(pPath);
    return 1;
}

//Creates file at specified path and returns file info
fileInfo* makeFile(parsedPath* pPath) {
    fileInfo* fi;
    int location = 0;
    int index = 0;

    fi = malloc(sizeof(fileInfo));

    //gets free block from freespace management and writes to disk
    location = getFree(1);
    setBit(freeSpaceMap, location);
    LBAwrite(freeSpaceMap, freeSpaceSize, 1);

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

    //write parent directory to disk
    char* writeFile = (char*) pPath->curDir;
    LBAwrite(writeFile, dirSize/fs_blockSize,  pPath->curDir[0].location);

    return fi;
}

//populates fileInfo sturct for file at path
//truncates or creates file if flags are set
fileInfo* getFileInfo(char* path, int flags) {
    fileInfo* fi;
    char* copy = (char*)malloc(strlen(path) + 1);
    strcpy(copy, path);
    parsedPath* pPath = parsePath(copy);
    free(copy);

    if(pPath->lastElement == file && (flags & (1 << 9))) { //truncates file if found
        fi = malloc(sizeof(fileInfo));
        strcpy(fi->fileName,pPath->lastElementName);
        fi->fileSize = 0;
        fi->location = pPath->curDir[pPath->index].location;
    } else if(pPath->lastElement == file) { //files file info if found
        fi = malloc(sizeof(fileInfo));
        strcpy(fi->fileName,pPath->lastElementName);
        fi->fileSize = pPath->curDir[pPath->index].fileSize;
        fi->location = pPath->curDir[pPath->index].location;
    } else if(pPath->lastElement == nf && (flags & (1 << 6))) { //creates file if not found
        fi = makeFile(pPath); 
    } else {
        fi = NULL;
    }

    return fi;
}

//Sets size in file info
void setFileSize(fileInfo* fi, int size) {
    fi->pPath->curDir[fi->pPath->index].fileSize = size;

    //writes parent dir of file to disk
    char* writeFile = (char*) fi->pPath->curDir;
    LBAwrite(writeFile, dirSize/fs_blockSize, fi->pPath->curDir[0].location);
}

//Moves dirEntry from src to dest
int moveDirEntry(char* src, char* dest) {
    char* copy = (char*)malloc(MAXFILEPATH);
    strcpy(copy, src);
    parsedPath* pPath = parsePath(copy);
    strcpy(copy,dest);
    parsedPath* pPathDest = parsePath(copy);
    free(copy);

    //finds free spot in parent directory of dest and 
    //sets src file info in free spot
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

    //write parent directory of dest and src to disk
    removeDir(pPath->parentDir, pPath->index);
    char* writeFile = (char*) pPath->parentDir;
    LBAwrite(writeFile,dirSize/fs_blockSize, pPath->parentDir[0].location);
    writeFile = (char*) pPathDest->parentDir;
    LBAwrite(writeFile,dirSize/fs_blockSize, pPathDest->parentDir[0].location);
    free(pPath);
    return 1;
}


fdDir * fs_opendir(const char *name){ 
   parsedPath* pPath = parsePath((char *)name);
       loadDir(pPath->parentDir->location, pPath->index);
       pPath->curDir[pPath->index].isDir;

       fdDir* newDir = malloc(sizeof(fdDir));
       newDir->ptr = malloc(dirSize);
       newDir->dirItemInfo = malloc(sizeof(struct fs_diriteminfo));

       newDir->ptr = pPath->curDir;
       newDir->index=pPath->index;
       newDir->ptr->inUse=1;

   return newDir;
}
 
struct fs_diriteminfo *fs_readdir(fdDir *dirp){ 
   for(int i=dirp->index;i<10;i++){
       if( dirp->ptr[i].inUse) {
           strcpy(dirp->dirItemInfo->d_name,dirp->ptr[i].fileName);
           dirp->dirItemInfo->fileType=dirp->ptr->isDir;
           dirp->index=i+1;
 
            return dirp->dirItemInfo;
       } else {
           dirp->dirItemInfo = NULL;
       }    
       return dirp->dirItemInfo;
   }
      
}
  
int fs_closedir(fdDir *dirp){
    free(dirp);
    return 0;
}
 
int fs_stat(const char *path, struct fs_stat *buf){
    char* copy = (char*)malloc(strlen(path) + 1);
    strcpy(copy, path);
    parsedPath* pPath = parsePath(copy);
    free(copy);

    buf->st_size = pPath->parentDir[pPath->index].fileSize;
    buf->st_blksize = fs_blockSize;      /* blocksize for file system I/O */
    buf->st_blocks = pPath->parentDir[pPath->index].fileSize / fs_blockSize; 
    buf->st_createtime = pPath->parentDir[pPath->index].dateCreated;
    buf->st_modtime = pPath->parentDir[pPath->index].dateModified;
 
    return 0;
}

