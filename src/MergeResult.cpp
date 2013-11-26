#include "cache/ResultSet.h"
#include "cache/MeasureFile.h"
#include "cache/Aggregate.h"
#include "cache/InternedString.h"

#include "rapidjson/document.h"

#include <unistd.h>

#include <string>
#include <fstream>
#include <iostream>
#include <vector>

/** Print usage */
void usage() {
  printf("Usage: mergeresults -i [FILE] -o [FILE]\n");
}

using namespace std;
using namespace rapidjson;


/** Main file */
int main(int argc, char *argv[]) {
  vector<char*> inputs;
  FILE* output = stdout;

  // Parse arguments
  int c;
  while ((c = getopt(argc, argv, "hi:o:")) != -1) {
    switch (c) {
      case 'i':
        inputs.push_back(optarg);
        break;
      case 'o':
        output = fopen(optarg, "w");
        break;
      case 'h':
        usage();
        exit(0);
        break;
      case '?':
        usage();
        return 1;
        break;
      default:
        abort();
    }
  }

  // Merge input in memory if more than one input file is given
  if (inputs.size() > 1) {
    // Input one by one
    ResultSet set;
    for (auto file : inputs) {
      ifstream stream(file);
      set.mergeStream(stream);
    }
    set.output(output);

  } else {
    // If a single file is given we read from it and output whenever the the
    // filePath changes. This will mergeresult of sorted input, hence,
    // perfect when piping in from GNU sort, which can efficiently merge sorted
    // files
    istream* input;

    // If no input file was given at all, we read from stdin
    if (inputs.empty()) {
      cin.sync_with_stdio(false);
      input = &cin;
    } else {
      input = new ifstream(inputs[0]);
    }

    // filePath and measureFile currently aggregated
    string                  filePath;
    MeasureFile*            measureFile = nullptr;
    InternedStringContext*  filterStringCtx = nullptr;

    string outline;
    outline.reserve(10 * 1024 * 1024);
    string line;
    line.reserve(10 * 1024 * 1024);
    int nb_line = 0;
    while (getline(*input, line)) {
      nb_line++;

      // Find delimiter
      size_t del = line.find('\t');
      if (del == string::npos) {
        fprintf(stderr, "No tab on line %i\n", nb_line);
        continue;
      }

      // Find current file path
      string currentFilePath = line.substr(0, del);

      // If we're reached a new filePath, output the old one
      if (filePath != currentFilePath) {
        if (measureFile) {
          measureFile->output(outline, filePath);
          fputs(outline.data(), output);
          delete measureFile;
          measureFile = nullptr;
          delete filterStringCtx;
          filterStringCtx = nullptr;
        }
        filePath = currentFilePath;
      }

      // Parse JSON document
      Document d;
      d.Parse<0>(line.data() + del + 1);

      // Check that we have an object
      if (!d.IsObject()) {
        fprintf(stderr, "JSON root is not an object on line %i\n", nb_line);
        continue;
      }

      // Allocate MeasureFile if not already aggregated
      if (!measureFile) {
        filterStringCtx = new InternedStringContext();
        measureFile = new MeasureFile(*filterStringCtx);
      }

      // Merge in JSON
      measureFile->mergeJSON(d);
    }

    // Output last MeasureFile, if there was ever one
    if (measureFile) {
      measureFile->output(outline, filePath);
      fputs(outline.data(), output);
      delete measureFile;
      measureFile = nullptr;
      delete filterStringCtx;
      filterStringCtx = nullptr;
    }
  }

  // Close output file
  fclose(output);

  return 0;
}
