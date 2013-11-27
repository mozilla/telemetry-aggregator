
#include "../../src/CompressedFileReader.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char* argv[]) {
  // Test XZ reading
  {
    FILE* input = fopen("aaa-bbb-ccc-ddd-eee.txt.xz", "r");

    CompressedFileReader reader(input);
    char* line;

    // 1. line: "a"
    line = reader.nextLine();
    assert(line);
    assert(strlen(line) == 5373);
    while(*line != '\0') {
        assert(*(line++) == 'a');
    }

    // 2. line: "b"
    line = reader.nextLine();
    assert(line);
    assert(strlen(line) == 5373);
    while(*line != '\0') {
        assert(*(line++) == 'b');
    }

    // 3. line: "c"
    line = reader.nextLine();
    assert(line);
    assert(strlen(line) == 5373);
    while(*line != '\0') {
        assert(*(line++) == 'c');
    }

    // 4. line: "d"
    line = reader.nextLine();
    assert(line);
    assert(strlen(line) == 5373);
    while(*line != '\0') {
        assert(*(line++) == 'd');
    }

    // 5. line: "e"
    line = reader.nextLine();
    assert(line);
    assert(strlen(line) == 5373);
    while(*line != '\0') {
        assert(*(line++) == 'e');
    }

    // End of input
    line = reader.nextLine();
    assert(line == nullptr);

    fclose(input);
  }

  // Test lzma reading (legacy only)
  {
    FILE* input = fopen("aaa-bbb-ccc-ddd-eee.txt.lzma", "r");

    CompressedFileReader reader(input);
    char* line;

    // 1. line: "a"
    line = reader.nextLine();
    assert(line);
    assert(strlen(line) == 5373);
    while(*line != '\0') {
        assert(*(line++) == 'a');
    }

    // 2. line: "b"
    line = reader.nextLine();
    assert(line);
    assert(strlen(line) == 5373);
    while(*line != '\0') {
        assert(*(line++) == 'b');
    }

    // 3. line: "c"
    line = reader.nextLine();
    assert(line);
    assert(strlen(line) == 5373);
    while(*line != '\0') {
        assert(*(line++) == 'c');
    }

    // 4. line: "d"
    line = reader.nextLine();
    assert(line);
    assert(strlen(line) == 5373);
    while(*line != '\0') {
        assert(*(line++) == 'd');
    }

    // 5. line: "e"
    line = reader.nextLine();
    assert(line);
    assert(strlen(line) == 5373);
    while(*line != '\0') {
        assert(*(line++) == 'e');
    }

    // End of input
    line = reader.nextLine();
    assert(!line);

    fclose(input);
  }

  return 0;
}

