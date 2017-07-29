#include <thread>
#include <vector>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <string>
#include <map>
#include <iostream>

#include "semaphore.h"

using std::cin;

using std::thread;

// Use string instead of the custom Word type\class
// Also strdup() is not a standard C++ function.
using std::string;

// Use hashtable to store all unique alphabetically words and counts. 
using std::map;

using std::unique_lock;

using std::mutex;

// Words and counts
static map<string, size_t> s_wordsArray;
static string s_word;
static int s_totalFound;
static semaphore s_semaphore;
static const string endText = "end";

// Worker thread: consume words passed from the main thread and insert them
// in the 'word list' (s_wordsArray), while removing duplicates. Terminate when
// the word 'end' is encountered.
 static void workerThread() { 	

	for (;;) {
		s_semaphore.wait();

		bool found = false;     
	
		//s_lock.lock();

    if (!s_word.empty()) // Do we have a new word?
    {            	  
	  string wordCopy(s_word.begin(), s_word.end() - endText.length());
	  s_word.clear(); // Inform the producer that we consumed the word
	  //s_lock.unlock();

      endEncountered = wordCopy == endText;
      
      if (!endEncountered)
      {
        // Do not insert duplicate words
        for ( auto p : s_wordsArray )
        {
         /* if (!std::strcmp( p->data, w->data ))
          {
            ++p->count;
            found = true;
            break;
          }*/
        }

        //if (!found)
        //  //s_wordsArray.push_back( w );
      }
    }
	else {
		//s_lock.unlock();
	}
  }
};

// Read input words from STDIN and pass them to the worker thread for
// inclusion in the word list.
// Terminate when the word 'end' has been entered.
//
static void readInputWords() {
  auto worker = new thread( workerThread );

  string input;

  for (;;) {
	cin >> input;

	if (input == endText) {

		if (worker != nullptr) {
			delete worker;
		}

		break;
	}
    
	s_word = input;
	s_semaphore.notify();

    // Wait for the worker thread to consume it
	s_semaphore.wait();
  };

  //worker->join(); // Wait for the worker to terminate
}

//// Repeatedly ask the user for a word and check whether it was present in the word list
//// Terminate on EOF
////
//static void lookupWords ()
//{
//  bool found;
//  char * linebuf = new char[32];
//    
//  for(;;)
//  {
//    std::printf( "\nEnter a word for lookup:" );
//    if (std::scanf( "%s", linebuf ) == EOF)
//      return;
//
//    // Initialize the word to search for
//    Word * w = new Word();
//    std::strcpy( w->data, linebuf );
//
//    // Search for the word
//    unsigned i;
//    for ( i = 0; i < s_wordsArray.size(); ++i )
//    {
//      if (std::strcmp( s_wordsArray[i]->data, w->data ) == 0)
//      {
//        found = true;
//        break;
//      }
//    }
//
//    if (found)
//    {
//      std::printf( "SUCCESS: '%s' was present %d times in the initial word list\n",
//                   s_wordsArray[i]->data, s_wordsArray[i]->count );
//      ++s_totalFound;
//    }
//    else
//      std::printf( "'%s' was NOT found in the initial word list\n", w->data );
//  }
//}

int main ()
{
  try
  {
    readInputWords();
    
    // Sort the words alphabetically
//    std::sort( s_wordsArray.begin(), s_wordsArray.end() );

    // Print the word list
    std::printf( "\n=== Word list:\n" );
    //for ( auto p : s_wordsArray )
    //  std::printf( "%s %d\n", p->data, p->count );

    //lookupWords();

    printf( "\n=== Total words found: %d\n", s_totalFound );
  }
  catch (std::exception & e)
  {
    std::printf( "error %s\n", e.what() );
  }
  
  return 0;
}

