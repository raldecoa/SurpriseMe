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
PLListPlus:: PLListPlus()
{ 
	Head = new PListPlus;
   Tail = Head;
   CurrentPtr = Head;
}


/* destructor ****************************************************************/
PLListPlus:: ~PLListPlus()
{
	PListPlusPtr temp = Head;
   CurrentPtr = Head;
	
   while(CurrentPtr != NULL) {
		CurrentPtr = CurrentPtr->Next;
		delete temp;
		temp=CurrentPtr;
   }
}


/*****************************************************************************/
PListPlusPtr PLListPlus::Previous(PListPlusPtr index)
{
	PListPlusPtr temp=Head;
	if(index==Head) //special case, index IS the head :)
		{ return Head;
		}
	
	while(temp->Next != index)
		{ temp=temp->Next;
		}
	return temp;
}

/*****************************************************************************/
void PLListPlus::AddANode()
{
	if(Head == NULL){
		Head = new PListPlus;
		Tail = Head;
		CurrentPtr = Head;
	} else { 
		Tail->Next = new PListPlus;
		Tail=Tail->Next;
	}
}


/*****************************************************************************/
void PLListPlus::InsertANodeAfter(PListPlusPtr point)
{
	PListPlusPtr temp;
	if(point == Tail){
		AddANode();
	} else {
		temp = new PListPlus;
		temp->Next = point->Next;
		point->Next = temp;
	}
}


/*****************************************************************************/
void PLListPlus::AddAtHead()
{
	PListPlusPtr temp;
	if(Head == NULL){
		Head = new PListPlus;
		Tail = Head;
		CurrentPtr = Head;
	} else { 
		temp = Head;
		Head = new PListPlus;
		Head->Next = temp;
	}
}


/*****************************************************************************/
void PLListPlus::Advance()
{ 
	if(CurrentPtr->Next != NULL) {  
		CurrentPtr=CurrentPtr->Next;
	}
}

/*****************************************************************************/
void PLListPlus::Rewind()
{ 
	if(CurrentPtr != Head) {
		CurrentPtr=Previous(CurrentPtr);
   }
}

/*****************************************************************************/
void PLListPlus::RewindToHead()
{ 
	CurrentPtr = Head;
}



/*****************************************************************************/
void PLListPlus::PrintList()
{
	PListPlusPtr temp = Head;
	
	cout<<"~ ";
	
	while(temp != NULL)
		{
			cout<<temp->Ptr->Vertex<<","<<temp->Ptr->To<<","<<temp->Ptr->Cost<<" -> ";
			temp=temp->Next;
		}
	cout<<"~"<<endl;
}

/*****************************************************************************/
void PLListPlus::DeleteANode(PListPlusPtr corpse)
{ 
	PListPlusPtr temp;
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
void PLListPlus::DeleteANodeFast(PListPlusPtr corpse, PListPlusPtr corpsePrev)
{ 
	PListPlusPtr temp;
	if(SANITY_CHECK){
		if(corpsePrev->Next != corpse)
			printf("SANITY CHECK: Problem in PLListPlus::DeleteANodeFast\n");
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
