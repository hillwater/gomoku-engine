#ifndef _BIT_BOARD_
#define _BIT_BOARD_

#include "const.h"
#include <string.h>
#include <assert.h>
#include <algorithm>
using namespace std;

struct NormalPattern {
public:
	int16 patterns[64];
	int64 bitmap;
	int count;
	int8 type;

	__forceinline void init(int8 type)
	{
		this->type = type;
		count = 0;
		bitmap = 0;
		memset(patterns, 0, sizeof(patterns));
	}

	__forceinline void setBit(int16 value)
	{
		assert(value != 0);
		assert(count<64);

		int16 hashCode = ((value>>6) ^ value) & 63;

		while(patterns[hashCode])
		{
			hashCode = (hashCode+7)&63;
		}

		patterns[hashCode] = value;
		bitmap ^= shiftArray[hashCode];

		count++;
	}

	__forceinline void unsetBit(int16 value)
	{
		assert(value != 0);
		assert(count>0);

		int tryCnt = 0;

		int16 hashCode = ((value>>6) ^ value) & 63;

		while(patterns[hashCode] != value)
		{
			assert(tryCnt++ < 64);
			hashCode = (hashCode+7)&63;
		}

		patterns[hashCode] = 0;
		bitmap ^= shiftArray[hashCode];

		count--;
	}
};

struct Active1Pattern {
public:
	int16 patterns[64*4];
	int64 bitmap[4];
	int count;
	int8 type;

	inline void init(int8 type)
	{
		this->type = type;
		count = 0;
		memset(bitmap,0,sizeof(bitmap));
		memset(patterns, 0, sizeof(patterns));
	}

	__forceinline void setBit(int16 value)
	{
		assert(value != 0);
		assert(count<64*4);

		int16 hashCode = ((value>>8) ^ value) & 0xFF;

		while(patterns[hashCode])
		{
			hashCode = (hashCode+7)&0xFF;
		}

		patterns[hashCode] = value;

		bitmap[hashCode>>6] ^= shiftArray[hashCode&63];

		count++;
	}

	__forceinline void unsetBit(int16 value)
	{
		assert(value != 0);
		assert(count>0);

		int tryCnt = 0;

		int16 hashCode = ((value>>8) ^ value) & 0xFF;

		while(patterns[hashCode] != value)
		{
			assert(tryCnt++ < 4*64);
			hashCode = (hashCode+7)&0xFF;
		}

		patterns[hashCode] = 0;
		bitmap[hashCode>>6] ^= shiftArray[hashCode&63];

		count--;
	}
};

// for windows
// __declspec(align(128))
struct GlobalPattern {
	Active1Pattern active1Pattern[2];// 0 black, 1 red
	NormalPattern normalPattern[2][MAX_COMMON_TYPE];// 0 black, 1 red
} __attribute__ ((aligned (128)));

class BitBoard
{
public:
	BitBoard(void);
	~BitBoard(void);

private:
	bool m_hasFive;

	//global pattern array
	GlobalPattern* m_globalPattern;

	int m_totalChessCount;

	//current global pattern
	GlobalPattern* m_currentGlobalPattern;

	//chess for 4 direction
	//0 black, 1 red, 2 empty, 3 boundary
	int64 m_horizontalChess[15];// 1 position needs 2 bits, 15 position + 2 boundary, so need (15+2)*2 = 34 bits
	int64 m_verticalChess[15];
	int64 m_diagnoalChess[29];
	int64 m_antiDiagnoalChess[29];

	int8 m_chess[256];

	static int16* SkipColorTypeArray;//high 4 bits for skip, 2 bits for color, low 6 bits for pattern type, max 64 kinds of type
	static int32* EmptyPosArray;
	static int startIndexArray[256][4];
	static int endIndexArray[256][4];
	static int8 startPosArray[256][4][15];

public:
	void clearBoard();
	void addPiece(int8 pos, int color); // add a piece to chess board
	//void removePiece(int8 pos, int color); // remove a piece from the chess board
	int GenerateMoves(int color, int8 *mvs) const;
	int Evaluate(int color) const;
	int mateEvaluate(int color) const;
	int getQuiescMoves(int color, int8* mvs) const;
	int getVCFAttackMoves(int color, int8* mvs) const;
	int getVCFDefendMoves(int color, int8* mvs) const;
	int getVCTAttackMoves(int color, int8* mvs) const;
	int getVCTDefendMoves(int color, int8* mvs) const;
	inline bool hasFive() const
	{
		return m_hasFive;
	}

