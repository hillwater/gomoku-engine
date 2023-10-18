#include "../includes/BitBoard.h"
#include "../includes/DataGenerator.h"
#include <algorithm>
#include <sys/malloc.h>
#include <iostream>

using namespace std;

int16* BitBoard::SkipColorTypeArray = DataGenerator::generateTypeArray();
int32* BitBoard::EmptyPosArray = DataGenerator::generateEmptyPosArray();

int BitBoard::startIndexArray[256][4] = {0};
int BitBoard::endIndexArray[256][4] = {0};
int8 BitBoard::startPosArray[256][4][15] = {0};


int moveCountStatisticMap[256];

int __cdecl  compareStatisticCount(const void *lpmv1, const void *lpmv2)
{
	return moveCountStatisticMap[*(int8 *) lpmv2] - moveCountStatisticMap[*(int8 *) lpmv1];
}

BitBoard::BitBoard(void)
{
	// use 128 byte aligned, make memcpy more quickly
//	m_globalPattern = (GlobalPattern*)memalign(sizeof(GlobalPattern)*CHESS_NUMBER, 128);

	m_globalPattern = (GlobalPattern*)malloc(sizeof(GlobalPattern)*CHESS_NUMBER);

	for (int pos = 0; pos < 256;pos++)
	{
		int x = RANK_X(pos);
		int y = RANK_Y(pos);

		int minxy = min(x,y);
		int start[4] = {max(x-5,0),max(y-5,0),max(minxy-5,0),max((x+y<BOARD_LENGTH ? x : BOARD_LENGTH-1-y)-5,0)};
		int end[4] = {min(x+1,BOARD_LENGTH-4),min(y+1,BOARD_LENGTH-4),min(minxy+1,BOARD_LENGTH-abs(x-y)-4),min((x+y<BOARD_LENGTH ? x : BOARD_LENGTH-1-y)+1,BOARD_LENGTH-abs(BOARD_LENGTH-1-x-y)-4)};

		int startXConst[4] = {-1, x, (x>=y?x-y-1:-1), ((x+y)<BOARD_LENGTH? -1 : (x+y-BOARD_LENGTH))};
		int startYConst[4] = {y, -1, (x>=y?-1:y-x-1), ((x+y)<BOARD_LENGTH? (x+y+1) : BOARD_LENGTH)};
		int startXDiff[4] = {1,0,1,1};
		int startYDiff[4] = {0,1,1,-1};

		for (int direction = 0;direction<4;direction++)
		{
			startIndexArray[pos][direction] = start[direction];
			endIndexArray[pos][direction] = end[direction];

			for(int i = 0;i<15;i++)
			{
				startPosArray[pos][direction][i] = COORD_XY(startXConst[direction] + i * startXDiff[direction], startYConst[direction] + i* startYDiff[direction]);
			}
		}
	}
}


BitBoard::~BitBoard(void)
{
}

void BitBoard::clearBoard()
{
	m_hasFive = false;

	memset(m_globalPattern, 0, sizeof(GlobalPattern)*CHESS_NUMBER);

	for(int i = 0;i<BOARD_LENGTH;i++)
	{
		m_horizontalChess[i] = 0x3AAAAAAAB;//32222222222222223, 3 => boundary, 2 => empty
		m_verticalChess[i] = 0x3AAAAAAAB;
	}

	for(int i = 0;i<2*BOARD_LENGTH-1;i++)
	{
		int len = i<BOARD_LENGTH?(i+1):(2*BOARD_LENGTH-1-i);
		int64 value = COLOR_BOUNDARY;
		for(int j = 0;j<len;j++)
		{
			value<<=2;
			value |= COLOR_EMPTY;
		}
		value<<=2;
		value |= COLOR_BOUNDARY;

		m_diagnoalChess[i] = value;
		m_antiDiagnoalChess[i] = value;
	}

	memset(m_chess,COLOR_EMPTY,sizeof(m_chess));

	m_totalChessCount = 0;
}



__forceinline void BitBoard::updateGlobalPattern(int x, int y, int color, bool isSetOrUnset)
{
	int64 line[4] = {m_horizontalChess[y],m_verticalChess[x],m_diagnoalChess[y-x+BOARD_LENGTH-1],m_antiDiagnoalChess[x+y]};

	int8 pos = COORD_XY(x,y);

	for (int16 direction = 0;direction <4;direction++)
	{
		int32 currentLine = (line[direction]>>(startIndexArray[pos][direction]<<1)) ;
		for(int i = startIndexArray[pos][direction];i<=endIndexArray[pos][direction];)
		{
			int16 skipColorType = SkipColorTypeArray[currentLine & 0x3FFFFF];
			if(skipColorType & 0x3F)
			{
				int16 patternColor = (skipColorType >> 6) & 0x3;
				int16 type = skipColorType & 0x3F;
				int16 startPos = startPosArray[pos][direction][i];
				int16 patternValue = (startPos<<8) | (direction<<6) | type;
				int simpleType = typeConvert[type];
				if(isSetOrUnset == SET_PATTERN)
				{
					if(simpleType == ACTIVE1)
					{
						m_currentGlobalPattern->active1Pattern[patternColor].setBit(patternValue);
					}
					else
					{
						m_currentGlobalPattern->normalPattern[patternColor][simpleType].setBit(patternValue);
					}
				}
				else
				{
					if(simpleType == ACTIVE1)
					{
						m_currentGlobalPattern->active1Pattern[patternColor].unsetBit(patternValue);
					}
					else
					{
						m_currentGlobalPattern->normalPattern[patternColor][simpleType].unsetBit(patternValue);
					}
				}
			}

			int16 skip = skipColorType>>8;

			i+=skip;

			currentLine>>=(skip<<1);
		}
	}
}

void BitBoard::addPiece(int8 pos, int color)
{
	int x = RANK_X(pos);
	int y = RANK_Y(pos);

	if(m_totalChessCount > 0)
	{
		//m_globalPattern[m_totalChessCount] = m_globalPattern[m_totalChessCount - 1];
		memcpy(&m_globalPattern[m_totalChessCount], &m_globalPattern[m_totalChessCount - 1],sizeof(GlobalPattern));
		//alignedMemcpyWithSSE2(&m_globalPattern[m_totalChessCount], &m_globalPattern[m_totalChessCount - 1],sizeof(GlobalPattern));

	}

	m_currentGlobalPattern = &m_globalPattern[m_totalChessCount];

	m_totalChessCount++;

	//update global pattern and incrementalPattern
	updateGlobalPattern(x,y,color,UNSET_PATTERN);
	xorPiece(x,y,color);//add piece
	updateGlobalPattern(x,y,color,SET_PATTERN);

	m_chess[pos] = color;

	m_hasFive = m_currentGlobalPattern->normalPattern[COLOR_BLACK][ACTIVE5].count > 0 || m_currentGlobalPattern->normalPattern[COLOR_RED][ACTIVE5].count > 0 ;
}



//__forceinline void BitBoard::alignedMemcpyWithSSE2(void* dest, const void* src, const unsigned long size_t)
//{
//	__asm
//	{
//		mov esi, src;    //src pointer
//		mov edi, dest;   //dest pointer
//
//		mov ebx, size_t; //ebx is our counter
//		shr ebx, 7;      //divide by 128 (8 * 128bit registers)
//
//
//loop_copy:
//		prefetchnta 128[ESI]; //SSE2 prefetch
//		prefetchnta 160[ESI];
//		prefetchnta 192[ESI];
//		prefetchnta 224[ESI];
//
//		movdqa xmm0, 0[ESI]; //move data from src to registers
//		movdqa xmm1, 16[ESI];
//		movdqa xmm2, 32[ESI];
//		movdqa xmm3, 48[ESI];
//		movdqa xmm4, 64[ESI];
//		movdqa xmm5, 80[ESI];
//		movdqa xmm6, 96[ESI];
//		movdqa xmm7, 112[ESI];
//
//		movntdq 0[EDI], xmm0; //move data from registers to dest
//		movntdq 16[EDI], xmm1;
//		movntdq 32[EDI], xmm2;
//		movntdq 48[EDI], xmm3;
//		movntdq 64[EDI], xmm4;
//		movntdq 80[EDI], xmm5;
//		movntdq 96[EDI], xmm6;
//		movntdq 112[EDI], xmm7;
//
//		add esi, 128;
//		add edi, 128;
//		dec ebx;
//
//		jnz loop_copy; //loop please
//loop_copy_end:
//	}
//}

