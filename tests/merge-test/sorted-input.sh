#!/bin/bash -ev

# Merge test files
cat unsorted-results.txt | sort -t $'\t' -k 1 | ../../mergeresults -s > output.txt;

# Check number of lines
test `cat output.txt | wc -l` -eq 4;

# Merge test files
cat unsorted-results.txt | ../../mergeresults -s > output.txt;

# Check number of lines, as we processed without sorting first and gave the
# -s (sort) argument we should have more than four
test `cat output.txt | wc -l` -gt 4;
