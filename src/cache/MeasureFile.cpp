#include "MeasureFile.h"

#include "Aggregate.h"

#include <stdio.h>

using namespace std;
using namespace rapidjson;

InternedStringContext  MeasureFile::_filterStringCtx;

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

    #if FIRST_DUMP_ONLY
    break;
    #endif
  }
}

/** Output to file */
void MeasureFile::output(FILE* f, PathNode<MeasureFile>* owner) {
  owner->output(f);
  fputs("\t{", f);

  Aggregate::OutputContext ctx;
  ctx.file = f;
  ctx.comma = false;
  _filterRoot.outputTargetTree(ctx);

  fputs("}\n", f);
}

void MeasureFile::output(FILE* f, const std::string& filePath) {
  fputs(filePath.data(), f);
  fputs("\t{", f);

  Aggregate::OutputContext ctx;
  ctx.file = f;
  ctx.comma = false;
  _filterRoot.outputTargetTree(ctx);

  fputs("}\n", f);
}