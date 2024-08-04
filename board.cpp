#include "definitions.h"



void updateListsMaterial(S_BOARD* pos) {
	int piece, sq, index, color;

	for (index = 0; index < BOARD_ARRAY_SIZE; ++index) {
		sq = index;
		piece = pos->pieces[index];
		//printf("Piece %d\n", pos->pieces[index]);
		if (piece != OFFBOARD && piece != EMPTY) {
			color = pieceColor[piece];

			if (piece == wP) {
				SETBIT(pos->pawns[WHITE], SQ64(sq));
				SETBIT(pos->pawns[BOTH_COLORS], SQ64(sq));
			}
			else if (piece == bP) {
				SETBIT(pos->pawns[BLACK], SQ64(sq));
				SETBIT(pos->pawns[BOTH_COLORS], SQ64(sq));
			}

			if (isPieceMajor[piece] == TRUE) pos->majorPieces[color]++;
			if (isPieceMid[piece] == TRUE) pos->midPieces[color]++;
			if (isPieceMinor[piece] == TRUE) pos->minorPieces[color]++;

			pos->material[color] += pieceValue[piece];

			pos->pList[piece][pos->pieceCount[piece]] = sq;
			pos->pieceCount[piece]++;

			if (piece == wK) pos->kingSquare[WHITE] = sq;
			if (piece == bK) pos->kingSquare[BLACK] = sq;

			//for (int t_piece = wP; t_piece <= bK; ++t_piece) {
			//	//ASSERT(t_pieceCount[t_piece] == pos->pieceCount[t_piece]); // check if the number of pieces is correct
			//	printf("%d", pos->pieceCount[t_piece]);
			//}
		}
	}
}

int parseFEN(const char* fen, S_BOARD* pos) {

	ASSERT(fen != NULL);
	ASSERT(pos != NULL);

	int  rank = RANK_8;
	int  file = FILE_A;
	int  piece = 0;
	int  count = 0;
	int  i = 0;
	int  sq64 = 0;
	int  sq120 = 0;

	resetBoard(pos);

	while ((rank >= RANK_1) && *fen) {
		count = 1;
		switch (*fen) {
		case 'p': piece = bP; break;
		case 'r': piece = bR; break;
		case 'n': piece = bN; break;
		case 'b': piece = bB; break;
		case 'k': piece = bK; break;
		case 'q': piece = bQ; break;
		case 'P': piece = wP; break;
		case 'R': piece = wR; break;
		case 'N': piece = wN; break;
		case 'B': piece = wB; break;
		case 'K': piece = wK; break;
		case 'Q': piece = wQ; break;

		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
			piece = EMPTY;
			count = *fen - '0';	// difference between 0 and the number
			break;

		case '/':
		case ' ':
			rank--;
			file = FILE_A;
			fen++;
			continue;

		default:
			printf("FEN error \n");
			return -1;
		}

		for (i = 0; i < count; i++) {
			sq64 = rank * 8 + file;
			sq120 = SQ120(sq64);
			if (piece != EMPTY) {
				pos->pieces[sq120] = piece;
			}
			file++;
			//printf("p: %d", pos->pieces[sq120]);
		}
		fen++;
	}

	ASSERT(*fen == 'w' || *fen == 'b');

	pos->side = (*fen == 'w') ? WHITE : BLACK;
	fen += 2;

	for (i = 0; i < 4; i++) {
		if (*fen == ' ') {
			break;
		}
		switch (*fen) {
		case 'K': pos->castlePerm |= WK_CASTLE; break;
		case 'Q': pos->castlePerm |= WQ_CASTLE; break;
		case 'k': pos->castlePerm |= BK_CASTLE; break;
		case 'q': pos->castlePerm |= BQ_CASTLE; break;
		default:	     break;
		}
		fen++;
	}
	fen++;

	ASSERT(pos->castlePerm >= 0 && pos->castlePerm <= 15);

	if (*fen != '-') {
		file = fen[0] - 'a';
		rank = fen[1] - '1';

		ASSERT(file >= FILE_A && file <= FILE_H);
		ASSERT(rank >= RANK_1 && rank <= RANK_8);

		pos->enPas = FR_TO_SQ(file, rank);
	}

	pos->posKey = generatePosKey(pos);
	//printf("posKey2: %llX\n", pos->posKey);

	updateListsMaterial(pos);
	return 0;
}

void resetBoard(S_BOARD* pos) {
	int index = 0;

	for (index = 0; index < BOARD_ARRAY_SIZE; ++index) {
		pos->pieces[index] = OFFBOARD;
	}

	for (index = 0; index < 64; ++index) {
		pos->pieces[SQ120(index)] = EMPTY;
	}

	for (index = 0; index < 2; ++index) {
		pos->majorPieces[index] = 0;
		pos->midPieces[index] = 0;
		pos->minorPieces[index] = 0;
		pos->material[index] = 0;
	}

	for (index = 0; index < 3; ++index) {
		pos->pawns[index] = 0ULL;
	}

	for (index = 0; index < 13; ++index) {
		pos->pieceCount[index] = 0;
	}

	for (index = 0; index < 2; ++index) {
		pos->kingSquare[index] = SQUARE_NONE;
	}

	pos->side = BOTH_COLORS;
	pos->enPas = SQUARE_NONE;
	pos->fiftyMove = 0;

	pos->ply = 0;
	pos->hisPly = 0;
	pos->castlePerm = 0;

	pos->posKey = 0ULL;

	//InitPvTable(pos->PvTable, 2);
}

