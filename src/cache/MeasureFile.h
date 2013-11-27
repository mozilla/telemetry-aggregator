#ifndef MEASUREFILE_H
#define MEASUREFILE_H

#include "PathNode.h"
#include "InternedString.h"

#include "rapidjson/document.h"

#include <stdio.h>

class Aggregate;

/**
 * In-memory representation of the aggregated data stored in a single JSON file.
 * This is called an MeasureFile as there may be multiple of these files for a
 * given measure under different channel, product, version and by-date.
 */
class MeasureFile {
  PathNode<Aggregate>           _filterRoot;
  InternedStringContext&        _filterStringCtx;
public:
  MeasureFile(InternedStringContext& filterStringCtx)
   : _filterStringCtx(filterStringCtx) {}

  /** Merge with JSON from file */
  void mergeJSON(rapidjson::Value& blob);

  /** Output to file */
  void output(FILE* f, PathNode<MeasureFile>* owner);

  /** Output to file */
  void output(FILE* f, const std::string& filePath);
  void output(std::string& outline, const std::string& filePath);
  void output(FILE* f);
};

#endif // MEASUREFILE_H
