/******************************************************************************
*******************************************************************************
*******************************************************************************
// All files written by Andrew D. King, 2004, except some of the linked list
// classes, but I honestly can't remember where I got them.  Feel free to
// distribute this in its unaltered form.  The author takes no responsibility
// for loss of sanity resulting from poorly written or documented code.
//
// Compiled under Fedora Core 2 Linux using the GNU C++ compiler verision 2.96.
// Probably won't work with newer or older versions or under different
// operating systems.
//
// As of September, 2004, the author will be reachable at the Department of
// Computer Science, McGill University, Montreal, Quebec, Canada.  The email
// address andrew.king@utoronto.ca will probably be in service until at least
// mid-2005.  Feel free to send any comments.
*******************************************************************************
*******************************************************************************
******************************************************************************/

struct List
{ 
	int Vertex;
	List* Next;
	List(){
		Next = NULL;
		Vertex = -1;
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
	void AddAtHead();
	void Advance();
	void Rewind();
	void DeleteANode(ListPtr corpse);
	void DeleteANodeFast(ListPtr corpse, ListPtr corpsePrev);
	void PrintList();
	void RewindToHead();
	void InsertANodeAfter(ListPtr point);

	ListPtr Head, Tail, CurrentPtr;
	
};



struct DListDouble
{ 
	long Vertex;
	long To;
	double Cost;
	DListDouble* Next;
	DListDouble* Previous;
	DListDouble(){
		Next = NULL;
		Previous = NULL;
		Vertex = -1;
		To = -1;
	}
};
typedef DListDouble* DListDoublePtr;

class DLListDouble
{
 private:
 public:
	DLListDouble();
	~DLListDouble();
	void AddANode();
	void Advance();
	void Rewind();
	void DeleteANode(DListDoublePtr corpse);
	void PrintList();
	void PrintListBackwards();
	void RewindToHead();
	void MoveToBefore(DListDoublePtr source, DListDoublePtr dest);
	void MoveToAfter(DListDoublePtr source, DListDoublePtr dest);
	DListDoublePtr InsertANodeBefore(DListDoublePtr point);
	DListDoublePtr InsertANodeAfter(DListDoublePtr point);

	DListDoublePtr Head, Tail, CurrentPtr;
	
};


struct PList
{
	DListDoublePtr Ptr;
	PList* Next;
	PList(){
		Next=NULL;
		Ptr=NULL;
	}
};
typedef PList* PListPtr;

class PLList
{
 private:
 public:
	PLList();
	~PLList();
	PListPtr Previous(PListPtr index);
	void AddANode();
	void AddAtHead();
	void Advance();
	void Rewind();
	void DeleteANode(PListPtr corpse);
	void DeleteANodeFast(PListPtr corpse, PListPtr corpsePrev);
	void PrintList();
	void RewindToHead();
	void InsertANodeAfter(PListPtr point);

	PListPtr Head, Tail, CurrentPtr;
	
};

//linkedList4.cpp
/*
struct PListPlus
{
	DListDoublePtr Ptr;
	PListPlus* Next;
	int Count;
	PListPlus(){
		Next=NULL;
		Ptr=NULL;
		Count=-1;
	}
};
typedef PListPlus* PListPlusPtr;

class PLListPlus
{
 private:
 public:
	PLListPlus();
	~PLListPlus();
	PListPlusPtr PLListPlus::Previous(PListPlusPtr index);
	void PLListPlus::AddANode();
	void PLListPlus::AddAtHead();
	void PLListPlus::Advance();
	void PLListPlus::Rewind();
	void PLListPlus::DeleteANode(PListPlusPtr corpse);
	void PLListPlus::DeleteANodeFast(PListPlusPtr corpse, PListPlusPtr corpsePrev);
	void PLListPlus::PrintList();
	void PLListPlus::RewindToHead();
	void PLListPlus::InsertANodeAfter(PListPlusPtr point);

	PListPlusPtr Head, Tail, CurrentPtr;
	
};
*/

