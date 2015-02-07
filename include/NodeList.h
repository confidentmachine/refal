#pragma once

#include <utility>

namespace Refal2 {

template<class T>
class CNodeList;

template<class T>
class CNode : public T {
	friend class CNodeList<T>;
	
public:
	CNode(): prev( 0 ), next( 0 ) {}
	explicit CNode( const T& value ): T( value ), prev( 0 ), next( 0 ) {}

	CNode* Next() { return next; }
	const CNode* Next() const { return next; }
	
	CNode* Prev() { return prev; }
	const CNode* Prev() const { return prev; }
	
private:
	CNode* prev;
	CNode* next;
};

template<class T>
class CNodeList {
public:
	typedef CNode<T> CNodeType;

	CNodeList(): first(0), last(0) {}
	CNodeList( CNodeType* first, CNodeType* last );
	
	~CNodeList() { Empty(); }
	
	void Assign( CNodeType* first, CNodeType* last );
	
	void Empty();
	bool IsEmpty() { return ( first == 0 ); }
	
	inline void Swap( CNodeList* swapWith );
	inline void Move( CNodeList* moveTo );
	
	inline CNodeType* DetachFirst();
	void RemoveFirst() { free( DetachFirst() ); }
	CNodeType* GetFirst() { return first; }
	const CNodeType* GetFirst() const { return first; }
	
	inline CNodeType* DetachLast();
	void RemoveLast() { free( DetachLast() ); }
	CNodeType* GetLast() { return last; }
	const CNodeType* GetLast() const { return last; }
	
	CNodeType* InsertBefore( CNodeType* nodeBefore, const T& value );
	
	CNodeType* InsertAfter( CNodeType* nodeAfter, const T& value );
	CNodeType* InsertAfter( CNodeType* nodeAfter, CNodeType* node );
	void InsertAfter( CNodeType* nodeAfter, CNodeType* fromNode,
		CNodeType* toNode );
	
	void Detach( CNodeType* node ) { Detach( node, node ); }
	void Detach( CNodeType* fromNode, CNodeType* toNode );
	
	void Remove( CNodeType* node) { Detach( node ); free( node ); }
	void Remove( CNodeType* fromNode, CNodeType* toNode );
	
	void Move( CNodeType* nodeAfter, CNodeType* node );
	void Move( CNodeType* nodeAfter, CNodeType* fromNode, CNodeType* toNode );
	
	void Copy( CNodeType* nodeAfter, CNodeType* node );
	void Copy( CNodeType* nodeAfter, CNodeType* fromNode, CNodeType* toNode );
	
	static void Duplicate( const CNodeType* fromNode, const CNodeType* toNode,
		CNodeType** fromNodeCopy, CNodeType** toNodeCopy );
	
	CNodeType* Append( const T& value );
	void Append( CNodeList* list );

private:
	CNodeList(const CNodeList&);
	CNodeList& operator=(const CNodeList&);

	static CNodeType* alloc( const T& value );
	static void free( CNodeType* node );
	