void BitBoard::generateNeedExtendBitmap(int64 (&needExtendBitmap)[4], int color)
{
	memset(needExtendBitmap,0, sizeof(needExtendBitmap));

	const NormalPattern& sleep3Pattern = m_currentGlobalPattern->normalPattern[color][SLEEP3];

	int64 bitmap = sleep3Pattern.bitmap;
	while(bitmap)
	{
		int32 emptyPoses = EmptyPosArray[sleep3Pattern.patterns[LBS64(bitmap)]];
		bitmap &= bitmap - 1;//remove last bit '1'

		//
		int8 pos = emptyPoses;
		needExtendBitmap[pos>>6] |= shiftArray[pos & 63];

		//
		pos = emptyPoses >> 8;
		needExtendBitmap[pos>>6] |= shiftArray[pos & 63];
	}


	//const NormalPattern& oppositeSleep4Pattern = m_currentGlobalPattern->normalPattern[1-color][SLEEP4];

	//bitmap = oppositeSleep4Pattern.bitmap;
	//while(bitmap)
	//{
	//	int32 emptyPoses = EmptyPosArray[oppositeSleep4Pattern.patterns[LBS64(bitmap)]];
	//	bitmap &= bitmap - 1;//remove last bit '1'

	//	//
	//	int8 pos = emptyPoses;
	//	needExtendBitmap[pos>>6] |= shiftArray[pos & 63];
	//}
}

int BitBoard::GenerateMoves(int color, int8 *mvs) const
{
	int index = 0;
	int64 mvsBitmap[4];

	const Active1Pattern& active1Pattern = m_currentGlobalPattern->active1Pattern[color];
	const NormalPattern& active2Pattern = m_currentGlobalPattern->normalPattern[color][ACTIVE2];
	const NormalPattern& active3Pattern = m_currentGlobalPattern->normalPattern[color][ACTIVE3];
	const NormalPattern& sleep2Pattern = m_currentGlobalPattern->normalPattern[color][SLEEP2];
	const NormalPattern& sleep3Pattern = m_currentGlobalPattern->normalPattern[color][SLEEP3];
	const NormalPattern& sleep4Pattern = m_currentGlobalPattern->normalPattern[color][SLEEP4];
	const NormalPattern& oppositeSleep4Pattern = m_currentGlobalPattern->normalPattern[1-color][SLEEP4];
	const NormalPattern& oppositeSleep3Pattern = m_currentGlobalPattern->normalPattern[1-color][SLEEP3];
	const NormalPattern& oppositeSleep2Pattern = m_currentGlobalPattern->normalPattern[1-color][SLEEP2];
	const NormalPattern& oppositeActive3Pattern = m_currentGlobalPattern->normalPattern[1-color][ACTIVE3];
	const NormalPattern& oppositeActive2Pattern = m_currentGlobalPattern->normalPattern[1-color][ACTIVE2];

	if(sleep4Pattern.count > 0)//I have sleep4
	{
		mvs[index++] = EmptyPosArray[sleep4Pattern.patterns[LBS64(sleep4Pattern.bitmap)]];
	}
	else if(oppositeSleep4Pattern.count > 0)//opposite has sleep4
	{
		int8 pos = EmptyPosArray[oppositeSleep4Pattern.patterns[LBS64(oppositeSleep4Pattern.bitmap)]];
		mvs[index++] = pos;
	}
	else if(active3Pattern.count > 0)//I have active3
	{
		mvs[index++] = (EmptyPosArray[active3Pattern.patterns[LBS64(active3Pattern.bitmap)]] >> 8) & 0xFF;
	}
	else if(oppositeActive3Pattern.count > 0 || hasVCFAttackMove(1-color))//opposite has active3 or VCF attack move
	{
		index = getVCTDefendMoves(color,mvs);
	}
	else
	{
		int64 bitmap = sleep3Pattern.bitmap;
		while(bitmap)
		{
			int32 emptyPoses = EmptyPosArray[sleep3Pattern.patterns[LBS64(bitmap)]];
			bitmap &= bitmap - 1;//remove last bit '1'

			mvs[index++] = emptyPoses & 0xFF;
			mvs[index++] = (emptyPoses>>8) & 0xFF;
		}


		bitmap = oppositeSleep3Pattern.bitmap;
		while(bitmap)
		{
			int32 emptyPoses = EmptyPosArray[oppositeSleep3Pattern.patterns[LBS64(bitmap)]];
			bitmap &= bitmap - 1;//remove last bit '1'

			mvs[index++] = emptyPoses & 0xFF;
			mvs[index++] = (emptyPoses>>8) & 0xFF;
		}


		bitmap = active2Pattern.bitmap;
		while(bitmap)
		{
			int32 emptyPoses = EmptyPosArray[active2Pattern.patterns[LBS64(bitmap)]];
			bitmap &= bitmap - 1;//remove last bit '1'

			mvs[index++] = (emptyPoses>>8) & 0xFF;
			mvs[index++] = (emptyPoses>>16) & 0xFF;
		}

		bitmap = oppositeActive2Pattern.bitmap;
		while(bitmap)
		{
			int16 pattern = oppositeActive2Pattern.patterns[LBS64(bitmap)];
			int32 emptyPoses = EmptyPosArray[pattern];
			bitmap &= bitmap - 1;//remove last bit '1'

			int16 type = pattern & 63;
			int16 direction = (pattern>>6)&3;

			if(type == 21){//_●●___
				if(direction == DIRECTION_ANTIDIAGNOAL)
				{
					mvs[index++] = (emptyPoses>>8) & 0xFF;
				}
				else
				{
					mvs[index++] = (emptyPoses>>16) & 0xFF;
				}
			}else if(type == 24){//___●●_
				if(direction == DIRECTION_ANTIDIAGNOAL)
				{
					mvs[index++] = (emptyPoses>>16) & 0xFF;
				}
				else
				{
					mvs[index++] = (emptyPoses>>8) & 0xFF;
				}
			}else {
				mvs[index++] = (emptyPoses>>8) & 0xFF;
				mvs[index++] = (emptyPoses>>16) & 0xFF;
			}
		}

		bitmap = sleep2Pattern.bitmap;
		while(bitmap)
		{
			int16 pattern = sleep2Pattern.patterns[LBS64(bitmap)];
			int32 emptyPoses = EmptyPosArray[pattern];
			bitmap &= bitmap - 1;//remove last bit '1'

			int16 type = pattern & 63;
			int16 direction = (pattern>>6)&3;

			if(type == 32){//○●●___
				if(direction == DIRECTION_ANTIDIAGNOAL)
				{
					mvs[index++] = emptyPoses & 0xFF;
					mvs[index++] = (emptyPoses>>8) & 0xFF;
				}else{
					mvs[index++] = (emptyPoses>>8) & 0xFF;
					mvs[index++] = (emptyPoses>>16) & 0xFF;
				}
			}else if(type == 33){//___●●○
				if(direction == DIRECTION_ANTIDIAGNOAL)
				{
					mvs[index++] = (emptyPoses>>8) & 0xFF;
					mvs[index++] = (emptyPoses>>16) & 0xFF;
				}else{
					mvs[index++] = emptyPoses & 0xFF;
					mvs[index++] = (emptyPoses>>8) & 0xFF;
				}
			}else{
				mvs[index++] = emptyPoses & 0xFF;
				mvs[index++] = (emptyPoses>>8) & 0xFF;
				mvs[index++] = (emptyPoses>>16) & 0xFF;
			}
		}


		//bitmap = oppositeSleep2Pattern.bitmap;
		//while(bitmap)
		//{
		//	int32 emptyPoses = EmptyPosArray[oppositeSleep2Pattern.patterns[LBS64(bitmap)]];
		//	bitmap &= bitmap - 1;//remove last bit '1'

		//	mvs[index++] = emptyPoses & 0xFF;
		//	mvs[index++] = (emptyPoses>>8) & 0xFF;
		//	mvs[index++] = (emptyPoses>>16) & 0xFF;
		//}


		for (int k = 0;k<4;k++)
		{
			int64 active1Bitmap = active1Pattern.bitmap[k];
			while(active1Bitmap)
			{
				int32 active1EmptyPoses = EmptyPosArray[active1Pattern.patterns[LBS64(active1Bitmap)+(k<<6)]];
				active1Bitmap &= active1Bitmap - 1;//remove last bit '1'

				for (int j = 1;j<3;j++)
				{
					int8 pos = (active1EmptyPoses>>(8*j)) & 0xFF;
					mvs[index++] = pos;
				}
			}
		}


		// for(int i = 0;i<currentPatternResult.countActive1[1-color];i++)
		// {
			// for (int j = 0;j<3;j++)
			// {
				// if(moveMap[currentPatternResult.emptyPosActive1[j][i][1-color]] == MOVEABLE)
				// {
					// mvs[index++] = currentPatternResult.emptyPosActive1[j][i][1-color];
				// }
			// }
		// }


		//if no move, generate the normal one
		if(index < 5)
		{
			int8 moveMap[256];
			memset(moveMap, UNMOVEABLE, sizeof(moveMap));
			for(int16 k = 0; k<256;k++)
			{
				int kx = k&0xF;
				int ky = (k>>4)&0xF;
				if(ccInBoard[k] != 0 && m_chess[k] != COLOR_EMPTY)
				{
					for(int i = -MOVE_DISTANCE;i<=MOVE_DISTANCE;i++)
					{
						for(int j = -MOVE_DISTANCE;j<=MOVE_DISTANCE;j++)
						{
							int x = i + kx;
							int y = j + ky;

							if(x>=0 && x <15 && y >=0 && y<15)
							{
								if(m_chess[y<<4 | x] == COLOR_EMPTY)
								{
									moveMap[y<<4 | x] = MOVEABLE;
								}
							}
						}
					}
				}
			}
			for(int i = 0;i<256;i++)
			{
				if(ccInBoard[i] != 0 && moveMap[i] == MOVEABLE)
				{
					mvs[index++] = i;
				}
			}
		}
	}

	//remove duplicate
	index = sortAndRemoveDuplicate(mvs, index,mvsBitmap);

#ifdef _DEBUG
	int output = 0;
	if(output)
	{
		cout<<"moves:"<<index<<"\n";
		printMove(mvs,index);
	}
#endif


	return index;
}

