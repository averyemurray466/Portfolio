/** 
    @file bufferTest.c
    @author dbsturgi
    Unit tests for the buffer component.
*/

#include <stdlib.h>
#include <stdio.h>
#include "buffer.h"

/** Total number or tests we tried. */
static int totalTests = 0;

/** Number of test cases passed. */
static int passedTests = 0;

/** Macro to terminate the program when an error occurs in the test
    code (rather than the component being tested). */
#define FAIL( msg ) {\
  fprintf( stderr, "%s\n", msg );\
  exit( EXIT_FAILURE );          \
}

/** Macro to check the condition on a test case, keep counts of
    passed/failed tests and report a message if the test fails. */
#define TestCase( conditional ) {\
  totalTests += 1; \
  if ( conditional ) { \
    passedTests += 1; \
  } else { \
    printf( "**** Failed unit test on line %d of %s\n", __LINE__, __FILE__ );    \
  } \
}

/** 
    For comparing an array of bytes.
    @param seqA One sequence of bytes to compare.
    @param seqB Another sequence of bytes to compre against seqA.
    @param len Number of bytes to compare.
    @return True if the sequences are identical.
*/
bool compareBytes( void const *seqA, void const *seqB, int len )
{
  for ( int i = 0; i < len; i++ )
    if ( *( (unsigned char const *)seqA + i ) != *( (unsigned char const *)seqB + i ) )
      return false;
  return true;
}

/**
   Top-level function for bufferTest.c the unit test.
   @return exit status of the program, success if all tests pass.
*/
int main()
{
  // As you finish parts of your implementation, move this directive
  // down past the blocks of code below.  That will enable tests of
  // various functions you're expected to implement.
  
//#ifdef DISABLE_TESTS
  
  ////////////////////////////////////////////////////////////
  // Test(s) for makeBuffer()
  
  {
    Buffer *b = makeBuffer();

    // Buffer should be empty, with a capacity of 5.
    TestCase( b->len == 0 );
    TestCase( b->cap == 5 );
    
    freeBuffer( b );
  }
  
  ////////////////////////////////////////////////////////////
  // Test(s) for appendByte()
  
  {
    Buffer *b = makeBuffer();

    appendByte( b, 'T' );
    appendByte( b, 'e' );
    appendByte( b, 's' );
    appendByte( b, 't' );

    // Check buffer size, capacity and contents.
    TestCase( b->len == 4 );
    TestCase( b->cap == 5 );
    TestCase( compareBytes( b->data, "Test", 4 ) );
    
    freeBuffer( b );
  }
  
  {
    Buffer *b = makeBuffer();

    byte arr[] = { 0x01, 0x25, 0x4B, 0x70, 0xA3, 0xE7 };
    for ( int i = 0; i < sizeof( arr ); i++ )
      appendByte( b, arr[ i ] );

    // Check buffer size, capacity and contents.
    TestCase( b->len == 6 );
    TestCase( b->cap == 10 );
    TestCase( compareBytes( b->data, arr, sizeof( arr ) ) );
    
    freeBuffer( b );
  }
  
  ////////////////////////////////////////////////////////////
  // Test(s) for appendBytes()
  
  {
    Buffer *b = makeBuffer();

    byte arr[] = { 0xB2, 0xE4, 0xDC, 0x34, 0xDF, 0x58, 0x91, 0x89,
                   0x1E, 0x8F, 0x43, 0x27, 0xEA, 0x76, 0xFB, 0x4C };
    // Append half of the array.
    appendBytes( b, arr, 8 );

    // Check buffer size, capacity and contents.
    TestCase( b->len == 8 );
    TestCase( b->cap == 10 );
    TestCase( compareBytes( b->data, arr, 8 ) );
    
    // Append the rest of the array.
    appendBytes( b, arr + 8, 8 );

    // Check buffer size, capacity and contents.
    TestCase( b->len == 16 );
    TestCase( b->cap == 20 );
    TestCase( compareBytes( b->data, arr, sizeof( arr ) ) );
    
    freeBuffer( b );
  }
  
  ////////////////////////////////////////////////////////////
  // Test(s) for extractByte()
  
  {
    Buffer *b = makeBuffer();

    byte arr[] = { 0xDB, 0x43, 0x83, 0x8F };
    appendBytes( b, arr, sizeof( arr ) );

    byte val;
    TestCase( extractByte( b, &val ) && val == 0xDB );
    TestCase( extractByte( b, &val ) && val == 0x43 );
    TestCase( extractByte( b, &val ) && val == 0x83 );
    TestCase( extractByte( b, &val ) && val == 0x8F );
    TestCase( b->pos == 4 );
    TestCase( ! extractByte( b, &val ) );
    
    freeBuffer( b );
  }
  
  ////////////////////////////////////////////////////////////
  // Test(s) for extractBytes()
  
  {
    Buffer *b = makeBuffer();

    byte arr[] = { 0x03, 0xB5, 0x14, 0x84, 0xE3, 0xE2, 0x70, 0x4F,
                   0x8C, 0x1F, 0xD8, 0xB6, 0x87, 0xD0, 0xE9, 0xB4 };
    appendBytes( b, arr, sizeof( arr ) );

    byte seq[ 8 ];
    TestCase( extractBytes( b, seq, 8 ) && compareBytes( seq, arr, 8 ) );
    TestCase( b->pos == 8 );
    TestCase( extractBytes( b, seq, 8 ) && compareBytes( seq, arr + 8, 8 ) );
    TestCase( b->pos == 16 );
    TestCase( ! extractBytes( b, seq, 8 ) );
    
    freeBuffer( b );
  }
  
  // Once you move the #ifdef DISABLE_TESTS to here, you've enabled
  // all the tests.

//#endif

  // Report how many tests are passing.
  printf( "*** Passed %d of %d unit tests for buffer.c.\n", passedTests,
          totalTests );

  // Exit successfully if all tests tests were passed.
  if ( passedTests == totalTests )
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}

