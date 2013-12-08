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
      fprintf(stderr, "Value of filter-path: %s is not an object\n", filterPath);
      continue;
    }

    // Find date
    const char* slash = strchr(filterPath, '/');
    if (!slash) {
      fprintf(stderr, "Slash not found in filter-path: '%s'\n", filterPath);
      continue;
    }

    // Hack make string
    *(const_cast<char*>(slash)) = '\0';

    // find date entry, if present
    DateEntry* entry = nullptr;
    for (auto e : _dates) {
      if (e->date == filterPath) {
        entry = e;
        break;
      }
    }

    // Insert new date entry
    if (!entry) {
      entry = new DateEntry();
      entry->date = _filterStringCtx.createString(filterPath);
      _dates.push_back(entry);
    }

    // Find PathNode
    PathNode<Aggregate>* n = entry->filterRoot.find(slash + 1, _filterStringCtx);
    if (!n->target()) {
      n->setTarget(new Aggregate());
    }
    n->target()->mergeJSON(it->value);

    // Hack to restore string
    *(const_cast<char*>(slash)) = '/';
  }
}

Aggregate* MeasureFile::aggregate(const char* date, const char* filterPath) {
      // find date entry, if present
    DateEntry* entry = nullptr;
    for (auto e : _dates) {
      if (e->date == date) {
        entry = e;
        break;
      }
    }

    // Insert new date entry
    if (!entry) {
      entry = new DateEntry();
      entry->date = _filterStringCtx.createString(date);
      _dates.push_back(entry);
    }

    // Find PathNode
    PathNode<Aggregate>* n = entry->filterRoot.find(filterPath, _filterStringCtx);
    if (!n->target()) {
      n->setTarget(new Aggregate());
    }

    return n->target();
}

/** Output to file */
void MeasureFile::output(FILE* f) {
  fputc('{', f);

  bool first = true;
  for(auto entry : _dates) {
    auto write = [&f, &first, &entry](Aggregate* aggregate,
                                      PathNode<Aggregate>* parent) -> void {
      if (first) {
        first = false;
      } else {
        fputc(',', f);
      }
      fputc('\"', f);
      fputs(entry->date.data(), f);
      fputc('/', f);
      parent->output(f);
      fputs("\":", f);
      aggregate->output(f);
    };

    entry->filterRoot.visitTargetTree(write);
  }

  fputc('}', f);
}


MeasureFile::~MeasureFile() {
  for(auto entry : _dates) {
    delete entry;
  }
  _dates.clear();
}