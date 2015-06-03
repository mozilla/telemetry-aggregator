#include "cache/ResultSet.h"
#include "cache/ChannelVersion.h"
#include "cache/MeasureFile.h"
#include "cache/Aggregate.h"

#include <unistd.h>
#include <stdio.h>

#include <iostream>
#include <fstream>

/** Print usage */
void usage() {
  printf("Usage: aggregate -o [FILE]\n");
}

using namespace std;

/** Main file */
int main(int argc, char *argv[]) {
  FILE* output      = stdout;
  const char* input = nullptr;

  // Parse arguments
  int c;
  while ((c = getopt(argc, argv, "ho:i:")) != -1) {
    switch (c) {
      case 'o':
        output = fopen(optarg, "w");
        break;
      case 'i':
        input = optarg;
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

  // Aggregated result set
  ResultSet set;

  // Read filename from stdin, if
  istream* pi = nullptr;
  if (input) {
    pi = new ifstream(input);
  } else {
    // Read filenames from stdin
    cin.sync_with_stdio(false);
    pi = &cin;
  }

  string line;
  line.reserve(4096);
  while (getline(*pi, line)) {
    // Trim carriage returns in case we have Windows style line endings
    if (!line.empty() && line[line.size() - 1] == '\r') {
        line.erase(line.size() - 1);
    }

    // Find first tab
    size_t tab = line.find('\t');
    if (tab == string::npos) {
      fprintf(stderr, "Got input-line without tab: '%s'\n", line.data());
      continue;
    }

    // Find prefix and filename
    string prefix   = line.substr(0, tab);
    string filename = line.substr(tab + 1);

    // Aggregate
    set.aggregate(prefix, filename);

    // Delete file
    remove(filename.data());
  }
  set.output(output);

  // If not reading from stdin, delete pi
  if (input) {
    delete pi;
  }

  // Close output file
  fclose(output);

  return 0;
}

