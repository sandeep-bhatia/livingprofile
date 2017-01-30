#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#include <memory>
#include "Model.h"
#include "HMMTagger.h"
#include "LinearInterpolationSmoothing.h"
#include "Model.h"
#include "KnownWordHandler.h"
#include "SuffixWordHandler.h"
#include "cwrap.h"
using namespace std;
using namespace citar::tagger;
