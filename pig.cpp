#include <iostream>
#include <string>
#include "definitions.h"

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define FEN1 "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"
#define FEN2 "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2"
#define FEN3 "rnbqkbnr/pp1ppppp/8/2p5/4P3/2N5/PPPP1PPP/R1BQKBNR b KQkq - 1 2"
#define FEN_PAWNMOVES "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define FEN_CASTLING "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1" 

// cd source\repos\ItchyPig

int main() {
	AllInit();
	S_BOARD board[1];
	InitPvTable(board->PvTable, 1024);

	parseFEN(START_FEN, board);
	S_MOVELIST list[1];
	generateAllMoves(board, list);
	S_SEARCHINFO info[1];

	char input[6];
	char *array[4]= {"e2e3", "e7e5", "g1f3", "b8c6"};
	for (int i = 0; i < 4; i++) {
		int move = parseMove(array[i], board);
		if (move != NO_MOVE) {
			//StorePvEntry(board, move);
			makeMove(board, move);
		}
		else {
			printf("** Move not valid **\n");
		}
	}

	printBoard(board);
	while (true) {
			printBoard(board);
			printf("Enter move: ");
			fgets(input, 6, stdin);
			if (input[0] == 'q') {
				break;
			}
			else if (input[0] == 't') {
				takeMove(board);
			}
			else if (input[0] == 'r'){
				parseFEN(START_FEN, board);
			}
			else if (input[0] == 'p') {
				//perftTest(5, board);
				//takeMove(board);
				int max = GetPvLine(4, board);
				for (int i = 0; i < max; i++) {
					printf("Move: %s\n", printMove(board->PvArray[i]));
				}
				printf("\n");
			}
			else if (input[0] == 'l') {
				printMoveList(list);
			}
			else if (input[0] == 'm') {
				generateAllMoves(board, list);
			}
			else if (input[0] == 'o') {
				printBoard(board);
			}
			else if (input[0] == 's') {
				info->depth = 4;
				SearchPosition(board, info);
			}

			else {
				int move = parseMove(input, board);
				if (move != NO_MOVE) {
					makeMove(board, move);
				}
				else {
					printf("** Move not valid **\n");
				}
			}
	}

	free(board->PvTable->pTable); 


	/*S_BOARD board[1];
	parseFEN(FEN3, board);
	printBoard(board);
	updateListsMaterial(board);
	
	ShowSqAtBySide(WHITE, board);
	printf("\n");
	ShowSqAtBySide(BLACK, board);*/

	/*parseFEN(FEN3, board);
	printBoard(board);
	ASSERT(checkBoard(board));*/
	//int move = 0;
	//int from = A3; int to = B4; int cap = wR; int prom = bN;
	//move = (from) | (to << 7) | (cap << 14) | (prom << 20); // see int move in definitions.h
	//printf("from: %d to: %d cap: %d prom: %d\n", FROMSQ(move), TOSQ(move), CAPTURED(move), PROMOTED(move));
	//printf("from: %s, to: %s, move: %s", printSquare(from), printSquare(to), printMove(move));

	//int MoveNum = 0; int move = 0;
	//for (MoveNum = 0; MoveNum < list->count; ++MoveNum) {
	//		move = list->moves[MoveNum].move;
	//		if (!makeMove(board, move)){continue;}
	//		printf("Move: > %s\n", printMove(move));
	//		printBoard(board);
	//		takeMove(board);
	//		printf("Taken Move: > %s\n", printMove(move));
	//		printBoard(board);
	//	}

}


void ShowSqAtBySide(const int side, const S_BOARD* pos) {
		int rank = 0;
	int file = 0;
	int sq = 0;
	int piece = 0;

	printf("\n\n\n");
    for (rank = RANK_8; rank >= RANK_1; rank--) {
        for (file = FILE_A; file <= FILE_H; file++) {
            sq = FR_TO_SQ(file, rank);
            piece = pos->pieces[sq];
            /*if (pieceColor[piece] == side) {
                printf(" %d", pieceColor[piece]);
            }
            else {
                printf(" .");
            }*/
            if (SqAttacked(sq, side, pos)) {
                printf(" X");
            } else {
                printf(" -");
            }
        }
        printf("\n");
    }
    printf("\n");
}