#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>
#include "linkedList.h"

/* constructor ***************************************************************/
CycList:: CycList()
{ 
	Head = new CList;
   Tail = Head;
	Tail->Next = Head;
   CurrentPtr = Head;
}


/* destructor ****************************************************************/
CycList:: ~CycList()
{
/*
	PrintList();
	printf(".");

   while(Head != Tail) {
		printf(".");

		DeleteANode(Head);

   }
*/
}


/*****************************************************************************/
CListPtr CycList::Previous(long index)
{ 
	CListPtr temp=Head;
	for(long count=0;count<index-1;count++)
		{temp=temp->Next;
		}
	return temp;
}

/*****************************************************************************/
CListPtr CycList::Previous(CListPtr index)
{
	CListPtr temp=Head;
	if(index==Head) //special case, index IS the head :)
		{ return Head;
		}
	
	while(temp->Next != index)
		{ temp=temp->Next;
		}
	return temp;
}

/*****************************************************************************/
void CycList::AddANode()
{
	if(Head == NULL){
		Head = new CList;
		Tail = Head;
		Tail->Next = Head;
		CurrentPtr = Head;
	} else { 
		Tail->Next = NULL;
		Tail->Next = new CList;
		Tail = Tail->Next;
		Tail->Next = Head;
	}
}


/*****************************************************************************/
void CycList::InsertANodeAfter(CListPtr point)
{
	CListPtr temp;
	if(point == Tail){
		AddANode();
	} else {
		temp = new CList;
		temp->Next = point->Next;
		point->Next = temp;
	}
}


/*****************************************************************************/
void CycList::Advance()
{ 
	if(CurrentPtr->Next != NULL) {  
		CurrentPtr=CurrentPtr->Next;
		 }
}

/*****************************************************************************/
void CycList::Rewind()
{ 
	if(CurrentPtr != Head) {
		CurrentPtr=Previous(CurrentPtr);
   }
}

/*****************************************************************************/
void CycList::RewindToHead()
{ 
	CurrentPtr = Head;
}



/*****************************************************************************/
void CycList::PrintList()
{
	CListPtr temp = Head;
	
	cout<<"~ ";
	
	while(temp != Tail)
		{
			cout<<temp->Vertex<<", "<<temp->To<<" -> ";
			temp=temp->Next;
		}
	cout<<temp->Vertex<<", "<<temp->To<<" -> ";
	
	cout<<"~"<<endl;
}

/*****************************************************************************/
void CycList::DeleteANode(CListPtr corpse)
{ 
	CListPtr temp;
	//	if(Head==Tail) printf("DELETING ON EMPTY LIST\n");
	if(corpse == Head) //case 1 corpse = Head
		{
			temp=Head;
			Head=Head->Next;
			Tail->Next = Head;
			delete temp;
		}
	else if(corpse == Tail) //case 2 corpse is at the end
		{ 
			temp=Previous(corpse);
			temp->Next=NULL;
			delete corpse;
			Tail=temp;
			Tail->Next = Head;
		}
	else //case 3 corpse is in middle somewhere
		{
			temp=Previous(corpse);
			temp->Next=corpse->Next;
			delete corpse;
		}
	
	CurrentPtr=Head; //Reset the class tempptr
}