int BitBoard::mateEvaluate(int color) const
{
	if(m_currentGlobalPattern->normalPattern[color][ACTIVE5].count > 0){//I have active5
		return MATE_VALUE+6;
	}else if(m_currentGlobalPattern->normalPattern[1-color][ACTIVE5].count > 0){//opposite has active5
		return -MATE_VALUE-6;
	}else if(m_currentGlobalPattern->normalPattern[color][SLEEP4].count > 0){//I have sleep4
		return MATE_VALUE+4;
	}else if(isDoubleSleep4(1-color)){//opposite has double sleep4
		return -MATE_VALUE-4;
	}else if(isSleep4AndActive3(1-color)){//opposite has sleep4+active3
		return -MATE_VALUE-3;
	}else if((m_currentGlobalPattern->normalPattern[color][ACTIVE3].count > 0) && (m_currentGlobalPattern->normalPattern[1-color][SLEEP4].count == 0)){//I have active3, opposite has no sleep4
		return MATE_VALUE+2;
	}/*else if(isOppositeSleep4OnMyActive3(color)){//I have active3, opposite has sleep4, but the sleep4 position is just on my active3
		return MATE_VALUE+1;
	}else if(isDoubleActive3(1-color)){//opposite has double active3
		if(m_currentGlobalPattern->normalPattern[color][ACTIVE3].count == 0 && m_currentGlobalPattern->normalPattern[color][SLEEP3].count == 0){//I have no sleep3 or active3, it must be loose
			return -MATE_VALUE-2;
		}else {
			return 0;
		}
	}*/else {
		return 0;
	}
}

int BitBoard::Evaluate(int color) const
{
	if(m_currentGlobalPattern->normalPattern[color][ACTIVE5].count > 0){//I have active5
		return MATE_VALUE+6;
	}else if(m_currentGlobalPattern->normalPattern[1-color][ACTIVE5].count > 0){//opposite has active5
		return -MATE_VALUE-6;
	}else if(m_currentGlobalPattern->normalPattern[color][SLEEP4].count > 0){//I have sleep4
		return MATE_VALUE+4;
	}else if(isDoubleSleep4(1-color)){//opposite has double sleep4
		return -MATE_VALUE-4;
	}else if(isSleep4AndActive3(1-color)){//opposite has sleep4+active3
		return -MATE_VALUE-3;
	}else if((m_currentGlobalPattern->normalPattern[color][ACTIVE3].count > 0) && (m_currentGlobalPattern->normalPattern[1-color][SLEEP4].count == 0)){//I have active3, opposite has no sleep4
		return MATE_VALUE+2;
	}else if(isOppositeSleep4OnMyActive3(color)){//I have active3, opposite has sleep4, but the sleep4 position is just on my active3
		return MATE_VALUE+1;
	}else if(isDoubleActive3(1-color)){//opposite has double active3
		if(m_currentGlobalPattern->normalPattern[color][ACTIVE3].count == 0 && m_currentGlobalPattern->normalPattern[color][SLEEP3].count == 0){//I have no sleep3 or active3, it must be loose
			return -MATE_VALUE-2;
		}else{//I have sleep3 or active3, it may not loose
			return -3000;
		}
	}else{// calculate evaluate value
		return getSimpleEvaluateValue(color);
	}
}

int BitBoard::getQuiescMoves(int color, int8* mvs) const
{
	return 0;
}

int BitBoard::getVCFAttackMoves(int color, int8* mvs) const
{
	int index = 0;
	int64 mvsBitmap[4];

	const NormalPattern& active3Pattern = m_currentGlobalPattern->normalPattern[color][ACTIVE3];
	const NormalPattern& sleep3Pattern = m_currentGlobalPattern->normalPattern[color][SLEEP3];
	const NormalPattern& oppositeSleep4Pattern = m_currentGlobalPattern->normalPattern[1-color][SLEEP4];

	int64 bitmap = active3Pattern.bitmap;
	while(bitmap)
	{
		int32 emptyPoses = EmptyPosArray[active3Pattern.patterns[LBS64(bitmap)]];
		bitmap &= bitmap - 1;//remove last bit '1'

		mvs[index++] = emptyPoses & 0xFF;
		mvs[index++] = (emptyPoses>>8) & 0xFF;
		mvs[index++] = (emptyPoses>>16) & 0xFF;
	}

	bitmap = sleep3Pattern.bitmap;
	while(bitmap)
	{
		int32 emptyPoses = EmptyPosArray[sleep3Pattern.patterns[LBS64(bitmap)]];
		bitmap &= bitmap - 1;//remove last bit '1'

		mvs[index++] = emptyPoses & 0xFF;
		mvs[index++] = (emptyPoses>>8) & 0xFF;
	}

	//remove duplicate
	index = sortAndRemoveDuplicate(mvs, index,mvsBitmap);

	//if opposite has sleep 4
	if(oppositeSleep4Pattern.count > 0)
	{
		//if opposite sleep4 empty point just on my sleep3 or active3, then return this position
		int8 sleep4Pos = EmptyPosArray[oppositeSleep4Pattern.patterns[LBS64(oppositeSleep4Pattern.bitmap)]];

		if(isInBitmap(mvsBitmap,sleep4Pos))
		{
			mvs[0] = sleep4Pos;
			return 1;
		}
		return 0;
	}

	return index;
}

int BitBoard::getVCFDefendMoves(int color, int8* mvs) const
{
	const NormalPattern& oppositeSleep4Pattern = m_currentGlobalPattern->normalPattern[1-color][SLEEP4];
	mvs[0] = EmptyPosArray[oppositeSleep4Pattern.patterns[LBS64(oppositeSleep4Pattern.bitmap)]];
	return 1;
}

int BitBoard::getVCTAttackMoves(int color, int8* mvs) const
{
	int index = 0;
	int64 mvsBitmap[4];

	//active3
	const NormalPattern& active3Pattern = m_currentGlobalPattern->normalPattern[color][ACTIVE3];
	int64 bitmap = active3Pattern.bitmap;
	while(bitmap)
	{
		int32 emptyPoses = EmptyPosArray[active3Pattern.patterns[LBS64(bitmap)]];
		bitmap &= bitmap - 1;//remove last bit '1'

		mvs[index++] = emptyPoses & 0xFF;
		mvs[index++] = (emptyPoses>>8) & 0xFF;
		mvs[index++] = (emptyPoses>>16) & 0xFF;
	}

	//sleep3
	const NormalPattern& sleep3Pattern = m_currentGlobalPattern->normalPattern[color][SLEEP3];
	bitmap = sleep3Pattern.bitmap;
	while(bitmap)
	{
		int32 emptyPoses = EmptyPosArray[sleep3Pattern.patterns[LBS64(bitmap)]];
		bitmap &= bitmap - 1;//remove last bit '1'

		mvs[index++] = emptyPoses & 0xFF;
		mvs[index++] = (emptyPoses>>8) & 0xFF;
	}

	//remove duplicate, and it's sorted
	index = sortAndRemoveDuplicate(mvs,index,mvsBitmap);


	//active2
	int8 active2AndVCFMvs[MAX_GEN_MOVES];
	int64 active2AndVCFMvsBitmap[4];
	int active2AndVCFMvsIndex = 0;

	const NormalPattern& active2Pattern = m_currentGlobalPattern->normalPattern[color][ACTIVE2];
	bitmap = active2Pattern.bitmap;
	while(bitmap)
	{
		int32 emptyPoses = EmptyPosArray[active2Pattern.patterns[LBS64(bitmap)]];
		bitmap &= bitmap - 1;//remove last bit '1'

		for (int j = 1;j<3;j++)
		{
			int8 pos = emptyPoses>>(8*j);
			//it should not on own active3 or sleep3, mvs is sorted
			if(!isInBitmap(mvsBitmap,pos)){
				active2AndVCFMvs[active2AndVCFMvsIndex++] = pos;
			}
		}
	}

	//make VCF attack move
	int8 makeVCFMvs[MAX_GEN_MOVES];

	////FIEME: currently, I ignore the Make VCF attack, because it will make 5 times slow
	int makeVCFMvsIndex = getMakeVCFMoves(color, makeVCFMvs);
	//int makeVCFMvsIndex = 0;

	//filter already existed moves
	//add to active2AndVCFMvs
	for(int i = 0;i<makeVCFMvsIndex;i++)
	{
		if(!isInBitmap(mvsBitmap,makeVCFMvs[i]))
		{
			active2AndVCFMvs[active2AndVCFMvsIndex++] = makeVCFMvs[i];
		}
	}

	assert(active2AndVCFMvsIndex < 225);

	//remove duplicate, and it's sorted
	active2AndVCFMvsIndex = sortAndRemoveDuplicate(active2AndVCFMvs,active2AndVCFMvsIndex,active2AndVCFMvsBitmap);


	//if opposite has sleep4
	const NormalPattern& oppositeSleep4Pattern = m_currentGlobalPattern->normalPattern[1-color][SLEEP4];
	if(oppositeSleep4Pattern.count > 0)
	{
		int8 sleep4Pos = EmptyPosArray[oppositeSleep4Pattern.patterns[LBS64(oppositeSleep4Pattern.bitmap)]];

		//if we still has avtive3, continue
		if(active3Pattern.count>0)
		{
			mvs[0] = sleep4Pos;
			return 1;
		}

		//if we still has VCF attack move, continue
		////FIXME: currently ignore make VCF attack move
		if(hasVCFAttackMove(color))
		{
			mvs[0] = sleep4Pos;
			return 1;
		}

		//if opposite sleep4 empty point just on my sleep3 or active3, then return this position
		if(isInBitmap(mvsBitmap,sleep4Pos))
		{
			mvs[0] = sleep4Pos;
			return 1;
		}

		//if opposite sleep4 empty point just on my active2 or VCF attack move, then return this position
		if(isInBitmap(active2AndVCFMvsBitmap,sleep4Pos))
		{
			mvs[0] = sleep4Pos;
			return 1;
		}

		//if no attack existed, return no move
		return 0;
	}

	//defend to opposite active3 and VCF
	//to find some positions, each one can stop all opposite active3 and VCF
	int8 intersection[100];
	int intersectionIndex = 0;
	bool intersectionInitial = getOppositeActive3AndVCFIntersectionMvs(intersection, intersectionIndex, color);

	if(intersectionInitial)
	{
		//if opposite has active 3 or VCF, only the active2 or VCF attack move position that can stop all opposite active3 and VCF, will be accepted
		int lastIndex = 0;
		for(int i = 0;i<intersectionIndex;i++)
		{
			//if the intersection pos belongs to avtive2 or VCF attack move
			if(isInBitmap(active2AndVCFMvsBitmap,intersection[i]))
			{
				intersection[lastIndex++] = intersection[i];
			}
		}

		for(int i = 0;i<lastIndex;i++)
		{
			active2AndVCFMvs[i] = intersection[i];
		}

		active2AndVCFMvsIndex = lastIndex;
	}


	//add active2 and VCF attack move position with active3 and sleep3
	for(int i = 0;i<active2AndVCFMvsIndex;i++)
	{
		mvs[index++] = active2AndVCFMvs[i];
	}

	return index;
}

