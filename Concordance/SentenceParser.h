#pragma once

#include <string>
#include <map>

#include "WordStats.h"

using namespace std;

namespace Concordance
{
	class SentenceParser
	{
	private:
		map<string, WordStats> m_hashtable;
		unsigned int m_longestWord = 0;
		string m_text;
	
		bool IsEndOfSentence(char letter);
		bool IsPartOfNewWord(string::iterator textIterator);
		void AddWord(string word, unsigned int sentence);

	public:
		map<string, WordStats> Run(string text);

		unsigned int GetLongestWord() const
		{
			return m_longestWord;
		}
	};
}