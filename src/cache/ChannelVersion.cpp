#include "ChannelVersion.h"
#include "MeasureFile.h"

#include "Aggregate.h"

#include <stdio.h>

using namespace std;
using namespace rapidjson;

void ChannelVersion::mergeMeasureJSON(const char* measure, Value& blob) {
  PathNode<MeasureFile>* n = _measureRoot.find(measure, _measureStringCtx);
  if(!n->target()) {
    n->setTarget(new MeasureFile(_filterStringCtx));
  }
  n->target()->mergeJSON(blob);
}

void ChannelVersion::output(FILE* f, const string& channelVersion) {
  auto write = [&channelVersion, &f](MeasureFile* measure,
                                     PathNode<MeasureFile>* parent) -> void {
    fputs(channelVersion.data(), f);
    fputc('/', f);
    parent->output(f);
    fputc('\t', f);
    measure->output(f);
    fputc('\n', f);
  };

  _measureRoot.visitTargetTree(write);
}