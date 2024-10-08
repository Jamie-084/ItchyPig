#include "definitions.h"
#include <stdio.h>

 // movegen.c

#define MOVE(f,t,ca,pro,fl) ( (f) | ((t) << 7) | ( (ca) << 14 ) | ( (pro) << 20 ) | (fl))
#define SQOFFBOARD(sq) (fileArray[(sq)]==OFFBOARD)

const int LoopSlidePce[8] = {
 wB, wR, wQ, 0, bB, bR, bQ, 0
};

const int LoopNonSlidePce[6] = { // without the pawns (done in separate functions)
 wN, wK, 0, bN, bK, 0
};

const int LoopSlideIndex[2] = { 0, 4 };
const int LoopNonSlideIndex[2] = { 0, 3 };

const int PceDir[13][8] = {		// Valid directions for each piece move (used in non-sliding move gen)
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ -8, -19,	-21, -12, 8, 19, 21, 12 },
	{ -9, -11, 11, 9, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 },
	{ 0, 0, 0, 0, 0, 0, 0 },
	{ -8, -19,	-21, -12, 8, 19, 21, 12 },
	{ -9, -11, 11, 9, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, 0, 0, 0, 0 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 },
	{ -1, -10,	1, 10, -9, -11, 11, 9 }
};

const int NumDir[13] = {		// The amount of directions each piece can move
 0, 0, 8, 4, 4, 8, 8, 0, 8, 4, 4, 8, 8
};

/*
PV Move
Cap -> MvvLVA
Killers
HistoryScore

*/
const int VictimScore[13] = { 0, 100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600 };
static int MvvLvaScores[13][13];

void InitMvvLva() {
	int Attacker;
	int Victim;
	for(Attacker = wP; Attacker <= bK; ++Attacker) {
		for(Victim = wP; Victim <= bK; ++Victim) {
			MvvLvaScores[Victim][Attacker] = VictimScore[Victim] + 6 - ( VictimScore[Attacker] / 100);
			//printf("%c -> %c = Score: %d\n", pieceChar[Attacker], pieceChar[Victim], MvvLvaScores[Victim][Attacker]);
			// example: P -> Q = 100 + 6 - (500/100) = 101
		}
	}
}


//  Move ordering:
//  Check for Pv Move
//	Search for captures (either MVV/LVA or SEE)
//	Search for killer moves (non-capture moves that caused a beta cutoff)
//	Search for history moves (moves that improved alpha)

// MVV/LVA (Most valuable victim/least valuable attacker):
// P takes Q
// N -> Q
// B -> Q ,ect

// SEE (Static exchange evaluation)

int MoveExists(S_BOARD *pos, const int move) {

	S_MOVELIST list[1];
    generateAllMoves(pos,list);

   int MoveNum = 0;
	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {

       if ( !makeMove(pos,list->moves[MoveNum].move))  {
           continue;
       }
       takeMove(pos);
		if(list->moves[MoveNum].move == move) {
			return TRUE;
		}
   }
	return FALSE;
}

// Add a non-capture move (killer move) to the move list
static void AddQuietMove( const S_BOARD *pos, int move, S_MOVELIST *list ) {

	ASSERT(SqOnBoard(FROMSQ(move)));
	ASSERT(SqOnBoard(TOSQ(move)));
	ASSERT(checkBoard(pos));
	ASSERT(pos->ply >=0 && pos->ply < MAX_DEPTH);

	list->moves[list->count].move = move;

	if(pos->searchKillers[0][pos->ply] == move) {	// No capture but killer move
		list->moves[list->count].score = 900000;
	} else if(pos->searchKillers[1][pos->ply] == move) {
		list->moves[list->count].score = 800000;
	} else { // No capture, no beta-cutoff
		list->moves[list->count].score = pos->searchHistory[pos->pieces[FROMSQ(move)]][TOSQ(move)];
	}
	list->count++;
}

