/*
* Copyright 2008 Daniel de Kok
*
* This file is part of citar.
*
* Citar is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* Citar is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Citar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>
#include <memory>
#include "TaggedWord.h"
#include "HMMTagger.h"
#include "LinearInterpolationSmoothing.h"
#include "Model.h"
#include "KnownWordHandler.h"
#include "SuffixWordHandler.h"
#include "..\\..\engine\core\core\KeywordExtractor.h"
#include <Windows.h>

using namespace std;
using namespace citar::corpus;
using namespace citar::tagger;


shared_ptr<citar::tagger::Model> model;
shared_ptr<HMMTagger> hmmTagger;
SuffixWordHandler* handler;
LinearInterpolationSmoothing* smoothing;
KnownWordHandler *knownWordHandler;

int InitModel(std::vector<std::string> args)
{
	if (args.size() != 2)
	{
		cerr << "Syntax: args" << endl;
		return 1;
	}

	ifstream nGramStream(args[0]);
	if (!nGramStream.good())
	{
		cerr << "Could not open ngrams: " << args[0] << endl;
		return 1;
	}

	ifstream lexiconStream(args[1]);
	if (!lexiconStream.good())
	{
		cerr << "Could not open lexicon: " << args[1] << endl;
		return 1;
	}

	model = Model::readModel(lexiconStream, nGramStream);
	handler = new SuffixWordHandler(model, 2, 2, 8);
	knownWordHandler = new KnownWordHandler(model, handler);
	smoothing = new LinearInterpolationSmoothing(model);
	hmmTagger.reset(new HMMTagger(model, knownWordHandler, smoothing));
	return 0;
}

extern "C"  __declspec(dllexport) void InitializeModel()
{
	std::vector<std::string> args;
	//TODO : standardize this path
	args.push_back("C:\\apps\\livingprofile\\engine\\citar\\brown-simplified.ngrams");
	args.push_back("C:\\apps\\livingprofile\\engine\\citar\\brown-simplified.lexicon");
	if(InitModel(args) != 0)
	{
		cerr << "there was an error in initialization" << endl;
	}
}

extern "C"  __declspec(dllexport) int ExtractKeywords(const char* array)
{
	std::vector<std::string> result;
	KeywordExtractor extractor;
	extractor.InitModel(hmmTagger);
	std::string text = *(new std::string(array));
#ifdef PERF
	LARGE_INTEGER frequency;        // ticks per second
	LARGE_INTEGER t1, t2;           // ticks
	double elapsedTime;
	
	// get ticks per second
	QueryPerformanceFrequency(&frequency);

	// start timer
	QueryPerformanceCounter(&t1);
	for(int i = 0; i < 100; i++)
	{
#endif
		result = extractor.ExtractKeywords(text);
#ifdef PERF
	}
	QueryPerformanceCounter(&t2);
	 // compute and print the elapsed time in millisec
    elapsedTime = (t2.QuadPart - t1.QuadPart) * 1000.0 / frequency.QuadPart;
    cout << elapsedTime << " ms.\n";
#endif

	for(auto elem: result)
	{
		cout << elem << endl;
	}

	return 0;
}