int BitBoard::getVCTDefendMoves(int color, int8* mvs) const
{
	int index = 0;
	int64 mvsBitmap[4];

	const NormalPattern& oppositeSleep4Pattern = m_currentGlobalPattern->normalPattern[1-color][SLEEP4];
	//if opposite has sleep4
	if(oppositeSleep4Pattern.count > 0)
	{
		mvs[0] = EmptyPosArray[oppositeSleep4Pattern.patterns[LBS64(oppositeSleep4Pattern.bitmap)]];
		return 1;
	}

	//attack with own sleep3
	const NormalPattern& sleep3Pattern = m_currentGlobalPattern->normalPattern[color][SLEEP3];
	int64 bitmap = sleep3Pattern.bitmap;
	while(bitmap)
	{
		int32 emptyPoses = EmptyPosArray[sleep3Pattern.patterns[LBS64(bitmap)]];
		bitmap &= bitmap - 1;//remove last bit '1'

		mvs[index++] = emptyPoses & 0xFF;
		mvs[index++] = (emptyPoses>>8) & 0xFF;
	}

	//attack with own active3
	const NormalPattern& active3Pattern = m_currentGlobalPattern->normalPattern[color][ACTIVE3];

	bitmap = active3Pattern.bitmap;
	while(bitmap)
	{
		int32 emptyPoses = EmptyPosArray[active3Pattern.patterns[LBS64(bitmap)]];
		bitmap &= bitmap - 1;//remove last bit '1'

		mvs[index++] = emptyPoses & 0xFF;
		mvs[index++] = (emptyPoses>>8) & 0xFF;
		mvs[index++] = (emptyPoses>>16) & 0xFF;
	}

	//defend to opposite active3 and VCF
	//to find some positions, each one can stop all opposite active3 and VCF
	int8 intersection[100];
	int intersectionIndex = 0;
	bool intersectionInitial = getOppositeActive3AndVCFIntersectionMvs(intersection, intersectionIndex, color);

	//add intersection to moves
	for(int i = 0;i<intersectionIndex;i++)
	{
		mvs[index++] = intersection[i];
	}

	//remove duplicate
	index = sortAndRemoveDuplicate(mvs, index,mvsBitmap);

	return index;
}

/**
 * it has 6 types to make VCF attack
 * Sleep3 + Active1
 * Sleep3 + Avtive2
 * Sleep3 + Sleep2
 * Active2 + Active2
 * Sleep2 + Avtive2
 * Sleep2 + Sleep2
 */