static void AddCaptureMove( const S_BOARD *pos, int move, S_MOVELIST *list ) {

	ASSERT(SqOnBoard(FROMSQ(move)));
	ASSERT(SqOnBoard(TOSQ(move)));
	ASSERT(PieceValid(CAPTURED(move)));
	ASSERT(checkBoard(pos));

	list->moves[list->count].move = move;
	// 1000000 is added so capture moves are searched first
	list->moves[list->count].score = MvvLvaScores[CAPTURED(move)][pos->pieces[FROMSQ(move)]] + 1000000;
	list->count++;
}

static void AddEnPassantMove( const S_BOARD *pos, int move, S_MOVELIST *list ) {

	ASSERT(SqOnBoard(FROMSQ(move)));
	ASSERT(SqOnBoard(TOSQ(move)));
	ASSERT(checkBoard(pos));
	ASSERT((rankArray[TOSQ(move)]==RANK_6 && pos->side == WHITE) || (rankArray[TOSQ(move)]==RANK_3 && pos->side == BLACK));

	list->moves[list->count].move = move;
	list->moves[list->count].score = 105 + 1000000;
	list->count++;
}

static void AddWhitePawnCapMove( const S_BOARD *pos, const int from, const int to, const int cap, S_MOVELIST *list ) {

	ASSERT(PieceValidEmpty(cap));
	ASSERT(SqOnBoard(from));
	ASSERT(SqOnBoard(to));
	ASSERT(checkBoard(pos));

	if(rankArray[from] == RANK_7) {	// Promoting+capture
		AddCaptureMove(pos, MOVE(from,to,cap,wQ,0), list);
		AddCaptureMove(pos, MOVE(from,to,cap,wR,0), list);
		AddCaptureMove(pos, MOVE(from,to,cap,wB,0), list);
		AddCaptureMove(pos, MOVE(from,to,cap,wN,0), list);
	} else {						// Regular capture
		AddCaptureMove(pos, MOVE(from,to,cap,EMPTY,0), list);
	}
}

static void AddWhitePawnMove( const S_BOARD *pos, const int from, const int to, S_MOVELIST *list ) {

	ASSERT(SqOnBoard(from));
	ASSERT(SqOnBoard(to));
	ASSERT(checkBoard(pos));

	if(rankArray[from] == RANK_7) {	// Promoting + no capture (quiet)
		AddQuietMove(pos, MOVE(from,to,EMPTY,wQ,0), list);
		AddQuietMove(pos, MOVE(from,to,EMPTY,wR,0), list);
		AddQuietMove(pos, MOVE(from,to,EMPTY,wB,0), list);
		AddQuietMove(pos, MOVE(from,to,EMPTY,wN,0), list);
	} else {						// Regular move (quiet)	
		AddQuietMove(pos, MOVE(from,to,EMPTY,EMPTY,0), list);
	}
}

static void AddBlackPawnCapMove( const S_BOARD *pos, const int from, const int to, const int cap, S_MOVELIST *list ) {

	ASSERT(PieceValidEmpty(cap));
	ASSERT(SqOnBoard(from));
	ASSERT(SqOnBoard(to));
	ASSERT(checkBoard(pos));

	if(rankArray[from] == RANK_2) {
		AddCaptureMove(pos, MOVE(from,to,cap,bQ,0), list);
		AddCaptureMove(pos, MOVE(from,to,cap,bR,0), list);
		AddCaptureMove(pos, MOVE(from,to,cap,bB,0), list);
		AddCaptureMove(pos, MOVE(from,to,cap,bN,0), list);
	} else {
		AddCaptureMove(pos, MOVE(from,to,cap,EMPTY,0), list);
	}
}

static void AddBlackPawnMove( const S_BOARD *pos, const int from, const int to, S_MOVELIST *list ) {

	ASSERT(SqOnBoard(from));
	ASSERT(SqOnBoard(to));
	ASSERT(checkBoard(pos));

	if(rankArray[from] == RANK_2) {
		AddQuietMove(pos, MOVE(from,to,EMPTY,bQ,0), list);
		AddQuietMove(pos, MOVE(from,to,EMPTY,bR,0), list);
		AddQuietMove(pos, MOVE(from,to,EMPTY,bB,0), list);
		AddQuietMove(pos, MOVE(from,to,EMPTY,bN,0), list);
	} else {
		AddQuietMove(pos, MOVE(from,to,EMPTY,EMPTY,0), list);
	}
}

