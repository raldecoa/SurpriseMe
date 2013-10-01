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


#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "linkedList.h"
#include "definitions.h"

using namespace std;


/* constructor ***************************************************************/
SLList:: SLList()
{ 
	Head = new List;
   Tail = Head;
   CurrentPtr = Head;
}


/* destructor ****************************************************************/
SLList:: ~SLList()
{
	ListPtr temp = Head;
   CurrentPtr = Head;
	
   while(CurrentPtr != NULL) {
		CurrentPtr = CurrentPtr->Next;
		delete temp;
		temp=CurrentPtr;
   }
}


/*****************************************************************************/
ListPtr SLList::Previous(long index)
{ 
	ListPtr temp=Head;
	for(long count=0;count<index-1;count++)
		{temp=temp->Next;
		}
	return temp;
}

/*****************************************************************************/
ListPtr SLList::Previous(ListPtr index)
{
	ListPtr temp=Head;
	if(index==Head) //special case, index IS the head :)
		{ return Head;
		}
	
	while(temp->Next != index)
		{ temp=temp->Next;
		}
	return temp;
}

/*****************************************************************************/
void SLList::AddANode()
{
	if(Head == NULL){
		Head = new List;
		Tail = Head;
		CurrentPtr = Head;
	} else { 
		Tail->Next = new List;
		Tail=Tail->Next;
	}
}


/*****************************************************************************/
void SLList::InsertANodeAfter(ListPtr point)
{
	ListPtr temp;
	if(point == Tail){
		AddANode();
	} else {
		temp = new List;
		temp->Next = point->Next;
		point->Next = temp;
	}
}


/*****************************************************************************/
void SLList::AddAtHead()
{
	ListPtr temp;
	if(Head == NULL){
		Head = new List;
		Tail = Head;
		CurrentPtr = Head;
	} else { 
		temp = Head;
		Head = new List;
		Head->Next = temp;
	}
}


/*****************************************************************************/
void SLList::Advance()
{ 
	if(CurrentPtr->Next != NULL) {  
		CurrentPtr=CurrentPtr->Next;
	}
}

/*****************************************************************************/
void SLList::Rewind()
{ 
	if(CurrentPtr != Head) {
		CurrentPtr=Previous(CurrentPtr);
   }
}

/*****************************************************************************/
void SLList::RewindToHead()
{ 
	CurrentPtr = Head;
}



/*****************************************************************************/
void SLList::PrintList()
{
	ListPtr temp = Head;
	
	cout<<"~ ";
	
	while(temp != NULL)
		{
			cout<<temp->Vertex<<" -> ";
			temp=temp->Next;
		}
	cout<<"~"<<endl;
}

/*****************************************************************************/
void SLList::DeleteANode(ListPtr corpse)
{ 
	ListPtr temp;
	// 	if(Head==Tail) printf("DELETING ON EMPTY LIST\n");
	if(corpse){
		if(corpse == Head) //case 1 corpse = Head
			{
				
				if(Head != NULL){
					temp=Head;
					Head=Head->Next;
					delete temp;
				}
			}
		else if(corpse == Tail) //case 2 corpse is at the end
			{ 
				if(corpse != NULL){
					temp=Previous(corpse);
					delete corpse;
					temp->Next=NULL;
					Tail=temp;
				}
			}
		else //case 3 corpse is in middle somewhere
			{
				temp=Previous(corpse);
				temp->Next=corpse->Next;
				delete corpse;
			}
		CurrentPtr=Head; //Reset the class tempptr
	}
}


/*****************************************************************************/
void SLList::DeleteANodeFast(ListPtr corpse, ListPtr corpsePrev)
{ 
	ListPtr temp;
	if(SANITY_CHECK){
		if(corpsePrev->Next != corpse)
			printf("SANITY CHECK: Problem in SLList::DeleteANodeFast\n");
	}

	// 	if(Head==Tail) printf("DELETING ON EMPTY LIST\n");
	if(corpse){
		if(corpse == Head) //case 1 corpse = Head
			{
				
				if(Head != NULL){
					temp=Head;
					Head=Head->Next;
					delete temp;
				}
			}
		else if(corpse == Tail) //case 2 corpse is at the end
			{ 
				if(corpse != NULL){
					temp=corpsePrev;
					delete corpse;
					temp->Next=NULL;
					Tail=temp;
				}
			}
		else //case 3 corpse is in middle somewhere
			{
				temp=corpsePrev;
				temp->Next=corpse->Next;
				delete corpse;
			}
		CurrentPtr=Head; //Reset the class tempptr
	}
}
