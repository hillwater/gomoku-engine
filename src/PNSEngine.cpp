#include "../includes/PNSEngine.h"
#include <iostream>

using namespace std;

int childCount = 0;
int parentCount = 0;
int updateCount = 0;
int scanCount = 0;

MemPool<ListItem> List::listItemPool = MemPool<ListItem>(MEM_POOL_INIT_SIZE/6*2);
MemPool<PNSNode> PNSEngine::pnsNodePool = MemPool<PNSNode>(MEM_POOL_INIT_SIZE/6);

__forceinline void List::clear()
{
	ListItem* item = first;

	while(item != NULL)
	{
		ListItem* tmp = item;
		item = item->next;
		List::listItemPool.Free(tmp);
	}

	first = NULL;
}

__forceinline void List::add(PNSNode* node, int move)
{
	ListItem* item = List::listItemPool.Alloc();

	item->node = node;
	item->move = move;
	item->next = NULL;

	if(first == NULL)
	{
		first = item;
		tail = item;
		item->prev = NULL;
	}
	else
	{
		tail->next=item;
		item->prev=tail;
		tail=item;
	}
}

__forceinline void List::addFront(PNSNode* node, int move)
{
	ListItem* item = List::listItemPool.Alloc();

	item->node = node;
	item->move = move;
	item->prev = NULL;

	if(first == NULL)
	{
		first=item;
		tail=item;
		item->next = NULL;
	}
	else
	{
		first->prev=item;
		item->next=first;
		first=item;
	}
}

__forceinline void List::removeFront()
{
	if (first != NULL)
	{
		ListItem* tmp = first;
		first = first->next;
		if (first != NULL)
		{
			first->prev = NULL;
		} else {
			tail = NULL;
		}
		List::listItemPool.Free(tmp);
	}
}

__forceinline void List::removeTail()
{
	if (tail != NULL)
	{
		ListItem* tmp = tail;
		tail = tail->prev;
		if (tail != NULL)
		{
			tail->next = NULL;
		} else {
			first = NULL;
		}
		List::listItemPool.Free(tmp);
	}
}

__forceinline ListItem* List::find(PNSNode* node)
{
	if(node == NULL)
	{
		return NULL;
	}

	ListItem* item = first;

	while(item != NULL && item->node != node)
	{
		item = item->next;
	}

	return item;
}

__forceinline void List::remove(ListItem* item)
{
	if(item == NULL)
	{
		return;
	}

	if(item == first)
	{
		first = item->next;
	}

	if(item == tail)
	{
		tail = item->prev;
	}

	if(item->next != NULL)
	{
		item->next->prev = item->prev;
	}

	if(item->prev != NULL)
	{
		item->prev->next = item->next;
	}

	List::listItemPool.Free(item);
}

__forceinline void List::remove(PNSNode* node)
{
	remove(find(node));
}

__forceinline bool List::isEmpty()
{
	return first == NULL;
}

int PNSNode::totalCount = 0;

__forceinline void PNSNode::initial()
{
	totalCount++;
	value = UNKNOWN;
	type = OR_NODE;
	depth = 0;
	expanded = false;
	isInQueue = false;
	proof = 1;
	disproof = 1;
	diffValue = 0;
	minChild = NULL;
	parents.init();
	children.init();
	nGenMoves = 0;
}

void PNSNode::deleteChildren()
{
	ListItem* item = children.first;
	while(item != NULL)
	{
		PNSNode* child = item->node;
		item = item->next;
		child->parents.remove(this);

		if(child->parents.isEmpty())
		{
			child->deleteChildren();
			PNSEngine::pnsNodePool.Free(child);
		}
	}

	children.clear();
	parents.clear();
}

PNSEngine::PNSEngine(SearchEngine* searchEngine)
{
	this->searchEngine = searchEngine;
}


PNSEngine::~PNSEngine(void)
{
}

bool PNSEngine::PNS()
{
	childCount = 0;
	parentCount = 0;
	updateCount = 0;
	scanCount = 0;

	PNSNode::totalCount = 0;

	searchEngine->transposition.initial();
	moveList.init();

	PNSNode* root = PNSEngine::pnsNodePool.Alloc();
	root->initial();
	evaluate(root);
	setProofAndDisproofNumberForUnexpandedNode(root);

	PNSNode* current = root;
	PNSNode* mostProving = NULL;

	while(root->proof != 0 && root->disproof != 0 && resourceAvailable())
	{
		mostProving = selectMostProvingNode(current);
		moveMostProvingNode(mostProving, root);
		expandNode(mostProving);
		current = updateAncestors(mostProving, root);
	}
	//clear undoMove
	moveMostProvingNode(root, root);

	bool result = root->proof == 0;

	// get result move
	if(result)
	{
		ListItem* item = root->children.first;
		while(item != NULL)
		{
			PNSNode* child = item->node;

			if(child->proof == 0)
			{
				searchEngine->mvResult = item->move;
				break;
			}

			item = item->next;
		}
	}

#ifdef _DEBUG
	cout<<"total node="<<PNSNode::totalCount<<"\n";
	cout<<"child:"<<childCount<<",parent:"<<parentCount<<",times"<<(childCount?parentCount/childCount : 0)<<"\n";
	cout<<"update:"<<updateCount<<",scan:"<<scanCount<<",rate"<<(updateCount? scanCount*100/updateCount : 0)<<"\n";
#endif


	root->deleteChildren();
	PNSEngine::pnsNodePool.Free(root);
	return result;
}

