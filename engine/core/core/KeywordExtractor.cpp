#include "stdafx.h"
#include "KeywordExtractor.h"
#include <map>
#include <string>
#include <memory>
#include <sstream>
#include <vector>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <iostream>
#include "..\..\citar\Model.h"
#include "..\..\citar\HMMTagger.h"
//using namespace citar::corpus supplied by the below
#include "..\..\citar\TaggedWord.h"
#include "..\..\citar\KnownWordHandler.h"
#include "..\..\citar\SuffixWordHandler.h"
#include "..\..\citar\LinearInterpolationSmoothing.h"
#include <ctype.h>
#include <queue>

using namespace std;
using namespace citar::corpus;
using namespace citar::tagger;

const int FREQ_LIMIT_COUNT = 50;

KeywordExtractor::KeywordExtractor(void)
{
	this->symbolTableId = 0;
}

void KeywordExtractor::InitModel(shared_ptr<HMMTagger> tagger)
{
	this->_tagger = tagger;
}

std::vector<pair<string, string>> KeywordExtractor::RemoveEndMarkers(vector<std::string>& marked, vector<std::string>& tagSentence)
{
	vector<pair<string, string>> tags;
	for (size_t i = 2; i < tagSentence.size() - 1; ++i)
	{
		tags.push_back(std::move(*(new pair<string, string>(marked[i], tagSentence[i]))));
	}

	return tags;
}

std::vector<std::string> KeywordExtractor::ApplyEndMarkers(std::string rawSentence)
{
	// Get the sequence of words, adding start/end markers.
	vector<string> sentence(2, "<START>");
	//revisit this with better matches RegEx
	stringstream ss(rawSentence);	std::string item;	char delim = ' ';

	while (std::getline(ss, item, delim)) {
		sentence.push_back(item);
	}

	sentence.push_back("<END>");
	return sentence;
}

void KeywordExtractor::SentenceMetaExtract(long sentenceId) 
{
	vector<pair<std::string, std::string>> sentence = this->_sentences[sentenceId];
	vector<long> candidateWords;

	long sentenceTermCount = 0;
	for(auto pair : sentence)
	{
		if(this->IsCandidateWord(pair))
		{
			long symbolId = GetSymbolId(pair.first);

			//TODO : convert to upper case so that we don't bail out on casing
			auto found = _sentenceCountForWord.find(symbolId);
			_sentenceCountForWord[symbolId] = (found == _sentenceCountForWord.end()) ? _sentenceCountForWord[symbolId] = 1 : _sentenceCountForWord[symbolId] + 1;
			candidateWords.push_back(symbolId);
			auto wfound = _wordCounts.find(symbolId);
			_wordCounts[symbolId] = wfound == _wordCounts.end() ? 1 : _wordCounts[symbolId] + 1;
			sentenceTermCount++;
		}
	}

	for(auto wordSymbolIdFirst : candidateWords)
	{
		_termSentenceTermsCount[wordSymbolIdFirst] = _termSentenceTermsCount.find(wordSymbolIdFirst) == _termSentenceTermsCount.end() ? sentenceTermCount : _termSentenceTermsCount[wordSymbolIdFirst] + sentenceTermCount;

		for(auto wordSymbolIdSecond : candidateWords)
		{
			if(wordSymbolIdFirst != wordSymbolIdSecond)
			{
				double hash = ComputePairHash(wordSymbolIdFirst, wordSymbolIdSecond);
				auto found = _pairCounts.find(hash);
				if(found == _pairCounts.end() )
				{
					_pairCounts[hash] = 1;
				}
				else
				{
					_pairCounts[hash] = _pairCounts[hash] + 1;
				}

				_symbolIdPairs[wordSymbolIdFirst].push_back(wordSymbolIdSecond);
				_symbolIdPairs[wordSymbolIdSecond].push_back(wordSymbolIdFirst);
			}
		}
	}
}

double KeywordExtractor::ComputePairHash(long firstWordId, long secondWordId) 
{
	long x;
	long y;

	//use the unique cantor pairing function here to compute the unique single no from the two numbers
	//create the positional bias by preferring the lesser number
	if (firstWordId < secondWordId) {
		x = firstWordId;
		y = secondWordId;
	}
	else {
		x = secondWordId;
		y = firstWordId;
	}
	//http://en.wikipedia.org/wiki/Pairing_function
	return 1 / 2 * (x + y) * (x + y + 1) + y;
};

long KeywordExtractor::GetSymbolId(const std::string word)
{
	auto symbolFound = _wordSymbolTable.find(word);

	if(symbolFound == _wordSymbolTable.end())
	{
		this->symbolTableId += 1;
		_wordSymbolTable[word] = this->symbolTableId;
		_symbolWordTable[this->symbolTableId] = word;
	}

	return _wordSymbolTable[word];
}

const std::string KeywordExtractor::ResolveSymbol(long symbolId)
{
	auto symbolFound = _symbolWordTable.find(symbolId);

	if(symbolFound != _symbolWordTable.end())
	{
		return _symbolWordTable[symbolId];
	}

	return NULL;
}

