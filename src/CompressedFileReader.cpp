#include "CompressedFileReader.h"

#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define INBUF_SIZE      (1024 * 1024)

CompressedFileReader::CompressedFileReader(FILE* input)
 : _input(input), _stream(nullptr), _inbuf(nullptr), _outbuf(nullptr),
   _size(1024 * 1024), _nextLine(nullptr) {
  // Allocate buffers
  _inbuf  = new uint8_t[INBUF_SIZE];
  _outbuf = (uint8_t*)malloc(_size);
  _nextLine = _outbuf;

  if (!input) {
    _stream = nullptr;
    fprintf(stderr, "CompressedFileReader: Input FILE* is NULL\n");
    return;
  }

  // Allocated an initialize stream
  _stream = new lzma_stream;
  *_stream = LZMA_STREAM_INIT;

  // Initialized decoding stream
  lzma_ret ret = lzma_auto_decoder(_stream, UINT64_MAX, LZMA_CONCATENATED);
  if (ret != LZMA_OK) {
    const char* msg;
    switch (ret) {
      case LZMA_MEM_ERROR:
        msg = "unable to allocate memory";
        break;
      case LZMA_OPTIONS_ERROR:
        msg = "invalid options";
        break;
      case LZMA_PROG_ERROR:
        msg = "unknown error";
        break;
      default:
        assert(false);
        msg = "invalid error code";
        break;
    }

    // Print error message
    fprintf(
      stderr,
      "CompressedFileReader: lzma_auto_decoder() failed, %s\n",
      msg
    );

    // Delete stream
    delete _stream;
    _stream = nullptr;

    return;
  }

  // Setup stream
  _stream->next_in    = NULL;
  _stream->avail_in   = 0;
  _stream->next_out   = _outbuf;
  _stream->avail_out  = _size;
}

char* CompressedFileReader::nextLine() {
  // If there is no decoder stream, then we're either done, or there was an
  // error somewhere in the process
  if (!_stream) {
    return nullptr;
  }

  lzma_action action = LZMA_RUN;

  // Bring us to a state where: _outbuf == _nextLine
  // do this by moving memory between _nextLine and _stream->next_out
  // to begin at _outbuf (update _nextLine = _outbuf, _stream->next_out =...)
  assert(_stream->next_out - _nextLine >= 0);
  memmove(_outbuf, _nextLine, _stream->next_out - _nextLine);
  _stream->next_out   -= _nextLine - _outbuf;
  _stream->avail_out  += _nextLine - _outbuf;
  _nextLine = _outbuf;
  assert(_stream->next_out >= _outbuf);

  // Optional optimization:
  // Search for line breaks in interval from _outbuf, to _stream->next_out,
  // if found, update _nextLine, flip '\n' to '\0' and return... This should
  // give slightly better locality
  assert(_nextLine == _outbuf);
  while (_nextLine < _stream->next_out) {
    if (*_nextLine == '\n') {
      *_nextLine = '\0';
      _nextLine += 1;
      return (char*)_outbuf;
    }
    _nextLine++;
  }
  // Okay, so there is no _nextLine, yet... we'll keep moving it forward though

  // Read until we reach a line break
  while (true) {
    // If there no available input, read from file
    if (_stream->avail_in == 0 && !feof(_input)) {
      _stream->next_in = _inbuf;
      _stream->avail_in = fread(_inbuf, 1, INBUF_SIZE, _input);

      // if there is an error
      if (ferror(_input)) {
        fprintf(
          stderr,
          "CompressedFileReader: fread() failed, %s\n",
          strerror(errno)
        );
        // Free streaming context, we can't continue after an error
        lzma_end(_stream);
        delete _stream;
        _stream = nullptr;

        return nullptr;
      }

      // if at end of file, finish decoding, flushing buffers
      if (feof(_input)) {
        action = LZMA_FINISH;
      }
    }

    // Decode LZMA stream
    lzma_ret ret = lzma_code(_stream, action);

    // If there is no more output buffer space, or we're at the end of the
    // stream, search for line breaks and return, if any is found
    if (_stream->avail_out == 0 || ret == LZMA_STREAM_END) {
      // Find next line and return it, if there is any...
      while (_nextLine < _stream->next_out) {
        if (*_nextLine == '\n') {
          *_nextLine = '\0';
          _nextLine += 1;
          return (char*)_outbuf;
        }
        _nextLine++;
      }

      // If there no more available space, allocate some
      if (_stream->avail_out == 0) {
        // Realloc _outbuf, note that we should have filled _outbuf first
        assert(_stream->next_out == _nextLine);
        assert((_nextLine - _outbuf) == (int)_size);

        // Double size, to get a nice amortized complexity, no we don't bother
        // scaling down the allocation
        _outbuf = (uint8_t*)realloc(_outbuf, _size * 2);

        // Update _nextLine, _stream->next_out and _stream->avail_out
        _nextLine           = _outbuf + _size;
        _stream->next_out   = _outbuf + _size;
        _stream->avail_out  = _size;

        // Store the updated size
        _size *= 2;
      }

      // If we're at the end of the input
      if (ret == LZMA_STREAM_END) {
        // End and release the stream
        lzma_end(_stream);
        delete _stream;
        _stream = nullptr;

        // If there is any input left, return it
        if (_nextLine != _outbuf) {
          *_nextLine = '\0';
          return (char*)_outbuf;
        }

        // If no input left, return done
        return nullptr;
      }
    }

    // Handle non-OK return code
    if (ret != LZMA_OK) {
      const char* msg;
      switch (ret) {
        case LZMA_MEM_ERROR:
          msg = "unable to allocate memory";
          break;
        case LZMA_FORMAT_ERROR:
          msg = "not valid xz/lzma input format";
          break;
        case LZMA_BUF_ERROR:
        case LZMA_DATA_ERROR:
          msg = "Compressed data file is corrupt";
          break;
        case LZMA_OPTIONS_ERROR:
          msg = "invalid options";
          break;
        case LZMA_PROG_ERROR:
          msg = "unknown error";
          break;
        default:
          assert(false);
          msg = "invalid error code";
          break;
      }

      // Print error message
      fprintf(
        stderr,
        "CompressedFileReader: lzma_code() failed, %s\n",
        msg
      );

      // End and release the stream
      lzma_end(_stream);
      delete _stream;
      _stream = nullptr;

      return nullptr;
    }
  }
}

CompressedFileReader::~CompressedFileReader() {
  // Free stream if not at end
  if (_stream) {
    lzma_end(_stream);
    delete _stream;
    _stream = nullptr;
  }

  // Release other buffers
  free(_outbuf);
  delete[] _inbuf;
}