__forceinline void PNSEngine::setProofAndDisproofNumberForUnexpandedNode(PNSNode* node)
{
	switch(node->value)
	{
	case DISPROVEN:
		node->proof = INFINITE_VALUE;
		node->disproof = 0;
		break;
	case PROVEN:
		node->proof = 0;
		node->disproof = INFINITE_VALUE;
		break;
	case UNKNOWN:
		int8 mvs[MAX_GEN_MOVES];
		int nGenMoves = 0;

		if(node->type == OR_NODE)
		{
			nGenMoves = searchEngine->board.getVCTAttackMoves(searchEngine->curColor,mvs);
			node->proof = 1;
			node->disproof = 1 + nGenMoves;
		}
		else
		{
			nGenMoves = searchEngine->board.getVCTDefendMoves(searchEngine->curColor,mvs);
			node->proof = 1 + nGenMoves;
			node->disproof = 1;
		}
		node->nGenMoves = min(nGenMoves,VCT_MAX_CHILD_COUNT);
		memcpy(node->mvs,mvs,node->nGenMoves);
		break;
	}
}

__forceinline void PNSEngine::setProofAndDisproofNumberForExpandedNode(PNSNode* node)
{
	updateCount++;

	if(node->type == AND_NODE)
	{
		node->proof += node->diffValue;
		node->diffValue = 0;

		if(node->proof >= INFINITE_VALUE)
		{
			node->proof = INFINITE_VALUE;
			node->disproof = 0;
		}
		else
		{
			if(node->minChild == NULL)
			{
				scanCount++;

				PNSNode* minNode = NULL;
				int minValue = INFINITE_VALUE + 1;

				ListItem* item = node->children.first;
				while(item != NULL)
				{
					PNSNode* child = item->node;
					item = item->next;

					if(child->disproof < minValue)
					{
						minValue = child->disproof;
						minNode = child;
					}
				}

				node->minChild = minNode;
			}

			// if no child, then minChild still is NULL
			node->disproof = node->minChild ? node->minChild->disproof : INFINITE_VALUE;
			if(node->disproof == 0)
			{
				node->proof = INFINITE_VALUE;
			}
		}
	}
	else
	{
		node->disproof += node->diffValue;
		node->diffValue = 0;

		if(node->disproof >= INFINITE_VALUE)
		{
			node->disproof = INFINITE_VALUE;
			node->proof = 0;
		}
		else
		{
			if(node->minChild == NULL)
			{
				scanCount++;

				PNSNode* minNode = NULL;
				int minValue = INFINITE_VALUE + 1;

				ListItem* item = node->children.first;
				while(item != NULL)
				{
					PNSNode* child = item->node;
					item = item->next;

					if(child->proof < minValue)
					{
						minValue = child->proof;
						minNode = child;
					}
				}

				node->minChild = minNode;
			}

			// if no child, then minChild still is NULL
			node->proof = node->minChild ? node->minChild->proof : INFINITE_VALUE;
			if(node->proof == 0)
			{
				node->disproof = INFINITE_VALUE;
			}
		}
	}
}

__forceinline PNSNode* PNSEngine::selectMostProvingNode(PNSNode* node)
{
	while(node->expanded)
	{
		if(node->minChild != NULL)
		{
			node = node->minChild;
		}
		else
		{
			break;
		}
	}

	return node;
}

PNSNode* PNSEngine::updateAncestors(PNSNode* node, PNSNode* root)
{
	PNSNode* queue[1024];
	int front = 0;
	int rear = 1;

	queue[0]=node;
	node->isInQueue = true;

	PNSNode* current = NULL;


	childCount++;

	while(front != rear)
	{
		parentCount++;

		current = queue[front];
		current->isInQueue = false;
		front = (front + 1) & 1023;

		int oldProof = current->proof;
		int oldDisproof = current->disproof;

		setProofAndDisproofNumberForExpandedNode(current);


		if((current->proof == oldProof && current->disproof == oldDisproof) || current == root)
		{
			continue;
		}


		ListItem* item = current->parents.first;
		while(item != NULL)
		{
			PNSNode* parent = item->node;
			item = item->next;

			if(parent->type == AND_NODE)
			{
				// proof number use add operate, so calculate the diff value
				parent->diffValue += current->proof - oldProof;

				// disproof number use min operate, so calculate the min child
				if(parent->minChild != NULL && current->disproof < parent->minChild->disproof)
				{
					// update min child
					parent->minChild = current;
				}
				else if(current->disproof > oldDisproof && current == parent->minChild)
				{
					// current node is the min child of parent, needs to recalculate min child
					parent->minChild = NULL;
				}
			}
			else
			{
				parent->diffValue += current->disproof - oldDisproof;


				// proof number use min operate, so calculate the min child
				if(parent->minChild != NULL && current->proof < parent->minChild->proof)
				{
					// update min child
					parent->minChild = current;
				}
				else if(current->proof > oldProof && current == parent->minChild)
				{
					// current node is the min child of parent, needs to recalculate min child
					parent->minChild = NULL;
				}
			}


			if(!parent->isInQueue)
			{
				queue[rear] = parent;
				parent->isInQueue = true;
				rear = (rear + 1) & 1023;
			}
		}
	}

	return root;
}

