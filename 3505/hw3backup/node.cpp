/* This node class is used to build linked lists for the
 * string_set class.
 *
 * Peter Jensen
 * January 29, 2019
 */

#include "node.h"
#include "string_set.h"
// We're not in a namespace.  We are not in any class.  Symbols defined
//   here are globally available.  We need to qualify our function names
//   so that we are definining our cs3505::node class functions.
//
// Note that we could also use the namespace cs3505 { } block.  This would
//   eliminate one level of name qualification.  The 'using' statement will
//   not help in this situation.  
// 
// Keep it as shown here for node.cpp.  I show the other way in string_set.cpp.

/*******************************************************
 * node member function definitions
 ***************************************************** */

/** Constructor:  Creates a node containing
  *   an element.  It is initialized to
  *   not point to any other node.
  */
cs3505::node::node(const std::string & s, cs3505::string_set & ss)
  : next(NULL),  // This syntax is used to call member variable constructors (or initialize them).
    data(s),      // This calls the copy constructor - we are making a copy of the string.
    set(ss)
  
{
  
  if (set.size == 0) //If set is empty, the doubly-linked list needs a new head (and tail) node.
    {
      set.head = this;
    }

  node *placeholder = set.tail;       // Using a placeholder for when we change tail
  set.tail = this;
  
  if (set.get_size() >= 1)    // Change the pointers for previous nodes only if the nodes exist so far
    {
      set.tail -> back = placeholder; // Use a placeholder to store old tail
      placeholder -> fore = this; // Assign previous tail's next node to the new tail
    }
}

  
/** Destructor:  release any memory allocated
  *   for this object.
  */
cs3505::node::~node()
{
  if (back != NULL && fore != NULL) // There exist nodes in front of, and behind, this one
    {                               // in the doubly-linked list (not tail or head)
      back -> fore = fore;
      fore -> back = back;
    }
  else if (back != NULL)            // This is the tail of the doubly-linked list
    {
      set.tail = back;
      back -> fore = NULL;
    }
  else if (fore != NULL)            // This is the head of the doubly-linked list
    {
      set.head = fore;
      fore -> back = NULL;
    }

  back = NULL;
  fore = NULL;

  int index = set.hash(data);
  
  if (set.table[index] == this)	// Current node is the start of the chain here
    {
      set.table[index] = this -> next;         // So change it to the "next" node
      this -> next = NULL;
    }
  else
    {
      node* current = set.table[index];

      while (current != NULL && current -> next -> data != data)
	{
	  current = current -> next;
	}

      if (current -> next  != NULL)
	{
	  current -> next = current -> next -> next;
	  if (current -> next != NULL)
	    current -> next -> next = NULL;
	}
      
	current -> next = NULL;
    }
    
  // I'm not convinced that the recursive delete is the
  //   best approach.  I'll keep it (and you'll keep it too).

  if (this->next != NULL)
    delete this->next;

  // Invalidate the entry so that it is not accidentally used.

  this->next = NULL;      
}