int BitBoard::getMakeVCFMoves(int color, int8* mvs) const
{
	int index = 0;
	int64 mvsBitmap[4];

	int8 VCFMvs[10];
	int VCFIndex = 0;
	const NormalPattern& sleep3Pattern = m_currentGlobalPattern->normalPattern[color][SLEEP3];
	const Active1Pattern& active1Pattern = m_currentGlobalPattern->active1Pattern[color];
	const NormalPattern& active2Pattern = m_currentGlobalPattern->normalPattern[color][ACTIVE2];
	const NormalPattern& sleep2Pattern = m_currentGlobalPattern->normalPattern[color][SLEEP2];


	//temp store active2
	int32 active2Mvs[64];
	int active2MvsIndex = 0;
	int64 active2Bitmap = active2Pattern.bitmap;
	while(active2Bitmap)
	{
		int32 active2EmptyPoses = EmptyPosArray[active2Pattern.patterns[LBS64(active2Bitmap)]];
		active2Bitmap &= active2Bitmap - 1;//remove last bit '1'

		active2Mvs[active2MvsIndex++] = active2EmptyPoses;
	}

	//temp store sleep2
	int32 sleep2Mvs[64];
	int sleep2MvsIndex = 0;
	int64 sleep2Bitmap = sleep2Pattern.bitmap;
	while(sleep2Bitmap)
	{
		int32 sleep2EmptyPoses = EmptyPosArray[sleep2Pattern.patterns[LBS64(sleep2Bitmap)]];
		sleep2Bitmap &= sleep2Bitmap - 1;//remove last bit '1'

		sleep2Mvs[sleep2MvsIndex++] = sleep2EmptyPoses;
	}



	// Sleep3
	int64 sleep3Bitmap = sleep3Pattern.bitmap;
	while(sleep3Bitmap)
	{
		int32 sleep3EmptyPoses = EmptyPosArray[sleep3Pattern.patterns[LBS64(sleep3Bitmap)]];
		sleep3Bitmap &= sleep3Bitmap - 1;//remove last bit '1'

		// Sleep3 + Active1
		for (int k = 0;k<4;k++)
		{
			int64 active1Bitmap = active1Pattern.bitmap[k];
			while(active1Bitmap)
			{
				int32 active1EmptyPoses = EmptyPosArray[active1Pattern.patterns[LBS64(active1Bitmap)+(k<<6)]];
				active1Bitmap &= active1Bitmap - 1;//remove last bit '1'

				// FIXME: below code is the correct active1, but there are too many active1, to make a tradeoff, we use a simple active1 instead

				//VCFMvs[0] = active1EmptyPoses & 0xFF;
				//VCFMvs[1] = (active1EmptyPoses>>8) & 0xFF;
				//VCFMvs[2] = (active1EmptyPoses>>16) & 0xFF;
				//VCFMvs[3] = sleep3EmptyPoses & 0xFF;
				//VCFMvs[4] = (sleep3EmptyPoses>>8) & 0xFF;

				////only remove duplicate, V[0-2] is already sorted from small to large, V[3-4] is already sorted from small to large
				//VCFIndex = 5;
				//for(int m = 3;m<VCFIndex;m++)
				//{
				//	for(int n = 0;n<3;n++)
				//	{
				//		if(VCFMvs[m] == VCFMvs[n])
				//		{
				//			VCFMvs[m--] = VCFMvs[--VCFIndex];
				//			break;
				//		}
				//	}
				//}


				//if(VCFIndex == 4 && !isOnSameLine(VCFMvs, VCFIndex))
				//{
				//	mvs[index++]  = active1EmptyPoses & 0xFF;
				//	mvs[index++]  = (active1EmptyPoses>>8) & 0xFF;
				//	mvs[index++]  = (active1EmptyPoses>>16) & 0xFF;
				//}


				VCFMvs[0] = active1EmptyPoses & 0xFF;
				VCFMvs[1] = (active1EmptyPoses>>8) & 0xFF;
				VCFMvs[2] = (active1EmptyPoses>>16) & 0xFF;
				VCFMvs[3] = (active1EmptyPoses>>24) & 0xFF;
				VCFMvs[4] = sleep3EmptyPoses & 0xFF;
				VCFMvs[5] = (sleep3EmptyPoses>>8) & 0xFF;

				//only remove duplicate, V[0-3] is already sorted from small to large, V[4-5] is already sorted from small to large
				VCFIndex = 6;
				int8 intersectPoint = NO_MOVE;
				for(int m = 4;m<VCFIndex;m++)
				{
					for(int n = 0;n<4;n++)
					{
						if(VCFMvs[m] == VCFMvs[n])
						{
							intersectPoint = VCFMvs[m];
							VCFMvs[m--] = VCFMvs[--VCFIndex];
							break;
						}
					}
				}


				if(VCFIndex == 5 && !isOnSameLine(VCFMvs, VCFIndex))
				{
					mvs[index++]  = (active1EmptyPoses>>8) & 0xFF;
					mvs[index++]  = (active1EmptyPoses>>16) & 0xFF;

					if((intersectPoint == ((active1EmptyPoses>>8) & 0xFF)) ||
						(intersectPoint == ((active1EmptyPoses>>16) & 0xFF)))
					{
						mvs[index++]  = (active1EmptyPoses) & 0xFF;
						mvs[index++]  = (active1EmptyPoses>>24) & 0xFF;
					}
				}
			}
		}


		// Sleep3 + Avtive2
		for(int i = 0;i<active2MvsIndex;i++)
		{
			VCFMvs[0] = sleep3EmptyPoses & 0xFF;
			VCFMvs[1] = (sleep3EmptyPoses>>8) & 0xFF;
			VCFMvs[2] = (active2Mvs[i]>>8) & 0xFF;
			VCFMvs[3] = (active2Mvs[i]>>16) & 0xFF;

			//only remove duplicate, V[0-1] is already sorted from small to large, V[2-3] is already sorted from small to large
			VCFIndex = 4;
			for(int m = 2;m<VCFIndex;m++)
			{
				for(int n = 0;n<2;n++)
				{
					if(VCFMvs[m] == VCFMvs[n])
					{
						VCFMvs[m--] = VCFMvs[--VCFIndex];
						break;
					}
				}
			}

			if(VCFIndex == 3 && !isOnSameLine(VCFMvs, VCFIndex))
			{
				mvs[index++] = active2Mvs[i] & 0xFF;
				mvs[index++] = (active2Mvs[i]>>24) & 0xFF;
			}
		}


		// Sleep3 + Sleep2
		for(int i = 0;i<sleep2MvsIndex;i++)
		{
			VCFMvs[0] = sleep2Mvs[i] & 0xFF;
			VCFMvs[1] = (sleep2Mvs[i]>>8) & 0xFF;
			VCFMvs[2] = (sleep2Mvs[i]>>16) & 0xFF;
			VCFMvs[3] = sleep3EmptyPoses & 0xFF;
			VCFMvs[4] = (sleep3EmptyPoses>>8) & 0xFF;

			//only remove duplicate, V[0-2] is already sorted from small to large, V[3-4] is already sorted from small to large
			VCFIndex = 5;
			for(int m = 3;m<VCFIndex;m++)
			{
				for(int n = 0;n<3;n++)
				{
					if(VCFMvs[m] == VCFMvs[n])
					{
						VCFMvs[m--] = VCFMvs[--VCFIndex];
						break;
					}
				}
			}

			if(VCFIndex == 4 && !isOnSameLine(VCFMvs, VCFIndex))
			{
				mvs[index++] = sleep2Mvs[i] & 0xFF;
				mvs[index++] = (sleep2Mvs[i]>>8) & 0xFF;
				mvs[index++] = (sleep2Mvs[i]>>16) & 0xFF;
			}
		}
	}

	// Active2 + Active2
	for(int i = 0;i<active2MvsIndex;i++)
	{
		for(int j = i+1; j<active2MvsIndex;j++)
		{
			VCFMvs[0] = (active2Mvs[i]>>8) & 0xFF;
			VCFMvs[1] = (active2Mvs[i]>>16) & 0xFF;
			VCFMvs[2] = (active2Mvs[j]>>8) & 0xFF;
			VCFMvs[3] = (active2Mvs[j]>>16) & 0xFF;

			//only remove duplicate, V[0-1] is already sorted from small to large, V[2-3] is already sorted from small to large
			VCFIndex = 4;
			for(int m = 2;m<VCFIndex;m++)
			{
				for(int n = 0;n<2;n++)
				{
					if(VCFMvs[m] == VCFMvs[n])
					{
						VCFMvs[m--] = VCFMvs[--VCFIndex];
						break;
					}
				}
			}

			if(VCFIndex == 3 && !isOnSameLine(VCFMvs, VCFIndex))
			{
				mvs[index++] = active2Mvs[i] & 0xFF;
				mvs[index++] = (active2Mvs[i]>>24) & 0xFF;
				mvs[index++] = active2Mvs[j] & 0xFF;
				mvs[index++] = (active2Mvs[j]>>24) & 0xFF;
			}
		}
	}

	// Sleep2 + Avtive2
	for(int i = 0;i<active2MvsIndex;i++)
	{
		for(int j = 0; j<sleep2MvsIndex;j++)
		{
			VCFMvs[0] = sleep2Mvs[j] & 0xFF;
			VCFMvs[1] = (sleep2Mvs[j]>>8) & 0xFF;
			VCFMvs[2] = (sleep2Mvs[j]>>16) & 0xFF;
			VCFMvs[3] = (active2Mvs[i]>>8) & 0xFF;
			VCFMvs[4] = (active2Mvs[i]>>16) & 0xFF;

			//only remove duplicate, V[0-2] is already sorted from small to large, V[3-4] is already sorted from small to large
			VCFIndex = 5;
			for(int m = 3;m<VCFIndex;m++)
			{
				for(int n = 0;n<3;n++)
				{
					if(VCFMvs[m] == VCFMvs[n])
					{
						VCFMvs[m--] = VCFMvs[--VCFIndex];
						break;
					}
				}
			}

			if(VCFIndex == 4 && !isOnSameLine(VCFMvs, VCFIndex))
			{
				mvs[index++] = sleep2Mvs[j] & 0xFF;
				mvs[index++] = (sleep2Mvs[j]>>8) & 0xFF;
				mvs[index++] = (sleep2Mvs[j]>>16) & 0xFF;
			}
		}
	}

	// Sleep2 + Sleep2
	for(int i = 0;i<sleep2MvsIndex;i++)
	{
		for(int j = i+1; j<sleep2MvsIndex;j++)
		{
			VCFMvs[0] = sleep2Mvs[i] & 0xFF;
			VCFMvs[1] = (sleep2Mvs[i]>>8) & 0xFF;
			VCFMvs[2] = (sleep2Mvs[i]>>16) & 0xFF;
			VCFMvs[3] = sleep2Mvs[j] & 0xFF;
			VCFMvs[4] = (sleep2Mvs[j]>>8) & 0xFF;
			VCFMvs[5] = (sleep2Mvs[j]>>16) & 0xFF;

			//only remove duplicate, V[0-2] is already sorted from small to large, V[3-5] is already sorted from small to large
			VCFIndex = 6;
			for(int m = 3;m<VCFIndex;m++)
			{
				for(int n = 0;n<3;n++)
				{
					if(VCFMvs[m] == VCFMvs[n])
					{
						VCFMvs[m--] = VCFMvs[--VCFIndex];
						break;
					}
				}
			}

			if(VCFIndex == 4)//must be on the same line
			{
				mvs[index++]  = (sleep2Mvs[i]>>8) & 0xFF;
				mvs[index++]  = (sleep2Mvs[j]>>8) & 0xFF;
			}
		}
	}

	assert(index < 225);
	return sortAndRemoveDuplicate(mvs, index,mvsBitmap);
}

__forceinline bool BitBoard::isDoubleSleep4(int color) const
{
	const NormalPattern& pattern = m_currentGlobalPattern->normalPattern[color][SLEEP4];
	if(pattern.count < 2)
	{
		return false;
	}
	int64 bitmap = pattern.bitmap;

	int8 firstPos = EmptyPosArray[pattern.patterns[LBS64(bitmap)]];

	for (int i = 1;i<pattern.count;i++)
	{
		bitmap &= bitmap - 1;//remove last bit '1'
		int8 pos = EmptyPosArray[pattern.patterns[LBS64(bitmap)]];

		if(pos != firstPos)
		{
			return true;
		}
	}

	return false;
}

