#include "definitions.h"
#include <stdio.h>

#define HASH_PCE(pce,sq) (pos->posKey ^= (pieceKeys[(pce)][(sq)]))	// Hash piece
#define HASH_CA (pos->posKey ^= (castleKeys[(pos->castlePerm)]))	// Hash castle permission
#define HASH_SIDE (pos->posKey ^= (sideKey))						// Hash side	
#define HASH_EP (pos->posKey ^= (pieceKeys[EMPTY][(pos->enPas)]))	// Hash en passant	

const int CastlePerm[120] = {
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 13, 15, 15, 15, 12, 15, 15, 14, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15,  7, 15, 15, 15,  3, 15, 15, 11, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15
};

static void ClearPiece(const int sq, S_BOARD *pos) {

	ASSERT(SqOnBoard(sq));
	ASSERT(checkBoard(pos));
	
    int pce = pos->pieces[sq];
	
    ASSERT(PieceValid(pce));
	
	int col = pieceColor[pce];
	int index = 0;
	int t_pceNum = -1;
	
	ASSERT(SideValid(col));
	
    HASH_PCE(pce,sq);
	
	pos->pieces[sq] = EMPTY;
    pos->material[col] -= pieceValue[pce];
	
	if(isPieceMid[pce]) {
			pos->midPieces[col]--;
		if(isPieceMajor[pce]) {
			pos->majorPieces[col]--;
		} else {
			pos->minorPieces[col]--;
		}
	} else {
		CLRBIT(pos->pawns[col],SQ64(sq));
		CLRBIT(pos->pawns[BOTH_COLORS],SQ64(sq));
	}

	// pos->pieceCount[wP] == 5, loop 0 to 4
	// pos->pList[wP][0] = E2, pos->pList[wP][1] = D2, pos->pList[wP][2] = A2, pos->pList[wP][3] = H2, pos->pList[wP][4] = C3
	// if sq = H2, t_pceNum = 3
	
	for(index = 0; index < pos->pieceCount[pce]; ++index) {
		if(pos->pList[pce][index] == sq) {
			t_pceNum = index;
			break;
		}
	}
	
	ASSERT(t_pceNum != -1);	// Check if piece was found (int t_pceNum = -1;)
	ASSERT(t_pceNum>=0 && t_pceNum<10);
	
	pos->pieceCount[pce]--;		
	// pos->pieceCount[wP] == 4
	
	pos->pList[pce][t_pceNum] = pos->pList[pce][pos->pieceCount[pce]];
	// pos->pList[wP][3] = pos->pList[wP][4]
	// Therefore, the last piece is now at pos->pList[wP][3]
	// pos->pList[wP][4] wont be read, since pos->pieceCount[wP] is now 4 (loop 0 to 3)
}


static void AddPiece(const int sq, S_BOARD *pos, const int pce) {

    ASSERT(PieceValid(pce));
    ASSERT(SqOnBoard(sq));
	
	int col = pieceColor[pce];
	ASSERT(SideValid(col));

    HASH_PCE(pce,sq);	// Hash piece into the position key
	
	pos->pieces[sq] = pce;

    if(isPieceMid[pce]) {			// If not a pawn
			pos->midPieces[col]++;
		if(isPieceMajor[pce]) {
			pos->majorPieces[col]++;
		} else {
			pos->minorPieces[col]++;
		}
	} else {
		SETBIT(pos->pawns[col],SQ64(sq));
		SETBIT(pos->pawns[BOTH_COLORS],SQ64(sq));
	}
	
	pos->material[col] += pieceValue[pce];
	pos->pList[pce][pos->pieceCount[pce]++] = sq; // Increment pieceCount after setting square to pList
	
}

static void MovePiece(const int from, const int to, S_BOARD *pos) {

    ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));
	
	int index = 0;
	int pce = pos->pieces[from];	
	int col = pieceColor[pce];
	ASSERT(SideValid(col));
    ASSERT(PieceValid(pce));
	
#ifdef DEBUG
	int t_PieceNum = FALSE;
#endif

	HASH_PCE(pce,from);
	pos->pieces[from] = EMPTY;
	
	HASH_PCE(pce,to);
	pos->pieces[to] = pce;
	
	if(!isPieceMid[pce]) {
		CLRBIT(pos->pawns[col],SQ64(from));
		CLRBIT(pos->pawns[BOTH_COLORS],SQ64(from));
		SETBIT(pos->pawns[col],SQ64(to));
		SETBIT(pos->pawns[BOTH_COLORS],SQ64(to));		
	}    
	
	for(index = 0; index < pos->pieceCount[pce]; ++index) {
		if(pos->pList[pce][index] == from) {
			pos->pList[pce][index] = to;
#ifdef DEBUG
			t_PieceNum = TRUE;
#endif
			break;
		}
	}
	ASSERT(t_PieceNum);
}

