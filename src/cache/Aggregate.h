#ifndef AGGREGATE_H
#define AGGREGATE_H

#include "PathNode.h"
#include "InternedString.h"

#include "rapidjson/document.h"

#include <string>

/** Representation of histogram aggregate */
class Aggregate {
  /** Latests revision of aggregated histograms */
  InternedString _revision;

  /**
   * Highest BuildId of aggregated histograms, used to find newest BuildId when
   * merging data into Aggregate.
   */
  InternedString _buildId;

  /** Aggregated values, null, if _length is zero */
  double*        _values;

  /** Length of _values, zero implies _values == null */
  size_t         _length;

  /** String context for interning BuildIds */
  static InternedStringContext  _buildIdStringCtx;

  /** String context for interning revision string */
  static InternedStringContext  _revisionStringCtx;
public:
  Aggregate()
   : _values(nullptr), _length(0) {}

  /** Merge aggregated values from JSON dump */
  void mergeJSON(const rapidjson::Value& dump);

  /** Output to file */
  void output(FILE* f);

  /** Intern a revision string */
  static InternedString internRevisionString(const char* revision);
  /** Intern a build id string */
  static InternedString internBuildIdString(const char* buildId);

  ~Aggregate() {
    if (_values) {
      delete[] _values;
    }
  }
};

#endif // AGGREGATE_H
