#include <stdio.h>
//size of bitMap, totals blocks in system, block size of system
int freeSpaceSize, totalBlockCount, fs_blockSize;
unsigned char* freeSpaceMap; //bitmap of freespace

void setBit(unsigned char* map, int i);
int getBit(unsigned char* map, int i);
int getFree(int blocksNeeded);
void clearBit(unsigned char* map, int i);