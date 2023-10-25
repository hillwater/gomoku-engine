#ifndef _SEARCH_ENGINE_
#define _SEARCH_ENGINE_

#include "BitBoard.h"
#include "Zobrist.h"
#include "Transposition.h"
#include "PNSEngine.h"
#include "NextMove.h"
#include <vector>

using namespace std;

// open book structure
struct BookItem {
	int32 dwLock;
	int16 wmv, wvl;
};

class PNSEngine;

class SearchEngine
{
public:
	SearchEngine(void);
	~SearchEngine(void);

	// for nodejs module
	int32 search(int32 level,
			vector<int32> posList,
			int32 type);

	static SearchEngine* getInstance();

//private:

	void initial();

	// main search function
	int SearchMain(int level);

	void addPiece(int mv);

	void removePiece(int mv);


	static SearchEngine* searchEngine;

	//board
	BitBoard board;
	int curColor;

	int nDistance;  // the distance from search tree root node
	int8 mvResult;                  // computer move result

	int8 mvKillers[LIMIT_DEPTH][2]; // kill move table

	int nBookSize;                 // open book size
	//BookItem BookTable[BOOK_SIZE]; // open book data

	int nHistoryTable[MAX_MOVES];      // history table
	
	Zobrist zobrist;
	Transposition transposition;

	PNSEngine* pnsEngine;

	//for statistic
	long totalSearchNode;

	// change player
	__forceinline void ChangeSide()
	{
		this->curColor = 1 - this->curColor;
		zobrist.changeSide();
	}
	__forceinline void NullMove(void)                        
	{
		ChangeSide();
		nDistance++;
	}
	__forceinline void UndoNullMove(void)                    
	{
		nDistance--;
		ChangeSide();
	}
	__forceinline void doMove(int mv)
	{
		board.addPiece(mv,this->curColor);
		zobrist.addPiece(mv,this->curColor);
		ChangeSide();
		nDistance++;
	}

	__forceinline void undoMove(int mv)
	{
		nDistance--;
		ChangeSide();
		zobrist.removePiece(mv,this->curColor);
		board.removePiece(mv,this->curColor);
	}

	bool NullOkay(void) const {return true;}                 // judge whether can do null move
	int SearchQuiesc(int vlAlpha, int vlBeta);
	int SearchFull(int vlAlpha, int vlBeta, int nDepth, bool bNoNull = false);// alpha-beta search
	int SearchVCF();//VCF search
	int SearchVCT();//VCT search

	int attack(bool isVCF);
	int defend(bool isVCF);
};

#endif // _SEARCH_ENGINE_
