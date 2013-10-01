#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>
#include "linkedList.h"

/*
** Doubly-linked list. (DLList, DListPtr, DList)
**
*/


/* constructor ***************************************************************/
DLList:: DLList()
{ 
	Head = new DList;
   Tail = Head;
   CurrentPtr = Head;
}

/* destructor ****************************************************************/
DLList:: ~DLList()
{
	DListPtr temp = Head;
   CurrentPtr = Head;
	
   while(CurrentPtr != NULL) {
		CurrentPtr = CurrentPtr->Next;
		delete temp;
		temp=CurrentPtr;
   }
}

/*****************************************************************************/
void DLList::AddANode()
{
	if(Head == NULL){
		Head = new DList;
		Tail = Head;
		CurrentPtr = Head;
	} else { 
		DListPtr temp = new DList;
		temp ->Previous = Tail;
		Tail->Next = temp;
		Tail = temp;
	}
}

/*****************************************************************************/
DListPtr DLList::InsertANodeBefore(DListPtr point)
{
	DListPtr temp, prev;
	temp = new DList;
	if(point == Head){
		point->Previous = temp;
		temp->Next = point;
		Head = temp;
		return Head;
	} else {
		temp->Next = point;
		prev = point->Previous;
		temp->Previous = prev;
		point->Previous = temp;
		prev->Next = temp;
		return temp;
	}
}

/*****************************************************************************/
DListPtr DLList::InsertANodeAfter(DListPtr point)
{
	DListPtr temp, next;
	if(point == Tail){
		AddANode();
		return Tail;
	} else {
		temp = new DList;
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
void DLList::MoveToBefore(DListPtr source, DListPtr dest)
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
void DLList::MoveToAfter(DListPtr source, DListPtr dest)
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
void DLList::Advance()
{ 
	if(CurrentPtr->Next != NULL) {  
		CurrentPtr=CurrentPtr->Next;
	}
}

/*****************************************************************************/
void DLList::Rewind()
{ 
	if(CurrentPtr != Head) {
		CurrentPtr=CurrentPtr->Previous;
   }
}

/*****************************************************************************/
void DLList::RewindToHead()
{ 
	CurrentPtr = Head;
}



/*****************************************************************************/
void DLList::PrintList()
{
	DListPtr temp = Head;
	
	cout<<"~ ";
	
	while(temp != NULL)
		{
			cout<<temp->Vertex<<","<<temp->To<<","<<temp->Cost<<" -> ";
			temp=temp->Next;
		}
	cout<<"~"<<endl;
}

/*****************************************************************************/
void DLList::PrintListBackwards()
{
	DListPtr temp = Tail;
	
	cout<<"~ ";
	
	while(temp != NULL)
		{
			cout<<temp->Vertex<<","<<temp->To<<","<<temp->Cost<<" <- ";
			temp=temp->Previous;
		}
	cout<<"~"<<endl;
}

/*****************************************************************************/
void DLList::DeleteANode(DListPtr corpse)
{ 
	DListPtr temp;
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
