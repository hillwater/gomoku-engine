#ifndef _NEXT_MOVE_
#define _NEXT_MODE_

#include "const.h"
#include "BitBoard.h"

class NextMove
{
public:
	NextMove(BitBoard* board, int* nHistoryTable, int color, int8 hashMove, int8 killMove1, int8 killMove2)
	{
		this->board = board;
		this->nHistoryTable = nHistoryTable;
		m_status = PHASE_GEN_NEED_EXTEND_BITMAP;
		m_hashMove = hashMove;
		m_killmove1 = killMove1;
		m_killmove2 = killMove2;
		m_color = color;
		maxHistoryValue = 0;
	}
	~NextMove(void);

	int8 next(bool& isNeedExtend);

	inline int getMaxHistoryValue() const
	{
		return maxHistoryValue;
	}

	inline void addKillMove(int8 mv)
	{
		if(m_killmove1 == NO_MOVE)
		{
			m_killmove1 = mv;
		}
		else if(m_killmove2 == NO_MOVE)
		{
			m_killmove2 = mv;
		}
		else
		{
			m_killmove1 = m_killmove2;
			m_killmove2 = mv;
		}
	}
private:
	int m_color;
	int m_status;
	int8 m_hashMove;
	int8 m_killmove1;
	int8 m_killmove2;
	int* nHistoryTable;
	BitBoard* board;
	int8 mvs[MAX_GEN_MOVES*3];
	int nGenMoves;
	int restIndex;
	int maxHistoryValue;

	int64 needExtendBitmap[4];

	void sort(int8* data, int len);
};

#endif // _NEXT_MOVE_
