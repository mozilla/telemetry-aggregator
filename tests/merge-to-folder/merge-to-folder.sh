#!/bin/bash -ev

# Create output folder
rm -rf output/;
mkdir -p output/;

# Merge test files
../../mergeresults -i result.txt -f output/;

# Check number of output folders
test `ls output/ | wc -l` -eq 1;

# Check number of output files
test `ls output/aurora/ | wc -l` -eq 2;

# Check that we have 4 lines
test `cat output/aurora/24 | wc -l` -eq 4;
test `cat output/aurora/25 | wc -l` -eq 4;

# Check that we have 10 in all lines
test `cat output/aurora/24 | grep 10 | wc -l` -eq 4;
test `cat output/aurora/25 | grep 10 | wc -l` -eq 4;

# Merge more into the same files
../../mergeresults -i result.txt -f output/;

# Check number of output files
test `ls output/aurora/ | wc -l` -eq 2;

# Check that we have 4 lines
test `cat output/aurora/24 | wc -l` -eq 4;
test `cat output/aurora/25 | wc -l` -eq 4;

