#include "ResultSet.h"
#include "ChannelVersion.h"
#include "MeasureFile.h"
#include "Aggregate.h"
#include "../CompressedFileReader.h"
#include "../../utils/ParseDate.h"
#include "../../utils/mkdirp.h"

#include "rapidjson/document.h"

#include <unistd.h>
#include <time.h>
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
    string measure        = line.substr(slash + 1, tab - slash - 1);

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
    // Find first slash (and there by channel)
    size_t slash = pair.first.find("/");
    if (slash == string::npos) {
      fprintf(stderr, "Cannot output channel/version without slash: '%s'\n",
              pair.first.data());
      continue;
    }
    string channel = pair.first.substr(0, slash);

    // Check if filename exists
    filename = folder + pair.first;
    if (access(filename.data(), F_OK) == 0) {
      // If it exists we'll want to merge it into the current data-set
      ifstream file(filename);
      mergeStream(file);
      file.close();
    }

    // Create channel folder if it doesn't already exists
    mkdirp((folder + channel).data());

    // Output updated file
    FILE* f = fopen(filename.data(), "w");
    if (!f) {
      fprintf(stderr, "Failed to open '%s'\n", filename.data());
      continue;
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

void ResultSet::aggregate(const std::string& prefix,
                          const std::string& filename) {
  // Parse prefix
  char* prefix_data = strdup(prefix.data());
  char* reason          = strtok(prefix_data, "/");
  char* appName         = strtok(NULL, "/");
  char* channel         = strtok(NULL, "/");
  char* version         = strtok(NULL, "/");
  char* buildId         = strtok(NULL, "/");
  char* submissionDate  = strtok(NULL, "/");
  if (!reason || !appName || !channel || !version || !buildId ||
      !submissionDate) {
    fprintf(stderr, "Prefix '%s' missing parts \n", prefix.data());
    return;
  }

  // Get build date, ignore the rest
  if (strlen(buildId) != 12) {
    fprintf(stderr, "BuildId '%s' is not valid, too short\n", buildId);
    return;
  }
  string buildDate(buildId, 8);

  // Decide if we should skip submission date
  bool skipBySubmissionDate = false;
  {
    // Parse submission date and buildDate
    time_t submissionTime = parseDate(submissionDate);
    time_t buildTime      = parseDate(buildDate.data());

    // Skip aggregation by submission date of it's been more than 60 days since
    // build date
    if (difftime(submissionTime, buildTime) > 60 * 60 * 24 * 60) {
      skipBySubmissionDate = true;
    }
  }

  FILE* input = fopen(filename.data(), "r");
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

  free(prefix_data);
}

