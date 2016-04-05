#ifndef _CONST_
#define _CONST_

//#define __forceinline __attribute__((always_inline))
#define __forceinline inline
#define __cdecl __attribute__((__cdecl__))

typedef unsigned short int16;
typedef unsigned short int14;
typedef unsigned int int32;
typedef unsigned long long int64;
typedef unsigned char int8;
typedef unsigned char int6;
typedef unsigned char int2;

typedef signed short signed_int16;

//for LMR
const int FullDepthMoves = 6;
const int ReductionLimit = 1;

//for futility forward pruning
const int PruningDepth = 5;
const int PruningMargin[8] = {0,20,20,30,30,40,40,50};

const int MEM_POOL_INIT_SIZE = 600*1000;

const int directionDiff[4] = {
	1,16,17,-15
};

const int64 shiftArray[64] = {
	1,2,4,8,16,32,64,128,256,512,1024,
	1<<11,1<<12,1<<13,1<<14,1<<15,1<<16,1<<17,1<<18,1<<19,1<<20,
	1<<21,1<<22,1<<23,1<<24,1<<25,1<<26,1<<27,1<<28,1<<29,1<<30,
	((int64)1)<<31,((int64)1)<<32,((int64)1)<<33,((int64)1)<<34,((int64)1)<<35,((int64)1)<<36,((int64)1)<<37,((int64)1)<<38,((int64)1)<<39,((int64)1)<<40,
	((int64)1)<<41,((int64)1)<<42,((int64)1)<<43,((int64)1)<<44,((int64)1)<<45,((int64)1)<<46,((int64)1)<<47,((int64)1)<<48,((int64)1)<<49,((int64)1)<<50,
	((int64)1)<<51,((int64)1)<<52,((int64)1)<<53,((int64)1)<<54,((int64)1)<<55,((int64)1)<<56,((int64)1)<<57,((int64)1)<<58,((int64)1)<<59,((int64)1)<<60,
	((int64)1)<<61,((int64)1)<<62,((int64)1)<<63
};

const int64 colorShiftArray[4][64] = {
	{ 0 },
	{	
		1<<0,1<<1,1<<2,1<<3,1<<4,1<<5,1<<6,1<<7,1<<8,1<<9,1<<10,
		1<<11,1<<12,1<<13,1<<14,1<<15,1<<16,1<<17,1<<18,1<<19,1<<20,
		1<<21,1<<22,1<<23,1<<24,1<<25,1<<26,1<<27,1<<28,1<<29,1<<30,
		((int64)1)<<31,((int64)1)<<32,((int64)1)<<33,((int64)1)<<34,((int64)1)<<35,((int64)1)<<36,((int64)1)<<37,((int64)1)<<38,((int64)1)<<39,((int64)1)<<40,
		((int64)1)<<41,((int64)1)<<42,((int64)1)<<43,((int64)1)<<44,((int64)1)<<45,((int64)1)<<46,((int64)1)<<47,((int64)1)<<48,((int64)1)<<49,((int64)1)<<50,
		((int64)1)<<51,((int64)1)<<52,((int64)1)<<53,((int64)1)<<54,((int64)1)<<55,((int64)1)<<56,((int64)1)<<57,((int64)1)<<58,((int64)1)<<59,((int64)1)<<60,
		((int64)1)<<61,((int64)1)<<62,((int64)1)<<63
	},
	{
		2<<0,2<<1,2<<2,2<<3,2<<4,2<<5,2<<6,2<<7,2<<8,2<<9,2<<10,
		2<<11,2<<12,2<<13,2<<14,2<<15,2<<16,2<<17,2<<18,2<<19,2<<20,
		2<<21,2<<22,2<<23,2<<24,2<<25,2<<26,2<<27,2<<28,2<<29,2<<30,
		((int64)2)<<31,((int64)2)<<32,((int64)2)<<33,((int64)2)<<34,((int64)2)<<35,((int64)2)<<36,((int64)2)<<37,((int64)2)<<38,((int64)2)<<39,((int64)2)<<40,
		((int64)2)<<41,((int64)2)<<42,((int64)2)<<43,((int64)2)<<44,((int64)2)<<45,((int64)2)<<46,((int64)2)<<47,((int64)2)<<48,((int64)2)<<49,((int64)2)<<50,
		((int64)2)<<51,((int64)2)<<52,((int64)2)<<53,((int64)2)<<54,((int64)2)<<55,((int64)2)<<56,((int64)2)<<57,((int64)2)<<58,((int64)2)<<59,((int64)2)<<60,
		((int64)2)<<61,((int64)2)<<62,((int64)2)<<63
	},
	{
		3<<0,3<<1,3<<2,3<<3,3<<4,3<<5,3<<6,3<<7,3<<8,3<<9,3<<10,
		3<<11,3<<12,3<<13,3<<14,3<<15,3<<16,3<<17,3<<18,3<<19,3<<20,
		3<<21,3<<22,3<<23,3<<24,3<<25,3<<26,3<<27,3<<28,3<<29,3<<30,
		((int64)3)<<31,((int64)3)<<32,((int64)3)<<33,((int64)3)<<34,((int64)3)<<35,((int64)3)<<36,((int64)3)<<37,((int64)3)<<38,((int64)3)<<39,((int64)3)<<40,
		((int64)3)<<41,((int64)3)<<42,((int64)3)<<43,((int64)3)<<44,((int64)3)<<45,((int64)3)<<46,((int64)3)<<47,((int64)3)<<48,((int64)3)<<49,((int64)3)<<50,
		((int64)3)<<51,((int64)3)<<52,((int64)3)<<53,((int64)3)<<54,((int64)3)<<55,((int64)3)<<56,((int64)3)<<57,((int64)3)<<58,((int64)3)<<59,((int64)3)<<60,
		((int64)3)<<61,((int64)3)<<62,((int64)3)<<63
	}
		
};

