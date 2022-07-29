#include <stdio.h>
int freeSpaceSize, totalBlockCount, fs_blockSize; //size of bitMap
unsigned char* freeSpaceMap; 

void setBit(unsigned char* map, int i);
int getBit(unsigned char* map, int i);
int getFree(int blocksNeeded);
void clearBit(unsigned char* map, int i);