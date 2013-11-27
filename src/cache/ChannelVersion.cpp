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

void ChannelVersion::output(FILE* f, PathNode<ChannelVersion>* owner) {
  string cv;
  owner->output(cv);
  cv += "/";

  auto out = [&cv, &f](MeasureFile* measure,
                               PathNode<MeasureFile>* parent) -> void {
    fputs(cv.data(), f);
    parent->output(f);
    fputc('\t', f);
    measure->output(f);
    fputc('\n', f);
  };

  _measureRoot.visitTargetTree(out);
}