static const char* COLOR_NAME[2] = {"Black","White"};

const int COLOR_BLACK = 0;
const int COLOR_RED = 1;
const int COLOR_EMPTY = 2;
const int COLOR_BOUNDARY = 3;

const int16 DIRECTION_HORIZONTAL = 0;
const int16 DIRECTION_VERTICAL = 1;
const int16 DIRECTION_DIAGNOAL = 2;
const int16 DIRECTION_ANTIDIAGNOAL = 3;

const int UNMOVEABLE = 0;
const int MOVEABLE = 1;
const int MOVE_DISTANCE = 1;

const int BOARD_LENGTH = 15;

const int LINE_NUM = BOARD_LENGTH*6-18;

const int CHESS_NUMBER = BOARD_LENGTH * BOARD_LENGTH;

const int MAX_GEN_MOVES = BOARD_LENGTH * BOARD_LENGTH; // max generate moves

const int NO_MOVE = 255; //not existed move
const int MAX_MOVES = 256;     // the max history moves
const int LIMIT_DEPTH = 30;    // max search depth for alpha-beta
const int MATE_VALUE = 10000;  // max evaluate value, it means win
const int ACTIVE_4_VALUE = MATE_VALUE - 1;
const int ACTIVE_3_VALUE = MATE_VALUE - 20;
const int WIN_VALUE = MATE_VALUE - 200; // if large than this value, it means win
const int DRAW_VALUE = 20;     // draw value for evaluation
const int NULL_DEPTH = 2;      // null move depth
const int HASH_SIZE = 1 << 20; // transposition table size
const int HASH_ALPHA = 1;      // alpha node
const int HASH_BETA = 2;       // beta node
const int HASH_PV = 3;         // PV node
const int BOOK_SIZE = 16384;   // open book size

//for PNS
const int VCT_MAX_CHILD_COUNT = 40;

const int8 PROVEN = 0;
const int8 DISPROVEN = 1;
const int8 UNKNOWN = 2;

const bool AND_NODE = false;
const bool OR_NODE = true;

const bool SET_PATTERN = true;
const bool UNSET_PATTERN = false;

const int INFINITE_VALUE = 100000000;

