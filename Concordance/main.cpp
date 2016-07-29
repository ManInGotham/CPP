/*
	File:	main.cpp
	Author: Gleb Karapish

	Date: 10/19/2014

	Description
	=-=-=-=-=-=
	Implementation of the concordance algorithm for English text which computes each distinct word
	and every line the word appears in.
 
	Notes
	=-=-=
	There are few assumptions about the input text:
		-All words are written in English and ASCII encoding is used for character representation.
		-Whenever abbreviation is used in the following form 'e.g.' or 'i.e.', the program expect the trailing dot to be removed.
			'e.g.' -> 'e.g'
			'etc.' -> 'etc'
			'i.e.' -> 'i.e'
		-Numbers are considered as characters non-contributing to word content. It means 'He was born in 1928' sentence 
		contains only 4 words according to program's logic.

	Usage
	=-=-=-
	CONCORDANCE.EXE was compiled with Visual Studio 2013 (SP3) and tested on Windows 8.1 Enterprise.
	Run the program. Copy somewhere (on a web-page, or a file) a sample text and paste directly into console's window.
	Press ENTER button to start the execution.

	If you can paste the code, try to enable the QuickEdit mode:
		Right-click on console's title bar
		Navigate to Options tab->Edit options
		Turn on QuickEdit Mode.
*/

#include <iostream>
#include <map>
#include <unordered_set>
#include <string>
#include <iomanip>

#include "common.h"
#include "WordStats.h"
#include "SentenceParser.h"

using namespace std;
using namespace Concordance;

int main()
{
	string inputText;

	// Print instructions and capture input text
	//
	cout << "Copy somewhere and paste here (into console) text for concordance generation." << endl;
	cout << "Press ENTER when done." << endl;
	cout << endl << endl;
	getline(cin, inputText);
	cout << endl << endl;

	// Invoke text parser which builds frequency info
	//
	SentenceParser sp;
	map<string, WordStats> wordsStats = sp.Run(inputText);

	// Print result on screen
	//
	string rowNumberAsText;
	int rowCharRepeats = 0;
	map<string, WordStats>::iterator wordStatsIterator = wordsStats.begin();

	// spin while loop until all words are printed out
	while (wordStatsIterator != wordsStats.end())
	{
		// 97 decimal code for 'a'
		// 122 decimal code for 'z' in ASCII table
		//
		// Build a row alpha-number when printing out
		for (int rowChar = 97; rowChar <= 122; ++rowChar)
		{
			if (wordStatsIterator == wordsStats.end())
				break;

			rowNumberAsText = static_cast<char>(rowChar);

			for (int repeats = 0; repeats < rowCharRepeats; ++repeats)
			{
				rowNumberAsText += static_cast<char>(rowChar);
			}

			// Row number is ready. Building a text of hits in sentences.
			vector<unsigned int> hits = wordStatsIterator->second.GetHitSentences();
			string hitsEnumerationAsText;
			hitsEnumerationAsText += "{";
			hitsEnumerationAsText += to_string(hits.size());
			hitsEnumerationAsText += ":";
				
			for (vector<unsigned int>::iterator hitsIterator = hits.begin(); hitsIterator != hits.end(); ++hitsIterator)
			{
				hitsEnumerationAsText += to_string(*hitsIterator) + ",";
			}

			// Replacing the last ',' character with '}'
			hitsEnumerationAsText.replace(hitsEnumerationAsText.size() - 1, 1, "}");

			// Printing on screen the line 
			cout << left << setw(wordsStats.size() / NUM_CHARS_IN_ALPHABET + 1) << rowNumberAsText << ". ";
			cout << setw(sp.GetLongestWord()) << wordStatsIterator->first << " ";
			cout << hitsEnumerationAsText << endl;

			wordStatsIterator++;
		}

		// Run out of row numbers. Add more chars to number.
		rowCharRepeats++;
	}

	// Wait for user to press any button
	getchar();

	return 0;
}