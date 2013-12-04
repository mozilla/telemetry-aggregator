#include "cache/ResultSet.h"
#include "cache/ChannelVersion.h"
#include "cache/MeasureFile.h"
#include "cache/Aggregate.h"

#include <unistd.h>

#include <iostream>

/** Print usage */
void usage() {
  printf("Usage: aggregate -o [FILE]\n");
}

using namespace std;

/** Main file */
int main(int argc, char *argv[]) {
  FILE* output = stdout;

  // Parse arguments
  int c;
  while ((c = getopt(argc, argv, "ho:")) != -1) {
    switch (c) {
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

  // Aggregated result set
  ResultSet set;

  // Read input file names from stdin
  cin.sync_with_stdio(false);
  string line;
  line.reserve(4096);
  while (getline(cin, line)) {
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
  }
  set.output(output);


  // Close output file
  fclose(output);

  return 0;
}

