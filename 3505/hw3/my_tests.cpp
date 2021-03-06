/* Tester class for validating the cs3505::string_set class
*/

#include <iostream>
#include "node.h"
#include "string_set.h"
#include <vector>

using namespace std;

int main()
{
  cout << "Testing main function called" << endl;

   bool ok = true;

  {
    cout << "Test 00: testing contains" << endl;
    
    cs3505::string_set set(32);
    set.add("hello");

    //Time to test the contains function, now that we have a new node!

    if (!set.contains("hello") || set.get_size() != 1)
      ok = false;

    if (!ok)
      return 0;

    cout << "Test 01: testing contains" << endl;

    set.add("WORLD");
    set.add("Three");
    set.add("six");
    set.add("nine");
    set.add("The");
    set.add("Moose");
    set.add("Drank");
    set.add("wINE");

    if (!set.contains("WORLD"))
      {
	ok = false;
	cout << "WORLD" << endl;
      }

    if (!set.contains("Three"))
      {
	ok = false;
	cout << "Three" << endl;
      }
    
    if (!set.contains("six"))
      {
	ok = false;
	cout << "six" << endl;
      }
    
    if (!set.contains("nine"))
      {
	ok = false;
	cout << "nine" << endl;
      }
    
    if (!set.contains("The"))
      {
	ok = false;
	cout << "The" << endl;
      }
    
    if (!set.contains("Moose"))
      {
	ok = false;
	cout << "Moose" << endl;
      }
    
    if (!set.contains("Drank"))
      {
	ok = false;
	cout << "Drank" << endl;
      }
    
    if (!set.contains("wINE"))
      {
	ok = false;
	cout << "wINE" << endl;
      }
    
    if (set.get_size() != 9)
      {
	cout << "Wrong size" << endl;
	ok = false;
      }

    //////////////////////////////////////////////////////
    if (ok)
      {
	cout << "Test 02: remove function" << endl;

	set.remove("nine");
	set.remove("Drank");

	vector<string> contents = set.get_elements();
	cout << "Contents: ";
	for (int i = 0; i < contents.size(); i++)
	  cout << contents[i] << ", ";
	cout << endl;

	if (set.get_size() != 7)
	  ok = false;
      }

    //////////////////////////////////////////////////////
    if (ok)
      {
	cout << "Test 03: remove what's not there" << endl;

	set.remove("Pineapple");
	set.remove("");

	if (set.get_size() != 7)
	  ok = false;

	if (set.contains("Pineapple") || set.contains(""))
	  ok = false;
      }

    //////////////////////////////////////////////////////
    if (ok)
      {
	vector<string> expected;
	expected.push_back("First");
	expected.push_back("Second");
	expected.push_back("Third");
	expected.push_back("Fourth");
	expected.push_back("Fifth");
	
	cout << "Test 04: Testing get_elements() function" << endl;

	cs3505::string_set new_set(17);

	cout << "Adding strings \"One\" through \"Five\"" << endl;
	
	new_set.add("First");
	new_set.add("Second");
	new_set.add("Third");
	new_set.add("Fourth");
	new_set.add("Fifth");

	vector<string> contents(new_set.get_elements());

	cout << "contents: " << endl;
	    
        for (int i = 0; i < contents.size(); i++)
	  cout << contents[i] << ", ";

	cout << endl;

	for (int i = 0; i < contents.size(); i++)
	  {
	    if (contents[i] != expected[i])
	      {
		ok = false;
		break;
	      }
	  }

	cout << "Removing strings \"Second\" and \"Fourth\"" << endl;

	 new_set.remove("Second");
	 new_set.remove("Fourth");

	contents = new_set.get_elements();

	vector<string> expected2;
	expected2.push_back("First");
	expected2.push_back("Third");
	expected2.push_back("Fifth");

	cout << "Contents: " << endl;
	for (int i = 0; i < contents.size(); i++)
	  cout << contents[i] << ", ";

	cout << endl;
	
	for (int i = 0; i < contents.size(); i++)
	  {
	    if (contents[i] != expected2[i])
	      {
		ok = false;
		break;
	      }
	  }

	cout << "Adding string \"Sixth\", and removing \"First\"" << endl;

	new_set.add("Sixth");
	new_set.remove("First");

	contents = new_set.get_elements();

	vector<string> expected3;
	expected3.push_back("Third");
	expected3.push_back("Fifth");
	expected3.push_back("Sixth");

	cout << "Contents: " << endl;
	for (int i = 0; i < contents.size(); i++)
	  cout << contents[i] << ", ";

	cout << endl;
	
	for (int i = 0; i < contents.size(); i++)
	  {
	    if (contents[i] != expected3[i])
	      {
		ok = false;
		break;
	      }
	  }

	cout << "Removing both the head and tail - removing \"Third\" and \"Sixth\"" << endl;

	new_set.remove("Third");
	new_set.remove("Sixth");

	contents = new_set.get_elements();

	vector<string> expected4;
	expected4.push_back("Fifth");

	cout << "Contents: " << endl;
	for (int i = 0; i < contents.size(); i++)
	  cout << contents[i] << ", ";

	cout << endl;
	
	for (int i = 0; i < contents.size(); i++)
	  {
	    if (contents[i] != expected4[i])
	      {
		ok = false;
		break;
	      }
	  }
      }
     /*

    if (ok)
      {
	
	 cout << "Stress test: adding, 100000 words, then removing 99997 of them" << endl;
	
	 cs3505::string_set new_set(60);
	 int num_words = 100000;
	
	 for (int i = 0; i < num_words; i++)
	   {
	     new_set.add(std::to_string(i));
	   }

	 vector<string> result = new_set.get_elements();

	 for (int i = 0; i < result.size(); i++)
	    cout << result[i] << ", ";
	
	 for (int i = num_words; i > 2; i--)
	   {
	     new_set.remove(std::to_string(i));
	   }

	 result = new_set.get_elements();
	
         cout << "Stress test complete. " << endl;
	 cout << "resulting contents: ";
	 for (int i = 0; i < result.size(); i++)
	   cout << result[i] << ", ";
	 



	
	   NOTE: I took this test out only because to_string() is not recognized
	   by the CADE lab's ancient compiler. If you're using an updated compiler
	   feel free to run this test - works great!
	 
	
      }
	 */
    /////////////////////////////////////////////////////////////

    if (ok)
      {
	cout << "Test 05: Assignment Operator" << endl;

	cs3505::string_set new_set(157);
	new_set.add("It");
	new_set.add("is");
	new_set.add("getting");
	new_set.add("late");
	new_set.add("and");
	new_set.add("I'm");
	new_set.add("getting");
	new_set.add("tired");

	cs3505::string_set newer_set = new_set;

	vector<string> new_set_result = new_set.get_elements();
	vector<string> newer_set_result = newer_set.get_elements();
	
	if (new_set_result != newer_set_result)
	  ok = false;

	//Debugging stuff:
	cout << "new_set contents: ";
	for (int i = 0; i < new_set_result.size(); i++)
	  cout << new_set_result[i] << ", ";
	cout << endl;

	cout << "newer_set contents: ";
	for (int i = 0; i < newer_set_result.size(); i++)
	  cout << newer_set_result[i] << ", ";
	cout << endl;
	//End debugging stuff

	cout << "Testing self-assignment" << endl;

	new_set = new_set;

	if (new_set.get_elements() != new_set_result)
	  ok = false;

	
      }
    
  }

  if (ok)
    cout << "End of tests reached" << endl;

  return 0; //Terminate program - all tests passed!
}
