#!/usr/bin/env bash

# Remove coverage information from previous execution.
rm -f *.gcda *.gcno *.gcda

# Remove any old object files.
rm -f compressTest.o compress.o buffer.o

# Building with this script rather than the Makefile so we can use
# different compler flags.
echo "Building unit test program"
set -x
gcc -Wall -std=c99 -g -c buffer.c
gcc -Wall -std=c99 -g -ftest-coverage -fprofile-arcs -c compress.c
gcc -Wall -std=c99 -g -c compressTest.c
gcc -ftest-coverage -fprofile-arcs buffer.o compress.o compressTest.o -o compressTest
set +x

if [ ! -x compressTest ]; then
    echo "**** Unit test program didn't build successfully."
    exit 1
fi

echo "Running unit test program"
./compressTest
STATUS=$?

if [ $STATUS -ne 0 ]; then
    echo "**** Unit test didn't run successfully."
fi

echo "Generating coverage report"
gcov compress


