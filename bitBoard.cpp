#include <stdio.h>
#include "definitions.h"

const int bitTable[64] = {
	63, 30, 3, 32, 25, 41, 22, 33,
	15, 50, 42, 13, 11, 53, 19, 34,
	61, 29, 2, 51, 21, 43, 45, 10,
	18, 47, 1, 54, 9, 57, 0, 35,
	62, 31, 40, 4, 49, 5, 52, 26,
	60, 6, 23, 44, 46, 27, 56, 16,
	7, 39, 48, 24, 59, 14, 12, 55,
	38, 28, 58, 20, 37, 17, 36, 8
};


int popBit(u64* bb) {				// pop the least significant bit (closest to A1)
	u64 b = *bb ^ (*bb - 1);
	unsigned fold = (unsigned)((b & 0xffffffff) ^ (b >> 32));
	*bb &= (*bb - 1);
	return bitTable[(fold * 0x783a9b23) >> 26];
}

int countBits(u64 b) {				// count the number of bits set
	int r;
	for (r = 0; b; r++, b &= b - 1);
	return r;
}


void printBitBoard(u64 bitboard) {
	u64 shiftMe = 1ULL; // Unsigned long long of value 1 (64-bit) (0000 0000 ... 0001)

	int rank = 0;
	int file = 0;
	int sq = 0;
	int sq64 = 0;

	printf("\n");
	for (rank = RANK_8; rank >= RANK_1; rank--) { // Printing board in correct order
		for (file = FILE_A; file <= FILE_H; file++) {
			sq = FR_TO_SQ(file, rank);
			sq64 = Sq120ToSq64[sq];
			if ((shiftMe << sq64) & bitboard) printf("X");	// Shift 1 to the left by sq64 and check if it's in the bitboard
			else printf("-");
		}
		printf("\n");
	}
	printf("\n\n");
}

