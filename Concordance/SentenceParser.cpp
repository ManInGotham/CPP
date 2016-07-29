#include "common.h"
#include "SentenceParser.h"

namespace Concordance
{
	map<string, WordStats> SentenceParser::Run(string text)
	{
		m_text = text;
		m_hashtable.clear();
		m_longestWord = 0;

		string wordSubstring; wordSubstring.clear();
		unsigned int sentence = 1;

		// Compute frequency table
		//
		for (string::iterator textIterator = m_text.begin(); /*loop body takes care of quiting*/ true; ++textIterator)
		{
			if (textIterator == m_text.end())
			{
				// If reached end of text but have a word that haven't been added. Add it to the hashtable
				AddWord(wordSubstring, sentence);

				// Quiting the loop
				break;
			}

			if (IsPartOfNewWord(textIterator))
			{
				*textIterator = tolower(*textIterator);
				wordSubstring += *textIterator;
			}
			else
			{
				// Character is not a valid letter, i.e. it's the end of word or end of sentence
				AddWord(wordSubstring, sentence);
				wordSubstring.clear();

				if (IsEndOfSentence(*textIterator))
				{
					sentence++;
				}
			}
		}

		return m_hashtable;
	}

	void SentenceParser::AddWord(string word, unsigned int sentence)
	{
		if (word.empty())
			return;

		// map::emplace() -- MSDN 
		//
		// If the function successfully inserts the element (because no equivalent element existed already in the map), 
		// the function returns a pair of an iterator to the newly inserted element and a value of true.
		//
		// Otherwise, it returns an iterator to the equivalent element within the container and a value of false.
		//
		pair<map<string, WordStats>::iterator, bool> emplacingResult = m_hashtable.emplace(word, WordStats(sentence));
		
		if (!emplacingResult.second)
		{
			((&(emplacingResult.first)._Ptr->_Myval)->second).AddHitSentence(sentence);
		}
		else
			// Keep track of the longest word
			m_longestWord = max(m_longestWord, word.size());
	}

	bool SentenceParser::IsPartOfNewWord(string::iterator textIterator)
	{
		char letter = *textIterator;
		if (isalpha(letter))
			return true;

		// handle cases like "e.g", "i.e"
		bool outOfRange = (textIterator + 1) == m_text.end();
		if (letter == '.' && !outOfRange && isalpha(*(textIterator + 1)) && islower(*(textIterator + 1)))
			return true;

		return false;
	}

	bool SentenceParser::IsEndOfSentence(char letter)
	{
		return letter == '!' || letter == '?' || letter == '.';
	}

}

