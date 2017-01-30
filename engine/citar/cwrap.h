#ifndef CITAR_CWRAP_HH
#define CITAR_CWRAP_HH

#include "HMMTagger.h"
#include "Smoothing.h"
#include "Model.h"
#include "WordHandler.h"

extern "C"
{
	struct citar_tagger {
		citar::tagger::WordHandler *knownWordHandler;
		citar::tagger::WordHandler *unknownWordHandler;
		citar::tagger::Smoothing *smoother;
		citar::tagger::HMMTagger *tagger;
	};

	citar_tagger *citar_tagger_new(char const *lexicon, char const *trigrams);
	void citar_tagger_free(citar_tagger *tagger);
	char **citar_tagger_tag(citar_tagger *tagger, char const *words[], int len);
}

#endif
