#include "ChannelVersion.h"
#include "MeasureFile.h"

#include "Aggregate.h"

#include <stdio.h>
#include <vector>

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
  vector<MeasureFileMap::Item> items;
  _measureFileMap.getSortedItems(items);
  for(MeasureFileMap::Item& item : items) {
    fputs(channelVersion.data(), f);
    fputc('/', f);
    fputs(item.key.data(), f);
    fputc('\t', f);
    item.value->output(f);
    fputc('\n', f);
  }
}

void ChannelVersion::loadSortedItems(std::vector<Item>& items) {
  _measureFileMap.getSortedItems(items);
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