__forceinline bool BitBoard::isSleep4AndActive3(int color) const
{
	const NormalPattern& sleep4Pattern = m_currentGlobalPattern->normalPattern[color][SLEEP4];
	const NormalPattern& active3Pattern = m_currentGlobalPattern->normalPattern[color][ACTIVE3];

	if(sleep4Pattern.count == 0 || active3Pattern.count == 0)
	{
		return false;
	}
	//only one Sleep 4
	int64 bitmap = sleep4Pattern.bitmap;
	int8 emptySleep4Pos = EmptyPosArray[sleep4Pattern.patterns[LBS64(bitmap)]];

	//check all own active3, whether there is one avtive3, all of its empty pos not equal to sleep 4 empty pos.
	bool isAssociateWithOwnActive3 = true;
	bitmap = active3Pattern.bitmap;
	while(bitmap)
	{
		int32 emptyPoses = EmptyPosArray[active3Pattern.patterns[LBS64(bitmap)]];
		bitmap &= bitmap - 1;//remove last bit '1'

		if((emptySleep4Pos != (emptyPoses & 0xFF)) && (emptySleep4Pos != ((emptyPoses >> 8) & 0xFF)) && (emptySleep4Pos != ((emptyPoses>>16)&0xFF)))
		{
			isAssociateWithOwnActive3 = false;
			break;
		}
	}

	if(isAssociateWithOwnActive3){
		return false;
	}

	//check opposite active3, whether own sleep4 empty pos is on any of them.
	const NormalPattern& oppoActive3Pattern = m_currentGlobalPattern->normalPattern[1-color][ACTIVE3];
	bitmap = oppoActive3Pattern.bitmap;
	while(bitmap)
	{
		int32 emptyPoses = EmptyPosArray[oppoActive3Pattern.patterns[LBS64(bitmap)]];
		bitmap &= bitmap - 1;//remove last bit '1'

		if((emptySleep4Pos == (emptyPoses & 0xFF)) || (emptySleep4Pos == ((emptyPoses >> 8) & 0xFF)) || (emptySleep4Pos == ((emptyPoses>>16)&0xFF)))
		{
			return false;
		}
	}


	//check opposite sleep3, whether own sleep4 empty pos is on any of them.
	const NormalPattern& oppoSleep3Pattern = m_currentGlobalPattern->normalPattern[1-color][SLEEP3];
	bitmap = oppoSleep3Pattern.bitmap;
	while(bitmap)
	{
		int32 emptyPoses = EmptyPosArray[oppoSleep3Pattern.patterns[LBS64(bitmap)]];
		bitmap &= bitmap - 1;//remove last bit '1'

		if((emptySleep4Pos == (emptyPoses & 0xFF)) || (emptySleep4Pos == ((emptyPoses >> 8) & 0xFF)))
		{
			return false;
		}
	}

	return true;
}
__forceinline bool BitBoard::isDoubleActive3(int color) const
{
	const NormalPattern& active3Pattern = m_currentGlobalPattern->normalPattern[color][ACTIVE3];

	if(active3Pattern.count < 2){
		return false;
	}

	//get all opposite active3 and sleep3 empty position
	int8 oppositeActive3Sleep3Mvs[256];
	int64 oppositeActive3Sleep3MvsBitmap[4];
	int oppositeActive3Sleep3MvsCnt = 0;
	const NormalPattern& oppositeActive3Pattern = m_currentGlobalPattern->normalPattern[1-color][ACTIVE3];
	const NormalPattern& oppositeSleep3Pattern = m_currentGlobalPattern->normalPattern[1-color][SLEEP3];

	//opposite avtive3
	int64 bitmap = oppositeActive3Pattern.bitmap;
	while(bitmap)
	{
		int32 emptyPoses = EmptyPosArray[oppositeActive3Pattern.patterns[LBS64(bitmap)]];
		bitmap &= bitmap - 1;

		oppositeActive3Sleep3Mvs[oppositeActive3Sleep3MvsCnt++] = emptyPoses & 0xFF;
		oppositeActive3Sleep3Mvs[oppositeActive3Sleep3MvsCnt++] = (emptyPoses>>8) & 0xFF;
		oppositeActive3Sleep3Mvs[oppositeActive3Sleep3MvsCnt++] = (emptyPoses>>16) & 0xFF;
	}

	//opposite sleep3
	bitmap = oppositeSleep3Pattern.bitmap;
	while(bitmap)
	{
		int32 emptyPoses = EmptyPosArray[oppositeSleep3Pattern.patterns[LBS64(bitmap)]];
		bitmap &= bitmap - 1;

		oppositeActive3Sleep3Mvs[oppositeActive3Sleep3MvsCnt++] = emptyPoses & 0xFF;
		oppositeActive3Sleep3Mvs[oppositeActive3Sleep3MvsCnt++] = (emptyPoses>>8) & 0xFF;
		oppositeActive3Sleep3Mvs[oppositeActive3Sleep3MvsCnt++] = (emptyPoses>>16) & 0xFF;
	}
	oppositeActive3Sleep3MvsCnt = sortAndRemoveDuplicate(oppositeActive3Sleep3Mvs,oppositeActive3Sleep3MvsCnt,oppositeActive3Sleep3MvsBitmap);


	//check opposite active3 and sleep3, they don't Intersect with own active3 at empty position
	int realActive3Cnt = 0;
	int32 posRealActive3[BOARD_LENGTH];

	bitmap = active3Pattern.bitmap;
	while(bitmap)
	{
		int32 emptyPoses = EmptyPosArray[active3Pattern.patterns[LBS64(bitmap)]];
		bitmap &= bitmap - 1;

		if (isInBitmap(oppositeActive3Sleep3MvsBitmap,emptyPoses & 0xFF) ||
			isInBitmap(oppositeActive3Sleep3MvsBitmap,(emptyPoses>>8) & 0xFF) ||
			isInBitmap(oppositeActive3Sleep3MvsBitmap,(emptyPoses>>16) & 0xFF))
		{
			continue;
		}

		posRealActive3[realActive3Cnt++] = emptyPoses;
	}

	//check own active3, whether has two active3, they don't Intersect at empty position
	for(int i = 0;i<realActive3Cnt;i++)
	{
		for (int j = i+1;j<realActive3Cnt;j++)
		{
			int32 posA = posRealActive3[i];
			int32 posB = posRealActive3[j];

			if(((posA & 0xFF) != (posB & 0xFF)) &&
			   ((posA & 0xFF) != ((posB>>8) & 0xFF)) &&
			   ((posA & 0xFF) != ((posB>>16) & 0xFF)) &&
			   (((posA>>8) & 0xFF) != (posB & 0xFF)) &&
			   (((posA>>8) & 0xFF) != ((posB>>8) & 0xFF)) &&
			   (((posA>>8) & 0xFF) != ((posB>>16) & 0xFF)) &&
			   (((posA>>16) & 0xFF) != (posB & 0xFF)) &&
			   (((posA>>16) & 0xFF) != ((posB>>8) & 0xFF)) &&
			   (((posA>>16) & 0xFF) != ((posB>>16) & 0xFF)))
			{
				return true;
			}
		}
	}

	return false;
}

__forceinline bool BitBoard::isOppositeSleep4OnMyActive3(int color) const
{
	const NormalPattern& oppositeSleep4Pattern = m_currentGlobalPattern->normalPattern[1-color][SLEEP4];
	const NormalPattern& active3Pattern = m_currentGlobalPattern->normalPattern[color][ACTIVE3];

	if(active3Pattern.count < 1 || oppositeSleep4Pattern.count < 1){
		return false;
	}

	int64 bitmap = oppositeSleep4Pattern.bitmap;
	int oppositeSleep4Pos = EmptyPosArray[oppositeSleep4Pattern.patterns[LBS64(bitmap)]];

	bitmap = active3Pattern.bitmap;
	while(bitmap)
	{
		int32 emptyPoses = EmptyPosArray[active3Pattern.patterns[LBS64(bitmap)]];
		bitmap &= bitmap - 1;

		//active 3 has 3 empty position, but only the middle one can make it to active4
		if(((emptyPoses>>8)&0xFF) == oppositeSleep4Pos)
		{
			return true;
		}
	}

	return false;
}

