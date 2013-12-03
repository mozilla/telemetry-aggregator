#ifndef MEASUREFILE_H
#define MEASUREFILE_H

#include "PathNode.h"
#include "InternedString.h"

#include "rapidjson/document.h"

#include <stdio.h>
#include <vector>

class Aggregate;
class ChannelVersion;

/**
 * In-memory representation of the aggregated data stored in a single JSON file.
 * This is called an MeasureFile as there may be multiple of these files for a
 * given measure under different channel, product, version and by-date.
 */
class MeasureFile {
  /** Entry holding date and filtered aggregates */
  struct DateEntry {
    InternedString      date;
    PathNode<Aggregate> filterRoot;
  };
  /** Filtered aggregates for each date */
  std::vector<DateEntry*>       _dates;
  InternedStringContext&        _filterStringCtx;
public:
  MeasureFile(InternedStringContext& filterStringCtx)
   : _filterStringCtx(filterStringCtx) {}

  /** Merge with JSON from file */
  void mergeJSON(rapidjson::Value& blob);

  /** Output to file */
  void output(FILE* f);

  ~MeasureFile();
};

#endif // MEASUREFILE_H
