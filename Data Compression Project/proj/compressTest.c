/** 
  @file compressTest.c
  @author Avery Murray
  Unit tests for the compress.c component.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "compress.h"

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
   Top-level function for compressTest.c the unit test.
   @return exit status of the program, success if all tests pass.
*/
int main()
{
  // As you finish parts of your implementation, move this directive
  // down past the blocks of code below.  That will enable tests of
  // various functions you're expected to implement.
  

  
  ////////////////////////////////////////////////////////////
  // Test(s) for the serializeBlock() function.
  
  {
    // A block containing two rules.
    Block block = { .data = { 0x41, 0x41, 0x41, 0x42, 0x42, 0x42 },
      .len = 6,
      .rlist = { { .code = 0x41, .first = 0x61, .second = 0x62 },
                 { .code = 0x42, .first = 0x63, .second = 0x64 } },
      .rcount = 2 };
                 
    byte expected[] = {
      0x06,
      0x00, 0x41, 0x41, 0x41, 0x42, 0x42, 0x42,
      0x02,
      0x41, 0x61, 0x62,
      0x42, 0x63, 0x64
    };

    Buffer expectedBuf = {.data = expected,
                  .len = 15,
                  .cap = 20,
                  .pos = 0};

    Buffer *dest = makeBuffer();
    serializeBlock( &block, dest );

    // Check the length of the result.
    TestCase( dest->len == expectedBuf.len );

    // Check the data of the result.
    TestCase ( compareBytes( dest->data, expectedBuf.data, expectedBuf.len) );

    // Check the cap of the expected block. 
    TestCase( dest->cap == expectedBuf.cap );

    // Check the pos of the result. 
    TestCase( dest->pos == expectedBuf.pos );

    // Check the contents of the serialized block.

    freeBuffer( dest );

  }

    ////////////////////////////////////////////////////////////
  // Test(s) for the deserializeBlock() function.
  {
    byte data1[] = {0x06, 0x00, 0x41, 0x41, 0x41, 0x42, 0x42, 0x42,
                           0x02, 0x41, 0x61, 0x62, 0x42, 0x63, 0x64};
    Buffer buf1 = {.data = data1,
                  .len = 15,
                  .cap = 15,
                  .pos = 0};

    // A block containing the expected two rules.
    Block expected1 = { .data = { 0x41, 0x41, 0x41, 0x42, 0x42, 0x42 },
      .len = 6,
      .rlist = { { .code = 0x41, .first = 0x61, .second = 0x62 },
                 { .code = 0x42, .first = 0x63, .second = 0x64 } },
      .rcount = 2 };

    Block *dest1 = (Block *) malloc(sizeof(Block));
    TestCase( deserializeBlock( dest1, &buf1 ) );

    // Check the length of the result.
    TestCase( dest1->len == expected1.len );

    // Check the data of the result
    TestCase ( compareBytes(dest1->data, expected1.data, expected1.len));

    // Check the rcount of the result 
    TestCase( dest1->rcount == expected1.rcount );
    
    // Check the rlist values
    for ( int i = 0; i < expected1.rcount; i++ ) {
      TestCase( dest1->rlist[i].code == expected1.rlist[i].code );
      TestCase( dest1->rlist[i].first == expected1.rlist[i].first );
      TestCase( dest1->rlist[i].second == expected1.rlist[i].second );
    }

    TestCase ( buf1.pos == buf1.len );

    free( dest1 );

    // Test cases for false deserialize blocks
    // Empty length
    byte data2[] = {};
    Buffer buf2 = {.data = data2,
                  .len = 0,
                  .cap = 15,
                  .pos = 0};

    Block *dest2 = (Block *) malloc(sizeof(Block));
    TestCase( !deserializeBlock( dest2, &buf2 ) );

    free( dest2 ); 

    // Empty data
    byte data3[] = {0x07, 0x00, 0x01};
    Buffer buf3 = {.data = data3,
                  .len = 7,
                  .cap = 15,
                  .pos = 0};

    Block *dest3 = (Block *) malloc(sizeof(Block));
    TestCase( !deserializeBlock( dest3, &buf3 ) );

    free( dest3 ); 

    // Empty rcount
    byte data4[] = {0x06, 0x00, 0x41, 0x41, 0x41, 0x42, 0x42, 0x42};
    Buffer buf4 = {.data = data4,
                  .len = 8,
                  .cap = 15,
                  .pos = 0};

    Block *dest4 = (Block *) malloc(sizeof(Block));
    TestCase( !deserializeBlock( dest4, &buf4 ) );

    free( dest4 ); 

    // Empty rdata code
    byte data5[] = {0x06, 0x00, 0x41, 0x41, 0x41, 0x42, 0x42, 0x42,
                    0x02};
    Buffer buf5 = {.data = data5,
                  .len = 9,
                  .cap = 10,
                  .pos = 0};

    Block *dest5 = (Block *) malloc(sizeof(Block));
    TestCase( !deserializeBlock( dest5, &buf5 ) );

    free( dest5 ); 

    // Empty rdata first
    byte data6[] = {0x06, 0x00, 0x41, 0x41, 0x41, 0x42, 0x42, 0x42,
                    0x02, 0xFB};
    Buffer buf6 = {.data = data6,
                  .len = 10,
                  .cap = 10,
                  .pos = 0};

    Block *dest6 = (Block *) malloc(sizeof(Block));
    TestCase( !deserializeBlock( dest6, &buf6 ) );

    free( dest6 );

    // Empty rdata second
    byte data7[] = {0x06, 0x00, 0x41, 0x41, 0x41, 0x42, 0x42, 0x42,
                    0x02, 0xFB, 0xAA};
    Buffer buf7 = {.data = data7,
                  .len = 11,
                  .cap = 10,
                  .pos = 0};

    Block *dest7 = (Block *) malloc(sizeof(Block));
    TestCase( !deserializeBlock( dest7, &buf7 ) );

    free( dest7 );

  }


  ////////////////////////////////////////////////////////////
  // Test(s) for the compressBlock() function.
  
  {
    // This is the same example block from the explanation of byte-pair encoding.
    Block block = { .data = {
        'C', 'D', 'A', 'B', 'A', 'B', 'A', 'B', 'C', 'D', 'A', 'B',
        'C', 'D', 'A', 'B', 'C', 'D' },
      .len = 18,
      .rcount = 0 };
                 
    byte expected[] = {
      0x01, 0x00, 0x00, 0x02, 0x02, 0x02
    };

    compressBlock( &block );

    // Make sure the block compresses to the right sequence of bytes.
    TestCase( block.len == 6 );
    TestCase( compareBytes( block.data, expected, sizeof( expected ) ) );
    
    // Check the rules used to compress the block.
    TestCase( block.rcount == 3 );
    TestCase( block.rlist[ 0 ].code == 0x00 && block.rlist[ 0 ].first == 'A' &&
              block.rlist[ 0 ].second == 'B' );
    TestCase( block.rlist[ 1 ].code == 0x01 && block.rlist[ 1 ].first == 'C' &&
              block.rlist[ 1 ].second == 'D' );
    TestCase( block.rlist[ 2 ].code == 0x02 && block.rlist[ 2 ].first == 0x00 &&
              block.rlist[ 2 ].second == 0x01 );
  }

  ////////////////////////////////////////////////////////////
  // Test(s) for the uncompressData() function.
 {
    // This is the same example block from the explanation of byte-pair encoding.
    Block blockExpected = { .data = {
        'C', 'D', 'A', 'B', 'A', 'B', 'A', 'B', 'C', 'D', 'A', 'B',
        'C', 'D', 'A', 'B', 'C', 'D' },
      .len = 18,
      .rcount = 0 };

    Replacement r0 = {.code = 0x00, .first = 'A', .second = 'B'};
    Replacement r1 = {.code = 0x01, .first = 'C', .second = 'D'};
    Replacement r2 = {.code = 0x02, .first = 0x00, .second = 0x01};
        
    Block test = { .data = {
        0x01, 0x00, 0x00, 0x02, 0x02, 0x02 },
      .len = 6,
      .rcount = 3,
      .rlist = { r0, r1, r2 } };

    uncompressBlock( &test );

    // Make sure the block compresses to the right sequence of bytes.
    TestCase( test.len == blockExpected.len );
    TestCase( compareBytes( test.data, blockExpected.data, sizeof( blockExpected.len ) ) );
    
  }

  ////////////////////////////////////////////////////////////
  // Test(s) for the compressData() function.
  
  {
    // Test based on the example from the Philip Gage paper (with
    // no null terminator at the end).
    byte srcBytes[ 9 ] = "ABABCABCD";
    Buffer *src = makeBuffer();
    appendBytes( src, srcBytes, sizeof( srcBytes ) );
    
    byte expected[] = {
      0x06, 0x00,                           // Block size
      0x00, 0x00, 0x43, 0x00, 0x43, 0x44,   // Block contents
      0x01,                                 // Number of rules
      0x00, 0x41, 0x42                      // Replacement rule
    };

    Buffer *dest = compressData( src );
    TestCase( dest != NULL );

    // Check the size then contents of the results.
    TestCase( dest->len == sizeof( expected ) );
    TestCase( compareBytes( dest->data, expected, sizeof( expected ) ) );

    freeBuffer( src );
    freeBuffer( dest );
  }

  {
    // This 16-byte input should make three replaceent rules.
    byte srcBytes[ 16 ] = "AAAAAAAAAAAAAAAA";
    Buffer *src = makeBuffer();
    appendBytes( src, srcBytes, sizeof( srcBytes ) );
    
    byte expected[] = {
      0x02, 0x00,
      0x02, 0x02,
      0x03,
      0x00, 0x41, 0x41,
      0x01, 0x00, 0x00,
      0x02, 0x01, 0x01
    };

    Buffer *dest = compressData( src );
    TestCase( dest != NULL );

    // Check size and then contents of the result.
    TestCase( dest->len == sizeof( expected ) );
    TestCase( compareBytes( dest->data, expected, sizeof( expected ) ) );

    freeBuffer( src );
    freeBuffer( dest );
  }

  {
    // Maximum possible block size, filled with zeros.
    byte srcBytes[ 16384 ];
    for ( int i = 0; i < sizeof( srcBytes ); i++ )
      srcBytes[ i ] = 0;
    Buffer *src = makeBuffer();
    appendBytes( src, srcBytes, sizeof( srcBytes ) );
    
    byte expected[] = {
      0x02, 0x00,
      0x0D, 0x0D,
      0x0D,
      0x01, 0x00, 0x00,
      0x02, 0x01, 0x01,
      0x03, 0x02, 0x02,
      0x04, 0x03, 0x03,
      0x05, 0x04, 0x04,
      0x06, 0x05, 0x05,
      0x07, 0x06, 0x06,
      0x08, 0x07, 0x07,
      0x09, 0x08, 0x08,
      0x0A, 0x09, 0x09,
      0x0B, 0x0A, 0x0A,
      0x0C, 0x0B, 0x0B,
      0x0D, 0x0C, 0x0C
    };

    Buffer *dest = compressData( src );
    TestCase( dest != NULL );

    // Check size and then contents of the result.
    TestCase( dest->len == sizeof( expected ) );
    TestCase( compareBytes( dest->data, expected, sizeof( expected ) ) );

    freeBuffer( src );
    freeBuffer( dest );
  }

  ////////////////////////////////////////////////////////////
  // Test(s) for the uncompressBlock() function.
  {
    byte srcBytes[] = {0x06, 0x00, 0x00, 0x00, 0x43, 0x00, 0x43, 0x44, 0x01, 0x00, 0x41, 0x42};
    Buffer *src = makeBuffer();
    appendBytes( src, srcBytes, sizeof( srcBytes ) );
    
    byte expected[] = {'A','B','A','B','C','A','B','C','D'};

    Buffer *dest = uncompressData( src );
    TestCase( dest != NULL );

    // Check the size then contents of the results.
    TestCase( dest->len == sizeof( expected ) );
    TestCase( compareBytes( dest->data, expected, sizeof( expected ) ) );

    freeBuffer( src );
    freeBuffer( dest );
  }
  
  // Once you move the #ifdef DISABLE_TESTS to here, you've enabled
  // all the tests.
  // Buffer *uncompressData(Buffer *src)
  //#endif

  // Report how many tests are passing.
  printf( "*** Passed %d of %d unit tests for compress.c.\n", passedTests,
          totalTests );

  // Exit successfully if all tests tests were passed.
  if ( passedTests == totalTests )
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}