bool BitBoard::getOppositeActive3AndVCFIntersectionMvs(int8* intersection, int &intersectionIndex, int color) const
{
	bool intersectionInitial = false;

	//opposite active3
	const NormalPattern& oppositeActive3Pattern = m_currentGlobalPattern->normalPattern[1-color][ACTIVE3];
	if(oppositeActive3Pattern.count > 0)
	{
		int64 oppositeActive3Bitmap = oppositeActive3Pattern.bitmap;
		int32 emptyPoses = EmptyPosArray[oppositeActive3Pattern.patterns[LBS64(oppositeActive3Bitmap)]];
		oppositeActive3Bitmap &= oppositeActive3Bitmap - 1;

		intersection[intersectionIndex++] = emptyPoses & 0xFF;
		intersection[intersectionIndex++] = (emptyPoses>>8) & 0xFF;
		intersection[intersectionIndex++] = (emptyPoses>>16) & 0xFF;

		intersectionInitial = true;

		//intersect all active3, each active3 position must be sorted from small to large
		while(oppositeActive3Bitmap && intersectionIndex > 0)
		{
			emptyPoses = EmptyPosArray[oppositeActive3Pattern.patterns[LBS64(oppositeActive3Bitmap)]];
			oppositeActive3Bitmap &= oppositeActive3Bitmap - 1;

			int8 active3Mvs[3];
			active3Mvs[0] = emptyPoses & 0xFF;
			active3Mvs[1] = (emptyPoses>>8) & 0xFF;
			active3Mvs[2] = (emptyPoses>>16) & 0xFF;

			intersectionIndex = getIntersection(intersection,intersectionIndex,active3Mvs,3);
		}
	}

	//opposite VCF
	int8 VCFMvs[100];
	int64 VCFMvsBitmap[4];
	int VCFIndex = 0;
	const NormalPattern& oppositeSleep3Pattern = m_currentGlobalPattern->normalPattern[1-color][SLEEP3];
	const NormalPattern& oppositeActive2Pattern = m_currentGlobalPattern->normalPattern[1-color][ACTIVE2];

	int32 oppositeSleep3Poses[64];
	int oppositeSleep3PosesCnt = 0;
	int64 oppositeSleep3PatternBitmap = oppositeSleep3Pattern.bitmap;

	while(oppositeSleep3PatternBitmap)
	{
		int32 emptyPoses = EmptyPosArray[oppositeSleep3Pattern.patterns[LBS64(oppositeSleep3PatternBitmap)]];
		oppositeSleep3PatternBitmap &= oppositeSleep3PatternBitmap - 1;

		oppositeSleep3Poses[oppositeSleep3PosesCnt++] = emptyPoses;
	}

	for(int i = 0;i<oppositeSleep3PosesCnt && (!intersectionInitial || intersectionIndex > 0);i++)
	{
		//double sleep3 judge
		for(int j = i+1; j<oppositeSleep3PosesCnt;j++)
		{
			VCFMvs[0] = oppositeSleep3Poses[i] & 0xFF;
			VCFMvs[1] = (oppositeSleep3Poses[i]>>8) & 0xFF;
			VCFMvs[2] = oppositeSleep3Poses[j] & 0xFF;
			VCFMvs[3] = (oppositeSleep3Poses[j]>>8) & 0xFF;

			//only remove duplicate, V[0-1] is already sorted from small to large, V[2-3] is already sorted from small to large
			VCFIndex = 4;
			for(int m = 2;m<VCFIndex;m++)
			{
				for(int n = 0;n<2;n++)
				{
					if(VCFMvs[m] == VCFMvs[n])
					{
						VCFMvs[m--] = VCFMvs[--VCFIndex];
						break;
					}
				}
			}

			//real double sleep3, they have and only have one same empty position
			if(VCFIndex == 3)
			{
				//sort VCFMvs, only 3 numbers, first 2 numbers already sorted
				for(int m = 2;m>0;m--)
				{
					if(VCFMvs[m] >= VCFMvs[m - 1])
					{
						break;
					}
					else
					{
						int8 temp = VCFMvs[m];
						VCFMvs[m] = VCFMvs[m - 1];
						VCFMvs[m - 1] = temp;
					}
				}

				if(!intersectionInitial)
				{
					for (int k = 0;k<VCFIndex;k++)
					{
						intersection[intersectionIndex++] = VCFMvs[k];
					}

					intersectionInitial = true;
				}
				else
				{
					intersectionIndex = getIntersection(intersection,intersectionIndex,VCFMvs,VCFIndex);
					if(intersectionIndex == 0)
					{
						break;
					}
				}
			}
		}

		if(intersectionInitial && intersectionIndex == 0)
		{
			break;
		}

		//sleep3+avtive2 judge
		int64 oppositeActive2Bitmap = oppositeActive2Pattern.bitmap;
		while(oppositeActive2Bitmap)
		{
			int32 oppositeActive2EmptyPoses = EmptyPosArray[oppositeActive2Pattern.patterns[LBS64(oppositeActive2Bitmap)]];
			oppositeActive2Bitmap &= oppositeActive2Bitmap - 1;

			VCFMvs[0] = oppositeSleep3Poses[i] & 0xFF;
			VCFMvs[1] = (oppositeSleep3Poses[i]>>8) & 0xFF;
			VCFMvs[2] = (oppositeActive2EmptyPoses>>8) & 0xFF;
			VCFMvs[3] = (oppositeActive2EmptyPoses>>16) & 0xFF;

			//remove duplicate, V[0-1] is already sorted from small to large, V[2-3] is already sorted from small to large
			VCFIndex = 4;
			for(int m = 2;m<VCFIndex;m++)
			{
				for(int n = 0;n<2;n++)
				{
					if(VCFMvs[m] == VCFMvs[n])
					{
						VCFMvs[m--] = VCFMvs[--VCFIndex];
						break;
					}
				}
			}

			if(VCFIndex == 3)
			{
				//find the sleep3 position which is not on active2, this position is the next attack position after defend.
				int nextDefendPosIndex = ((oppositeSleep3Poses[i] & 0xFF) != ((oppositeActive2EmptyPoses>>8) & 0xFF)) &&
						((oppositeSleep3Poses[i] & 0xFF) != ((oppositeActive2EmptyPoses>>16) & 0xFF)) ? 0 : 1;
				int8 nextSleep3DefendPos = (oppositeSleep3Poses[i]>>(8*nextDefendPosIndex)) & 0xFF;


				VCFMvs[3] = oppositeActive2EmptyPoses & 0xFF;
				VCFMvs[4] = (oppositeActive2EmptyPoses>>24) & 0xFF;

				//only remove duplicate, V[0-2] is different from each other, V[3-4] is already sorted from small to large
				VCFIndex = 5;
				for(int m = 3;m<VCFIndex;m++)
				{
					for(int n = 0;n<3;n++)
					{
						if(VCFMvs[m] == VCFMvs[n])
						{
							VCFMvs[m--] = VCFMvs[--VCFIndex];
							break;
						}
					}
				}

				//real sleep3+active2, they have and only have one same empty position
				if(VCFIndex == 5)
				{
					//fix a bug, when exist two attack points, active3 and sleep3+active2, in the previous, it will directly return 0, means no move,
					//but this attack needs 3 steps to win for the attacker,
					//during these steps, the defender can make a active4 or sleep4 from his active2 or sleep2, stop the attacker.
					//calculate the positions where defender can make a active4 or sleep4, add them to VCF defend array
					const NormalPattern& active2Pattern = m_currentGlobalPattern->normalPattern[color][ACTIVE2];
					int64 active2Bitmap = active2Pattern.bitmap;
					while(active2Bitmap)
					{
						int32 active2EmptyPoses = EmptyPosArray[active2Pattern.patterns[LBS64(active2Bitmap)]];
						active2Bitmap &= active2Bitmap - 1;

						if(((active2EmptyPoses & 0xFF) == nextSleep3DefendPos) ||
							(((active2EmptyPoses>>8)& 0xFF) == nextSleep3DefendPos) ||
							(((active2EmptyPoses>>16)& 0xFF) == nextSleep3DefendPos) ||
							(((active2EmptyPoses>>24)& 0xFF) == nextSleep3DefendPos))
						{
							for(int m = 0;m<4;m++)
							{
								if(((active2EmptyPoses>>(m*8)) & 0xFF) != nextSleep3DefendPos)
								{
									VCFMvs[VCFIndex++] = (active2EmptyPoses>>(m*8)) & 0xFF;
								}
							}
						}
					}

					const NormalPattern& sleep2Pattern = m_currentGlobalPattern->normalPattern[color][SLEEP2];
					int64 sleep2Bitmap = sleep2Pattern.bitmap;
					while(sleep2Bitmap)
					{
						int32 sleep2EmptyPoses = EmptyPosArray[sleep2Pattern.patterns[LBS64(sleep2Bitmap)]];
						sleep2Bitmap &= sleep2Bitmap - 1;

						if(((sleep2EmptyPoses & 0xFF) == nextSleep3DefendPos) || (((sleep2EmptyPoses>>8)& 0xFF) == nextSleep3DefendPos)
							|| (((sleep2EmptyPoses>>16)& 0xFF) == nextSleep3DefendPos))
						{
							for(int m = 0;m<3;m++)
							{
								if(((sleep2EmptyPoses>>(m*8)) & 0xFF) != nextSleep3DefendPos)
								{
									VCFMvs[VCFIndex++] = (sleep2EmptyPoses>>(m*8)) & 0xFF;
								}
							}
						}
					}

					VCFIndex = sortAndRemoveDuplicate(VCFMvs, VCFIndex,VCFMvsBitmap);


					if(!intersectionInitial)
					{
						for (int k = 0;k<VCFIndex;k++)
						{
							intersection[intersectionIndex++] = VCFMvs[k];
						}

						intersectionInitial = true;
					}
					else
					{
						intersectionIndex = getIntersection(intersection,intersectionIndex,VCFMvs,VCFIndex);

						if(intersectionIndex == 0)
						{
							break;
						}
					}
				}
			}
		}
	}

	return intersectionInitial;
}

