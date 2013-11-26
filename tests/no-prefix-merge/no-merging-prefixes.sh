#!/bin/bash -ev

# Merge test files
../../mergeresults -i result.txt -o output.txt;

# Check number of lines
test `cat output.txt | wc -l` -eq 4;
