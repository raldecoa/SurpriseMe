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

using namespace std;

/*
** Doubly-linked list. (DLListDouble, DListDoublePtr, DListDouble)
**
*/


/* constructor ***************************************************************/
DLListDouble:: DLListDouble()
{ 
	Head = new DListDouble;
   Tail = Head;
   CurrentPtr = Head;
}

/* destructor ****************************************************************/
DLListDouble:: ~DLListDouble()
{
	DListDoublePtr temp = Head;
   CurrentPtr = Head;
	
   while(CurrentPtr != NULL) {
		CurrentPtr = CurrentPtr->Next;
		delete temp;
		temp=CurrentPtr;
   }
}

/*****************************************************************************/
void DLListDouble::AddANode()
{
	if(Head == NULL){
		Head = new DListDouble;
		Tail = Head;
		CurrentPtr = Head;
	} else { 
		DListDoublePtr temp = new DListDouble;
		temp ->Previous = Tail;
		Tail->Next = temp;
		Tail = temp;
	}
}

/*****************************************************************************/
DListDoublePtr DLListDouble::InsertANodeBefore(DListDoublePtr point)
{
	DListDoublePtr temp, prev;
	temp = new DListDouble;
	if(point == Head){
		point->Previous = temp;
		temp->Next = point;
		Head = temp;
		return Head;
	} else {
		prev = point->Previous;
		prev->Next = temp;
		temp->Previous = prev;
		temp->Next = point;
		point->Previous = temp;
		
		return temp;
	}
}

/*****************************************************************************/
DListDoublePtr DLListDouble::InsertANodeAfter(DListDoublePtr point)
{
	DListDoublePtr temp, next;
	if(point == Tail){
		AddANode();
		return Tail;
	} else {
		temp = new DListDouble;
		temp->Next = point->Next;
		next = temp->Next;
		point->Next = temp;
		temp->Previous = point;
		next->Previous = temp;
		return temp;
	}
}


/*****************************************************************************/
//must work if source = dest or source = dest->Next.
void DLListDouble::MoveToBefore(DListDoublePtr source, DListDoublePtr dest)
{

	if((dest == source) || (dest == source->Next))
		return;//the function does nothing. also covers case where head==tail.

	if((dest == Head) && (source == Tail)){
		source->Previous->Next = NULL;
		Tail = source->Previous;

		source->Next = dest;
		source->Previous = NULL;
		dest->Previous = source;
		Head = source;
		
	} else if(dest == Head){
		source->Previous->Next = source->Next;
		source->Next->Previous = source->Previous;
		
		source->Next = dest;
		source->Previous = NULL;
		dest->Previous = source;
		Head = source;

	} else if(source == Head){
		source->Next->Previous = NULL;
		Head = source->Next;

		source->Next = dest;
		dest->Previous->Next = source;
		source->Previous = dest->Previous;
		dest->Previous = source;

	} else if(source == Tail){
		source->Previous->Next = NULL;
		Tail = source->Previous;
		
		source->Next = dest;
		dest->Previous->Next = source;
		source->Previous = dest->Previous;
		dest->Previous = source;

	} else {//normal case.
		source->Previous->Next = source->Next;
		source->Next->Previous = source->Previous;
		
		source->Next = dest;
		dest->Previous->Next = source;
		source->Previous = dest->Previous;
		dest->Previous = source;

	}
}


/*****************************************************************************/
//must work if source = dest or source = dest->Previous.
void DLListDouble::MoveToAfter(DListDoublePtr source, DListDoublePtr dest)
{

	if((dest == source) || (dest == source->Previous))
		return;//the function does nothing. also covers case where head==tail.

	if((dest == Tail) && (source == Head)){
		source->Next->Previous = NULL;
		Head = source->Previous;

		source->Previous = dest;
		source->Next = NULL;
		dest->Next = source;
		Tail = source;

	} else if(dest == Tail){
		source->Previous->Next = source->Next;
		source->Next->Previous = source->Previous;
		
		source->Previous = dest;
		source->Next = NULL;
		dest->Next = source;
		Tail = source;

	} else if(source == Head){
		source->Next->Previous = NULL;
		Head = source->Next;

		source->Previous = dest;
		dest->Next->Previous = source;
		source->Next = dest->Next;
		dest->Next = source;

	} else if(source == Tail){
		source->Previous->Next = NULL;
		Tail = source->Previous;
		
		source->Previous = dest;
		dest->Next->Previous = source;
		source->Next = dest->Next;
		dest->Next = source;

	} else {//normal case.
		source->Previous->Next = source->Next;
		source->Next->Previous = source->Previous;
		
		source->Previous = dest;
		dest->Next->Previous = source;
		source->Next = dest->Next;
		dest->Next = source;

	}
}

/*****************************************************************************/
void DLListDouble::Advance()
{ 
	if(CurrentPtr->Next != NULL) {  
		CurrentPtr=CurrentPtr->Next;
	}
}

/*****************************************************************************/
void DLListDouble::Rewind()
{ 
	if(CurrentPtr != Head) {
		CurrentPtr=CurrentPtr->Previous;
   }
}

/*****************************************************************************/
void DLListDouble::RewindToHead()
{ 
	CurrentPtr = Head;
}



/*****************************************************************************/
void DLListDouble::PrintList()
{
	DListDoublePtr temp = Head;
	
	cout<<"~ ";
	
	while(temp != NULL)
		{
			cout<<temp->Vertex<<","<<temp->To<<","<<temp->Cost<<" -> ";
			temp=temp->Next;
		}
	cout<<"~"<<endl;
}

/*****************************************************************************/
void DLListDouble::PrintListBackwards()
{
	DListDoublePtr temp = Tail;
	
	cout<<"~ ";
	
	while(temp != NULL)
		{
			cout<<temp->Vertex<<","<<temp->To<<","<<temp->Cost<<" <- ";
			temp=temp->Previous;
		}
	cout<<"~"<<endl;
}

/*****************************************************************************/
void DLListDouble::DeleteANode(DListDoublePtr corpse)
{ 
	DListDoublePtr temp;
	// 	if(Head==Tail) printf("DELETING ON EMPTY LIST\n");
	if(corpse == Head) //case 1 corpse = Head
		{
			
			if(Head != NULL){
				temp=Head;
				Head=Head->Next;
				delete temp;
				Head->Previous = NULL;
			}
		}
	else if(corpse == Tail) //case 2 corpse is at the end
		{ 
			if(corpse != NULL){
				temp=corpse->Previous;
				temp->Next=NULL;
				delete corpse;
				Tail=temp;
			}
		}
	else //case 3 corpse is in middle somewhere
		{
			temp=corpse->Previous;
			temp->Next=corpse->Next;
			temp->Next->Previous = temp;
			delete corpse;
		}
	CurrentPtr=Head; //Reset the class tempptr
	
}
