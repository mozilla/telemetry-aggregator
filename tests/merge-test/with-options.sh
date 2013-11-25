#!/bin/bash -e

echo "got here"

# Merge test files
../../mergeresults -i result-1.txt -i result-2.txt -o output.txt;

# Check number of lines
test `cat output.txt | wc -l` -eq 1;

# Check that we have 15 in there
test `cat output.txt | grep 15 | wc -l` -eq 1;