void generateAllMoves(const S_BOARD *pos, S_MOVELIST *list) {

	ASSERT(checkBoard(pos));

	list->count = 0;

	int pce = EMPTY;
	int side = pos->side;
	int sq = 0; int t_sq = 0;
	int pieceCount = 0;
	int dir = 0;
	int index = 0;
	int pceIndex = 0;

	if(side == WHITE) {

		for(pieceCount = 0; pieceCount < pos->pieceCount[wP]; ++pieceCount) {
			sq = pos->pList[wP][pieceCount];
			ASSERT(SqOnBoard(sq));

			if(pos->pieces[sq + 10] == EMPTY) {
				AddWhitePawnMove(pos, sq, sq+10, list);
				if(rankArray[sq] == RANK_2 && pos->pieces[sq + 20] == EMPTY) { // double move
					AddQuietMove(pos, MOVE(sq,(sq+20),EMPTY,EMPTY,PAWNSTART_F),list);
				}
			}

			if(!SQOFFBOARD(sq + 9) && pieceColor[pos->pieces[sq + 9]] == BLACK) {
				AddWhitePawnCapMove(pos, sq, sq+9, pos->pieces[sq + 9], list);
			}
			if(!SQOFFBOARD(sq + 11) && pieceColor[pos->pieces[sq + 11]] == BLACK) {
				AddWhitePawnCapMove(pos, sq, sq+11, pos->pieces[sq + 11], list);
			}

			if(pos->enPas != SQUARE_NONE) {
				if(sq + 9 == pos->enPas) {
					AddEnPassantMove(pos, MOVE(sq,sq + 9,EMPTY,EMPTY,ENPASSANT_F), list);
				}
				if(sq + 11 == pos->enPas) {
					AddEnPassantMove(pos, MOVE(sq,sq + 11,EMPTY,EMPTY,ENPASSANT_F), list);
				} 	
			}
		}

		if(pos->castlePerm & WK_CASTLE) {
			if(pos->pieces[F1] == EMPTY && pos->pieces[G1] == EMPTY) {
				if(!SqAttacked(E1,BLACK,pos) && !SqAttacked(F1,BLACK,pos) ) {
					AddQuietMove(pos, MOVE(E1, G1, EMPTY, EMPTY, CASTLE_F), list);
					//printf("KING-SIDE castle for white\n");
				}
			}
		}

		if(pos->castlePerm & WQ_CASTLE) {
			if(pos->pieces[D1] == EMPTY && pos->pieces[C1] == EMPTY && pos->pieces[B1] == EMPTY) {
				if(!SqAttacked(E1,BLACK,pos) && !SqAttacked(D1,BLACK,pos) ) {
					AddQuietMove(pos, MOVE(E1, C1, EMPTY, EMPTY, CASTLE_F), list);
				}
			}
		}

	} else {

		for(pieceCount = 0; pieceCount < pos->pieceCount[bP]; ++pieceCount) {
			sq = pos->pList[bP][pieceCount];
			ASSERT(SqOnBoard(sq));

			if(pos->pieces[sq - 10] == EMPTY) {
				AddBlackPawnMove(pos, sq, sq-10, list);
				if(rankArray[sq] == RANK_7 && pos->pieces[sq - 20] == EMPTY) {
					AddQuietMove(pos, MOVE(sq,(sq-20),EMPTY,EMPTY,PAWNSTART_F),list);
				}
			}

			if(!SQOFFBOARD(sq - 9) && pieceColor[pos->pieces[sq - 9]] == WHITE) {
				AddBlackPawnCapMove(pos, sq, sq-9, pos->pieces[sq - 9], list);
			}

			if(!SQOFFBOARD(sq - 11) && pieceColor[pos->pieces[sq - 11]] == WHITE) {
				AddBlackPawnCapMove(pos, sq, sq-11, pos->pieces[sq - 11], list);
			}
			if(pos->enPas != SQUARE_NONE) {
				if(sq - 9 == pos->enPas) {
					AddEnPassantMove(pos, MOVE(sq,sq - 9,EMPTY,EMPTY,ENPASSANT_F), list);
				}
				if(sq - 11 == pos->enPas) {
					AddEnPassantMove(pos, MOVE(sq,sq - 11,EMPTY,EMPTY,ENPASSANT_F), list);
				}
			}
		}

		// castling
		if(pos->castlePerm &  BK_CASTLE) {
			if(pos->pieces[F8] == EMPTY && pos->pieces[G8] == EMPTY) {
				if(!SqAttacked(E8,WHITE,pos) && !SqAttacked(F8,WHITE,pos) ) {
					AddQuietMove(pos, MOVE(E8, G8, EMPTY, EMPTY, CASTLE_F), list);
				}
			}
		}

		if(pos->castlePerm &  BQ_CASTLE) {
			if(pos->pieces[D8] == EMPTY && pos->pieces[C8] == EMPTY && pos->pieces[B8] == EMPTY) {
				if(!SqAttacked(E8,WHITE,pos) && !SqAttacked(D8,WHITE,pos) ) {
					AddQuietMove(pos, MOVE(E8, C8, EMPTY, EMPTY, CASTLE_F), list);
				}
			}
		}
	}
	

	/* Loop for slide pieces */
	pceIndex = LoopSlideIndex[side];
	pce = LoopSlidePce[pceIndex++]; // index incremented after pce is assigned
	//printf("\nPiece: %d", pce);
	while( pce != 0) {
		ASSERT(PieceValid(pce));

		for(pieceCount = 0; pieceCount < pos->pieceCount[pce]; ++pieceCount) {
			sq = pos->pList[pce][pieceCount];
			ASSERT(SqOnBoard(sq));

			for(index = 0; index < NumDir[pce]; ++index) {
				dir = PceDir[pce][index];
				t_sq = sq + dir;

				while(!SQOFFBOARD(t_sq)) {
					// BLACK ^ 1 == WHITE       WHITE ^ 1 == BLACK
					//printf("\nPiece: %d, sq: %d", pce, SQ64(t_sq));
					if(pos->pieces[t_sq] != EMPTY) {
						if( pieceColor[pos->pieces[t_sq]] == (side ^ 1)) {	// if pointing to enemy piece
							AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
						}
						break;
					}
					AddQuietMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
					t_sq += dir;
				}
			}
		}

		pce = LoopSlidePce[pceIndex++];
	}

	/* Loop for non slide */
	pceIndex = LoopNonSlideIndex[side];
	pce = LoopNonSlidePce[pceIndex++];

	while( pce != 0) {
		ASSERT(PieceValid(pce));

		for(pieceCount = 0; pieceCount < pos->pieceCount[pce]; ++pieceCount) {
			sq = pos->pList[pce][pieceCount];
			ASSERT(SqOnBoard(sq));

			for(index = 0; index < NumDir[pce]; ++index) {
				dir = PceDir[pce][index];
				t_sq = sq + dir;

				if(SQOFFBOARD(t_sq)) {
					continue;
				}

				if(pos->pieces[t_sq] != EMPTY) {
					if( pieceColor[pos->pieces[t_sq]] == (side ^ 1)) {	// if pointing to enemy piece
						AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
					}
					continue;
				}
				AddQuietMove(pos, MOVE(sq, t_sq, EMPTY, EMPTY, 0), list);
			}
		}

		pce = LoopNonSlidePce[pceIndex++];
	}

    //ASSERT(MoveListOk(list,pos));
}

