
#include "../../src/CompressedFileReader.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char* argv[]) {
  // Test XZ reading
  {
    FILE* input = fopen("a-b-c-def-g.txt.xz", "r");

    CompressedFileReader reader(input);
    char* line;

    // 1. line: "a"
    line = reader.nextLine();
    assert(line);
    assert(strcmp(line, "a") == 0);

    // 2. line: "b"
    line = reader.nextLine();
    assert(line);
    assert(strcmp(line, "b") == 0);

    // 3. line: "c"
    line = reader.nextLine();
    assert(line);
    assert(strcmp(line, "c") == 0);

    // 4. line: "def"
    line = reader.nextLine();
    assert(line);
    assert(strcmp(line, "def") == 0);

    // 5. line: "g"
    line = reader.nextLine();
    assert(line);
    assert(strcmp(line, "g") == 0);

    // End of input
    line = reader.nextLine();
    assert(!line);

    fclose(input);
  }

  // Test lzma reading (legacy only)
  {
    FILE* input = fopen("a-b-c-def-g.txt.lzma", "r");

    CompressedFileReader reader(input);
    char* line;

    // 1. line: "a"
    line = reader.nextLine();
    assert(line);
    assert(strcmp(line, "a") == 0);

    // 2. line: "b"
    line = reader.nextLine();
    assert(line);
    assert(strcmp(line, "b") == 0);

    // 3. line: "c"
    line = reader.nextLine();
    assert(line);
    assert(strcmp(line, "c") == 0);

    // 4. line: "def"
    line = reader.nextLine();
    assert(line);
    assert(strcmp(line, "def") == 0);

    // 5. line: "g"
    line = reader.nextLine();
    assert(line);
    assert(strcmp(line, "g") == 0);

    // End of input
    line = reader.nextLine();
    assert(!line);

    fclose(input);
  }

  return 0;
}

