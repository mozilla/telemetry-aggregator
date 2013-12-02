#ifndef RESULTSET_H
#define RESULTSET_H

#include "PathNode.h"
#include "InternedString.h"

#include <iostream>
#include <stdio.h>
#include <string>
#include <unordered_map>

class ChannelVersion;

/** A collection of results for various measures files */
class ResultSet {
  typedef std::unordered_map<std::string, ChannelVersion*> ChannelVersionMap;
  ChannelVersionMap         _channelVersionMap;
  InternedStringContext     _measureStringCtx;
  InternedStringContext     _filterStringCtx;
public:
  /** Merge a result-set file into this ResultSet */
  void mergeStream(std::istream& stream);

  /** Output result-set to file */
  void output(FILE* f);

  /** Decompress and aggregated file */
  void aggregate(const char* filename);

  /** Free elements from unordered_map */
  ~ResultSet();
};

#endif // RESULTSET_H
