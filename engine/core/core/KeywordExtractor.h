#pragma once
#include <map>
#include <string>
#include <vector>
#include <memory>
#include "..\..\citar\HMMTagger.h"

class KeywordExtractor
{
private:
	long symbolTableId;
	std::vector<std::string> sortedChi;
	void ComputeChiValues(const std::map<long, bool> candidates, const std::vector<long> highFreq);
	void IdentifyCardinalWords();
	long GetSymbolId(const std::string word);
	const std::string ResolveSymbol(long symbolId);
	std::map<long, long> _termSentenceTermsCount;
	std::map<const std::string, double> _chiValues;
	std::map<long, std::vector<std::pair<std::string, std::string>>> _sentences;
	std::map<double, long> _pairCounts;
	std::map<long, long> _wordCounts;
	std::map<long, std::vector<long>> _symbolIdPairs;
	std::map<std::string, long> _wordSymbolTable;
	std::map<long, std::string> _symbolWordTable;
	std::map<long, long> _sentenceCountForWord;
	std::shared_ptr<citar::tagger::HMMTagger> _tagger;
	bool IsCandidateWord(const std::pair<std::string, std::string> pair);
	std::vector<std::string> ExtractSentences(std::string& text);
	std::vector<std::string> ApplyEndMarkers(std::string sentence);
	double ComputePairHash(long firstWordId, long secondWordId);
	void SentenceMetaExtract(long id);
	std::vector<std::pair<std::string, std::string>> RemoveEndMarkers(std::vector<std::string>& marked, std::vector<std::string>& sentence);
	
public:
	void InitModel(std::shared_ptr<citar::tagger::HMMTagger> tagger);
	KeywordExtractor(void);
	std::vector<std::string> ExtractKeywords(std::string& text);

	~KeywordExtractor(void);
};

