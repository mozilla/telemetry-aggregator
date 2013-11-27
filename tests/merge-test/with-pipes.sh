#!/bin/bash -ev

# Merge test files
cat result-1.txt result-2.txt | ../../mergeresults > output.txt;

# Check number of lines
test `cat output.txt | wc -l` -eq 1;

# Check that we have 15 in there
test `cat output.txt | grep 15 | wc -l` -eq 1;
