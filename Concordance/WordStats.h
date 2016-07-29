#pragma once

#include <vector>

using namespace std;

namespace Concordance
{
	// Class represents information about the word occurrence in text
	class WordStats
	{
	private:
		// Stores numbers of all sentences a word hit in text
		vector<unsigned int> m_sentenceHits;

	public:
		WordStats(unsigned int firstHitSentence)
		{
			AddHitSentence(firstHitSentence);
		}

		void AddHitSentence(unsigned int sentenceNumberHit)
		{
			m_sentenceHits.push_back(sentenceNumberHit);
		}

		const vector<unsigned int> GetHitSentences()
		{
			return m_sentenceHits;
		}
	};
}