int makeMove(S_BOARD *pos, int move) {

	ASSERT(checkBoard(pos));
	
	int from = FROMSQ(move);
    int to = TOSQ(move);
    int side = pos->side;
	
	ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));
    ASSERT(SideValid(side));
    ASSERT(PieceValid(pos->pieces[from]));
	ASSERT(pos->hisPly >= 0 && pos->hisPly < MAX_GAME_MOVES);
	ASSERT(pos->ply >= 0 && pos->ply < MAX_DEPTH);
	
	pos->history[pos->hisPly].posKey = pos->posKey; // Store hash key of current game
	
	if(move & ENPASSANT_F) {
        if(side == WHITE) {
            ClearPiece(to-10,pos); // Remove white pawn
        } else {
            ClearPiece(to+10,pos); // Remove black pawn
        }
    } else if (move & CASTLE_F) {
        switch(to) {		// If king move two spaces, move rook to the other side of the king
            case C1:
                MovePiece(A1, D1, pos);
			break;
            case C8:
                MovePiece(A8, D8, pos);
			break;
            case G1:
                MovePiece(H1, F1, pos);
			break;
            case G8:
                MovePiece(H8, F8, pos);
			break;
            default: ASSERT(FALSE); break;
        }
    }	
	
	if(pos->enPas != SQUARE_NONE) HASH_EP;	// Hash out en passant square
    HASH_CA;								// Hash out castling permissions
	
	pos->history[pos->hisPly].move = move;					// Add move to history
    pos->history[pos->hisPly].fiftyMove = pos->fiftyMove;
    pos->history[pos->hisPly].enPas = pos->enPas;
    pos->history[pos->hisPly].castlePerm = pos->castlePerm;

    pos->castlePerm &= CastlePerm[from];	// Adjust castling permissions
    pos->castlePerm &= CastlePerm[to];
    pos->enPas = SQUARE_NONE;				// Reset en passant square

	HASH_CA;	// Hash in castling permissions
	
	int captured = CAPTURED(move);
    pos->fiftyMove++;
	
	if(captured != EMPTY) {				// Clear captured piece from pos
        ASSERT(PieceValid(captured));
        ClearPiece(to, pos);
        pos->fiftyMove = 0;
    }
	
	pos->hisPly++;
	pos->ply++;
	
	ASSERT(pos->hisPly >= 0 && pos->hisPly < MAX_GAME_MOVES);
	ASSERT(pos->ply >= 0 && pos->ply < MAX_DEPTH);
	
	if(isPawn[pos->pieces[from]]) {
        pos->fiftyMove = 0;
        if(move & PAWNSTART_F) {
            if(side==WHITE) {
                pos->enPas=from+10;						// Add the invisible pawn behind the white pawn (on rank 3)
				if (rankArray[pos->enPas]!=RANK_3){
					printf("enPas: %d rankArray: %d\n", SQ64(pos->enPas), rankArray[pos->enPas]);
					printf("Color: %d From: %d To: %d\n", side, SQ64(from), SQ64(to));
				}
                ASSERT(rankArray[pos->enPas]==RANK_3);
            } else {
                pos->enPas=from-10;						// Add the invisible pawn behind the black pawn (on rank 6)
                ASSERT(rankArray[pos->enPas]==RANK_6);
            }
            HASH_EP;
        }
    }
	
	MovePiece(from, to, pos);
	
	int prPce = PROMOTED(move);		// If the move is a promotion, remove the pawn and add the promoted piece
    if(prPce != EMPTY)   {
        ASSERT(PieceValid(prPce) && !isPawn[prPce]);
        ClearPiece(to, pos);
        AddPiece(to, pos, prPce);
    }
	
	if(isKing[pos->pieces[to]]) {		// If the piece moved is a king, update the king square
        pos->kingSquare[pos->side] = to;
    }
	
	pos->side ^= 1;	// Switch side using XOR
    HASH_SIDE;

    ASSERT(checkBoard(pos));
		
	if(SqAttacked(pos->kingSquare[side], pos->side, pos))  { // side != pos->side, because MovePiece was called. 
															 //	If king attacked by the next move, take back the move
        takeMove(pos);
        return FALSE;
    }
	
	return TRUE;
	
}

