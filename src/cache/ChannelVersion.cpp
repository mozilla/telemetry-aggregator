#include "ChannelVersion.h"
#include "MeasureFile.h"

#include "Aggregate.h"

#include <stdio.h>

using namespace std;
using namespace rapidjson;

void ChannelVersion::mergeMeasureJSON(const char* measure, Value& blob) {
  MeasureFile* mf = _measureFileMap.get(measure, nullptr);
  if (!mf) {
    mf = new MeasureFile(_filterStringCtx);
    InternedString m = _measureStringCtx.createString(measure);
    _measureFileMap.set(m, mf);
  }
  mf->mergeJSON(blob);
}

MeasureFile* ChannelVersion::measure(const char* measure) {
  MeasureFile* mf = _measureFileMap.get(measure, nullptr);
  if (!mf) {
    mf = new MeasureFile(_filterStringCtx);
    InternedString m = _measureStringCtx.createString(measure);
    _measureFileMap.set(m, mf);
  }
  return mf;
}

void ChannelVersion::output(FILE* f, const string& channelVersion) {
  auto write = [&channelVersion, &f](InternedString& measure,
                                     MeasureFile* measureFile) -> void {
    fputs(channelVersion.data(), f);
    fputc('/', f);
    fputs(measure.data(), f);
    fputc('\t', f);
    measureFile->output(f);
    fputc('\n', f);
  };
  _measureFileMap.each(write);
}

void ChannelVersion::clear() {
  auto free = [](InternedString& measure, MeasureFile* measureFile) -> void {
    delete measureFile;
  };
  _measureFileMap.each(free);
  _measureFileMap.clear();
}

ChannelVersion::~ChannelVersion() {
  clear();
}