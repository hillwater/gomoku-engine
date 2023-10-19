#include "../includes/SearchEngine.h"
#include "../includes/PNSEngine.h"
#include <math.h>
#include <time.h>
#include <sys/timeb.h>
#include <fstream>
#include <iostream>

using namespace std;

//#ifndef _DEBUG
//	ofstream cout("output.log");
//#endif

#ifdef _DEBUG
	int64 totalNodes = 0;
	int64 skipNodes = 0;
	int64 reductNodes = 0;
	int64 reductFailedNodes = 0;
	int64 pvsTriedNodes = 0;
	int64 pvsFailedNodes = 0;
#endif

SearchEngine::SearchEngine(void)
{
	this->pnsEngine = new PNSEngine(this);
}


SearchEngine::~SearchEngine(void)
{
	delete this->pnsEngine;
}

SearchEngine* SearchEngine::searchEngine = new SearchEngine();

SearchEngine* SearchEngine::getInstance() {
	return searchEngine;
}

int32 SearchEngine::search(int32 level,
			vector<int32> posList,
			bool useMultiCore,
			bool useMultiMachine,
			int32 machineCount,
			int32 type)
{
	cout<<"start to do AI search :"<<endl;
	cout<<"	Level : "<<level<<endl;
	cout<<"	Type : "<<type<<endl;
	cout<<"	Position List Count("<<posList.size()<<") [";
	for(int i = 0;i< posList.size();i++) {
		cout<<posList[i]<<",";
	}
	cout<<"]"<<endl;
	cout<<"	Use Multiple Core Speed Up : "<<useMultiCore<<endl;
	cout<<"	Use Multiple Machine Speed Up : "<<useMultiMachine<<", Machine Count :"<<machineCount<<endl;


	if(posList.size() == 0) {
		int result = COORD_XY(7,7);

		cout<<"finish to do AI search, result:("<<RANK_X(result)<<","<<RANK_Y(result)<<")"<<endl;

		return result;
	}

	initial();

	for(int i = 0;i< posList.size();i++) {
		addPiece(posList[i]);
	}

	int result = SearchMain(level);

	cout<<"finish to do AI search, result:("<<RANK_X(result)<<","<<RANK_Y(result)<<")"<<endl;

	return result;
}

void SearchEngine::initial()
{
	board.clearBoard();
	this->curColor = COLOR_BLACK;
	zobrist.initial();
}

void SearchEngine::addPiece(int mv)
{
	board.addPiece(mv,this->curColor);
	zobrist.addPiece(mv,this->curColor);
	ChangeSide();
}

void SearchEngine::removePiece(int mv)
{
	ChangeSide();
	zobrist.removePiece(mv,this->curColor);
	board.removePiece(mv,this->curColor);
}

