#include "definitions.h"
#include <stdio.h>


int rootDepth;

static void CheckUp(S_SEARCHINFO *info) {// check if time up, or interrupt from GUI
	if(info->timeset == TRUE && GetTimeMs() > info->stoptime) {
		info->stopped = TRUE;
	}

	//ReadInput(info);
}

static void PickNextMove(int moveNum, S_MOVELIST *list) {

	S_MOVE temp;
	int index = 0;
	int bestScore = 0;
	int bestNum = moveNum;

	for (index = moveNum; index < list->count; ++index) {
		if (list->moves[index].score > bestScore) {
			bestScore = list->moves[index].score;
			bestNum = index;
		}
	}

	ASSERT(moveNum>=0 && moveNum<list->count);
	ASSERT(bestNum>=0 && bestNum<list->count);
	ASSERT(bestNum>=moveNum);

	temp = list->moves[moveNum];
	list->moves[moveNum] = list->moves[bestNum];
	list->moves[bestNum] = temp;
}

static int IsRepetition(const S_BOARD *pos) {

	int index = 0;
	// If fiftyMove is reset to zero, then a repetition is not possible (since its impossible to move a pawn back, recapture, promote, ect)
	// Only need to check for repetistions up untill fiftyMove was reset to 0
	for(index = pos->hisPly - pos->fiftyMove; index < pos->hisPly-1; ++index) {	
		ASSERT(index >= 0 && index < MAXGAMEMOVES);
		if(pos->posKey == pos->history[index].posKey) {
			return TRUE;
		}
	}
	return FALSE;
}

static void ClearForSearch(S_BOARD *pos, S_SEARCHINFO *info) { // clear search info

	int index = 0;
	int index2 = 0;

	for(index = 0; index < 13; ++index) {
		for(index2 = 0; index2 < BOARD_ARRAY_SIZE; ++index2) {
			pos->searchHistory[index][index2] = 0;
		}
	}

	for(index = 0; index < 2; ++index) {
		for(index2 = 0; index2 < MAX_DEPTH; ++index2) {
			pos->searchKillers[index][index2] = 0;
		}
	}

	//ClearPvTable(pos->PvTable);

	pos->PvTable->overWrite=0;
	pos->PvTable->hit=0;
	pos->PvTable->cut=0;
	pos->ply = 0;		

	info->stopped = 0;
	info->nodes = 0;
	info->fh = 0;
	info->fhf = 0;
}

static int Quiescence(int alpha, int beta, S_BOARD *pos, S_SEARCHINFO *info) { // search only captures moves, prevent horizon effect, see wiki

	ASSERT(CheckBoard(pos));
	ASSERT(beta>alpha);
	if(( info->nodes & 2047 ) == 0) {
		CheckUp(info);
	}

	info->nodes++;

	if(IsRepetition(pos) || pos->fiftyMove >= 100) {
		return 0;
	}

	if(pos->ply > MAX_DEPTH - 1) {
		return EvalPosition(pos);
	}

	int Score = EvalPosition(pos);

	ASSERT(Score>-INFINITE && Score<INFINITE);

	if(Score >= beta) {
		return beta;
	}

	if(Score > alpha) {
		alpha = Score;
	}

	S_MOVELIST list[1];
    //GenerateAllCaps(pos,list);

   int MoveNum = 0;
	int Legal = 0;
	Score = -INFINITE;

	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {

		PickNextMove(MoveNum, list);

       if ( !makeMove(pos,list->moves[MoveNum].move))  {
           continue;
       }

		Legal++;
		Score = -Quiescence( -beta, -alpha, pos, info);
       takeMove(pos);

		if(info->stopped == TRUE) {
			return 0;
		}

		if(Score > alpha) {
			if(Score >= beta) {
				if(Legal==1) {
					info->fhf++;
				}
				info->fh++;
				return beta;
			}
			alpha = Score;
		}
   }

	ASSERT(alpha >= OldAlpha);

	return alpha;
}

