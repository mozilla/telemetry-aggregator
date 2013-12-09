#include "ResultSet.h"
#include "ChannelVersion.h"
#include "MeasureFile.h"
#include "Aggregate.h"
#include "../CompressedFileReader.h"
#include "../../utils/ParseDate.h"
#include "../../utils/mkdirp.h"
#include "../../stringencoders/modp_numtoa.h"

#include "rapidjson/document.h"

#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <string>
#include <fstream>

using namespace std;
using namespace rapidjson;

// Revision string used when missing, this is just a random recent one...
#define FALLBACK_REVISION "http://hg.mozilla.org/mozilla-central/rev/518f5bff0ae4"

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
  char* strBuildId      = strtok(NULL, "/");
  char* submissionDate  = strtok(NULL, "/");
  if (!reason || !appName || !channel || !version || !strBuildId ||
      !submissionDate) {
    fprintf(stderr, "Prefix '%s' missing parts \n", prefix.data());
    return;
  }
  // Intern strBuildId
  InternedString buildId = Aggregate::internBuildIdString(strBuildId);

  // Get build date, ignore the rest
  if (strlen(strBuildId) != 12) {
    fprintf(stderr, "BuildId '%s' is not valid, too short\n", strBuildId);
    return;
  }
  string buildDate(strBuildId, 8);

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

  // Reduce version to major version (discard everything after the dot)
  {
    char* c = version;
    while(*c != '.' && *c != '\0') c++;
    *c = '\0';
  }

  // Find/create ChannelVersion object
  ChannelVersion* cv = nullptr;
  {
    string channelVersion = string(channel) + "/" + string(version);
    // Find channel version in hash table
    auto it = _channelVersionMap.find(channelVersion);
    // If not present create ChannelVersion
    if (it == _channelVersionMap.end()) {
      cv = new ChannelVersion(_measureStringCtx, _filterStringCtx);
      it = _channelVersionMap.insert({channelVersion, cv}).first;
    }
    cv = it->second;
  }
  assert(cv);

  // Create filterPath start
  string reasonAppName = string(reason) + "/" + string(appName) + "/";

  // Allocate a string to hold <measure>/<by-date-type>
  string measureFilename;
  measureFilename.reserve(2048);

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

      // Check that we have an object
      if (!d.IsObject()) {
        fprintf(stderr, "JSON root is not an object on line %i\n", nb_line);
        continue;
      }

      // Find the info field
      Value::Member* infoField = d.FindMember("info");
      if (!infoField || !infoField->value.IsObject()) {
        fprintf(stderr, "'info' in payload isn't an object, line %i\n", nb_line);
        continue;
      }
      Value& info = infoField->value;

      // Find OS, osVersion, arch and revision
      Value::Member* osField    = d.FindMember("OS");
      Value::Member* osVerField = d.FindMember("version");
      Value::Member* archField  = d.FindMember("arch");
      Value::Member* revField   = d.FindMember("revision");
      if (!osField || !osField->value.IsString()) {
        fprintf(stderr, "'OS' in 'info' isn't a string\n");
        continue;
      }
      if (!osVerField || !(osVerField->value.IsString() ||
                           osVerField->value.IsNumber())) {
        fprintf(stderr, "'version' in 'info' isn't a string or number\n");
        continue;
      }
      if (!archField || !archField->value.IsString()) {
        fprintf(stderr, "'arch' in 'info' isn't a string\n");
        continue;
      }
      InternedString revision;
      if (!revField) { 
        if (!revField->value.IsString()) {
          fprintf(stderr, "'revision' in 'info' isn't a string\n");
          continue;
        }
        // Get InternedString for revision
        revision = Aggregate::internRevisionString(revField->value.GetString());
      } else {
        // Get InternedString for revision
        revision = Aggregate::internRevisionString(FALLBACK_REVISION);
      }

      // Create filterPath as <reason>/<appName>/<OS>/<osVersion>/<arch>
      const char* OS = osField->value.GetString();
      string filterPath = reasonAppName + OS;
      filterPath += "/";
      string osVersion;
      if (osVerField->value.IsString()) {
        osVersion = osVerField->value.GetString();
      } else {
        char b[64];
        modp_dtoa2(osVerField->value.GetDouble(), b, 9);
        osVersion = b;
      }
      // For Linux we only add 3 first characters
      if (strcmp(OS, "Linux") == 0) {
        filterPath += osVersion.substr(0, 3);
      } else {
        filterPath += osVersion;
      }
      filterPath += "/";

      // Append arch to filterPath
      filterPath += archField->value.GetString();

      // Aggregate histograms

      Value::Member* hgramField = d.FindMember("histograms");
      if (hgramField && hgramField->value.IsObject()) {
        Value& hgrams = hgramField->value;
        for (auto hgram = hgrams.MemberBegin(); hgram != hgrams.MemberEnd();
                                                                      ++hgram) {
          // Get histogram name and values
          const char* name  = hgram->name.GetString();
          Value& values     = hgram->value;

          // MeasureFilename
          measureFilename = name;
          measureFilename += "/by-build-date";

          // Get measure file
          MeasureFile* mf = cv->measure(measureFilename.data());
          Aggregate*   aggregate = mf->aggregate(buildDate.data(), filterPath.data());

          // TODO: Add to aggregate

          // Skip aggregation by submission date, unless requested
          if (skipBySubmissionDate) {
            continue;
          }

          // MeasureFilename
          measureFilename = name;
          measureFilename += "/by-submission-date";

          // Get measure file
          mf = cv->measure(measureFilename.data());
          aggregate = mf->aggregate(submissionDate, filterPath.data());

          // TODO: Add to aggregate
        }
      } else {
        fprintf(stderr, "'histograms' of payload isn't an object\n");
      }


      // TODO: Aggregate simple measures
    }
  }
  fclose(input);

  free(prefix_data);
}