int SearchEngine::SearchMain(int level)
{
	mvResult = NO_MOVE;

	struct timeb startTime , endTime;

	ftime(&startTime);

	nDistance = 0;
	memset(mvKillers,NO_MOVE,sizeof(mvKillers));

	//board.print();

	//if only one choice move, then do it
	int8 mvs[MAX_GEN_MOVES];
	int nGenMoves = board.GenerateMoves(this->curColor,mvs);
	board.printMove(mvs,nGenMoves);
	if(nGenMoves == 1)
	{
#ifdef _DEBUG
		cout<<"only one choice.\n";
#endif
		mvResult = mvs[0];
		return mvResult;
	}

	//search VCF
	transposition.initialVCFVCT();
	if(SearchVCF() == 1 && IN_BOARD(mvResult)){
#ifdef _DEBUG
		ftime(&endTime);

		long time = (endTime.time-startTime.time)*1000 + (endTime.millitm - startTime.millitm);

		cout<<"VCF existed!!!!\n";
		cout<<"Time(ms):"<<time<<"\n";
#endif
		return mvResult;
	}

	transposition.initialVCFVCT();
	if(pnsEngine->PNS() && IN_BOARD(mvResult))
	{
//#ifdef _DEBUG
		ftime(&endTime);

		long time = (endTime.time-startTime.time)*1000 + (endTime.millitm - startTime.millitm);

		cout<<"PNS VCT existed!!!!\n";
		cout<<"Time:"<<time<<"\n";
		cout<<"total node="<<PNSNode::totalCount<<"\n";
		cout<<""<<(time?PNSNode::totalCount/time:1000000)<<"K/s\n";
//#endif
		return mvResult;
	}

	memset(nHistoryTable, 0, MAX_MOVES*sizeof(int)); // clear history table
	transposition.initial();

	totalSearchNode = 0;

	// depth deep search + expected window search
	int alpha = -MATE_VALUE;
	int beta = MATE_VALUE;
	const int expectedWindow = 41;
	const int MAX_LEVEL = level;
	long levelNode = 0;
	long fifthLastLevelNode = 0;
	long lastLevelNode = 0;
	for (int i = 1; i <=MAX_LEVEL; i ++) {//LIMIT_DEPTH
		levelNode = totalSearchNode;
		int value = SearchFull(alpha, beta, i);
		levelNode = totalSearchNode - levelNode;

		cout<<"level="<<i<<",value="<<value<<"\n";

		if(i == MAX_LEVEL -4)
		{
			fifthLastLevelNode = levelNode;
		}else if(i == MAX_LEVEL)
		{
			lastLevelNode = levelNode;
			cout<<"Branch Factor = "<<sqrt(sqrt(fifthLastLevelNode?lastLevelNode*1.0/fifthLastLevelNode : 0))<<"\n";
		}

		// if win or loose, stop search
		if (value > WIN_VALUE || value < -WIN_VALUE) {
			break;
		}
		// if more than 1 second, stop search
		//if (clock() - t > 400 * CLOCKS_PER_SEC) {
		//	break;
		//}
	}

	/*cout<<"transaction use rate="<<transposition.getUseRate()<<"%%\n";
	cout<<"transaction conflict rate="<<transposition.getConflictRate()<<"%\n";*/

	if(!IN_BOARD(mvResult))//if no computer move, it means draw or already loose
	{
		mvResult = board.getLooseMove(this->curColor);
		cout<<"loose move: "<<mvResult<<endl;
	}

#ifdef _DEBUG
	cout<<"Move: "<<mvResult<<"\n";

	cout<<"totalNodes:"<<totalNodes<<"\n";
	cout<<"skipNodes:"<<skipNodes<<"\n";
	cout<<"reductNodes:"<<reductNodes<<"\n";
	cout<<"reductFailedNodes:"<<reductFailedNodes<<"\n";
	cout<<"pvsTriedNodes:"<<pvsTriedNodes<<"\n";
	cout<<"pvsFailedNodes:"<<pvsFailedNodes<<"\n";
#endif
	ftime(&endTime);

	long time = (endTime.time-startTime.time)*1000 + (endTime.millitm - startTime.millitm);

	cout<<"Time:"<<time<<"ms\n";
	cout<<"Nodes:"<<totalSearchNode<<"\n";
	cout<<"Speed:"<<(time?totalSearchNode/time:1000000)<<"K/s\n";

#ifndef _DEBUG
	cout.flush();
#endif

	return mvResult;
}

int SearchEngine::SearchVCF()
{
	return attack(true);
}

/**
 * Deprecated
 */
int SearchEngine::SearchVCT()
{
	transposition.initialVCFVCT();
	return attack(false);
}

int SearchEngine::attack(bool isVCF)
{
	int hashValue = transposition.ProbeVCFVCTHash(this->zobrist.currentZobr());
	if(hashValue != 0){
		return hashValue;
	}

	int vl = board.Evaluate(this->curColor);

	int result = -1;

	if(vl >= WIN_VALUE){
		result = 1;
	}else if(vl > -WIN_VALUE){
		int8 mvs[MAX_GEN_MOVES];
		int nGenMoves = 0;

		if(!isVCF && nDistance >= LIMIT_DEPTH){
			return -1;
		}

		if(isVCF){
			nGenMoves= board.getVCFAttackMoves(this->curColor,mvs);
		}else{
			nGenMoves = board.getVCTAttackMoves(this->curColor,mvs);
		}

		for (int i = 0;i<nGenMoves;i++)
		{
			doMove(mvs[i]);
			int value = defend(isVCF);
			undoMove(mvs[i]);

			if(value == 1)
			{
				if(nDistance == 0){
					mvResult = mvs[i];
				}
				result = 1;
				break;
			}
		}
	}

	transposition.RecordVCFVCTHash(this->zobrist.currentZobr(),nDistance,result);

	return result;
}

