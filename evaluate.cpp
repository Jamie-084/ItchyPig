// evaluate.c

#include <stdio.h>
#include "definitions.h"

const int PawnIsolated = -10;
const int PawnPassed[8] = {0, 5, 10, 20, 35, 60, 100, 200};
const int RookOpenFile = 10;
const int RookSemiOpenFile = 5;
const int QueenOpenFile = 5;
const int QueenSemiOpenFile = 3;
const int BishopPair = 30;

const int PawnTable[64] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	10, 10, 0, -10, -10, 0, 10, 10,
	5, 0, 0, 5, 5, 0, 0, 5,
	0, 0, 10, 20, 20, 10, 0, 0,
	5, 5, 5, 10, 10, 5, 5, 5,
	10, 10, 10, 20, 20, 10, 10, 10,
	20, 20, 20, 30, 30, 20, 20, 20,
	0, 0, 0, 0, 0, 0, 0, 0};

const int KnightTable[64] = {
	0, -10, 0, 0, 0, 0, -10, 0,
	0, 0, 0, 5, 5, 0, 0, 0,
	0, 0, 10, 10, 10, 10, 0, 0,
	0, 0, 10, 20, 20, 10, 5, 0,
	5, 10, 15, 20, 20, 15, 10, 5,
	5, 10, 10, 20, 20, 10, 10, 5,
	0, 0, 5, 10, 10, 5, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0};

const int BishopTable[64] = {
	0, 0, -10, 0, 0, -10, 0, 0,
	0, 0, 0, 10, 10, 0, 0, 0,
	0, 0, 10, 15, 15, 10, 0, 0,
	0, 10, 15, 20, 20, 15, 10, 0,
	0, 10, 15, 20, 20, 15, 10, 0,
	0, 0, 10, 15, 15, 10, 0, 0,
	0, 0, 0, 10, 10, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0};

const int RookTable[64] = {
	0, 0, 5, 10, 10, 5, 0, 0,
	0, 0, 5, 10, 10, 5, 0, 0,
	0, 0, 5, 10, 10, 5, 0, 0,
	0, 0, 5, 10, 10, 5, 0, 0,
	0, 0, 5, 10, 10, 5, 0, 0,
	0, 0, 5, 10, 10, 5, 0, 0,
	25, 25, 25, 25, 25, 25, 25, 25,
	0, 0, 5, 10, 10, 5, 0, 0};

const int KingE[64] = {
	-50, -10, 0, 0, 0, 0, -10, -50,
	-10, 0, 10, 10, 10, 10, 0, -10,
	0, 10, 20, 20, 20, 20, 10, 0,
	0, 10, 20, 40, 40, 20, 10, 0,
	0, 10, 20, 40, 40, 20, 10, 0,
	0, 10, 20, 20, 20, 20, 10, 0,
	-10, 0, 10, 10, 10, 10, 0, -10,
	-50, -10, 0, 0, 0, 0, -10, -50};

const int KingO[64] = {
	0, 5, 5, -10, -10, 0, 10, 5,
	-30, -30, -30, -30, -30, -30, -30, -30,
	-50, -50, -50, -50, -50, -50, -50, -50,
	-70, -70, -70, -70, -70, -70, -70, -70,
	-70, -70, -70, -70, -70, -70, -70, -70,
	-70, -70, -70, -70, -70, -70, -70, -70,
	-70, -70, -70, -70, -70, -70, -70, -70,
	-70, -70, -70, -70, -70, -70, -70, -70};