bool KeywordExtractor::IsCandidateWord(const pair<std::string, std::string> pair)
{
	return pair.second == "VBG" || pair.second == "NN" 	|| pair.second == "NP" 	|| pair.second == "NNS" ? true : false;
}

std::vector<std::string> KeywordExtractor::ExtractKeywords(std::string& text)
{
	vector<std::string> rawSentences = this->ExtractSentences(text);

	long id = 0;

	for(auto sentence : rawSentences) {
		vector<std::string> marked;
		marked = ApplyEndMarkers(sentence);
		vector<string> taggedWords = _tagger->tag(marked);
		this->_sentences[id] = RemoveEndMarkers(marked, taggedWords);
		SentenceMetaExtract(id);
	}

	this->IdentifyCardinalWords();
	return this->sortedChi;
}

void KeywordExtractor::IdentifyCardinalWords()
{
	auto comp = [](std::pair<long, long> a, std::pair<long, long> b ) { return a.second < b.second; };
	std::priority_queue<std::pair<long, long>, vector<std::pair<long, long>>, decltype(comp)> sortedWords(this->_wordCounts.begin(), this->_wordCounts.end(), comp);
	std::map<long, bool> candidateWords;
	std::vector<long> highFreqWords;

	int freqWordIdx = 0;
	while(!sortedWords.empty() && freqWordIdx < FREQ_LIMIT_COUNT)
	{
		std::pair<long, long> freqWord = sortedWords.top();

		//if there are some pairs encountered. Usually you will always have, unless there is text like 
		//John John John etc
		if(this->_symbolIdPairs.find(freqWord.first) != this->_symbolIdPairs.end())
		{
			auto candidateWordVector = this->_symbolIdPairs[freqWord.first];
			for(auto candidateWord : candidateWordVector)
			{
				if(candidateWords.find(candidateWord) == candidateWords.end())
				{
					candidateWords[candidateWord] = true;
				}
			}
		}

		highFreqWords.push_back(freqWord.first);
		sortedWords.pop();
	}


	ComputeChiValues(candidateWords, highFreqWords);
}

void KeywordExtractor::ComputeChiValues(const std::map<long, bool> candidates, const std::vector<long> frequentWords)
{
#ifdef DEBUG
	for(auto elem : candidates)
	{
		cout << "Candidate Word :" + ResolveSymbol(elem.first) << endl;
	}

	for(auto elem: frequentWords)
	{
		cout << "The Freq Word: " << ResolveSymbol(elem) << endl;
	}
#endif

	for(auto candidateWordTag : candidates)
	{
		double max = -1;
		auto candidateWord = candidateWordTag.first;

		/* nw as the total number of terms in sentences where w appears.*/
		double nw = this->_termSentenceTermsCount[candidateWord];

		double sumOverallFreq = 0;

		//the computed chi values over all freq words
		for(auto freqWord : frequentWords)
		{
			if(freqWord == candidateWord) continue;

			/*pg as (the sum of the total number of terms in sentences where g appears)	divided by (the total number of terms in the document)*/
			double pg = (double)(this->_termSentenceTermsCount[freqWord]) / (double)(this->_symbolWordTable.size());

			/* Frequency of co-occurrence of term w and term g is written as freq(w; g) : freqwg */
			double hash = ComputePairHash(candidateWord, freqWord);
			auto found = this->_pairCounts.find(hash);
			if(found == this->_pairCounts.end())
			{
				continue;
			}

			double freqwg = (double)(this->_pairCounts[hash]);
			double freqwContribution = ((freqwg - nw * pg) * (freqwg - nw * pg)) / (nw * pg);
			if(freqwContribution > max)
			{
				max =  freqwContribution;
			}
			sumOverallFreq += freqwContribution;
		}

		if(sumOverallFreq > 0)
		{
			auto word = this->ResolveSymbol(candidateWord);
			if(_chiValues.find(word) == _chiValues.end())
			{
				_chiValues[word] = sumOverallFreq - max;
			}
		}
	}



	auto comp = [](std::pair<std::string, double> a, std::pair<std::string, double> b ) { return a.second < b.second; };
	std::priority_queue<std::pair<std::string, double>, vector<std::pair<std::string, double>>, decltype(comp)> sortedChis(this->_chiValues.begin(), this->_chiValues.end(), comp);
	while(!sortedChis.empty())
	{
		auto elem = sortedChis.top();
		this->sortedChi.push_back(elem.first);

#ifdef DEBUG 
		cout << "The word : " << elem.first << " has Chi Value as :" << elem.second << endl;
#endif
		sortedChis.pop();
	}
}

std::vector<string> KeywordExtractor::ExtractSentences(std::string& text) 
{
	//revisit this with better matches RegEx
	stringstream ss(text);	std::string item;	char delim = '.';
	vector<string> sentences;

	while (std::getline(ss, item, delim)) {
		sentences.push_back(item);
	}

	return sentences;
}

KeywordExtractor::~KeywordExtractor(void)
{
}
