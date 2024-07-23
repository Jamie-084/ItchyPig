#include "definitions.h"
#include <stdio.h>

char *printSquare(const int sq) {
	static char sqStr[3];

	int file = fileArray[sq];
	int rank = rankArray[sq];
	
	sprintf(sqStr, "%c%c", ('a' + file), ('1' + rank));

	return sqStr;
}

char *printMove(const int move) {
	static char MvStr[6];

	int ff = fileArray[FROMSQ(move)];
	int rf = rankArray[FROMSQ(move)];
	int ft = fileArray[TOSQ(move)];
	int rt = rankArray[TOSQ(move)];

	int promoted = PROMOTED(move);
	//printf("%c%c %c%c\n", ('a' + ff), ('1' + rf), ('a' + ft), ('1' + rt));

	if (promoted) {
			char pchar = 'q';
			if (isRookQueen[promoted] && !isBishopQueen[promoted]) {
						pchar = 'r';
					}
			else if (isBishopQueen[promoted] && !isRookQueen[promoted]) {
						pchar = 'b';
					}
			else if (isKnight[promoted]) {
						pchar = 'n';
					}
			sprintf(MvStr, "%c%c%c%c%c", ('a' + ff), ('1' + rf), ('a' + ft), ('1' + rt), pchar);
		}
	else {
			sprintf(MvStr, "%c%c%c%c", ('a' + ff), ('1' + rf), ('a' + ft), ('1' + rt));
		}

	return MvStr;
}

void printMoveList(const S_MOVELIST *list)
{
	int index = 0;
	int score = 0;
	int move = 0;
	printf("MoveList:\n");

	for (index = 0; index < list->count; ++index)
	{
				move = list->moves[index].move;
		score = list->moves[index].score;
		printf("Move:%d > %s (score:%d)\n", index + 1, printMove(move), score);
	}
	printf("MoveList Total %d Moves:\n\n", list->count);
}	


int parseMove(char *ptrChar, S_BOARD *pos) {
	
	ASSERT(checkBoard(pos));
	// ptrChar example: a2a4
	// ptrChar promotion example: e7e8q
	if (ptrChar[1] > '8' || ptrChar[1] < '1') return NO_MOVE;
	if (ptrChar[3] > '8' || ptrChar[3] < '1') return NO_MOVE;
	if (ptrChar[0] > 'h' || ptrChar[0] < 'a') return NO_MOVE;
	if (ptrChar[2] > 'h' || ptrChar[2] < 'a') return NO_MOVE;
	
	int from = FR_TO_SQ(ptrChar[0] - 'a', ptrChar[1] - '1');
	int to = FR_TO_SQ(ptrChar[2] - 'a', ptrChar[3] - '1');
	
	ASSERT(SqOnBoard(from) && SqOnBoard(to));
	
	S_MOVELIST list[1];
	generateAllMoves(pos, list);
	
	int MoveNum = 0;
	int move = NO_MOVE;
	int PromPce = EMPTY;
	int found = FALSE;
	
	for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {
		move = list->moves[MoveNum].move;
		if (FROMSQ(move) == from && TOSQ(move) == to) {
			PromPce = PROMOTED(move);
			if (PromPce != EMPTY) {
				if (isRookQueen[PromPce] && !isBishopQueen[PromPce] && ptrChar[4] == 'r') {
					return move;
				}
				else if (isBishopQueen[PromPce] && !isRookQueen[PromPce] && ptrChar[4] == 'b') {
					return move;
				}
				else if (isKnight[PromPce] && ptrChar[4] == 'n') {
					return move;
				}
				else if (isPawn[PromPce] && ptrChar[4] == 'q') {
					return move;
				}
				continue;
			}
			return move;
		}
		}
	
	return NO_MOVE;
}