#include "ResultSet.h"
#include "ChannelVersion.h"
#include "MeasureFile.h"
#include "Aggregate.h"
#include "../CompressedFileReader.h"

#include "rapidjson/document.h"

#include <unistd.h>
#include <stdio.h>
#include <string>
#include <fstream>

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

    // Find channel version in hash table
    auto it = _channelVersionMap.find(channelVersion);
    // If not present create ChannelVersion
    if (it == _channelVersionMap.end()) {
      auto cv = new ChannelVersion(_measureStringCtx, _filterStringCtx);
      it = _channelVersionMap.insert({channelVersion, cv}).first;
    }
    // Merge our data in
    it->second->mergeMeasureJSON(measure.data(), d);
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
  for(auto pair : _channelVersionMap) {
    pair.second->output(f, pair.first);
  }
}

void ResultSet::updateFileInFolder(string folder) {
  // Ensure folder ends with a slash
  if (folder[folder.length() - 1] != '/') {
    folder += '/';
  }

  // For each channel/version
  string filename;
  filename.reserve(256 + folder.size());
  for(auto pair : _channelVersionMap) {
    string channelDashVersion = pair.first;
    for(size_t i = 0; i < channelDashVersion.length(); i++) {
      if(channelDashVersion[i] == '/')
        channelDashVersion[i] = '-';
    }
    filename = folder + channelDashVersion;
    // Check if filename exists
    if (access(filename.data(), F_OK) != 0) {
      // If it exists we'll want to merge it into the current data-set
      ifstream file(filename);
      mergeStream(file);
      file.close();
    }
    FILE* f = fopen(filename.data(), "w");
    if (!f) {
      fprintf(stderr, "Failed to open '%s'\n", filename.data());
    }
    pair.second->output(f, pair.first);
    fclose(f);
  }
}


ResultSet::~ResultSet() {
  for(auto pair : _channelVersionMap) {
    delete pair.second;
  }
}