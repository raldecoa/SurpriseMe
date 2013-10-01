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
PLList:: PLList()
{ 
	Head = new PList;
   Tail = Head;
   CurrentPtr = Head;
}


/* destructor ****************************************************************/
PLList:: ~PLList()
{
	PListPtr temp = Head;
   CurrentPtr = Head;
	
   while(CurrentPtr != NULL) {
		CurrentPtr = CurrentPtr->Next;
		delete temp;
		temp=CurrentPtr;
   }
}


/*****************************************************************************/
PListPtr PLList::Previous(PListPtr index)
{
	PListPtr temp=Head;
	if(index==Head) //special case, index IS the head :)
		{ return Head;
		}
	
	while(temp->Next != index)
		{ temp=temp->Next;
		}
	return temp;
}

/*****************************************************************************/
void PLList::AddANode()
{
	if(Head == NULL){
		Head = new PList;
		Tail = Head;
		CurrentPtr = Head;
	} else { 
		Tail->Next = new PList;
		Tail=Tail->Next;
	}
}


/*****************************************************************************/
void PLList::InsertANodeAfter(PListPtr point)
{
	PListPtr temp;
	if(point == Tail){
		AddANode();
	} else {
		temp = new PList;
		temp->Next = point->Next;
		point->Next = temp;
	}
}


/*****************************************************************************/
void PLList::AddAtHead()
{
	PListPtr temp;
	if(Head == NULL){
		Head = new PList;
		Tail = Head;
		CurrentPtr = Head;
	} else { 
		temp = Head;
		Head = new PList;
		Head->Next = temp;
	}
}


/*****************************************************************************/
void PLList::Advance()
{ 
	if(CurrentPtr->Next != NULL) {  
		CurrentPtr=CurrentPtr->Next;
	}
}

/*****************************************************************************/
void PLList::Rewind()
{ 
	if(CurrentPtr != Head) {
		CurrentPtr=Previous(CurrentPtr);
   }
}

/*****************************************************************************/
void PLList::RewindToHead()
{ 
	CurrentPtr = Head;
}



/*****************************************************************************/
void PLList::PrintList()
{
	PListPtr temp = Head;
	
	cout<<"~ ";
	
	while(temp != NULL)
		{
			cout<<temp->Ptr->Vertex<<","<<temp->Ptr->To<<","<<temp->Ptr->Cost<<" -> ";
			temp=temp->Next;
		}
	cout<<"~"<<endl;
}

/*****************************************************************************/
void PLList::DeleteANode(PListPtr corpse)
{ 
	PListPtr temp;
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
					temp->Next=NULL;
					delete corpse;
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
void PLList::DeleteANodeFast(PListPtr corpse, PListPtr corpsePrev)
{ 
	PListPtr temp;
	if(SANITY_CHECK){
		if(corpsePrev->Next != corpse)
			printf("SANITY CHECK: Problem in PLList::DeleteANodeFast\n");
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
					temp->Next=NULL;
					delete corpse;
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