void printBoard(const S_BOARD* pos) {
	int sq, file, rank, piece;

	printf("\nGame Board:\n\n");

	for (rank = RANK_8; rank >= RANK_1; rank--) {
		printf("%d  ", rank + 1);
		for (file = FILE_A; file <= FILE_H; file++) {
			sq = FR_TO_SQ(file, rank);
			piece = pos->pieces[sq];
			printf("%3c", pieceChar[piece]);
		}
		printf("\n");
	}

	printf("\n   ");
	for (file = FILE_A; file <= FILE_H; file++) {
		printf("%3c", 'a' + file);
	}

	printf("\n");
	printf("side:%c\n", sideChar[pos->side]);
	printf("enPas:%d\n", SQ64(pos->enPas));
	printf("castle:%c, %c, %c, %c\n",
		pos->castlePerm & WK_CASTLE ? 'K' : '-',
		pos->castlePerm & WQ_CASTLE ? 'Q' : '-',
		pos->castlePerm & BK_CASTLE ? 'k' : '-',
		pos->castlePerm & BQ_CASTLE ? 'q' : '-'
	);
	printf("PosKey:%llX\n", pos->posKey);
}

int checkBoard(const S_BOARD* pos) {
	// t_ are temp variables
	int t_pieceCount[13] = { 0 };
	int t_pieceBig[2] = { 0 };
	int t_pieceMaj[2] = { 0 };
	int t_pieceMin[2] = { 0 };
	int t_pieceVal[2] = { 0 };
	u64 t_pawns[3] = { 0ULL };
	int sq64, sq120, pcount;

	t_pawns[WHITE] = pos->pawns[WHITE];
	t_pawns[BLACK] = pos->pawns[BLACK];
	t_pawns[BOTH_COLORS] = pos->pawns[BOTH_COLORS];

	for (int t_piece = wP; t_piece <= bK; ++t_piece) {
		for (int t_pieceNum = 0; t_pieceNum < pos->pieceCount[t_piece]; ++t_pieceNum) {
			int sq120 = pos->pList[t_piece][t_pieceNum];
			ASSERT(pos->pieces[sq120] == t_piece);
		}
	}

	for (int sq64 = 0; sq64 < 64; ++sq64) {
		int sq120 = SQ120(sq64);
		int t_piece = pos->pieces[sq120];
		t_pieceCount[t_piece]++;
		int color = pieceColor[t_piece];
		if (isPieceMajor[t_piece] == TRUE) t_pieceBig[color]++;
		if (isPieceMid[t_piece] == TRUE) t_pieceMaj[color]++;
		if (isPieceMinor[t_piece] == TRUE) t_pieceMin[color]++;
		t_pieceVal[color] += pieceValue[t_piece];
	}


	for (int t_piece = wP; t_piece <= bK; ++t_piece) {
		ASSERT(t_pieceCount[t_piece] == pos->pieceCount[t_piece]); // check if the number of pieces is correct
		//printf("t_piece:%d pieceCount:%d\n", t_pieceCount[t_piece], pos->pieceCount[t_piece]);
	}

	pcount = CNT(t_pawns[WHITE]);
	ASSERT(pcount == pos->pieceCount[wP]);
	pcount = CNT(t_pawns[BLACK]);
	ASSERT(pcount == pos->pieceCount[bP]);
	pcount = CNT(t_pawns[BOTH_COLORS]);
	ASSERT(pcount == (pos->pieceCount[wP] + pos->pieceCount[bP]));

	while (t_pawns[WHITE]) {
		sq64 = POP(&t_pawns[WHITE]);
		ASSERT(pos->pieces[SQ120(sq64)] == wP);
	}

	while (t_pawns[BLACK]) {
		sq64 = POP(&t_pawns[BLACK]);
		ASSERT(pos->pieces[SQ120(sq64)] == bP);
	}

	while (t_pawns[BOTH_COLORS]) {
		sq64 = POP(&t_pawns[BOTH_COLORS]);
		ASSERT(pos->pieces[SQ120(sq64)] == wP || pos->pieces[SQ120(sq64)] == bP);
	}

	ASSERT(t_pieceVal[WHITE] == pos->material[WHITE] && t_pieceVal[BLACK] == pos->material[BLACK]);
	ASSERT(t_pieceBig[WHITE] == pos->majorPieces[WHITE] && t_pieceBig[BLACK] == pos->majorPieces[BLACK]);
	ASSERT(t_pieceMaj[WHITE] == pos->midPieces[WHITE] && t_pieceMaj[BLACK] == pos->midPieces[BLACK]);
	ASSERT(t_pieceMin[WHITE] == pos->minorPieces[WHITE] && t_pieceMin[BLACK] == pos->minorPieces[BLACK]);
	ASSERT(pos->side == WHITE || pos->side == BLACK);
	ASSERT(generatePosKey(pos) == pos->posKey);

	ASSERT(pos->enPas == SQUARE_NONE || (rankArray[pos->enPas] == RANK_6 && pos->side == WHITE) || (rankArray[pos->enPas] == RANK_3 && pos->side == BLACK));
	ASSERT(pos->pieces[pos->kingSquare[WHITE]] == wK);
	ASSERT(pos->pieces[pos->kingSquare[BLACK]] == bK);
	return TRUE;
}