#!/bin/bash -ex

# Check for memory leaks with valgrind
valgrind --error-exitcode=1 --leak-check=full ../../mergeresults -i result.txt > /dev/null;