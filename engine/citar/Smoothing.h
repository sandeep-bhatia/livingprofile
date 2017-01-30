#ifndef LANGKIT_TAGGER_SMOOTHING
#define LANGKIT_TAGGER_SMOOTHING

#include "NonCopyable.h"
#include "TriGram.h"

namespace citar {
	namespace tagger {

		class Smoothing : public citar::util::NonCopyable
		{
		public:
			virtual double triGramProb(TriGram const &triGram) const = 0;
			virtual ~Smoothing() {}
		};

	}
}

#endif
