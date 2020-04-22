/* A 'string set' is defined as a set of strings stored
 * in a hashtable that uses chaining to resolve collisions.
 *
 * Peter Jensen
 * January 29, 2019
 */

#ifndef STRING_SET_H
#define STRING_SET_H

#include <vector>
#include <string>

namespace cs3505
{
  class node;

  class string_set
    {
      friend class cs3505::node;
      // Default visibility is private.

      node** table;  // The hashtable, a pointer to a node pointer
                     //   (which will really be an array of node pointers)
      int capacity;  // The size of the hashtable array
      int size;      // The number of elements in the set

    public:
      string_set(int capacity = 100);        // Constructor.  Notice the default parameter value.
      string_set(const string_set & other);  // Copy constructor
      ~string_set();                         // Destructor

      void add      (const std::string & target);        // Not const - modifiesthe object
      void remove   (const std::string & target);        // Not const - modifies the object
      bool contains (const std::string & target) const;  // Const - does not change the object
      int  get_size () const;                            // Const - does not change object
      
      std::vector<std::string> get_elements() const;
      string_set & operator= (const string_set & rhs);   // Not const - modifies this object

    private:
      node* head;    // Variable pointing to the first item of the doubly-linked list
      node* tail;    // Variable pointing to the last item in the doubly-linked list 
      int  hash (const std::string & s) const;           // Const - does not change this object
      void clean ();                                     // Not const - modifies the object
      node * walk_chain (const std::string & target) const;// Function for iterating over chains

  };

}

#endif
