#include "freeSpace.h"

void setBit(unsigned char* map, int i) {
	map[i/8] |= 1 << (i % 8);
}

int getBit(unsigned char* map, int i) {
	return map[i/8] & (1 << (i % 8)) != 0;
}

int getFree(int blocksNeeded) {
	int freeCount = 0;
	int firstFree = 0;
	for(int i = 0; i < totalBlockCount; i++) {
		/*
		if(i<10){
		printf("bit[%d]\n",getBit(freeSpaceMap, i));
		}
		*/
		if(getBit(freeSpaceMap, i) == 0) {
			if(freeCount == 0) {
				firstFree = i;
				//printf("first free [%d]\n",firstFree);
			}
			freeCount++;
			if(freeCount == blocksNeeded) {
				return firstFree;
			}
		}
	}
}

void clearBit(unsigned char* map, int i) {
	map[i/8] &= ~(1 << (i % 8));
}