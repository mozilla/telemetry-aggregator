#!/bin/bash -ex

echo "### Setting up test environment";

# Create test-folders
mkdir -p input;

# Copy in test files
cp ss-ff-n-22.lzma input/ss-ff-n-22.lzma;
cp ss-ff-n-28.lzma input/ss-ff-n-28.lzma;

# Run tests
cat input.txt | ../../aggregator -o result.txt;

# Test we have output from both version/channels
test `cat result.txt | cut -f 1 | cut -d / -f 1,2 | uniq | wc -l` -eq "2";

# Test that input files are deleted
test `ls input/ | wc -l` -eq "0";