int SearchEngine::defend(bool isVCF)
{
	int hashValue = transposition.ProbeVCFVCTHash(this->zobrist.currentZobr());
	if(hashValue != 0){
		return hashValue;
	}

	int vl = board.Evaluate(this->curColor);
	int result = -1;

	if(vl<-WIN_VALUE){
		result = 1;
	}else if(vl < WIN_VALUE){
		int8 mvs[MAX_GEN_MOVES];
		int nGenMoves = 0;

		if(!isVCF && nDistance >= LIMIT_DEPTH){
			return -1;
		}

		if(isVCF){
			nGenMoves = board.getVCFDefendMoves(this->curColor,mvs);
		}else{
			nGenMoves = board.getVCTDefendMoves(this->curColor,mvs);
		}

		result = 1;
		for (int i =0;i<nGenMoves;i++)
		{
			doMove(mvs[i]);
			int value = attack(isVCF);
			undoMove(mvs[i]);

			if(value == -1){
				result= -1;
				break;
			}
		}
	}

	transposition.RecordVCFVCTHash(this->zobrist.currentZobr(),nDistance,result);

	return result;
}

int SearchEngine::SearchFull(int vlAlpha, int vlBeta, int nDepth, bool bNoNull)
{
	totalSearchNode++;

	int vlBest = -MATE_VALUE -100; // the search result value, if not changed, it means no move existed
	int mvBest = NO_MOVE;           // the best move, it will be store into transposition
	int nHashFlag = HASH_ALPHA; //node type, alpha, beta, pv, it used to store in transposition table
	int8 mvHash = NO_MOVE;

	if(nDistance>0)
	{
		if(nDepth <= 0){
			return board.Evaluate(this->curColor);
			//return SearchQuiesc(vlAlpha,vlBeta);
		}

		if(nDistance == LIMIT_DEPTH)
		{
			return board.Evaluate(this->curColor);
		}

		//mate check
		int mateEvaluate = board.mateEvaluate(this->curColor);
		if(mateEvaluate != 0)
		{
			return mateEvaluate;
		}


		// 1. lookup transposition, if already searched, directly return the value
		static int total = 0, hit = 0;
		total++;
		int vl = transposition.ProbeHash(zobrist.currentZobr(),nDistance,vlAlpha, vlBeta, nDepth, mvHash);
		if (vl > -MATE_VALUE-100) {
			hit++;
			//cout<<total<<","<<hit<<", rate="<<hit*100/total<<"\n";
			return vl;
		}

		// 2. do null move
		if (!bNoNull && NullOkay() && !board.hasFive()) {
			NullMove();
			int value = -SearchFull(-vlBeta, 1 - vlBeta, nDepth - NULL_DEPTH - 1, true);
			UndoNullMove();
			if (value >= vlBeta) {
				//cout<<"Null Move!!!"<<"beta="<<vlBeta<<",value="<<value<<",depth="<<nDepth<<"\n";
				return value;
			}
		}
	}

	//continue if no five chess
	if(!board.hasFive()){
		int8 move = NO_MOVE;
		bool isNeedExtend = false;
		NextMove m_next(&board,nHistoryTable,this->curColor,mvHash,mvKillers[nDistance][0],mvKillers[nDistance][1]);

		int searchedMoveCnt = 0;
		int totalMoveCnt = 0;
		while((move = m_next.next(isNeedExtend)) != NO_MOVE)
		{
			totalMoveCnt++;

			int oldSimpleEval = board.getSimpleEvaluateValue(this->curColor);
			doMove(move);

			int nNewDepth = isNeedExtend ? nDepth : nDepth - 1;

			int value;
			// PVS + LMR + Futility Pruning
			if (vlBest == -MATE_VALUE - 100) {
				value = -SearchFull(-vlBeta, -vlAlpha, nNewDepth);
			} else {
				int simpleEvalValue = board.getSimpleEvaluateValue(1-this->curColor);//let opposite evaluate
				int diffValue = simpleEvalValue - oldSimpleEval;
#ifdef _DEBUG
				//cout<<"diff evaluate value:"<<diffValue<<","<<PruningMargin[nDepth] + simpleEvalValue - vlAlpha<<"\n";
				totalNodes++;
#endif


				if(searchedMoveCnt  >= 4 && !isNeedExtend && nDepth < PruningDepth && diffValue < 20 && PruningMargin[nDepth] + simpleEvalValue <= vlAlpha)
				{
					undoMove(move);
#ifdef _DEBUG
					skipNodes++;
#endif
					continue;
				}

				int diffValueMargin = isNeedExtend? 10 : (board.getTotalChessCount()<=5 ? 15 : 20);
				if(searchedMoveCnt > 1 && nDepth > ReductionLimit && diffValue < diffValueMargin)
				{
					// Search this move with reduced depth:
					int extensition = board.getTotalChessCount()<=8 || searchedMoveCnt < 15 || diffValue > 13  || isNeedExtend || nDepth < 3 ? 1 : 2;

					value = -SearchFull(-vlAlpha-1, -vlAlpha, nNewDepth-extensition);

#ifdef _DEBUG
					reductNodes++;
					if(value > vlAlpha)
					{
						reductFailedNodes++;
					}
#endif
				}
				else
				{
					// Hack to ensure that full-depth search is done.
					value = vlAlpha+1;
				}

				if(value > vlAlpha){
#ifdef _DEBUG
					pvsTriedNodes++;
#endif
					value = -SearchFull(-vlAlpha-1, -vlAlpha, nNewDepth);
					if (value > vlAlpha && value < vlBeta) {
#ifdef _DEBUG
						pvsFailedNodes++;
#endif
						value = -SearchFull(-vlBeta, -vlAlpha, nNewDepth);
					}
				}
			}

			undoMove(move);
			searchedMoveCnt++;

			// 3. Alpha-Beta judge
			if (value > vlBest) {
				vlBest = value;        // update best value
				if (value >= vlBeta) { // if larger than beta value, it's enough good, can break
					nHashFlag = HASH_BETA;
					mvBest = move;  // used to store in history table
					break;            // Beta break
				}
				if (value > vlAlpha) { // it's a PV move
					nHashFlag = HASH_PV;
					mvBest = move;  // used to store in history table
					vlAlpha = value;     // decrease the Alpha-Beta boundary
				}
			}

		}
#ifdef _DEBUG
		if(totalMoveCnt>30){
			//cout<<"searched moves:"<<searchedMoveCnt<<",rate:"<<(totalMoveCnt-searchedMoveCnt)*100/totalMoveCnt<<"\n";
		}
#endif
	}



	// 4. if no move searched, return
	if (vlBest <= -MATE_VALUE -100) {
		return nDistance + vlBest;
	}

	// store best move to transposition
	transposition.RecordHash(zobrist.currentZobr(),nDistance,nHashFlag, vlBest, nDepth, mvBest);

	if (mvBest != NO_MOVE) {
		// if not Alpha move，store in history table
		nHistoryTable[mvBest] += nDepth * nDepth;
		int8* lpmvKillers = mvKillers[nDistance];
		if (lpmvKillers[0] != mvBest) {
			lpmvKillers[1] = lpmvKillers[0];
			lpmvKillers[0] = mvBest;
		}
		if (nDistance == 0) {
			// is root node, then the best move is the computer move
			mvResult = mvBest;
		}
	}
	return vlBest;
}

