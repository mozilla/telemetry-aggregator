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

  /** Output context */
  struct OutputContext {
    FILE* file;
    bool  comma;
  };

  /** Output context that buffers to string */
  struct StringOutputContext {
    StringOutputContext(std::string& outline, bool comma)
     : outline(outline), comma(comma) {}
    std::string&  outline;
    bool          comma;
  };

  /** Output to file */
  void output(OutputContext& ctx, PathNode<Aggregate>* owner);

  /** Output to string */
  void output(StringOutputContext& ctx, PathNode<Aggregate>* owner);


  ~Aggregate() {
    if (_values) {
      delete[] _values;
    }
  }
};

#endif // AGGREGATE_H
