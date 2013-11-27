#ifndef CHANNELVERSION_H
#define CHANNELVERSION_H

#include "PathNode.h"
#include "InternedString.h"

#include "rapidjson/document.h"

#include <stdio.h>

class MeasureFile;

/** Object that manages a collection of files a given channel/version */
class ChannelVersion {
  PathNode<MeasureFile> _measureRoot;
  InternedStringContext& _measureStringCtx;
  InternedStringContext& _filterStringCtx;
public:
  ChannelVersion(InternedStringContext& measureStringCtx,
                 InternedStringContext& filterStringCtx)
   : _measureStringCtx(measureStringCtx), _filterStringCtx(filterStringCtx) {}

  /** Merge in a JSON blob for a specific measure */
  void mergeMeasureJSON(const char* measure, rapidjson::Value& blob);

  /** Output to file */
  void output(FILE* f, PathNode<ChannelVersion>* owner);
};


#endif // CHANNELVERSION_H