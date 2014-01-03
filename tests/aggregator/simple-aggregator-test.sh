#!/bin/bash -ex

echo "### Setting up test environment";

# Create test-folders
rm -rf input;
mkdir -p input;

# Copy in test files
cp ss-ff-n-22-simple.lzma input/ss-ff-n-22-simple.lzma;

# Run tests
cat simple-input.txt | ../../aggregator -o result.txt;

# Test we have output from both version/channels
cat result.txt | ./test-simple.py
