#include "definitions.h"
#include <stdlib.h>

// 0000 000000000000000 000000000000000 000000000000000 111111111111111 First 15 bits are randomised
// 0000 000000000000000 000000000000000 111111111111111 000000000000000 Next 15 bits are randomised, by shifting 15 (<< 15)
// 0000 000000000000000 111111111111111 000000000000000 000000000000000
// 0000 111111111111111 000000000000000 000000000000000 000000000000000
// 1111 000000000000000 000000000000000 000000000000000 000000000000000
#define RAND_64 ((u64)rand() | (u64)rand() << 15 | (u64)rand() << 30 | (u64)rand() << 45 | ((u64)rand() & 0xf) << 60)
//Last 4 bits are randomised (the & 0xf removes 11 bits)
// All 5 lines are or'd together to create a 64-bit random number:
// 1111 111111111111111 111111111111111 111111111111111 111111111111111

int Sq120ToSq64[BOARD_ARRAY_SIZE];
int Sq64ToSq120[64];
u64 setMask[64];
u64 clearMask[64];

u64 pieceKeys[13][120];
u64 sideKey;		
u64 castleKeys[16];

int fileArray[BOARD_ARRAY_SIZE];
int rankArray[BOARD_ARRAY_SIZE];

void InitFilesRanksBrd() {
	int index = 0;
	int file = FILE_A;
	int rank = RANK_1;
	int sq = A1;
	int sq64 = 0;

	for (index = 0; index < BOARD_ARRAY_SIZE; ++index) {
		fileArray[index] = OFFBOARD;	// set all elements to OFFBOARD: 100
		rankArray[index] = OFFBOARD;
	}

	for (rank = RANK_1; rank <= RANK_8; ++rank) {
		for (file = FILE_A; file <= FILE_H; ++file) {
			sq = FR_TO_SQ(file, rank);
			fileArray[sq] = file;
			rankArray[sq] = rank;
		}
	}
}

void InitHashKeys() {
	int index = 0;
	int index2 = 0;
	for (index = 0; index < 13; ++index) {
		for (index2 = 0; index2 < 120; ++index2) {
			pieceKeys[index][index2] = RAND_64;
		}
	}
	sideKey = RAND_64;
	for (index = 0; index < 16; ++index) {
		castleKeys[index] = RAND_64;
	}
}

void InitBitMasks() {
	int index = 0;

	for (index = 0; index < 64; ++index) {	// set all masks to 0
		setMask[index] = 0ULL;
		clearMask[index] = 0ULL;
	}

	for (index = 0; index < 64; ++index) {
		setMask[index] |= (1ULL << index);	// set the bit at index to 1
		clearMask[index] = ~setMask[index];	// opposite of setMask
	}
}

void InitSq120To64() {
	int index = 0;
	int file = FILE_A;
	int rank = RANK_1;
	int sq120 = A1;
	int sq64 = 0;

	for (index = 0; index < BOARD_ARRAY_SIZE; ++index) {
		Sq120ToSq64[index] = 65;
	}

	for (index = 0; index < 64; ++index) {
		Sq64ToSq120[index] = 120;
	}

	for (rank = RANK_1; rank <= RANK_8; ++rank) {
		for (file = FILE_A; file <= FILE_H; ++file) {
			sq120 = FR_TO_SQ(file, rank);	// get the index of square in 120 array
			Sq64ToSq120[sq64] = sq120;		// set first element of 64 array to index of square in 120 array
			Sq120ToSq64[sq120] = sq64;		// set 21st element of 120 array to index of square in 64 array
			sq64++;
		}
	}
}

void AllInit() {
	InitSq120To64();
	InitBitMasks();
	InitHashKeys();
	InitFilesRanksBrd();
	InitMvvLva();
}