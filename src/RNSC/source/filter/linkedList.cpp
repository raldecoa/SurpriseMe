#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "linkedList.h"
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
			cout<<temp->Data<<" -> ";
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
