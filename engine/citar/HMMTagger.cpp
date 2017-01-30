#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "HMMTagger.h"
#include "HMMTaggerPrivate.h"

using namespace citar::tagger;

HMMTagger::HMMTagger(std::shared_ptr<Model> model, WordHandler const *wordHandler,
	Smoothing const *smoothing, double beamFactor)
	: d_private(new HMMTaggerPrivate(model, wordHandler, smoothing, beamFactor))
{}

HMMTagger::~HMMTagger()
{
	delete d_private;
}

std::vector<std::string> HMMTagger::tag(std::vector<std::string> const &sentence) const
{
	return d_private->tag(sentence);
}
