#ifndef _PNS_ENGINE_
#define _PNS_ENGINE_

#include "const.h"
#include "SearchEngine.h"
#include "MemPool.h"

class SearchEngine;
class PNSNode;

// for windows
// __declspec(align(8))
class ListItem
{
public:
	ListItem* next;
	ListItem* prev;
	int move;
	PNSNode* node;
} __attribute__ ((aligned (8)));

// for windows
// __declspec(align(8))
class List
{
public:
	ListItem* first;
	ListItem* tail;
	List(){init();}
	~List(){clear();}
	inline void init(){first = tail = NULL;}
	void clear();
	void add(PNSNode* node, int move);
	void addFront(PNSNode* node, int move);
	void removeFront();
	void removeTail();
	ListItem* find(PNSNode* node);
	void remove(ListItem* item);
	void remove(PNSNode* node);
	bool isEmpty();

private:
	static MemPool<ListItem> listItemPool;
} __attribute__ ((aligned (8)));

// for windows
// __declspec(align(8))
class PNSNode{
public:
	static int totalCount;
	int8 value;
	int8 depth;
	bool type;//true: OR NODE, false: AND NODE
	bool expanded;
	bool isInQueue;
	int proof;
	int disproof;
	int diffValue;
	PNSNode* minChild;
	List parents;
	List children;
	int nGenMoves;
	int8 mvs[VCT_MAX_CHILD_COUNT];

	void initial();
	void deleteChildren();
} __attribute__ ((aligned (8)));

class PNSEngine
{
public:
	PNSEngine(SearchEngine* searchEngine);
	~PNSEngine(void);

	bool PNS();
	static MemPool<PNSNode> pnsNodePool;

private:
	SearchEngine* searchEngine;
	List moveList;

	void setProofAndDisproofNumberForUnexpandedNode(PNSNode* node);
	void setProofAndDisproofNumberForExpandedNode(PNSNode* node);
	PNSNode* selectMostProvingNode(PNSNode* node);
	PNSNode* updateAncestors(PNSNode* node, PNSNode* root);
	void expandNode(PNSNode* node);
	void evaluate(PNSNode* node);
	bool resourceAvailable();
	void moveMostProvingNode(PNSNode* mostProving, PNSNode* root);
};

#endif // _PNS_ENGINE_
