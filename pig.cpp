#include <iostream>
#include "definitions.h"

#define FEN1 "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1"
#define FEN2 "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2"
#define FEN3 "rnbqkbnr/pp1ppppp/8/2p5/4P3/2N5/PPPP1PPP/R1BQKBNR b KQkq - 1 2"
#define FEN_PAWNMOVES "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define FEN_CASTLING "r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1" 

// cd source\repos\ItchyPig

void ShowSqAtBySide(const int side, const S_BOARD* pos) {
	int rank = 0;
	int file = 0;
	int sq = 0;
	int piece = 0;

	std::cout << "\n\n\n";
	for (rank = RANK_8; rank >= RANK_1; rank--) {
		for (file = FILE_A; file <= FILE_H; file++) {
			sq = FR_TO_SQ(file, rank);
			piece = pos->pieces[sq];
			/*if (pieceColor[piece] == side) {
				std::cout << " " << pieceColor[piece];
			}
			else {
				std::cout << " .";
			}*/
			if (SqAttacked(sq, side, pos)) {
				std::cout << " X";
			}
			else {
				std::cout << " -";
			}
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

int main() {
	AllInit();
	S_BOARD board[1];
	/*parseFEN(FEN3, board);
	printBoard(board);
	ASSERT(checkBoard(board));*/
	int move = 0;
	int from = A3; int to = B4; int cap = wR; int prom = bN;
	move = (from) | (to << 7) | (cap << 14) | (prom << 20); // see int move in definitions.h
	//printf("from: %d to: %d cap: %d prom: %d\n", FROMSQ(move), TOSQ(move), CAPTURED(move), PROMOTED(move));
	//printf("from: %s, to: %s, move: %s", printSquare(from), printSquare(to), printMove(move));

	parseFEN(FEN_CASTLING, board);
	printBoard(board);
	S_MOVELIST list[1];
	generateAllMoves(board, list);
	printMoveList(list);


	//parseFEN(FEN1, board);
	//printBoard(board);
	//parseFEN(FEN2, board);
	//printBoard(board);
	//parseFEN(FEN3, board);
	//printBoard(board);

	/*S_BOARD board[1];
	parseFEN(FEN3, board);
	printBoard(board);
	updateListsMaterial(board);
	
	ShowSqAtBySide(WHITE, board);
	printf("\n");
	ShowSqAtBySide(BLACK, board);*/

}
// Compare this snippet from bitBoard.cpp: