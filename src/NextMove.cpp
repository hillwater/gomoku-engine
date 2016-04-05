#include "../includes/NextMove.h"

NextMove::~NextMove(void)
{
}

int8 NextMove::next(bool& isNeedExtend)
{
	isNeedExtend = false;

	switch(m_status)
	{
	case PHASE_GEN_NEED_EXTEND_BITMAP:
		m_status = PHASE_HASH_MOVE;
		board->generateNeedExtendBitmap(needExtendBitmap, m_color);
		/* no break */

	case PHASE_HASH_MOVE:
		m_status = PHASE_KILLER_MOVE;
		if (m_hashMove != NO_MOVE && board->isEmptyPos(m_hashMove))
		{
			isNeedExtend = needExtendBitmap[m_hashMove>>6] & shiftArray[m_hashMove&63];
			return m_hashMove;
		}
		/* no break */

	case PHASE_KILLER_MOVE:
		m_status = PHASE_KILLER_MOVE2;
		if (m_killmove1 != NO_MOVE && m_killmove1 != m_hashMove && board->isEmptyPos(m_killmove1))
		{
			isNeedExtend = needExtendBitmap[m_killmove1>>6] & shiftArray[m_killmove1&63];
			return m_killmove1;
		}
		/* no break */
		
	case PHASE_KILLER_MOVE2:
		m_status = PHASE_GEN_MOVE;
		if (m_killmove2 != NO_MOVE && m_killmove2 != m_hashMove && board->isEmptyPos(m_killmove2))
		{
			isNeedExtend = needExtendBitmap[m_killmove2>>6] & shiftArray[m_killmove2&63];
			return m_killmove2;
		}
		/* no break */

	case PHASE_GEN_MOVE:
		m_status = PHASE_REST_MOVE;
		nGenMoves = board->GenerateMoves(m_color,mvs);
		sort(mvs, nGenMoves);
		maxHistoryValue = nGenMoves > 0 ? nHistoryTable[mvs[0]] : 0;
#ifdef _DEBUG
		//afxDump<<"gen moves:"<<nGenMoves<<"\n";
#endif
		restIndex = 0;
		/* no break */
		
	case PHASE_REST_MOVE:
		while (restIndex < nGenMoves) {
			int8 mv = mvs[restIndex];
			restIndex ++;
			if (mv != m_hashMove && mv != m_killmove1 && mv != m_killmove2) {
				isNeedExtend = needExtendBitmap[mv>>6] & shiftArray[mv&63];
				return mv;
			}
		}
		/* no break */
	default:
		return NO_MOVE;
	}


	
}

/************************************************************************/
/* insert sort                                                          */
/************************************************************************/
void NextMove::sort(int8* mvs, int len)
{
	for(int i = 1; i < len; ++i) {  
		int temp = mvs[i];  
		int low = 0;  
		int high = i - 1; 
		while(low <= high) {  
			int mid = (low + high)>>1;  
			if(nHistoryTable[temp] > nHistoryTable[mvs[mid]])  
				high = mid - 1;  
			else  
				low = mid + 1;  
		}  

		for(int j = i; j > low; --j)  
			mvs[j] = mvs[j-1];  

		mvs[low] = temp;  
	}  
}