int SearchEngine::SearchQuiesc(int vlAlpha, int vlBeta)
{
	//int i, nGenMoves = 0;
	//int vl, vlBest;
	//int8 mvs[MAX_GEN_MOVES];

	//if (nDistance == LIMIT_DEPTH) {
	//	return board.Evaluate(this->curColor);
	//}

	//vlBest = -MATE_VALUE;

	//vl = board.Evaluate(this->curColor);
	//if (vl > vlBest) {
	//	vlBest = vl;
	//	if (vl >= vlBeta) {
	//		return vl;
	//	}
	//	if (vl > vlAlpha) {
	//		vlAlpha = vl;
	//	}
	//}

	////continue if no five chess
	//if(!board.hasFive()){
	//	nGenMoves = board.getQuiescMoves(this->curColor,mvs);
	//	sort(mvs, nGenMoves);


	//	for (i = 0; i < nGenMoves; i ++) {
	//		doMove(mvs[i]);
	//		vl = -SearchQuiesc(-vlBeta, -vlAlpha);
	//		undoMove(mvs[i]);

	//		if (vl > vlBest) {
	//			vlBest = vl;
	//			if (vl >= vlBeta) {
	//				return vl;
	//			}
	//			if (vl > vlAlpha) {
	//				vlAlpha = vl;
	//			}
	//		}
	//	}
	//}

	//return vlBest <= -MATE_VALUE ? nDistance + vlBest : vlBest;
	return 0;
}
