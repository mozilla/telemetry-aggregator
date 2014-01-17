#ifndef COMPRESSEDFILEREADER_H
#define COMPRESSEDFILEREADER_H

#include <stdio.h>
#include <lzma.h>

/** Read compressed files line by line */
class CompressedFileReader {
  /** Input file object */
  FILE*         _input;

  /** lzma decoder stream */
  lzma_stream*  _stream;

  /** Streaming action to take place next */
  lzma_action   _action;

  /** Input buffer, buffering data from input file to decoder */
  uint8_t*      _inbuf;

  /** Output buffer, returned when a line is read */
  uint8_t*      _outbuf;

  /** Size of bytes allocated for _outbuf, doubled when more space is needed */
  size_t        _size;

  /** Position where next line starts, _outbuf, if no line available */
  uint8_t*      _nextLine;
public:
  /** Create a compressed file reader */
  CompressedFileReader(FILE* input);

  /**
   * Get next line, null, if at end of stream or error, errors are also printed
   * to stderr. The returned pointer is valid until next invocation or
   * destruction of the CompressedFileReader.
   */
  char* nextLine();

  /** Destroy compressed file reader, freeing all allocated memory */
  ~CompressedFileReader();
};


#endif // COMPRESSEDFILEREADER_H