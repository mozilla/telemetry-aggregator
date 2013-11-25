#include "ResultSet.h"
#include "MeasureFile.h"
#include "Aggregate.h"

#include "rapidjson/document.h"

#include <stdio.h>
#include <string>

using namespace std;
using namespace rapidjson;


void ResultSet::mergeStream(istream& stream) {
  // Read input line by line
  string line;
  int nb_line = 0;
  while (getline(stream, line)) {
    nb_line++;

    // Find delimiter
    size_t del = line.find('\t');
    if (del == string::npos) {
      fprintf(stderr, "No tab on line %i\n", nb_line);
      continue;
    }

    // Find filePath
    string filePath = line.substr(0, del);

    // Parse JSON document
    Document d;
    d.Parse<0>(line.data() + del + 1);

    // Check that we have an object
    if (!d.IsObject()) {
      fprintf(stderr, "JSON root is not an object on line %i\n", nb_line);
      continue;
    }

    // Find blob and merge with it
    PathNode<MeasureFile>* n = _fileRoot.find(filePath.data(), _pathStringCtx);
    if(!n->target()) {
      n->setTarget(new MeasureFile());
    }
    n->target()->mergeJSON(d);
  }
}

void ResultSet::aggregate(const char* filename) {

}

void ResultSet::output(FILE* f) {
  _fileRoot.outputTargetTree(f);
}
