#ifndef RESULTSET_H
#define RESULTSET_H

#include "PathNode.h"
#include "InternedString.h"

#include <iostream>
#include <stdio.h>

class ChannelVersion;

/** A collection of results for various measures files */
class ResultSet {
  PathNode<ChannelVersion>  _channelVersionRoot;
  InternedStringContext     _channelVersionStringCtx;
  InternedStringContext     _measureStringCtx;
  InternedStringContext     _filterStringCtx;
public:
  /** Merge a result-set file into this ResultSet */
  void mergeStream(std::istream& stream);

  /** Output result-set to file */
  void output(FILE* f);

  /** Decompress and aggregated file */
  void aggregate(const char* filename);
};

#endif // RESULTSET_H
