#ifndef _MEM_POOL_
#define _MEM_POOL_

#include <malloc.h>
// for Mac use
// #include <sys/malloc.h>
#include <assert.h>

template<class T>
class MemPool
{
public:
	MemPool(unsigned int nItemCount = EXPANSION_SIZE);
	~MemPool(void);

    T* Alloc();
    void Free(T* p);

protected:
    void ExpandFreeList(unsigned nItemCount = EXPANSION_SIZE);//allocate memory and push to the list

private:
	static const int EXPANSION_SIZE = 50000;
    MemPool<T>* m_pFreeList;
};


template<class T>
MemPool<T>::MemPool(unsigned int nItemCount)
{
	ExpandFreeList(nItemCount);
}

template<class T>
MemPool<T>::~MemPool(void)
{
	//free all memory in the list
	/*MemPool<T>* pNext = NULL;
	for(pNext = m_pFreeList; pNext != NULL; pNext = m_pFreeList)
	{
		m_pFreeList = m_pFreeList->m_pFreeList;
		delete [](char*)pNext;
	}*/
}

template<class T>
__forceinline T* MemPool<T>::Alloc()
{
	if(m_pFreeList == NULL)
	{
		ExpandFreeList();
	}

	//get free memory from head
	MemPool<T>* pHead = m_pFreeList;
	m_pFreeList = m_pFreeList->m_pFreeList;
	return static_cast<T*>(static_cast<void*>(pHead));
}

template<class T>
__forceinline void MemPool<T>::Free(T* p)
{
	//push the free memory back to list
	MemPool<T>* pHead = static_cast<MemPool<T>*>(static_cast<void*>(p));
	pHead->m_pFreeList = m_pFreeList;
	m_pFreeList = pHead;
}

//allocate memory and push to the list
template<class T>
void MemPool<T>::ExpandFreeList(unsigned nItemCount)
{
	assert(sizeof(T) > sizeof(MemPool<T>*));

	// for windows use
//	T* items = static_cast<T*>(static_cast<void*>(aligned_alloc(sizeof(T)*nItemCount,8)));

	// for linux use
	T* items = static_cast<T*>(static_cast<void*>(malloc(sizeof(T)*nItemCount)));

	MemPool<T>* pLastItem = static_cast<MemPool<T>*>(static_cast<void*>(&items[0]));
	m_pFreeList = pLastItem;
	for(unsigned int i=1; i<nItemCount; ++i)
	{
		pLastItem->m_pFreeList = static_cast<MemPool<T>*>(static_cast<void*>(&items[i]));
		pLastItem = pLastItem->m_pFreeList;
	}

	pLastItem->m_pFreeList = NULL;
}

#endif // _MEM_POOL_
