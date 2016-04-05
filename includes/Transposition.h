#ifndef _TRANSPOSITION_
#define _TRANSPOSITION_

#include "const.h"
#include "Zobrist.h"

// transposition table structure
struct HashItem {
	int8 ucDepth, ucFlag;
	signed_int16 svl;
	int16 wmv, wReserved;
	int32 dwLock0, dwLock1;
};

// transposition table structure
struct VCFVCTHashItem {
	int8 ucDepth, ucFlag;
	signed_int16 svl;
	int16 wmv, wReserved;
	int32 dwLock0, dwLock1;
	void* pointer;
};

class Transposition
{
public:
	Transposition(void);
	~Transposition(void);

	HashItem HashTable[HASH_SIZE]; // transposition table for alpha-beta search and PNS VCT search

	VCFVCTHashItem VCFVCTHashTable[HASH_SIZE]; // VCF/VCT transposition table

	int storeCnt;

	void initial();
	void initialVCFVCT();

	// get transposition
	int ProbeHash(const ZobristStruct& key, int nDistance, int vlAlpha, int vlBeta, int nDepth, int8 &mv);
	// store transposition
	void RecordHash(const ZobristStruct& key,int nDistance, int nFlag, int vl, int nDepth, int8 mv);

	//get VCF/VCT hash item
	int ProbeVCFVCTHash(const ZobristStruct& key);

	//store VCF/VCT hash item
	void RecordVCFVCTHash(const ZobristStruct& key, int nDistance, int vl);

	void* ProbePNSHash(const ZobristStruct& key);

	void RecordPNSHash(const ZobristStruct& key, void* node, int nDepth);

	int getUseRate();
	int getConflictRate();
};

#endif // _TRANSPOSITION_
