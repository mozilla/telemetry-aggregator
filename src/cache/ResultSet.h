#ifndef RESULTSET_H
#define RESULTSET_H

#include "PathNode.h"
#include "InternedString.h"

#include <iostream>

class MeasureFile;

/** A collection of results for various measures files */
class ResultSet {
  PathNode<MeasureFile> _fileRoot;
  InternedStringContext _pathStringCtx;
  InternedStringContext _filterStringCtx;
public:
  /** Merge a result-set file into this ResultSet */
  void mergeStream(std::istream& stream);

  /** Output result-set to file */
  void output(FILE* f);

  /** Decompress and aggregated file */
  void aggregate(const char* filename);
};

#endif // RESULTSET_H