// Generate all capture moves
void GenerateAllCaps(const S_BOARD *pos, S_MOVELIST *list) {

	ASSERT(checkBoard(pos));

	list->count = 0;

	int pce = EMPTY;
	int side = pos->side;
	int sq = 0; int t_sq = 0;
	int pieceCount = 0;
	int dir = 0;
	int index = 0;
	int pceIndex = 0;

	if(side == WHITE) {

		for(pieceCount = 0; pieceCount < pos->pieceCount[wP]; ++pieceCount) {
			sq = pos->pList[wP][pieceCount];
			ASSERT(SqOnBoard(sq));

			if(!SQOFFBOARD(sq + 9) && pieceColor[pos->pieces[sq + 9]] == BLACK) {
				AddWhitePawnCapMove(pos, sq, sq+9, pos->pieces[sq + 9], list);
			}
			if(!SQOFFBOARD(sq + 11) && pieceColor[pos->pieces[sq + 11]] == BLACK) {
				AddWhitePawnCapMove(pos, sq, sq+11, pos->pieces[sq + 11], list);
			}

			if(pos->enPas != SQUARE_NONE) {
				if(sq + 9 == pos->enPas) {
					AddEnPassantMove(pos, MOVE(sq,sq + 9,EMPTY,EMPTY,ENPASSANT_F), list);
				}
				if(sq + 11 == pos->enPas) {
					AddEnPassantMove(pos, MOVE(sq,sq + 11,EMPTY,EMPTY,ENPASSANT_F), list);
				}
			}
		}

	} else {

		for(pieceCount = 0; pieceCount < pos->pieceCount[bP]; ++pieceCount) {
			sq = pos->pList[bP][pieceCount];
			ASSERT(SqOnBoard(sq));

			if(!SQOFFBOARD(sq - 9) && pieceColor[pos->pieces[sq - 9]] == WHITE) {
				AddBlackPawnCapMove(pos, sq, sq-9, pos->pieces[sq - 9], list);
			}

			if(!SQOFFBOARD(sq - 11) && pieceColor[pos->pieces[sq - 11]] == WHITE) {
				AddBlackPawnCapMove(pos, sq, sq-11, pos->pieces[sq - 11], list);
			}
			if(pos->enPas != SQUARE_NONE) {
				if(sq - 9 == pos->enPas) {
					AddEnPassantMove(pos, MOVE(sq,sq - 9,EMPTY,EMPTY,ENPASSANT_F), list);
				}
				if(sq - 11 == pos->enPas) {
					AddEnPassantMove(pos, MOVE(sq,sq - 11,EMPTY,EMPTY,ENPASSANT_F), list);
				}
			}
		}
	}

	/* Loop for slide pieces */
	pceIndex = LoopSlideIndex[side];
	pce = LoopSlidePce[pceIndex++];
	while( pce != 0) {
		ASSERT(PieceValid(pce));

		for(pieceCount = 0; pieceCount < pos->pieceCount[pce]; ++pieceCount) {
			sq = pos->pList[pce][pieceCount];
			ASSERT(SqOnBoard(sq));

			for(index = 0; index < NumDir[pce]; ++index) {
				dir = PceDir[pce][index];
				t_sq = sq + dir;

				while(!SQOFFBOARD(t_sq)) {
					// BLACK ^ 1 == WHITE       WHITE ^ 1 == BLACK
					if(pos->pieces[t_sq] != EMPTY) {
						if( pieceColor[pos->pieces[t_sq]] == (side ^ 1)) {
							AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
						}
						break;
					}
					t_sq += dir;
				}
			}
		}

		pce = LoopSlidePce[pceIndex++];
	}

	/* Loop for non slide */
	pceIndex = LoopNonSlideIndex[side];
	pce = LoopNonSlidePce[pceIndex++];

	while( pce != 0) {
		ASSERT(PieceValid(pce));

		for(pieceCount = 0; pieceCount < pos->pieceCount[pce]; ++pieceCount) {
			sq = pos->pList[pce][pieceCount];
			ASSERT(SqOnBoard(sq));

			for(index = 0; index < NumDir[pce]; ++index) {
				dir = PceDir[pce][index];
				t_sq = sq + dir;

				if(SQOFFBOARD(t_sq)) {
					continue;
				}

				// BLACK ^ 1 == WHITE       WHITE ^ 1 == BLACK
				if(pos->pieces[t_sq] != EMPTY) {
					if( pieceColor[pos->pieces[t_sq]] == (side ^ 1)) {
						AddCaptureMove(pos, MOVE(sq, t_sq, pos->pieces[t_sq], EMPTY, 0), list);
					}
					continue;
				}
			}
		}

		pce = LoopNonSlidePce[pceIndex++];
	}
   //ASSERT(MoveListOk(list,pos));
}