void takeMove(S_BOARD *pos) {
	
	ASSERT(checkBoard(pos));
	
	pos->hisPly--;
    pos->ply--;
	
	ASSERT(pos->hisPly >= 0 && pos->hisPly < MAX_GAME_MOVES);
	ASSERT(pos->ply >= 0 && pos->ply < MAX_DEPTH);
	
    int move = pos->history[pos->hisPly].move;
    int from = FROMSQ(move);// Square to move back to
    int to = TOSQ(move);	// Current square
	
	ASSERT(SqOnBoard(from));
    ASSERT(SqOnBoard(to));
	
	if(pos->enPas != SQUARE_NONE) HASH_EP;	// Hash out en passant square
    HASH_CA;								// Hash out castling permissions	

    pos->castlePerm = pos->history[pos->hisPly].castlePerm;		// Recover old castling permissions, fiftyMove and enPas
    pos->fiftyMove = pos->history[pos->hisPly].fiftyMove;
    pos->enPas = pos->history[pos->hisPly].enPas;

    if(pos->enPas != SQUARE_NONE) HASH_EP; // Hash in the old en passant square
    HASH_CA;							   // Hash in the old castling permissions

    pos->side ^= 1;	// Switch side using XOR
    HASH_SIDE;		// Hash in the side
	
	if(ENPASSANT_F & move) {
        if(pos->side == WHITE) {
            AddPiece(to-10, pos, bP);	// Add pawn infront of 'invisible' pawn
        } else {
            AddPiece(to+10, pos, wP);
        }
    } else if(CASTLE_F & move) {
        switch(to) {
            case C1: MovePiece(D1, A1, pos); break; // Move rook back to original position
            case C8: MovePiece(D8, A8, pos); break;
            case G1: MovePiece(F1, H1, pos); break;
            case G8: MovePiece(F8, H8, pos); break;
            default: ASSERT(FALSE); break;
        }
    }
	
	MovePiece(to, from, pos);
	
	if(isKing[pos->pieces[from]]) {
        pos->kingSquare[pos->side] = from;
    }
	
	int captured = CAPTURED(move);
    if(captured != EMPTY) {
        ASSERT(PieceValid(captured));
        AddPiece(to, pos, captured);
    }
	
	if(PROMOTED(move) != EMPTY)   {
        ASSERT(PieceValid(PROMOTED(move)) && !isPawn[PROMOTED(move)]);
        ClearPiece(from, pos);
        AddPiece(from, pos, (pieceColor[PROMOTED(move)] == WHITE ? wP : bP));
    }
	
    ASSERT(checkBoard(pos));

}


//void MakeNullMove(S_BOARD *pos) {
//
//    ASSERT(CheckBoard(pos));
//    ASSERT(!SqAttacked(pos->KingSq[pos->side],pos->side^1,pos));
//
//    pos->ply++;
//    pos->history[pos->hisPly].posKey = pos->posKey;
//
//    if(pos->enPas != NO_SQ) HASH_EP;
//
//    pos->history[pos->hisPly].move = NOMOVE;
//    pos->history[pos->hisPly].fiftyMove = pos->fiftyMove;
//    pos->history[pos->hisPly].enPas = pos->enPas;
//    pos->history[pos->hisPly].castlePerm = pos->castlePerm;
//    pos->enPas = NO_SQ;
//
//    pos->side ^= 1;
//    pos->hisPly++;
//    HASH_SIDE;
//   
//    ASSERT(CheckBoard(pos));
//	ASSERT(pos->hisPly >= 0 && pos->hisPly < MAXGAMEMOVES);
//	ASSERT(pos->ply >= 0 && pos->ply < MAXDEPTH);
//
//    return;
//} // MakeNullMove
//
//void TakeNullMove(S_BOARD *pos) {
//    ASSERT(CheckBoard(pos));
//
//    pos->hisPly--;
//    pos->ply--;
//
//    if(pos->enPas != NO_SQ) HASH_EP;
//
//    pos->castlePerm = pos->history[pos->hisPly].castlePerm;
//    pos->fiftyMove = pos->history[pos->hisPly].fiftyMove;
//    pos->enPas = pos->history[pos->hisPly].enPas;
//
//    if(pos->enPas != NO_SQ) HASH_EP;
//    pos->side ^= 1;
//    HASH_SIDE;
//  
//    ASSERT(CheckBoard(pos));
//	ASSERT(pos->hisPly >= 0 && pos->hisPly < MAXGAMEMOVES);
//	ASSERT(pos->ply >= 0 && pos->ply < MAXDEPTH);
//}