bool BitBoard::hasVCFAttackMove(int color) const
{
	int8 VCFMvs[10];
	int VCFIndex = 0;

	const NormalPattern& sleep3Pattern = m_currentGlobalPattern->normalPattern[color][SLEEP3];
	const NormalPattern& active2Pattern = m_currentGlobalPattern->normalPattern[color][ACTIVE2];

	int32 sleep3Poses[64];
	int sleep3PosesCnt = 0;
	int64 sleep3PatternBitmap = sleep3Pattern.bitmap;

	while(sleep3PatternBitmap)
	{
		int32 emptyPoses = EmptyPosArray[sleep3Pattern.patterns[LBS64(sleep3PatternBitmap)]];
		sleep3PatternBitmap &= sleep3PatternBitmap - 1;

		sleep3Poses[sleep3PosesCnt++] = emptyPoses;
	}

	for(int i = 0;i<sleep3PosesCnt;i++)
	{
		//double sleep3 judge
		for(int j = i+1; j<sleep3PosesCnt;j++)
		{
			VCFMvs[0] = sleep3Poses[i] & 0xFF;
			VCFMvs[1] = (sleep3Poses[i]>>8) & 0xFF;
			VCFMvs[2] = sleep3Poses[j] & 0xFF;
			VCFMvs[3] = (sleep3Poses[j]>>8) & 0xFF;

			//only remove duplicate, V[0-1] is already sorted from small to large, V[2-3] is already sorted from small to large
			VCFIndex = 4;
			for(int m = 2;m<VCFIndex;m++)
			{
				for(int n = 0;n<2;n++)
				{
					if(VCFMvs[m] == VCFMvs[n])
					{
						VCFMvs[m--] = VCFMvs[--VCFIndex];
						break;
					}
				}
			}

			//real double sleep3, they have and only have one same empty position
			if(VCFIndex == 3 && !isOnSameLine(VCFMvs, VCFIndex))
			{
				return true;
			}
		}

		//sleep3+avtive2 judge
		int64 active2Bitmap = active2Pattern.bitmap;
		while(active2Bitmap)
		{
			int32 active2EmptyPoses = EmptyPosArray[active2Pattern.patterns[LBS64(active2Bitmap)]];
			active2Bitmap &= active2Bitmap - 1;

			VCFMvs[0] = sleep3Poses[i] & 0xFF;
			VCFMvs[1] = (sleep3Poses[i]>>8) & 0xFF;
			VCFMvs[2] = (active2EmptyPoses>>8) & 0xFF;
			VCFMvs[3] = (active2EmptyPoses>>16) & 0xFF;

			//only remove duplicate, V[0-1] is already sorted from small to large, V[2-3] is already sorted from small to large
			VCFIndex = 4;
			for(int m = 2;m<VCFIndex;m++)
			{
				for(int n = 0;n<2;n++)
				{
					if(VCFMvs[m] == VCFMvs[n])
					{
						VCFMvs[m--] = VCFMvs[--VCFIndex];
						break;
					}
				}
			}

			if(VCFIndex == 3 && !isOnSameLine(VCFMvs, VCFIndex))
			{
				return true;
			}
		}
	}

	return false;
}

__forceinline int BitBoard::sortAndRemoveDuplicate(int8* mvs, int length, int64 bitmap[4]) const
{
	memset(bitmap,0,4*sizeof(int64));

	for (int i = 0;i<length;i++)
	{
		int8 pos = mvs[i];

		bitmap[pos>>6] |= shiftArray[pos & 63];
	}

	int index = 0;

	for (int i = 0;i<4;i++)
	{
		int64 map = bitmap[i];

		while(map)
		{
			mvs[index++] = i<<6 | LBS64(map);

			map &= map - 1;
		}
	}

	return index;
}

void BitBoard::generateMoveCountStatistic(int color) const
{
	memset(moveCountStatisticMap, 256, sizeof(int));

	/*for (int i = 0;i<currentPatternResult.countActive3[color];i++)
	{
	for(int j = 0;j<3;j++)
	{
	moveCountStatisticMap[currentPatternResult.emptyPosActive3[j][i][color]]+=3;
	}
	}
	for (int i = 0;i<currentPatternResult.countSleep3[color];i++)
	{
	for(int j = 0;j<2;j++)
	{
	moveCountStatisticMap[currentPatternResult.emptyPosSleep3[j][i][color]]+=3;
	}
	}
	for (int i = 0;i<currentPatternResult.countActive2[color];i++)
	{
	for(int j = 0;j<4;j++)
	{
	moveCountStatisticMap[currentPatternResult.emptyPosActive2[j][i][color]]+=2;
	}
	}
	for (int i = 0;i<currentPatternResult.countSleep2[color];i++)
	{
	for(int j = 0;j<3;j++)
	{
	moveCountStatisticMap[currentPatternResult.emptyPosSleep2[j][i][color]]+=2;
	}
	}

	for (int i = 0;i<currentPatternResult.countActive1[color];i++)
	{
	for(int j = 0;j<3;j++)
	{
	moveCountStatisticMap[currentPatternResult.emptyPosActive1[j][i][color]]++;
	}
	}*/
}

__forceinline int BitBoard::getIntersection(int8* intersection, int n1, const int8* arraySrc, int n2) const
{
	int i=0,j=0,k=0;

	while(i<n1&&j<n2)
	{
		if(intersection[i]==arraySrc[j])
		{
			intersection[k++]=intersection[i];
			i++;
			j++;
		}
		else if(intersection[i]>arraySrc[j])
		{
			j++;
		}
		else if(intersection[i]<arraySrc[j])
		{
			i++;
		}
	}

	return k;
}

bool BitBoard::isOnSameLine(const int8* mvs, int index) const
{
	for (int i = 0;i+2<index;i++)
	{
		if(getLineType(mvs[i],mvs[i+1]) != getLineType(mvs[i],mvs[i+2]))
		{
			return false;
		}
	}

	return true;
}

__forceinline int BitBoard::getLineType(int8 pos1, int8 pos2) const
{
	//x = b
	if(RANK_X(pos1) == RANK_X(pos2))
	{
		return (1 << 16) | RANK_X(pos1);
	}

	//y = b
	if(RANK_Y(pos1) == RANK_Y(pos2))
	{
		return (2 << 16) | RANK_Y(pos1);
	}

	//x+y = b
	if((RANK_X(pos1) + RANK_Y(pos1)) == (RANK_X(pos2) + RANK_Y(pos2)))
	{
		return (3 << 16) | (RANK_X(pos1) + RANK_Y(pos1));
	}

	//y - x = b
	if((RANK_Y(pos1) - RANK_X(pos1)) == (RANK_Y(pos2) - RANK_X(pos2)))
	{
		return (4 << 16) | (RANK_Y(pos1) - RANK_X(pos1) + 16);// add 16 to make it a positive value
	}

	return -1;
}

int8 BitBoard::getLooseMove(int color)
{
	const NormalPattern& oppositeSleep4Pattern = m_currentGlobalPattern->normalPattern[1-color][SLEEP4];
	const NormalPattern& oppositeSleep3Pattern = m_currentGlobalPattern->normalPattern[1-color][SLEEP3];
	const NormalPattern& oppositeActive3Pattern = m_currentGlobalPattern->normalPattern[1-color][ACTIVE3];
	const NormalPattern& oppositeActive2Pattern = m_currentGlobalPattern->normalPattern[1-color][ACTIVE2];

	if(oppositeSleep4Pattern.count > 0)
	{
		return EmptyPosArray[oppositeSleep4Pattern.patterns[LBS64(oppositeSleep4Pattern.bitmap)]];
	}
	else if(oppositeActive3Pattern.count > 0)
	{
		return (EmptyPosArray[oppositeActive3Pattern.patterns[LBS64(oppositeActive3Pattern.bitmap)]] >> 8) & 0xFF;
	}
	else if(oppositeSleep3Pattern.count > 0)
	{
		return (EmptyPosArray[oppositeSleep3Pattern.patterns[LBS64(oppositeSleep3Pattern.bitmap)]]) & 0xFF;
	}
	else if(oppositeActive2Pattern.count > 0)
	{
		return (EmptyPosArray[oppositeActive2Pattern.patterns[LBS64(oppositeActive2Pattern.bitmap)]] >> 8) & 0xFF;
	}
	else
	{
		return NO_MOVE;
	}
}

void BitBoard::print() const
{
#ifdef _DEBUG
	cout<<"============Board=============\n";
	for(int y = 0;y<BOARD_LENGTH;y++)
	{
		for(int x = 0;x<BOARD_LENGTH;x++)
		{
			if(m_chess[COORD_XY(x,y)] == COLOR_BLACK)
			{
				cout<<"●" ;
			}
			else if(m_chess[COORD_XY(x,y)] == COLOR_RED)
			{
				cout<<"○";
			}
			else
			{
				cout<<"　";
			}
		}
		cout<<"\n";
	}
#endif
}

void BitBoard::printMove(int8* mvs, int n) const
{
#ifdef _DEBUG
	cout<<"============"<<n<<" Moves===========\n";
	for(int y = 0;y<BOARD_LENGTH;y++)
	{
		for(int x = 0;x<BOARD_LENGTH;x++)
		{
			if(m_chess[COORD_XY(x,y)] == COLOR_BLACK)
			{
				cout<<"●";
			}
			else if(m_chess[COORD_XY(x,y)] == COLOR_RED)
			{
				cout<<"○";
			}
			else if(binary_search(mvs,mvs+n,COORD_XY(x,y)))
			{
				cout<<"▲";
			}
			else
			{
				cout<<"　";
			}
		}
		cout<<"\n";
	}
#endif
}
