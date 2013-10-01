struct List
{ 
	long Data;
	List* Next;
	List(){
		Next = NULL;
		Data = 0;
	}
};
typedef List* ListPtr;

class SLList
{
 private:
 public:
	SLList();
	~SLList();
	ListPtr Previous(long index);
	ListPtr Previous(ListPtr index);
	void AddANode();
	void Advance();
	void Rewind();
	void DeleteANode(ListPtr corpse);
	void PrintList();
	void RewindToHead();
	void InsertANodeAfter(ListPtr point);

	ListPtr Head, Tail, CurrentPtr;
	
};
