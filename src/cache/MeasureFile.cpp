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
void MeasureFile::output(FILE* f, PathNode<MeasureFile>* owner) {
  owner->output(f);
  fputs("\t{", f);

  Aggregate::OutputContext ctx;
  ctx.file = f;
  ctx.comma = false;
  _filterRoot.outputTargetTree(ctx);

  fputs("}\n", f);
}

/** Output to file */
void MeasureFile::output(FILE* f, const string& filePath) {
  fputs(filePath.data(), f);
  fputs("\t{", f);

  Aggregate::OutputContext ctx;
  ctx.file = f;
  ctx.comma = false;
  _filterRoot.outputTargetTree(ctx);

  fputs("}\n", f);
}

/** Output to string, allows for better buffering */
void MeasureFile::output(string& outline, const string& filePath) {
  outline = filePath;
  outline += "\t{";

  Aggregate::StringOutputContext ctx(outline, false);
  _filterRoot.outputTargetTree(ctx);

  outline += "}\n";
}

/** Output to file */
void MeasureFile::output(FILE* f) {
  fputc('{', f);

  Aggregate::OutputContext ctx;
  ctx.file = f;
  ctx.comma = false;
  _filterRoot.outputTargetTree(ctx);

  fputc('}', f);
}