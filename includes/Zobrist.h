#ifndef _ZOBRIST_
#define _ZOBRIST_

#include "const.h"

// RC4 generator
struct RC4Struct {
	int8 s[256];
	int x, y;

	void InitZero(void);   // initial RC4 Generator with empty password
	int8 NextByte(void);  // generate next RC4
	int32 NextLong(void);  // generate next 4 bytes
};


class ZobristStruct
{
public:
	int32 dwKey, dwLock0, dwLock1;
public:
	ZobristStruct(void);
	~ZobristStruct(void);	
	void InitZero(void);
	void InitRC4(RC4Struct &rc4);        // fill with RC4
	inline void Xor(const ZobristStruct &zobr) //XOR operation
	{
		dwKey ^= zobr.dwKey;
		dwLock0 ^= zobr.dwLock0;
		dwLock1 ^= zobr.dwLock1;
	}
	void Xor(const ZobristStruct &zobr1, const ZobristStruct &zobr2);
};

class Zobrist
{
private:
	ZobristStruct player;
	ZobristStruct curZobr;
	ZobristStruct table[2][256];
public:
	void initial();
	
	inline void changeSide()
	{
		curZobr.Xor(player);
	}

	inline void addPiece(int pos, int color)
	{
		curZobr.Xor(table[color][pos]);
	}

	inline void removePiece(int pos, int color)
	{
		curZobr.Xor(table[color][pos]);
	}

	inline const ZobristStruct& currentZobr() const
	{
		return curZobr;
	}
};

#endif // _ZOBRIST_
