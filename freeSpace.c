#include "freeSpace.h"

//sets bit to used in freespace map at locaiton i
void setBit(unsigned char* map, int i) {
	map[i/8] |= 1 << (i % 8);
}

//gets bit from freepsace map at location i
int getBit(unsigned char* map, int i) {
	return map[i/8] & (1 << (i % 8)) != 0;
}

//gets free blocks in map
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

//sets bit to unused at location i
void clearBit(unsigned char* map, int i) {
	map[i/8] &= ~(1 << (i % 8));
}