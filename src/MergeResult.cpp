#include "cache/ResultSet.h"
#include "cache/MeasureFile.h"
#include "cache/Aggregate.h"

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

  // If input files are given read them
  if (!inputs.empty()) {
    // Input one by one
    ResultSet set;
    for (auto file : inputs) {
      ifstream stream(file);
      set.mergeStream(stream);
    }
    set.output(output);

  } else {
    // if no input files are given, we read from cin and output whenever the
    // the filePath changes. This will mergeresult of sorted input, hence,
    // perfect when piping in from GNU sort, which can efficiently merge sorted
    // files
    cin.sync_with_stdio(false);

    // filePath and measureFile currently aggregated
    string        filePath;
    MeasureFile*  measureFile = nullptr;

    string line;
    int nb_line = 0;
    while (getline(cin, line)) {
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
          measureFile->output(output, filePath);
          delete measureFile;
          measureFile = nullptr;
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
        measureFile = new MeasureFile();
      }

      // Merge in JSON
      measureFile->mergeJSON(d);
    }

    // Output last MeasureFile, if there was ever one
    if (measureFile) {
      measureFile->output(output, filePath);
      delete measureFile;
      measureFile = nullptr;
    }
  }

  // Close output file
  fclose(output);

  return 0;
}
