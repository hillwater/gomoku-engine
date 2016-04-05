#include "../includes/Transposition.h"
#include <stddef.h>
#include <string.h>

Transposition::Transposition(void)
{
	storeCnt = 0;
}


Transposition::~Transposition(void)
{
}

void Transposition::initial()
{
	memset(HashTable, 0, HASH_SIZE * sizeof(HashItem));  // clear hash table

	storeCnt = 0;
}

void Transposition::initialVCFVCT()
{
	memset(VCFVCTHashTable, 0, HASH_SIZE * sizeof(VCFVCTHashItem));  // clear VCF table
}

// get transposition
int Transposition::ProbeHash(const ZobristStruct& key,int nDistance, int vlAlpha, int vlBeta, int nDepth, int8 &mv) {
	bool bMate; // win or loose flag
	HashItem hsh;

	hsh = HashTable[key.dwKey & (HASH_SIZE - 1)];
	if (hsh.dwLock0 != key.dwLock0 || hsh.dwLock1 != key.dwLock1) {
		mv = NO_MOVE;
		return -MATE_VALUE-100;
	}

	mv = hsh.wmv;
	bMate = false;
	if (hsh.svl > WIN_VALUE) {
		hsh.svl -= nDistance;
		bMate = true;
	} else if (hsh.svl < -WIN_VALUE) {
		hsh.svl += nDistance;
		bMate = true;
	}
	if (hsh.ucDepth >= nDepth || bMate) {
		if (hsh.ucFlag == HASH_BETA) {
			return (hsh.svl >= vlBeta ? hsh.svl : -MATE_VALUE-100);
		} else if (hsh.ucFlag == HASH_ALPHA) {
			return (hsh.svl <= vlAlpha ? hsh.svl : -MATE_VALUE-100);
		}
		return hsh.svl;
	}
	return -MATE_VALUE-100;
}

// store transposition
void Transposition::RecordHash(const ZobristStruct& key,int nDistance, int nFlag, int vl, int nDepth, int8 mv) {
	HashItem hsh;
	hsh = HashTable[key.dwKey & (HASH_SIZE - 1)];
	if (hsh.ucDepth > nDepth) {
		return;
	}

	//
	storeCnt++;


	hsh.ucFlag = nFlag;
	hsh.ucDepth = nDepth;
	if (vl > WIN_VALUE) {
		hsh.svl = vl + nDistance;
	} else if (vl < -WIN_VALUE) {
		hsh.svl = vl - nDistance;
	} else {
		hsh.svl = vl;
	}
	hsh.wmv = mv;
	hsh.dwLock0 = key.dwLock0;
	hsh.dwLock1 = key.dwLock1;
	HashTable[key.dwKey & (HASH_SIZE - 1)] = hsh;
}

int Transposition::ProbeVCFVCTHash(const ZobristStruct& key)
{
	VCFVCTHashItem hsh;

	hsh = VCFVCTHashTable[key.dwKey & (HASH_SIZE - 1)];
	if (hsh.dwLock0 != key.dwLock0 || hsh.dwLock1 != key.dwLock1) {
		return 0;//means not find
	}

	return hsh.svl;
}

void Transposition::RecordVCFVCTHash(const ZobristStruct& key, int nDistance, int vl)
{
	VCFVCTHashItem has;
	has = VCFVCTHashTable[key.dwKey & (HASH_SIZE - 1)];

	//choose higher layer, wReserved used to store nDistance
	if(has.wReserved > 10000 - nDistance)
	{
		return;
	}

	has.wReserved = 10000 - nDistance;
	has.svl = vl;
	has.dwLock0 = key.dwLock0;
	has.dwLock1 = key.dwLock1;

	VCFVCTHashTable[key.dwKey & (HASH_SIZE - 1)] = has;
}

void* Transposition::ProbePNSHash(const ZobristStruct& key)
{
	VCFVCTHashItem hsh;

	hsh = VCFVCTHashTable[key.dwKey & (HASH_SIZE - 1)];
	if (hsh.dwLock0 != key.dwLock0 || hsh.dwLock1 != key.dwLock1) {
		return NULL;//means not find
	}

	return hsh.pointer;
	//return (void*)(hsh.wReserved<<16 | hsh.wmv);
}

void Transposition::RecordPNSHash(const ZobristStruct& key, void* node, int nDepth)
{
	VCFVCTHashItem has;
	has = VCFVCTHashTable[key.dwKey & (HASH_SIZE - 1)];

	//choose lower nDepth
	if(has.ucDepth >= 255 - nDepth)
	{
		return;
	}

	has.ucDepth = 255 - nDepth;
	has.dwLock0 = key.dwLock0;
	has.dwLock1 = key.dwLock1;

	has.pointer = node;

//	has.wmv = (int32)node & 0xffff;
//	has.wReserved = ((int32)node >> 16) & 0xffff;

	VCFVCTHashTable[key.dwKey & (HASH_SIZE - 1)] = has;
}

int Transposition::getUseRate()
{
	int cnt = 0;
	for (int i = 0;i< HASH_SIZE;i++)
	{
		if(HashTable[i].dwLock0 != 0)
			cnt++;
	}

	return cnt*10000/HASH_SIZE;
}

int Transposition::getConflictRate()
{
	int cnt = 0;
	for (int i = 0;i< HASH_SIZE;i++)
	{
		if(HashTable[i].dwLock0 != 0)
			cnt++;
	}

	if(storeCnt == 0){
		return 0;
	}

	return (storeCnt-cnt)*100/storeCnt;
}