	inline int getTotalChessCount() const
	{
		return m_totalChessCount;
	}

	inline int getSimpleEvaluateValue(int color) const
	{
		return 1 + m_currentGlobalPattern->normalPattern[color][SLEEP4].count*10 + m_currentGlobalPattern->normalPattern[color][ACTIVE3].count*10 + m_currentGlobalPattern->normalPattern[color][SLEEP3].count*6+
			m_currentGlobalPattern->normalPattern[color][ACTIVE2].count*6 + m_currentGlobalPattern->normalPattern[color][SLEEP2].count*2 - m_currentGlobalPattern->normalPattern[color][DEAD4].count*4
			- m_currentGlobalPattern->normalPattern[1-color][SLEEP4].count*10 - m_currentGlobalPattern->normalPattern[1-color][ACTIVE3].count*10 - m_currentGlobalPattern->normalPattern[1-color][SLEEP3].count*4-
			m_currentGlobalPattern->normalPattern[1-color][ACTIVE2].count*5 - m_currentGlobalPattern->normalPattern[1-color][SLEEP2].count*2 + m_currentGlobalPattern->normalPattern[1-color][DEAD4].count*4;
	}

	inline bool isEmptyPos(int8 pos) const
	{
		return m_chess[pos] == COLOR_EMPTY;
	}

	__forceinline void removePiece(int8 pos, int color)
	{
		int x = RANK_X(pos);
		int y = RANK_Y(pos);

		m_totalChessCount--;

		if(m_totalChessCount > 0)
		{
			m_currentGlobalPattern = &m_globalPattern[m_totalChessCount - 1];
		}

		xorPiece(x,y,color);//remove piece

		m_chess[pos] = COLOR_EMPTY;

		m_hasFive = false;
	}

	int8 getLooseMove(int color);
	void generateNeedExtendBitmap(int64 (&needExtendBitmap)[4], int color);

	void print() const;
	void printMove(int8* mvs, int n) const;
private:
	__forceinline void xorPiece(int x, int y, int color)
	{
		int colorXorEmpty = color ^ COLOR_EMPTY;

		const int64* lookup = colorShiftArray[colorXorEmpty];

		m_horizontalChess[y] ^= lookup[2*(x+1)];
		m_verticalChess[x] ^= lookup[2*(y+1)];

		m_diagnoalChess[y-x+BOARD_LENGTH-1] ^= lookup[2*(min(x,y)+1)];
		m_antiDiagnoalChess[x+y] ^= lookup[2*((x+y<BOARD_LENGTH ? x : BOARD_LENGTH-1-y)+1)];
	}

	bool isDoubleSleep4(int color) const;
	bool isSleep4AndActive3(int color) const;
	bool isDoubleActive3(int color) const;
	bool isOppositeSleep4OnMyActive3(int color) const;
	int sortAndRemoveDuplicate(int8* mvs, int length, int64 bitmap[4]) const;//mvs will be sorted from small to large
	void generateMoveCountStatistic(int color) const;
	int getIntersection(int8* intersection, int n1, const int8* arraySrc, int n2) const;//input array must be sorted from small to large
	bool hasVCFAttackMove(int color) const;
	bool getOppositeActive3AndVCFIntersectionMvs(int8* intersection, int &intersectionIndex, int color) const;//to find some intersect positions, each one can stop all opposite active3 and VCF
	int getMakeVCFMoves(int color, int8* mvs) const;
	bool isOnSameLine(const int8* mvs, int index) const;
	int getLineType(int8 pos1, int8 pos2) const;
	//void xorPiece(int x, int y, int color);
	void updateGlobalPattern(int x, int y, int color, bool isSetOrUnset);
	void alignedMemcpyWithSSE2(void* dest, const void* src, const unsigned long size_t);
};

#endif // _BIT_BOARD_
