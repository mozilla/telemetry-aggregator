#ifndef INTERNED_STRING_H
#define INTERNED_STRING_H

#include <string.h>

#include <string>
#include <unordered_map>

class InternedStringContext;

/**
 * Interned immutable string, used to reduce memory allocations when dealing
 * with a lot of instances of the same string.
 */
class InternedString {
  /** Buffer storing the contents of an interned string */
  struct Buffer {
    Buffer(const char* s, InternedStringContext* owner)
     : refCount(1), payload(s), owner(owner) {}
    size_t                  refCount;
    std::string             payload;
    InternedStringContext*  owner;
  };

  /** Internal Buffer, nullptr, for empty strings */
  Buffer* _buffer;

  /**
   * Initialized InternedString from buffer
   * Note, buffers are always allocated by instances of InternedStringContext.
   */
  explicit InternedString(Buffer* buffer)
    : _buffer(buffer) {}

  /** Release current buffer, decrementing refCount and freeing it if needed */
  void releaseBuffer();

  /** Empty string to return when _buffer is null */
  static const char* _emptyString;
public:
  /** Initialized empty InternedString */
  InternedString()
   : _buffer(nullptr) {}

  /** Copy-construct InternedString */
  InternedString(const InternedString& s) {
    _buffer = s._buffer;
    if(_buffer) {
      _buffer->refCount++;
    }
  }

  /** Assignment operator */
  InternedString& operator= (const InternedString& s) {
    releaseBuffer();
    _buffer = s._buffer;
    if(_buffer) {
      _buffer->refCount++;
    }
    return *this;
  }

  /** Compare two InternedStrings */
  bool operator==(const InternedString& s) const {
    if(_buffer && s._buffer) {
      if(_buffer->owner != s._buffer->owner || !_buffer->owner) {
        return _buffer->payload == s._buffer->payload;
      }
    }
    return _buffer == s._buffer;
  }

  /** Compare to C string */
  bool operator==(const char* s) const {
    if(!_buffer) {
      return *s == '\0';
    }
    return _buffer->payload == s;
  }

  /** Compare to std::string */
  bool operator==(const std::string& s) const {
    if(!_buffer) {
      return s.empty();
    }
    return _buffer->payload == s;
  }

  /** Compare strings */
  bool operator<(const char* s) const {
    if(!_buffer) {
      return *s != '\0';
    }
    return _buffer->payload < s;
  }

  /** Get string as const char* */
  const char* data() const {
    if (_buffer)
      return _buffer->payload.data();
    return _emptyString;
  }

  /** Write to FILE */
  void output(FILE* f) {
    if(_buffer) {
      fputs(_buffer->payload.data(), f);
    }
  }

  /** Check if this is an empty string */
  bool empty() {
    return _buffer;
  }

  /** Destroy and deref InternedScript */
  ~InternedString() {
    releaseBuffer();
  }

  friend class InternedStringContext;
};

/** Context that owns a collection of interned strings */
class InternedStringContext {
  /** Hash for C strings */
  struct StrHash {
    size_t operator()(const char* s) const {
      size_t hash = 0;
      while(*s != '\0') {
        hash = (hash << 6) ^ *(s++);
      }
      return hash;
    }
  };

  /** Comparison operator for C strings */
  struct StrCmp {
    bool operator()(const char* s1, const char* s2) const {
      return strcmp(s1, s2) == 0;
    }
  };

  /** Buffer Cache type */
  typedef std::unordered_map<const char*, InternedString::Buffer*, StrHash, StrCmp>
                                                                    BufferCache;

  /** Buffer Cache */
  BufferCache _cache;
public:
  /** Create new interned string from C string */
  InternedString createString(const char* s);

  /** Create new interned string from const char* and string length */
  InternedString createString(const char* s, size_t n);

  /** Create new interned string from std::string */
  InternedString createString(const std::string& s) {
    return createString(s.data());
  }

  /** Free InternedStringContext and freeing buffers when they are deleted */
  ~InternedStringContext();

  friend class InternedString;
};

#endif // INTERNED_STRING_H
