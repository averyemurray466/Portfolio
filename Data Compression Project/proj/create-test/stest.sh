#!/bin/bash
# this is a system test script for creating archives, adding files to
# them and saving the archive.  

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

# Test with input redirected from a file
userInputTest() {
    TESTNO="$1"
    ESTATUS="$2"

    echo "Running test $TESTNO"
    echo "   ../snark ${args[@]} < user-input-$TESTNO.txt > stdout.txt 2> stderr.txt"
    ../snark ${args[@]} < user-input-$TESTNO.txt > stdout.txt 2> stderr.txt
    ASTATUS=$?
    if ! checkStatus "$ESTATUS" "$ASTATUS" ||
       ! checkFile "Terminal output" "expected-stdout-$TESTNO.txt" "stdout.txt" ||
       ! checkFileOrEmpty "Stderr output" "expected-stderr-$TESTNO.txt" "stderr.txt" 
    then
	return 1
    fi

    if [ -f "expected-arc-$TESTNO.snk" ] &&
       ! checkFile "Archive file" "expected-arc-$TESTNO.snk" "arc.snk"
    then
	return 1
    fi
    
    echo "Test $TESTNO PASSED"
    return 0
}

# Test with commands read froma script
scriptInputTest() {
    TESTNO="$1"
    ESTATUS="$2"

    echo "Running test $TESTNO"
    echo "  ../snark ${args[@]} > stdout.txt 2> stderr.txt"
    ../snark ${args[@]} > stdout.txt 2> stderr.txt
    ASTATUS=$?
    if ! checkStatus "$ESTATUS" "$ASTATUS" ||
       ! checkFile "Terminal output" "expected-stdout-$TESTNO.txt" "stdout.txt" ||
       ! checkFileOrEmpty "Stderr output" "expected-stderr-$TESTNO.txt" "stderr.txt" 
    then
	return 1
    fi

    if [ -f "expected-arc-$TESTNO.snk" ] &&
       ! checkFile "Archive file" "expected-arc-$TESTNO.snk" "arc.snk"
    then
	return 1
    fi
    
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
    args=()
    userInputTest 01 0
    
    args=()
    userInputTest 02 0
    
    args=()
    userInputTest 03 0
     
    args=()
    userInputTest 04 0
     
    args=()
    userInputTest 05 0
     
    args=()
    userInputTest 06 0

    args=(-s script-input-07.txt)
    scriptInputTest 07 0

    args=(-s script-input-08.txt)
    scriptInputTest 08 0

    args=(-s script-input-09.txt)
    scriptInputTest 09 0

    args=()
    userInputTest 10 0

    args=()
    userInputTest 11 0

    args=()
    userInputTest 12 0

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