	CNodeType* first;
	CNodeType* last;
};

template<class T>
inline void CNodeList<T>::Swap( CNodeList* swapWith )
{
	std::swap( first, swapWith->first );
	std::swap( last, swapWith->last );
}

template<class T>
inline void CNodeList<T>::Move( CNodeList* moveTo )
{
	if( this != moveTo ) {
		moveTo->Empty();
		Swap( moveTo );
	}
}

template<class T>
inline typename CNodeList<T>::CNodeType* CNodeList<T>::DetachFirst()
{
	CNodeType* detachedNode = GetFirst();
	Detach( detachedNode );
	return detachedNode;
}

template<class T>
inline typename CNodeList<T>::CNodeType* CNodeList<T>::DetachLast()
{
	CNodeType* detachedNode = GetLast();
	Detach( detachedNode );
	return detachedNode;
}

template<class T>
CNodeList<T>::CNodeList( CNodeType* _first, CNodeType* _last ):
	first( _first ), last( _last )
{
	assert( _first != 0 && _last != 0 );
}

template<class T>
void CNodeList<T>::Assign( CNodeType* _first, CNodeType* _last )
{
	Empty();
	first = _first;
	last = _last;
}

template<class T>
void CNodeList<T>::Empty()
{
	if( first != 0 ) {
		Remove(first, last);
	}
	first = 0;
	last = 0;
}

template<class T>
typename CNodeList<T>::CNodeType* CNodeList<T>::InsertBefore(
	CNodeType* nodeBefore, const T& value )
{
	CNodeType* newNode = alloc( value );
	
	newNode->next = nodeBefore;
	newNode->prev = nodeBefore->prev;
	if( nodeBefore == first ) {
		first = newNode;
	} else {
		nodeBefore->prev->next = newNode;
	}
	nodeBefore->prev = newNode;
	
	return newNode;
}

template<class T>
typename CNodeList<T>::CNodeType* CNodeList<T>::InsertAfter(
	CNodeType* nodeAfter, const T& value )
{
	return InsertAfter( nodeAfter, alloc( value ) );
}

template<class T>
typename CNodeList<T>::CNodeType* CNodeList<T>::InsertAfter(
	CNodeType* nodeAfter, CNodeType* node )
{
	InsertAfter( nodeAfter, node, node ); return node;
}

template<class T>
void CNodeList<T>::InsertAfter( CNodeType* nodeAfter, CNodeType* fromNode,
	CNodeType* toNode )
{
	Detach( fromNode, toNode );

	toNode->next = nodeAfter->next;
	fromNode->prev = nodeAfter;
	if( nodeAfter == last ) {
		last = toNode;
	} else {
		nodeAfter->next->prev = toNode;
	}
	nodeAfter->next = fromNode;
}

template<class T>
void CNodeList<T>::Detach( CNodeType* fromNode, CNodeType* toNode )
{
	if( fromNode->prev != 0 ) {
		fromNode->prev->next = toNode->next;
	}
	if( toNode->next != 0 ) {
		toNode->next->prev = fromNode->prev;
	}
	if( fromNode == first ) {
		first = toNode->next;
	}
	if( toNode == last ) {
		last = fromNode->prev;
	}
	fromNode->prev = 0;
	toNode->next = 0;
}

template<class T>
void CNodeList<T>::Remove( CNodeType* fromNode, CNodeType* toNode )
{
	Detach( fromNode, toNode );
	
	while( fromNode != toNode ) {
		CNodeType* tmp = fromNode;
		fromNode = fromNode->next;
		free( tmp );
	}
	free( fromNode );
}

template<class T>
void CNodeList<T>::Move( CNodeType* nodeAfter, CNodeType* node )
{
	InsertAfter( nodeAfter, node );
}

template<class T>
void CNodeList<T>::Move( CNodeType* nodeAfter, CNodeType* fromNode,
	CNodeType* toNode )
{
	InsertAfter( nodeAfter, fromNode, toNode );
}

template<class T>
void CNodeList<T>::Copy( CNodeType* nodeAfter, CNodeType* node )
{
	Copy( nodeAfter, node, node );
}

template<class T>
void CNodeList<T>::Copy( CNodeType* nodeAfter, CNodeType* fromNode,
	CNodeType* toNode )
{
	CNodeType* fromNodeCopy = 0;
	CNodeType* toNodeCopy = 0;
	Duplicate( fromNode, toNode, &fromNodeCopy, &toNodeCopy );
	InsertAfter( nodeAfter, fromNodeCopy, toNodeCopy );
}

template<class T>
void CNodeList<T>::Duplicate( const CNodeType* fromNode,
	const CNodeType* toNode, CNodeType** fromNodeCopy, CNodeType** toNodeCopy )
{
	CNodeType* tmpFirst = alloc( *fromNode );
	CNodeType* tmpLast = tmpFirst;
	
	while( fromNode != toNode ) {
		fromNode = fromNode->next;
		
		tmpLast->next = alloc( *fromNode );
		tmpLast->next->prev = tmpLast;
		tmpLast = tmpLast->next;
	}
	
	*fromNodeCopy = tmpFirst;
	*toNodeCopy = tmpLast;
}

template<class T>
typename CNodeList<T>::CNodeType* CNodeList<T>::Append( const T& value )
{
	if( last != 0 ) {
		InsertAfter( last, value );
	} else {
		first = alloc( value );
		last = first;
	}
	return last;
}

template<class T>
void CNodeList<T>::Append( CNodeList* list )
{
	if( !list->IsEmpty() ) {
		CNodeType* first = list->GetFirst();
		CNodeType* last = list->GetLast();
		list->Detach( first, last );
		assert( list->IsEmpty() );
		if( IsEmpty() ) {
			Assign( first, last );
		} else {
			InsertAfter( GetLast(), first, last );
		}
	}
}

template<class T>
typename CNodeList<T>::CNodeType* CNodeList<T>::alloc( const T& value )
{
	return new CNodeType( value );
}

template<class T>
void CNodeList<T>::free( CNodeType* node )
{
	delete node;
}

} // end of namespace Refal2
