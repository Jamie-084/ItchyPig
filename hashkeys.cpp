#include "definitions.h"
#include <cstdio>
//https://youtu.be/WqVwQBXLwE0?list=PLZ1QII7yudbc-Ky058TEaOstZHVbT-2hg

u64 generatePosKey(const S_BOARD* pos) {
	int sq = 0;
	u64 finalKey = 0;
	int piece = EMPTY;

	for (sq = 0; sq < BOARD_ARRAY_SIZE; ++sq) {
		piece = pos->pieces[sq];
		if (piece != SQUARE_NONE && piece != EMPTY && piece != OFFBOARD) {
			ASSERT(piece >= wP && piece <= bK);
			finalKey ^= pieceKeys[piece][sq]; // hash in the piece to the final key
			//printf("piece: %d, peicefinalKey: %llu\n",piece, finalKey);
		}
	}

	if (pos->side == WHITE) {
		finalKey ^= sideKey;
	}

	if (pos->enPas != SQUARE_NONE) {
		ASSERT(pos->enPas >= 0 && pos->enPas < BOARD_ARRAY_SIZE);
		finalKey ^= pieceKeys[EMPTY][pos->enPas];
	}

	ASSERT(pos->castlePerm >= 0 && pos->castlePerm <= 15);
	finalKey ^= castleKeys[pos->castlePerm];
	return finalKey;
}