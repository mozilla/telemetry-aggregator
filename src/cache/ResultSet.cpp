#include "ResultSet.h"
#include "ChannelVersion.h"
#include "MeasureFile.h"
#include "Aggregate.h"
#include "../CompressedFileReader.h"

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

    // Find tab
    size_t tab = line.find('\t');
    if (tab == string::npos) {
      fprintf(stderr, "No tab on line %i\n", nb_line);
      continue;
    }

    // Find second slash
    size_t slash = line.find('/');
    if (slash == string::npos) {
      fprintf(stderr, "No slash on line %i\n", nb_line);
      continue;
    }
    slash = line.find('/', slash + 1);
    if (slash == string::npos) {
      fprintf(stderr, "No channel/version on line %i\n", nb_line);
      continue;
    }

    // Find filePath
    string channelVersion = line.substr(0, slash);
    string measure        = line.substr(slash + 1, tab - slash);

    // Parse JSON document
    Document d;
    d.Parse<0>(line.data() + tab + 1);

    // Check that we have an object
    if (!d.IsObject()) {
      fprintf(stderr, "JSON root is not an object on line %i\n", nb_line);
      continue;
    }

    // Find blob and merge with it
    auto n = _channelVersionRoot.find(channelVersion.data(), _channelVersionStringCtx);
    if(!n->target()) {
      n->setTarget(new ChannelVersion(_measureStringCtx, _filterStringCtx));
    }
    n->target()->mergeMeasureJSON(measure.data(), d);
  }
}

void ResultSet::aggregate(const char* filename) {
  FILE* input = fopen(filename, "r");
  {
    // Read file line by line
    CompressedFileReader reader(input);
    char* line = nullptr;
    int nb_line = 0;
    while ((line = reader.nextLine()) != nullptr) {
      nb_line++;
      // Find tab
      char* tab = strchr(line, '\t');
      if (!tab) {
        fprintf(stderr, "No tab on line %i\n", nb_line);
        continue;
      }

      // Set tab = \0 creating two C-strings
      *tab = '\0';
      char* uuid = line;
      char* json = tab + 1;

      // Parse the JSON line
      Document d;
      d.Parse<0>(json);

      // Find file paths
      // Lookup path and aggregate
    }
  }
  fclose(input);
}

void ResultSet::output(FILE* f) {
  string cv;
  cv.reserve(1024);

  auto write = [&f, &cv](ChannelVersion* channelVersion,
                         PathNode<ChannelVersion>* parent) -> void {
    cv.clear();
    parent->output(cv);
    channelVersion->output(f, cv);
  };

  _channelVersionRoot.visitTargetTree(write);
}
