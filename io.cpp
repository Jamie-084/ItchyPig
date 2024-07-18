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