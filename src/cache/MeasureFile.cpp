#include "MeasureFile.h"

#include "Aggregate.h"

#include <stdio.h>

using namespace std;
using namespace rapidjson;

void MeasureFile::mergeJSON(Value& blob) {
  // For each member
  for (auto it = blob.MemberBegin(); it != blob.MemberEnd(); ++it) {
    // First find filter path
    const char* filterPath = it->name.GetString();

    // Check that we have an object
    if (!it->value.IsObject()) {
      printf("Value of filterPath: %s is not an object\n", filterPath);
      continue;
    }

    // Find PathNode
    PathNode<Aggregate>* n = _filterRoot.find(filterPath, _filterStringCtx);
    if (!n->target()) {
      n->setTarget(new Aggregate());
    }
    n->target()->mergeJSON(it->value);
  }
}


/** Output to file */
void MeasureFile::output(FILE* f) {
  fputc('{', f);

  bool first = true;
  auto write = [&f, &first](Aggregate* aggregate,
                            PathNode<Aggregate>* parent) -> void {
    if (first) {
      first = false;
    } else {
      fputc(',', f);
    }
    fputc('\"', f);
    parent->output(f);
    fputs("\":", f);
    aggregate->output(f);
  };

  _filterRoot.visitTargetTree(write);

  fputc('}', f);
}