const int8 ACTIVE5 = 0;
const int8 SLEEP4 = 1;
const int8 ACTIVE3 = 2;
const int8 SLEEP3 = 3;
const int8 ACTIVE2 = 4;
const int8 SLEEP2 = 5;
const int8 ACTIVE1 = 6;
const int8 DEAD4 = 7;
const int8 MAX_COMMON_TYPE = 8;

const int MAX_TYPE = 41;

const int8 NO_TYPE = -1;
static const int8 typeConvert[MAX_TYPE + 1] = {
	NO_TYPE,
	ACTIVE5,
	SLEEP4,SLEEP4,SLEEP4,SLEEP4,SLEEP4,
	ACTIVE3,ACTIVE3,ACTIVE3,ACTIVE3,
	SLEEP3,SLEEP3,SLEEP3,SLEEP3,SLEEP3,SLEEP3,SLEEP3,SLEEP3,SLEEP3,SLEEP3,
	ACTIVE2,ACTIVE2,ACTIVE2,ACTIVE2,ACTIVE2,ACTIVE2,
	SLEEP2,SLEEP2,SLEEP2,SLEEP2,SLEEP2,SLEEP2,SLEEP2,SLEEP2,SLEEP2,SLEEP2,
	ACTIVE1,ACTIVE1,ACTIVE1,ACTIVE1,
	DEAD4
};


const int PHASE_GEN_NEED_EXTEND_BITMAP = 0;
const int PHASE_HASH_MOVE = 1;
const int PHASE_KILLER_MOVE = 2;
const int PHASE_KILLER_MOVE2 = 3;
const int PHASE_GEN_MOVE = 4;
const int PHASE_REST_MOVE = 5;


//x: -->, y: |
// judge whether a chess is in board
static const char ccInBoard[256]= {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0,
	1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 1, 0,
	1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 2, 1, 0,
	1, 2, 3, 4, 5, 5, 5, 5, 5, 5, 5, 4, 3, 2, 1, 0,
	1, 2, 3, 4, 5, 6, 6, 6, 6, 6, 5, 4, 3, 2, 1, 0,
	1, 2, 3, 4, 5, 6, 7, 7, 7, 6, 5, 4, 3, 2, 1, 0,
	1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 5, 4, 3, 2, 1, 0,
	1, 2, 3, 4, 5, 6, 7, 7, 7, 6, 5, 4, 3, 2, 1, 0,
	1, 2, 3, 4, 5, 6, 6, 6, 6, 6, 5, 4, 3, 2, 1, 0,
	1, 2, 3, 4, 5, 5, 5, 5, 5, 5, 5, 4, 3, 2, 1, 0,
	1, 2, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 2, 1, 0,
	1, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 1, 0,
	1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 0,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

//judge whether in board
__forceinline bool IN_BOARD(int8 pos) {
	return pos >= 0 && ccInBoard[pos] != 0;
}

__forceinline int RANK_Y(int8 pos) {
	return (pos >> 4) & 0xF;
}

__forceinline int RANK_X(int8 pos) {
	return pos & 0xF;
}

__forceinline int8 COORD_XY(int x, int y) {
	return (x & 0xF) + ((y & 0xF) << 4);
}

__forceinline char digit2char(int d)
{
	return d + '0';
}

__forceinline int32 LBS32(int32 value)
{
#ifdef _WIN32
	int32 index = 0;
	if(BitScanForward(&index,value)){
		return index;
	}else{
		return 32;
	}
#else
	//	GCC: int __builtin_ffs (unsigned int x)
	//	Returns one plus the index of the least significant 1-bit of x, or if x is zero, returns zero.
	int32 result = __builtin_ffs(value);
	if(result == 0) {
		return 32;
	}else {
		return result - 1;
	}
#endif
}

__forceinline int32 LBS64(int64 value)
{
	if(value & 0xFFFFFFFF)
	{
		return LBS32(value);
	}else{
		return LBS32(value>>32) + 32;
	}
}

__forceinline bool isInBitmap(int64 bitmap[4], int8 value)
{
	return bitmap[(value>>6)&3] & shiftArray[value&63];
}

#endif // _CONST_
