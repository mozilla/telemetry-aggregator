#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "InternedString.h"

#define UNUSED(x)     (void)(x)

#ifdef LOG_INTERNEDSTRING
#define log(...)                fprintf(stderr, __VA_ARGS__);
#else
#define log(...)
#endif

void InternedString::releaseBuffer() {
  // If we have a buffer
  if (_buffer) {
    // Decrement reference count
    _buffer->refCount--;
    assert(_buffer->refCount >= 0);
    log("DEC: to %i of '%s'\n", _buffer->refCount, _buffer->payload.data());

    // If there are no more references
    if (_buffer->refCount == 0) {
      // Erase from owners cache, if owner is still alive
      if (_buffer->owner) {
        size_t count = _buffer->owner->_cache.erase(_buffer->payload.data());
        UNUSED(count);
        assert(count == 1);
      }
      log("DEL:         '%s'\n", _buffer->payload.data());
      // Free buffer
      delete _buffer;
    }

    // Remove pointer to buffer
    _buffer = nullptr;
  }
}

const char* InternedString::_emptyString = "";

InternedString InternedStringContext::createString(const char* s) {
  // Empty InternedStrings are a special case
  if(strlen(s) == 0) {
    return InternedString();
  }

  // Find buffer
  InternedString::Buffer* buf = nullptr;
  auto it = _cache.find(s);

  // If a buffer doesn't exist create a new buffer
  if (it == _cache.end()) {
    buf = new InternedString::Buffer(s, this);
    _cache.insert(std::make_pair<const char*, InternedString::Buffer*>(
                  buf->payload.data(), (InternedString::Buffer*)buf));
    assert(buf->refCount == 1);
  } else {
    buf = it->second;
    buf->refCount++;
  }
  assert(buf);
  assert(buf->payload == s);
  return InternedString(buf);
}


InternedStringContext::~InternedStringContext() {
  for(auto item : _cache) {
    assert(item.second->owner == this);
    item.second->owner = nullptr;
  }
  _cache.clear();
}