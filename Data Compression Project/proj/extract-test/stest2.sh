#!/bin/bash

# This shell program runs system tests for viewing and extracting files from an archive.

# Assume we've succeeded until we see otherwise.
FAIL=0

# Print an error message and set the fail flag.
fail() {
    echo "**** $1"
    FAIL=1
}

# Check the exit status.  The actual exit status (ASTATUS) should match
# the expected status (ESTATUS)
checkStatus() {
    ESTATUS="$1"
    ASTATUS="$2"

    # Make sure the program exited with the right exit status.
    if [ "$ASTATUS" -ne "$ESTATUS" ]; then
	fail "FAILED - incorrect program exit status. (expected $ESTATUS,  Got: $ASTATUS)"
	return 1
    fi

    return 0
}

# Check the contents of an a file.  If the expected file (EFILE)
# exists, then the actual file (AFILE) should exist and it should match.
checkFile() {
    NAME="$1"
    EFILE="$2"
    AFILE="$3"

    # Make sure the expected output file we're looking for is really there.
    if [ ! -f "$EFILE" ]; then
	echo "Missing expected output file: $EFILE"
    fi

    # Make sure the output matches the expected output.
    echo "   diff $EFILE $AFILE"
    diff -q "$EFILE" "$AFILE" >/dev/null 2>&1
    if [ $? -ne 0 ]; then
	fail "FAILED - $NAME ($AFILE) doesn't match expected ($EFILE)"
	return 1
    fi

    return 0
}

# Same as checkFile, but if the expected file (EFILE) doesn't exist, the
# actual file (AFILE) should be empty.
checkFileOrEmpty() {
    NAME="$1"
    EFILE="$2"
    AFILE="$3"

    # if the expected output file doesn't exist, the actual file should be
    # empty.
    if [ ! -f "$EFILE" ]; then
	if [ -s "$AFILE" ]; then
	    fail "FAILED - $NAME ($AFILE) should be empty."
	    return 1
	fi
	return 0
    fi

    # Make sure the output matches the expected output.
    echo "   diff $EFILE $AFILE"
    diff -q "$EFILE" "$AFILE" >/dev/null 2>&1
    if [ $? -ne 0 ]; then
	fail "FAILED - $NAME ($AFILE) doesn't match expected ($EFILE)"
	return 1
    fi

    return 0
}

# The given file should exist but should be empty.
checkEmpty() {
    NAME="$1"
    AFILE="$2"
    
    if [ -s "$AFILE" ]; then
	fail "FAILED - $NAME ($AFILE) should be empty."
	return 1
    fi

    return 0
}

# Test extracting files running in interactive mode.
userInputTest() {
    TESTNO="$1"
    ESTATUS="$2"
    shift
    shift

    echo "Running test $TESTNO"

    if [ "$#" -gt 0 ]; then
	echo "   # Removing old files before attempting to extract files with the same names"
	echo "   rm -f $*"
	rm -f $*
    fi

    echo "   ../snark ${args[@]} < user-input-$TESTNO.txt > stdout.txt 2> stderr.txt"
    ../snark ${args[@]} < user-input-$TESTNO.txt > stdout.txt 2> stderr.txt
    ASTATUS=$?
    if ! checkStatus "$ESTATUS" "$ASTATUS" ||
       ! checkFile "Terminal output" "expected-stdout-$TESTNO.txt" "stdout.txt" ||
       ! checkFileOrEmpty "Stderr output" "expected-stderr-$TESTNO.txt" "stderr.txt"
    then
	return 1
    fi

    # Check extracted files against the copy in the create-test directory.
    while [ "$#" -gt 0 ]; do
	if ! checkFile "Extracted file $1" "../create-test/$1" "$1"
	then
	    return 1
	fi

	shift
    done

    echo "Test $TESTNO PASSED"
    return 0
}

# Test extracting files running in script mode
scriptInputTest() {
    TESTNO="$1"
    ESTATUS="$2"
    shift
    shift

    echo "Running test $TESTNO"

    if [ "$#" -gt 0 ]; then
	echo "   # Removing old files before attempting to extract files with the same names"
	echo "   rm -f $*"
	rm -f $*
    fi

    echo "   ../snark ${args[@]} -s script-input-$TESTNO.txt > stdout.txt 2> stderr.txt"
    ../snark ${args[@]} -s script-input-$TESTNO.txt > stdout.txt 2> stderr.txt
    ASTATUS=$?
    if ! checkStatus "$ESTATUS" "$ASTATUS" ||
       ! checkFile "Terminal output" "expected-stdout-$TESTNO.txt" "stdout.txt" ||
       ! checkFileOrEmpty "Stderr output" "expected-stderr-$TESTNO.txt" "stderr.txt"
    then
	return 1
    fi

    # Check extracted files against the copy in the create-test directory.
    while [ "$#" -gt 0 ]; do
	if ! checkFile "Extracted file $1" "../create-test/$1" "$1"
	then
	    return 1
	fi

	shift
    done

    echo "Test $TESTNO PASSED"
    return 0
}

# Go up a directory and build the project.
dir="$(pwd)"
cd ..
make clean
make
cd "$dir"

if [ $? -ne 0 ]; then
    fail "Make exited unsuccessfully"
fi

# Run test cases
if [ -x ../snark ]; then
    args=(-a arc-13.snk)
    userInputTest 13 0
    
    args=(-a arc-14.snk)
    userInputTest 14 0
    
    args=(-a arc-15.snk)
    userInputTest 15 0

    args=(-a arc-16.snk)
    userInputTest 16 0 alphabet.txt
    
    args=(-a arc-17.snk)
    userInputTest 17 0 preamble.txt
    
    args=(-a arc-18.snk)
    scriptInputTest 18 0 10K.bin
    
    args=(-a arc-19.snk)
    userInputTest 19 0 hamlet.txt
    
    args=(-a arc-20.snk -a arc-20.snk)
    userInputTest 20 1
    
    args=(-a missing-archive.snk)
    userInputTest 21 1
    
    args=(-a bad-arc-22.snk)
    userInputTest 22 1
    
else
    fail "Since your program didn't compile, no tests were run."
fi

if [ $FAIL -ne 0 ]; then
  echo "FAILING TESTS!"
  exit 13
else 
  echo "TESTS SUCCESSFUL"
  exit 0
fi