// sjeng 11.2
// 8/6R1/2k5/6P1/8/8/4nP2/6K1 w - - 1 41
int MaterialDraw(const S_BOARD *pos)
{

	ASSERT(checkBoard(pos));

	if (!pos->pieceCount[wR] && !pos->pieceCount[bR] && !pos->pieceCount[wQ] && !pos->pieceCount[bQ])
	{
		if (!pos->pieceCount[bB] && !pos->pieceCount[wB])
		{
			if (pos->pieceCount[wN] < 3 && pos->pieceCount[bN] < 3)
			{
				return TRUE;
			}
		}
		else if (!pos->pieceCount[wN] && !pos->pieceCount[bN])
		{
			if (abs(pos->pieceCount[wB] - pos->pieceCount[bB]) < 2)
			{
				return TRUE;
			}
		}
		else if ((pos->pieceCount[wN] < 3 && !pos->pieceCount[wB]) || (pos->pieceCount[wB] == 1 && !pos->pieceCount[wN]))
		{
			if ((pos->pieceCount[bN] < 3 && !pos->pieceCount[bB]) || (pos->pieceCount[bB] == 1 && !pos->pieceCount[bN]))
			{
				return TRUE;
			}
		}
	}
	else if (!pos->pieceCount[wQ] && !pos->pieceCount[bQ])
	{
		if (pos->pieceCount[wR] == 1 && pos->pieceCount[bR] == 1)
		{
			if ((pos->pieceCount[wN] + pos->pieceCount[wB]) < 2 && (pos->pieceCount[bN] + pos->pieceCount[bB]) < 2)
			{
				return TRUE;
			}
		}
		else if (pos->pieceCount[wR] == 1 && !pos->pieceCount[bR])
		{
			if ((pos->pieceCount[wN] + pos->pieceCount[wB] == 0) && (((pos->pieceCount[bN] + pos->pieceCount[bB]) == 1) || ((pos->pieceCount[bN] + pos->pieceCount[bB]) == 2)))
			{
				return TRUE;
			}
		}
		else if (pos->pieceCount[bR] == 1 && !pos->pieceCount[wR])
		{
			if ((pos->pieceCount[bN] + pos->pieceCount[bB] == 0) && (((pos->pieceCount[wN] + pos->pieceCount[wB]) == 1) || ((pos->pieceCount[wN] + pos->pieceCount[wB]) == 2)))
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

#define ENDGAME_MAT (1 * pieceValue[wR] + 2 * pieceValue[wN] + 2 * pieceValue[wP] + pieceValue[wK])

int EvalPosition(const S_BOARD *pos)
{

	ASSERT(checkBoard(pos));

	int pce;
	int pieceCount;
	int sq;
	int score = pos->material[WHITE] - pos->material[BLACK];

	if (!pos->pieceCount[wP] && !pos->pieceCount[bP] && MaterialDraw(pos) == TRUE)
	{
		return 0;
	}

	//// *** Peice Square Value Tables:

	pce = wP;
	for (pieceCount = 0; pieceCount < pos->pieceCount[pce]; ++pieceCount)
	{
		sq = pos->pList[pce][pieceCount];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq) >= 0 && SQ64(sq) <= 63);
		score += PawnTable[SQ64(sq)];

		// if( (IsolatedMask[SQ64(sq)] & pos->pawns[WHITE]) == 0) {
		// 	//printf("wP Iso:%s\n",PrSq(sq));
		// 	score += PawnIsolated;
		// }

		// if( (WhitePassedMask[SQ64(sq)] & pos->pawns[BLACK]) == 0) {
		// 	//printf("wP Passed:%s\n",PrSq(sq));
		// 	score += PawnPassed[rankArray[sq]];
		// }
	}

	pce = bP;
	for (pieceCount = 0; pieceCount < pos->pieceCount[pce]; ++pieceCount)
	{
		sq = pos->pList[pce][pieceCount];
		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR64(SQ64(sq)) >= 0 && MIRROR64(SQ64(sq)) <= 63);
		score -= PawnTable[MIRROR64(SQ64(sq))];

		// if( (IsolatedMask[SQ64(sq)] & pos->pawns[BLACK]) == 0) {
		// 	//printf("bP Iso:%s\n",PrSq(sq));
		// 	score -= PawnIsolated;
		// }

		// if( (BlackPassedMask[SQ64(sq)] & pos->pawns[WHITE]) == 0) {
		// 	//printf("bP Passed:%s\n",PrSq(sq));
		// 	score -= PawnPassed[7 - rankArray[sq]];
		// }
	}

	pce = wN;
	for (pieceCount = 0; pieceCount < pos->pieceCount[pce]; ++pieceCount)
	{
		sq = pos->pList[pce][pieceCount];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq) >= 0 && SQ64(sq) <= 63);
		score += KnightTable[SQ64(sq)];
	}

	pce = bN;
	for (pieceCount = 0; pieceCount < pos->pieceCount[pce]; ++pieceCount)
	{
		sq = pos->pList[pce][pieceCount];
		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR64(SQ64(sq)) >= 0 && MIRROR64(SQ64(sq)) <= 63);
		score -= KnightTable[MIRROR64(SQ64(sq))];
	}

	pce = wB;
	for (pieceCount = 0; pieceCount < pos->pieceCount[pce]; ++pieceCount)
	{
		sq = pos->pList[pce][pieceCount];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq) >= 0 && SQ64(sq) <= 63);
		score += BishopTable[SQ64(sq)];
	}

	pce = bB;
	for (pieceCount = 0; pieceCount < pos->pieceCount[pce]; ++pieceCount)
	{
		sq = pos->pList[pce][pieceCount];
		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR64(SQ64(sq)) >= 0 && MIRROR64(SQ64(sq)) <= 63);
		score -= BishopTable[MIRROR64(SQ64(sq))];
	}

	pce = wR;
	for (pieceCount = 0; pieceCount < pos->pieceCount[pce]; ++pieceCount)
	{
		sq = pos->pList[pce][pieceCount];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq) >= 0 && SQ64(sq) <= 63);
		score += RookTable[SQ64(sq)];

		ASSERT(FileRankValid(fileArray[sq]));

		// if(!(pos->pawns[BOTH_COLORS] & FileBBMask[fileArray[sq]])) {
		// 	score += RookOpenFile;
		// } else if(!(pos->pawns[WHITE] & FileBBMask[fileArray[sq]])) {
		// 	score += RookSemiOpenFile;
		// }
	}

	pce = bR;
	for (pieceCount = 0; pieceCount < pos->pieceCount[pce]; ++pieceCount)
	{
		sq = pos->pList[pce][pieceCount];
		ASSERT(SqOnBoard(sq));
		ASSERT(MIRROR64(SQ64(sq)) >= 0 && MIRROR64(SQ64(sq)) <= 63);
		score -= RookTable[MIRROR64(SQ64(sq))];
		ASSERT(FileRankValid(fileArray[sq]));
		// if(!(pos->pawns[BOTH_COLORS] & FileBBMask[fileArray[sq]])) {
		// 	score -= RookOpenFile;
		// } else if(!(pos->pawns[BLACK] & FileBBMask[fileArray[sq]])) {
		// 	score -= RookSemiOpenFile;
		// }
	}

	pce = wQ;
	for (pieceCount = 0; pieceCount < pos->pieceCount[pce]; ++pieceCount)
	{
		sq = pos->pList[pce][pieceCount];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq) >= 0 && SQ64(sq) <= 63);
		ASSERT(FileRankValid(fileArray[sq]));
		// if(!(pos->pawns[BOTH_COLORS] & FileBBMask[fileArray[sq]])) {
		// 	score += QueenOpenFile;
		// } else if(!(pos->pawns[WHITE] & FileBBMask[fileArray[sq]])) {
		// 	score += QueenSemiOpenFile;
		// }
	}

	pce = bQ;
	for (pieceCount = 0; pieceCount < pos->pieceCount[pce]; ++pieceCount)
	{
		sq = pos->pList[pce][pieceCount];
		ASSERT(SqOnBoard(sq));
		ASSERT(SQ64(sq) >= 0 && SQ64(sq) <= 63);
		ASSERT(FileRankValid(fileArray[sq]));
		// if(!(pos->pawns[BOTH_COLORS] & FileBBMask[fileArray[sq]])) {
		// 	score -= QueenOpenFile;
		// } else if(!(pos->pawns[BLACK] & FileBBMask[fileArray[sq]])) {
		// 	score -= QueenSemiOpenFile;
		// }
	}
	// 8/p6k/6p1/5p2/P4K2/8/5pB1/8 b - - 2 62
	pce = wK;
	sq = pos->pList[pce][0];
	ASSERT(SqOnBoard(sq));
	ASSERT(SQ64(sq) >= 0 && SQ64(sq) <= 63);

	if ((pos->material[BLACK] <= ENDGAME_MAT))
	{
		score += KingE[SQ64(sq)];
	}
	else
	{
		score += KingO[SQ64(sq)];
	}

	pce = bK;
	sq = pos->pList[pce][0];
	ASSERT(SqOnBoard(sq));
	ASSERT(MIRROR64(SQ64(sq)) >= 0 && MIRROR64(SQ64(sq)) <= 63);

	if ((pos->material[WHITE] <= ENDGAME_MAT))
	{
		score -= KingE[MIRROR64(SQ64(sq))];
	}
	else
	{
		score -= KingO[MIRROR64(SQ64(sq))];
	}

	//// ***

	if (pos->pieceCount[wB] >= 2)
		score += BishopPair;
	if (pos->pieceCount[bB] >= 2)
		score -= BishopPair;

	if (pos->side == WHITE)
	{ //
		return score;
	}
	else
	{
		return -score;
	}
}
