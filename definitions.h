//#pragma once
#include <stdlib.h>
#include <stdio.h>

#define DEBUG

#ifndef DEBUG
#define ASSERT(n)
#else
#define ASSERT(n) \
if(!(n)) { \
printf("%s - Failed",#n); \
printf("On %s ",__DATE__); \
printf("At %s ",__TIME__); \
printf("In File %s ",__FILE__); \
printf("At Line %d\n",__LINE__); \
exit(1);}
#endif
// ASSERT(1 == 5) will print the error message and exit the program

#ifndef INIT_H
#define INIT_H

typedef unsigned long long u64; // 64-bit unsigned integer

#define BOARD_ARRAY_SIZE 120
#define MAX_GAME_MOVES 2048
#define MAX_POSITION_MOVES 256
#define MAX_DEPTH 64
#define MAX_HASH 0x100000 * 1024
#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define NO_MOVE 0
#define INFINITE 30000

enum { EMPTY, wP, wN, wB, wR, wQ, wK, bP, bN, bB, bR, bQ, bK };
enum {FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, FILE_NONE};
enum {RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, RANK_NONE};
enum {WHITE, BLACK, BOTH_COLORS};
enum {
	A1 = 21, B1, C1, D1, E1, F1, G1, H1,
	A2 = 31, B2, C2, D2, E2, F2, G2, H2,
	A3 = 41, B3, C3, D3, E3, F3, G3, H3,
	A4 = 51, B4, C4, D4, E4, F4, G4, H4,
	A5 = 61, B5, C5, D5, E5, F5, G5, H5,
	A6 = 71, B6, C6, D6, E6, F6, G6, H6,
	A7 = 81, B7, C7, D7, E7, F7, G7, H7,
	A8 = 91, B8, C8, D8, E8, F8, G8, H8, SQUARE_NONE, OFFBOARD // SQUARE_NONE: 99, OFFBOARD: 100
};
enum { FALSE, TRUE };
enum {  HFNONE, HFALPHA, HFBETA, HFEXACT}; // hash flag
enum { UCIMODE, XBOARDMODE, CONSOLEMODE }; // game mode
enum { WK_CASTLE = 1, WQ_CASTLE = 2, BK_CASTLE = 4, BQ_CASTLE = 8 };// 1 0 0 1 Means white can castle kingside and black can castle queenside

typedef struct {
	int move;
	int score;
} S_MOVE;	

// int move: 
// 0000 0000 0000 0000 0000 0111 1111 -> From			0X7F
// 0000 0000 0000 0011 1111 1000 0000 -> To				>> 7, 0x7F
// 0000 0000 0011 1100 0000 0000 0000 -> Captured piece	>> 14, 0xF
// 0000 0000 0100 0000 0000 0000 0000 -> En passant		0x40000
// 0000 0000 1000 0000 0000 0000 0000 -> Pawn start		0x80000
// 0000 1111 0000 0000 0000 0000 0000 -> Promoted piece >> 20, 0xF
// 0001 0000 0000 0000 0000 0000 0000 -> Castle			0x1000000

typedef struct
{
	S_MOVE moves[MAX_POSITION_MOVES];
	int count;
} S_MOVELIST;

typedef struct {
	u64 posKey;
	int move;
} S_PVENTRY;

typedef struct {
	S_PVENTRY *pTable;
	int numEntries;
	int newWrite;
	int overWrite;
	int hit;
	int cut;
} S_PVTABLE;

#define FROMSQ(m) ((m) & 0x7F)				// int move: from square
#define TOSQ(m) (((m) >> 7) & 0x7F)			// int move: to square
#define CAPTURED(m) (((m) >> 14) & 0xF)		// get captured peice from
#define PROMOTED(m) (((m) >> 20) & 0xF)		// get promoted peice from move
#define ENPASSANT_F (0x40000)				// en passant flag
#define PAWNSTART_F (0x80000)				// pawn start flag
#define CASTLE_F (0x1000000)				// castle flag
#define CAPTURE_F (0x7C000)					// capture flag
#define PROMOTION_F (0xF00000)				// promotion flag

typedef struct {
	int move;
	int validCastles;
	int enPas;
	int fiftyMove;
	int castlePerm;
	u64 posKey;
} S_UNDO;

typedef struct {
	int pieces[BOARD_ARRAY_SIZE];	// wP, wB, bQ, ect
	u64 pawns[3];					// 0: white, 1: black, 2: both
	int kingSquare[2];				// 0: white, 1: black
	int side;						// 0: white, 1: black
	int enPas;						// The square where the 'invisible' pawn is
	int fiftyMove;					// 50 move rule
	int castlePerm;					// castle permissions
	int ply;						// half moves played in current search
	int hisPly;						// half moves played in total
	u64 posKey;						// hash key for each unique position
	//	See data.cpp:
	int pieceCount[13];				// number of pieces (pceNum) { 0, 8, 2, 2, 2, 1, 1, 8, 2, 2, 2, 1, 1 };
	int majorPieces[2];				// number of rooks and queens
	int midPieces[2];				// number of bishops and knights
	int minorPieces[2];				// number of pawns
	int material[2];				// material value of each side

	S_UNDO history[MAX_GAME_MOVES];	// history of moves

	int pList[13][10];				// A 120-square for each piece (10) of each type (13)
									// 10 is the max number for pieces of one type
									// e.g setting first pawn to E1: pList[wP][0] = E1;
									// Can loop using pList[wP][i] until it equals NO_SQUARE

	S_PVTABLE PvTable[1];
	int PvArray[MAX_DEPTH];

	int searchHistory[13][BOARD_ARRAY_SIZE];	// history of moves in search tree
	int searchKillers[2][MAX_DEPTH];			// history of moves that caused a beta cut-off, two non-capturing moves

} S_BOARD;

typedef struct {
	int starttime;
	int stoptime;
	int depth;
	int timeset;
	int movestogo;
	long nodes;	// count of all posistions visited in tree

	int quit;
	int stopped;
	float fh;	// fail high
	float fhf;  // fail high first
	int nullCut;
	int GAME_MODE;	//	
	int POST_THINKING;
} S_SEARCHINFO;

// MACROS
#define FR_TO_SQ(f, r) ( (21 + (f) ) + ( (r) * 10 ) ) // converts file and rank to index of 120-array square
#define SQ64(sq120) (Sq120ToSq64[(sq120)])			// converts index of 120-array square to index of 64-array square
#define SQ120(sq64) (Sq64ToSq120[(sq64)])			// converts index of 64-array square to index of 120-array square
#define POP(b) popBit(b)							// pop the least significant bit
#define CNT(b) countBits(b)							// count the number of bits set
#define CLRBIT(bb, sq) ((bb) &= clearMask[(sq)])	// clear the bit at sq
#define SETBIT(bb, sq) ((bb) |= setMask[(sq)])		// set the bit at sq
#define isBQ(p) (isBishopQueen[(p)])				// check if piece is bishop or queen
#define isRQ(p) (isRookQueen[(p)])					// check if piece is rook or queen
#define isKn(p) (isKnight[(p)])						// check if piece is knight
#define isKi(p) (isKing[(p)])						// check if piece is king
#define MIRROR64(sq) (Mirror64[(sq)])				// mirror the board (64-square)

// GLOBALS
extern int Sq120ToSq64[BOARD_ARRAY_SIZE];	// 120 element array to 64 element array
extern int Sq64ToSq120[64];					// 64 element array to 120 element array
extern u64 setMask[64];
extern u64 clearMask[64];

extern u64 pieceKeys[13][120];				// 13 pieces, 120 squares: hash key for each piece for each square
extern u64 sideKey;
extern u64 castleKeys[16];

extern char pieceChar[];
extern char sideChar[];
extern char rankChar[];
extern char fileChar[];

extern int isPieceMid[13];
extern int isPieceMajor[13];
extern int isPieceMinor[13];
extern int pieceValue[13];
extern int pieceColor[13];

extern int fileArray[BOARD_ARRAY_SIZE];
extern int rankArray[BOARD_ARRAY_SIZE];

// To check a square for peice type in attack.cpp
extern int isPawn[13];
extern int isKnight[13];
extern int isKing[13];
extern int isRookQueen[13];
extern int isBishopQueen[13];
extern int isSliding[13]; // Queen, Rook, Bishop
extern int Mirror64[64];

// FUNCTIONS

// init.cpp
extern void AllInit();
void InitSq120To64();
void InitBitMasks();

// bitBoard.cpp
extern void printBitBoard(u64 bitboard);
int popBit(u64 *bb);
int countBits(u64 b);

// data.cpp
extern void updateListsMaterial(S_BOARD *pos);
extern void resetBoard(S_BOARD *pos);

// io.cpp
extern int parseFEN(const char *fen, S_BOARD *pos);
extern void printBoard(const S_BOARD *pos);
extern char *printMove(const int move);
extern char *printSquare(const int sq);
extern void printMoveList(const S_MOVELIST *list);

// validate.cpp
extern int SqOnBoard(const int sq);
extern int SideValid(const int side);
extern int FileRankValid(const int fr);
extern int PieceValidEmpty(const int pce);
extern int PieceValid(const int pce);
extern void MirrorEvalTest(S_BOARD *pos);

// moveGen.cpp
extern void generateAllMoves(const S_BOARD *pos, S_MOVELIST *list);
extern void GenerateAllCaps(const S_BOARD *pos, S_MOVELIST *list);
extern void InitMvvLva();

// makeMove.cpp
extern int makeMove(S_BOARD *pos, int move);
extern void takeMove(S_BOARD *pos);
extern int MoveExists(S_BOARD *pos, const int move);

// perft.cpp
extern void perftTest(int depth, S_BOARD *pos);

// search.cpp
extern int GetTimeMs();
extern void SearchPosition(S_BOARD *pos, S_SEARCHINFO *info);

// pvTable.cpp
extern void InitPvTable(S_PVTABLE *table, const int MB);
extern void ClearPvTable(S_PVTABLE *table);
extern int ProbePvMove(const S_BOARD *pos);
extern int GetPvLine(const int depth, S_BOARD *pos);	// get a list of moves that lead to the best move
extern void StorePvEntry(S_BOARD *pos, const int move, int score, const int flags, const int depth);
extern int ProbePvEntry(S_BOARD *pos, int *move, int *score, int alpha, int beta, int depth);

// board.cpp
extern int checkBoard(const S_BOARD *pos);
extern u64 generatePosKey(const S_BOARD *pos);
extern int SqAttacked(const int sq, const int side, const S_BOARD* pos);
extern int parseMove(char *ptrChar, S_BOARD *pos);
extern void mirrorBoard(S_BOARD *pos);

// evaluate.cpp
extern int EvalPosition(const S_BOARD *pos);

// hashkeys.cpp
// (No functions listed in the provided code snippet)

// attack.cpp
// (No functions listed in the provided code snippet)

// xboard.cpp
extern void Console_Loop(S_BOARD *pos, S_SEARCHINFO *info); 


#endif