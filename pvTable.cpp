#include "definitions.h"
#include <stdio.h>


int ProbePvMove(const S_BOARD *pos) {
	int index = pos->posKey % pos->PvTable->numEntries;		// get index of key in the hash table
	ASSERT(index >= 0 && index <= pos->PvTable->numEntries - 1);
	
	//printf("Move %s, Key %llx, Index %d\n", pos->PvTable->pTable[index].move, pos->posKey, index);
	
	if( pos->PvTable->pTable[index].posKey == pos->posKey ) {	// if the key is found in the hash table
		return pos->PvTable->pTable[index].move;				// return the move
	}
	
	return NO_MOVE;
}

int GetPvLine(const int depth, S_BOARD *pos) {	// Fills PvArray with the best line from the hash table

	ASSERT(depth < MAX_DEPTH && depth >= 1);

	int move = ProbePvMove(pos);
	int count = 0;
	//printf("Pv move: %s\n", printMove(move));

	while(move != NO_MOVE && count < depth) {
		ASSERT(count < MAX_DEPTH);
	
		if( MoveExists(pos, move) ) {
			makeMove(pos, move);
			pos->PvArray[count++] = move;
		} else {
			break;
		}		
		move = ProbePvMove(pos);	
	}
	
	while(pos->ply > 0) {	// Restart to depth 0
		takeMove(pos);
	}
	
	return count;
	
}

void ClearPvTable(S_PVTABLE *table) {

  S_PVENTRY *tableEntry;
  
  for (tableEntry = table->pTable; tableEntry < table->pTable + table->numEntries; tableEntry++) {
    tableEntry->posKey = 0ULL;
    tableEntry->move = NO_MOVE;
    // tableEntry->depth = 0;
    // tableEntry->score = 0;
    // tableEntry->flags = 0;
  }
  table->newWrite=0;
}

void InitPvTable(S_PVTABLE *table, const int MB) {  
	
	int HashSize = 0x100000 * MB;
    table->numEntries = HashSize / sizeof(S_PVENTRY);	// How many entries can be fitted in the table
    table->numEntries -= 2;
	
// 	if (table->pTable != nullptr) {
//     delete[] table->pTable;  // Free old table
// }
// 	// Dynamic allocation:
// 	table->pTable = new S_PVENTRY[table->numEntries];

	if (table->pTable != NULL) {
    free(table->pTable);  // Free old table
	}
	// Dynamic allocation:
	table->pTable = (S_PVENTRY*)malloc(table->numEntries * sizeof(S_PVENTRY));

	if(table->pTable == NULL) {
		printf("Hash Allocation Failed, trying %dMB...\n",MB/2);
		InitPvTable(table,MB/2);
	} else {
		ClearPvTable(table);
		printf("PvTable init complete with %d entries\n",table->numEntries);
	}
	
}

int ProbePvEntry(S_BOARD *pos, int *move, int *score, int alpha, int beta, int depth) {

	int index = pos->posKey % pos->PvTable->numEntries;
	
	ASSERT(index >= 0 && index <= pos->PvTable->numEntries - 1);
    ASSERT(depth>=1&&depth<MAX_DEPTH);
    ASSERT(alpha<beta);
    ASSERT(alpha>=-INFINITE&&alpha<=INFINITE);
    ASSERT(beta>=-INFINITE&&beta<=INFINITE);
    ASSERT(pos->ply>=0&&pos->ply<MAX_DEPTH);
	
	if( pos->PvTable->pTable[index].posKey == pos->posKey ) {
		*move = pos->PvTable->pTable[index].move;
		/*if(pos->PvTable->pTable[index].depth >= depth){
			pos->PvTable->hit++;
			
			ASSERT(pos->PvTable->pTable[index].depth>=1&&pos->PvTable->pTable[index].depth<MAX_DEPTH);
            ASSERT(pos->PvTable->pTable[index].flags>=HFALPHA&&pos->PvTable->pTable[index].flags<=HFEXACT);
			
			*score = pos->PvTable->pTable[index].score;
			if(*score > ISMATE) *score -= pos->ply;
            else if(*score < -ISMATE) *score += pos->ply;
			
			switch(pos->PvTable->pTable[index].flags) {
				
                ASSERT(*score>=-INFINITE&&*score<=INFINITE);

                // case HFALPHA: if(*score<=alpha) {
                //     *score=alpha;
                //     return TRUE;
                //     }
                //     break;
                // case HFBETA: if(*score>=beta) {
                //     *score=beta;
                //     return TRUE;
                //     }
                //     break;
                // case HFEXACT:
                //     return TRUE;
                //     break;
                // default: ASSERT(FALSE); break;
            }
		}*/
	}
	
	return FALSE;
}

void StorePvEntry(S_BOARD *pos, const int move, int score, const int flags, const int depth) {

	int index = pos->posKey % pos->PvTable->numEntries;	// Get the hash index from the key (revise hash tables)
	
	ASSERT(index >= 0 && index <= pos->PvTable->numEntries - 1);
	//ASSERT(depth>=1&&depth<MAX_DEPTH);
    //ASSERT(flags>=HFALPHA&&flags<=HFEXACT);
    //ASSERT(score>=-INFINITE&&score<=INFINITE);
    ASSERT(pos->ply>=0&&pos->ply<MAX_DEPTH);
			
	// if( pos->PvTable->pTable[index].posKey == 0) {
	// 	pos->PvTable->newWrite++;
	// } else {
	// 	pos->PvTable->overWrite++;
	// }
	
	// if(score > ISMATE) score += pos->ply;
    // else if(score < -ISMATE) score -= pos->ply;
	
	pos->PvTable->pTable[index].move = move;
    pos->PvTable->pTable[index].posKey = pos->posKey;
	printf("Move %s, Key %llx, Index %d\n", printMove(move), pos->posKey, index);
	// pos->PvTable->pTable[index].flags = flags;
	// pos->PvTable->pTable[index].score = score;
	// pos->PvTable->pTable[index].depth = depth;
}















