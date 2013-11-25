#include "cache/ResultSet.h"
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

/*
  // Read input file names from stdin
  cin.sync_with_stdio(false);
  string filename;
  while(getline(cin, filename)) {
    set.aggregate(filename.data());
  }
  set.output(output);

*/

  // Close output file
  fclose(output);

  return 0;
}