// find a shortest way move from current node to target node,
// but current implementation is not the best one, it doesn't find the shortest path,
// just choose a simple and fast way
void PNSEngine::moveMostProvingNode(PNSNode* mostProving, PNSNode* root)
{
	List doList;
	PNSNode* next = mostProving;
	while(next != root)
	{
		// here use last parent, it has a high possibility to have max common path with old path
		ListItem* item = next->parents.tail;

		doList.addFront(item->node,item->move);

		next = item->node;
	}

	ListItem* next1 = moveList.first;
	ListItem* next2 = doList.first;

	while(next1 != NULL && next2 != NULL && next1->move == next2->move){
		next1 = next1->next;
		next2 = next2->next;
	}

	if(next1 != NULL)
	{
		ListItem* lastRemainNode = next1->prev;
		while(moveList.tail != NULL && moveList.tail != lastRemainNode)
		{
			searchEngine->undoMove(moveList.tail->move);
			moveList.removeTail();
		}
	}

	while(next2 != NULL)
	{
		searchEngine->doMove(next2->move);
		moveList.add(next2->node,next2->move);

		next2 = next2->next;
	}
}

void PNSEngine::expandNode(PNSNode* node)
{
	node->expanded = true;

	int8* mvs = node->mvs;
	int nGenMoves = node->nGenMoves;

	if(node->type == OR_NODE)
	{
		node->diffValue = -node->disproof;
	}
	else
	{
		node->diffValue = -node->proof;
	}

	for(int i = 0; i < nGenMoves; i++)
	{
		searchEngine->doMove(mvs[i]);

		//check transposition
		PNSNode* child = (PNSNode*)searchEngine->transposition.ProbePNSHash(searchEngine->zobrist.currentZobr());

		const PNSNode* IGNORED_NODE = (PNSNode*)0x55aa;

		if(child != NULL)
		{
			searchEngine->undoMove(mvs[i]);

			if(child == IGNORED_NODE)
			{
				continue;
			}

			child->parents.add(node,mvs[i]);
			node->children.add(child, mvs[i]);
		}
		else
		{
			child = PNSEngine::pnsNodePool.Alloc();
			child->initial();
			child->type = !node->type;
			child->depth = node->depth + 1;

			evaluate(child);

			//can be ignored child, to save memory
			if((node->type == OR_NODE && child->value == DISPROVEN) || (node->type == AND_NODE && child->value == PROVEN))
			{
				//store in transposition
				searchEngine->transposition.RecordPNSHash(searchEngine->zobrist.currentZobr(),(void*)IGNORED_NODE, child->depth);
				searchEngine->undoMove(mvs[i]);
				PNSEngine::pnsNodePool.Free(child);
				continue;
			}


			child->parents.add(node,mvs[i]);
			node->children.add(child,mvs[i]);

			child->expanded = false;


			//store in transposition
			searchEngine->transposition.RecordPNSHash(searchEngine->zobrist.currentZobr(),(void*)child, child->depth);

			setProofAndDisproofNumberForUnexpandedNode(child);


			//must put in the end
			searchEngine->undoMove(mvs[i]);
		}

		if(node->type == OR_NODE)
		{
			node->diffValue += child->disproof;
		}
		else
		{
			node->diffValue += child->proof;
		}

		if((node->type == OR_NODE && child->proof == 0) || (node->type == AND_NODE && child->disproof == 0))
		{
			break;
		}
	}
}

void PNSEngine::evaluate(PNSNode* node)
{
	int value = searchEngine->board.Evaluate(searchEngine->curColor);
	if(value < -WIN_VALUE)
	{
		if(node->type == OR_NODE)
		{
			node->value = DISPROVEN;
		}
		else
		{
			node->value = PROVEN;
		}
	}
	else if(value > WIN_VALUE)
	{
		if(node->type == OR_NODE)
		{
			node->value = PROVEN;
		}
		else
		{
			node->value = DISPROVEN;
		}
	}
	else
	{
		node->value = UNKNOWN;
	}
}

__forceinline bool PNSEngine::resourceAvailable()
{
	return PNSNode::totalCount < MEM_POOL_INIT_SIZE;
}
