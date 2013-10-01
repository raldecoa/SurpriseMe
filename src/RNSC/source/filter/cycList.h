typedef struct CList
{ 
	long Vertex;
	long To;
	CList* Next;
	CList(){
		Next = NULL;
		Vertex = -1;
		To = -1;
	}
};
typedef CList* CListPtr;

class CycList
{
 private:
 public:
	CycList();
	~CycList();
	CListPtr CycList::Previous(long index);
	CListPtr CycList::Previous(CListPtr index);
	void CycList::AddANode();
	void CycList::Advance();
	void CycList::Rewind();
	void CycList::DeleteANode(CListPtr corpse);
	void CycList::PrintList();
	void CycList::RewindToHead();
	void CycList::InsertANodeAfter(CListPtr point);

	CListPtr Head, Tail, CurrentPtr;
	
};