static int AlphaBeta(int alpha, int beta, int depth, S_BOARD *pos, S_SEARCHINFO *info, int DoNull) {

	ASSERT(CheckBoard(pos));
	ASSERT(beta>alpha);
	ASSERT(depth>=0);

	if(depth <= 0) {
		return Quiescence(alpha, beta, pos, info);
		// return EvalPosition(pos);
	}

	if(( info->nodes & 2047 ) == 0) {
		CheckUp(info);
	}

	info->nodes++;

	if((IsRepetition(pos) || pos->fiftyMove >= 100) && pos->ply) {
		return 0;
	}

	if(pos->ply > MAX_DEPTH - 1) {
		return EvalPosition(pos);
	}

	int InCheck = SqAttacked(pos->kingSquare[pos->side],pos->side^1,pos);

	if(InCheck == TRUE) {
		depth++;
	}

	int Score = -INFINITE;
	int PvMove = NO_MOVE;

	// if( ProbePvEntry(pos, &PvMove, &Score, alpha, beta, depth) == TRUE ) {
	// 	pos->PvTable->cut++;
	// 	return Score;
	// }

	// if( DoNull && !InCheck && pos->ply && (pos->bigPce[pos->side] > 0) && depth >= 4) {
	// 	MakeNullMove(pos);
	// 	Score = -AlphaBeta( -beta, -beta + 1, depth-4, pos, info, FALSE);
	// 	TakeNullMove(pos);
	// 	if(info->stopped == TRUE) {
	// 		return 0;
	// 	}

	// 	if (Score >= beta && abs(Score) < ISMATE) {
	// 		info->nullCut++;
	// 		return beta;
	// 	}
	// }

	S_MOVELIST list[1];
   	generateAllMoves(pos,list);

   	int MoveNum = 0;
	int Legal = 0;
	int OldAlpha = alpha;
	int BestMove = NO_MOVE;
	int BestScore = -INFINITE;

	Score = -INFINITE;

	if( PvMove != NO_MOVE) {
		for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {
			if( list->moves[MoveNum].move == PvMove) {
				list->moves[MoveNum].score = 2000000;
				//printf("Pv move found \n");
				break;
			}
		}
	}

	for(MoveNum = 0; MoveNum < list->count; ++MoveNum) {

		PickNextMove(MoveNum, list);

       if ( !makeMove(pos,list->moves[MoveNum].move))  {
           continue;
       }

		Legal++;
		Score = -AlphaBeta( -beta, -alpha, depth-1, pos, info, TRUE);
		takeMove(pos);

		if(info->stopped == TRUE) {
			return 0;
		}
		if(Score > BestScore) {
			BestScore = Score;
			BestMove = list->moves[MoveNum].move;
			if(Score > alpha) {  // update alpha if score is better
				if(Score >= beta) { // beta cut-off, else go to next move
					if(Legal==1) {
						info->fhf++;	// the best move was found first
					}
					info->fh++;	//

					if(!(list->moves[MoveNum].move & CAPTURE_F)) {
						pos->searchKillers[1][pos->ply] = pos->searchKillers[0][pos->ply];
						pos->searchKillers[0][pos->ply] = list->moves[MoveNum].move;
					}

					StorePvEntry(pos, BestMove, beta, HFBETA, depth);

					return beta;
				}
				alpha = Score;

				if(!(list->moves[MoveNum].move & CAPTURE_F)) {
					pos->searchHistory[pos->pieces[FROMSQ(BestMove)]][TOSQ(BestMove)] += depth;
				}
			}
		}
   }

	if(Legal == 0) {	// If no legal moves found
		if(InCheck) {
			return -INFINITE + pos->ply;
		} else {
			return 0;
		}
	}

	ASSERT(alpha>=OldAlpha);

	if(alpha != OldAlpha) {	// If alpha was improved, store the best move in the PvTable
		StorePvEntry(pos, BestMove, BestScore, HFEXACT, depth);
	} else {
		StorePvEntry(pos, BestMove, alpha, HFALPHA, depth);
	}

	return alpha;
}

void SearchPosition(S_BOARD *pos, S_SEARCHINFO *info) { // iterative deepening
	// for depth = 1 to max depth
		// call AlphaBeta with depth
		// if time up, 
			//break
		// if depth = max depth, 
			//print best move

	int bestMove = NO_MOVE;
	int bestScore = -INFINITE;
	int currentDepth = 0;
	int pvMoves = 0;
	int pvNum = 0;

	ClearForSearch(pos,info);
	
	// if(EngineOptions->UseBook == TRUE) {
	// 	bestMove = GetBookMove(pos);
	// }

	//printf("Search depth:%d\n",info->depth);

	// iterative deepening
	if(bestMove == NO_MOVE) {
		for( currentDepth = 1; currentDepth <= info->depth; ++currentDepth ) {
			rootDepth = currentDepth;
								  // alpha,  beta
			bestScore = AlphaBeta(-INFINITE, INFINITE, currentDepth, pos, info, TRUE);

			if(info->stopped == TRUE) {
				break;
			}

			pvMoves = GetPvLine(currentDepth, pos);
			bestMove = pos->PvArray[0];
			if(info->GAME_MODE == UCIMODE) {
				printf("info score cp %d depth %d nodes %ld time %d ",
					bestScore,currentDepth,info->nodes,GetTimeMs()-info->starttime);
			} else if(info->GAME_MODE == XBOARDMODE && info->POST_THINKING == TRUE) {
				printf("%d %d %d %ld ",
					currentDepth,bestScore,(GetTimeMs()-info->starttime)/10,info->nodes);
			} else if(info->POST_THINKING == TRUE) {
				printf("score:%d depth:%d nodes:%ld time:%d(ms) ",
					bestScore,currentDepth,info->nodes,GetTimeMs()-info->starttime);
			}
			if(info->GAME_MODE == UCIMODE || info->POST_THINKING == TRUE) {		// Print the moves that lead to the best move (pvLine)
				pvMoves = GetPvLine(currentDepth, pos);
				if(!info->GAME_MODE == XBOARDMODE) {
					printf("pv");
				}
				for(pvNum = 0; pvNum < pvMoves; ++pvNum) {
					printf(" %s",printMove(pos->PvArray[pvNum]));
				}
				printf("\n");
			}

			//printf("Hits:%d Overwrite:%d NewWrite:%d Cut:%d\nOrdering %.2f NullCut:%d\n",pos->PvTable->hit,pos->PvTable->overWrite,pos->PvTable->newWrite,pos->PvTable->cut,
			//(info->fhf/info->fh)*100,info->nullCut);
		}
	}

	if(info->GAME_MODE == UCIMODE) {
		printf("bestmove %s\n",printMove(bestMove));
	} else if(info->GAME_MODE == XBOARDMODE) {
		printf("move %s\n",printMove(bestMove));
		makeMove(pos, bestMove);
	} else {
		printf("\n\n***!! Vice makes move %s !!***\n\n",printMove(bestMove));
		makeMove(pos, bestMove);
		printBoard(pos);
	}

}



















