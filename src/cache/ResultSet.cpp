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
#include <string.h>
#include <string>
#include <fstream>
#include <algorithm>
#include <vector>
#include <errno.h>

using namespace std;
using namespace rapidjson;

#define UNUSED(x)     (void)(x)

// Revision string used when missing, this is just a random recent one...
#define FALLBACK_REVISION "http://hg.mozilla.org/mozilla-central/rev/518f5bff0ae4"

/** Output while merging with input from sorted file */
void outputWithInput(FILE* f, istream& stream,
                     const string& channelVersion,
                     vector<ChannelVersion::Item>& items);

void ResultSet::mergeStream(istream& stream) {
  // Read input line by line
  string line;
  line.reserve(5 * 1024 * 1024); // just reserve enough memory
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
  // Get list of channel versions
  vector<string> channelVersions;
  channelVersions.reserve(_channelVersionMap.size());
  for(auto pair : _channelVersionMap) {
    channelVersions.push_back(pair.first);
  }

  // Sort list of channelVersions
  sort(channelVersions.begin(), channelVersions.end());

  for(string& channelVersion : channelVersions) {
    auto it = _channelVersionMap.find(channelVersion);
    assert(it != _channelVersionMap.end());
    it->second->output(f, channelVersion);
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
      //Load sorted items
      vector<ChannelVersion::Item> items;
      pair.second->loadSortedItems(items);
      string tmp_filename = filename + "_tmp";
      FILE* f = fopen(tmp_filename.data(), "w");
      outputWithInput(f, file, pair.first, items);
      file.close();
      fclose(f);

      // Rename tmp file to new file
      remove(filename.data());
      rename(tmp_filename.data(), filename.data());

    } else {
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
}


void outputWithInput(FILE* f, istream& stream, const string& channelVersion,
                     vector<ChannelVersion::Item>& items) {
  size_t nextItem = 0;

  // Read input line by line
  string line;
  line.reserve(5 * 1024 * 1024); // just reserve enough memory
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
    string cv       = line.substr(0, slash);
    string measure  = line.substr(slash + 1, tab - slash - 1);

    // The JSON document
    const char* payload   = line.data() + tab + 1;

    // These should be the same
    assert(cv == channelVersion);

    bool written = false;
    while(!written && nextItem < items.size()) {
      // Get nextItem
      ChannelVersion::Item& item = items[nextItem];

      // If we shouldn't output item now, break...
      if(cv < channelVersion || measure < item.key.asString()) {
        break; //leaving written == false
      } else {
        // If we have a match we should merge them
        if (cv == channelVersion && measure == item.key.asString()) {
          // Merge into item
          written = true;
          // Parse JSON document
          Document d;
          d.Parse<0>(payload);

          // Check that we have an object
          if (!d.IsObject()) {
            fprintf(stderr, "JSON root is not an object on line %i\n", nb_line);
          } else {
            // Merge into item
            item.value->mergeJSON(d);
          }
        }
        // output item
        fputs(channelVersion.data(), f);
        fputc('/', f);
        fputs(item.key.data(), f);
        fputc('\t', f);
        item.value->output(f);
        fputc('\n', f);
        nextItem++;
      }
    }
    // If it wasn't merged and output above, output payload now
    if(!written) {
      // Output payload from stream
      fputs(cv.data(), f);
      fputc('/', f);
      fputs(measure.data(), f);
      fputc('\t', f);
      fputs(payload, f);
      fputc('\n', f);
    }
  }

  // Output remaining items from cache
  while(nextItem < items.size()) {
    ChannelVersion::Item& item = items[nextItem];
    // Output next item
    fputs(channelVersion.data(), f);
    fputc('/', f);
    fputs(item.key.data(), f);
    fputc('\t', f);
    item.value->output(f);
    fputc('\n', f);
    nextItem++;
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
  char* strBuildId      = strtok(NULL, ".");
  char* submissionDate  = strtok(NULL, ".");
  if (!reason || !appName || !channel || !version || !strBuildId ||
      !submissionDate) {
    fprintf(stderr, "Prefix '%s' missing parts \n", prefix.data());
    free(prefix_data);
    return;
  }
  // Intern strBuildId
  InternedString buildId = Aggregate::internBuildIdString(strBuildId);

  // Get build date, ignore the rest
  if (strlen(strBuildId) < 8) {
    fprintf(stderr, "BuildId '%s' is not valid, too short\n", strBuildId);
    free(prefix_data);
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
  if (!input) {
    fprintf(stderr, "Could not read (errno=%d) input file '%s'\n", errno, filename.data());
  }

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

      UNUSED(uuid);

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
      Value::Member* osField    = info.FindMember("OS");
      Value::Member* osVerField = info.FindMember("version");
      Value::Member* archField  = info.FindMember("arch");
      Value::Member* revField   = info.FindMember("revision");
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
      if (revField) {
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

          // Add to aggregate
          aggregate->aggregate(revision, buildId, values);

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

          // Add to aggregate
          aggregate->aggregate(revision, buildId, values);
        }
      } else {
        fprintf(stderr, "'histograms' of payload isn't an object\n");
      }

      // Aggregate simple measures
      Value::Member* smField = d.FindMember("simpleMeasurements");
      if (smField && smField->value.IsObject()) {
        Value& sms = smField->value;
        for (auto sm = sms.MemberBegin(); sm != sms.MemberEnd(); ++sm) {
          // Get simple measure name and values
          string name   = sm->name.GetString();
          Value& value  = sm->value;

          // Convert name to uppercase
          for(size_t i = 0; i < name.length(); i++) {
            name[i] = toupper(name[i]);
          }

          // Aggregate numbers
          if (value.IsNumber()) {
            // Create measure filename
            measureFilename = "SIMPLE_MEASURES_";
            measureFilename += name;
            measureFilename += "/by-build-date";

            // Get measure file
            MeasureFile* mf = cv->measure(measureFilename.data());
            Aggregate*   aggregate = mf->aggregate(buildDate.data(),
                                                   filterPath.data());

            // Aggregate simple measure
            aggregate->aggregate(value.GetDouble());

            // Aggregate by submission date if desired
            if (!skipBySubmissionDate) {
              // Create measure filename
              measureFilename = "SIMPLE_MEASURES_";
              measureFilename += name;
              measureFilename += "/by-submission-date";

              mf = cv->measure(measureFilename.data());
              aggregate = mf->aggregate(submissionDate, filterPath.data());

              // Aggregate simple measure
              aggregate->aggregate(value.GetDouble());
            }
          } else if (value.IsObject()) {
            // If we have an object read key/values of it
            for (auto ssm = value.MemberBegin(); ssm != value.MemberEnd();
                                                                        ++ssm) {
              string subname   = ssm->name.GetString();
              Value& subvalue  = ssm->value;

              // Convert subname to uppercase
              for(size_t i = 0; i < subname.length(); i++) {
                subname[i] = toupper(subname[i]);
              }

              // Aggregate subvalue, if number, ignore errors there are lots of
              // weird simple measures
              if (subvalue.IsNumber()) {
                // Create measure filename
                measureFilename = "SIMPLE_MEASURES_";
                measureFilename += name;
                measureFilename += "_";
                measureFilename += subname;
                measureFilename += "/by-build-date";

                // Get measure file
                MeasureFile* mf = cv->measure(measureFilename.data());
                Aggregate*   aggregate = mf->aggregate(buildDate.data(),
                                                       filterPath.data());

                // Aggregate simple measure
                aggregate->aggregate(subvalue.GetDouble());

                // Aggregate by submission date if desired
                if (!skipBySubmissionDate) {
                  // Create measure filename
                  measureFilename = "SIMPLE_MEASURES_";
                  measureFilename += name;
                  measureFilename += "_";
                  measureFilename += subname;
                  measureFilename += "/by-submission-date";

                  mf = cv->measure(measureFilename.data());
                  aggregate = mf->aggregate(submissionDate, filterPath.data());

                  // Aggregate simple measure
                  aggregate->aggregate(subvalue.GetDouble());
                }
              }
            }
          }
        }
      }
    }
  }
  fclose(input);

  free(prefix_data);
}

