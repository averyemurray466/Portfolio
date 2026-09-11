/**
 * @file compress.h
 * @author Avery Murray (aemurra3)
 * 
 * Declares the methods and structs used in compress.h
 * to compress and decompress data in the form of
 * blocks and buffers. 
 */
#ifndef COMPRESS_H
#define COMPRESS_H

#include <stdbool.h>
#include "buffer.h"

/** Number of different codes a byte can represent. */
#define BYTE_CODES 256

/** Maximum size of a block. */
#define BLOCK_SIZE_LIMIT 16384

/** Rule for replacing a pair of bytes with a single-byte code. */
typedef struct {
  /** One-byte code we're replacing the two-byte sequence with. */
  byte code;
  
  /** Two-byte sequence we're replacing. */
  byte first, second;
} Replacement;

/** Representation for a block being compressed or decompressed. */
typedef struct {
  /** Sequence of bytes in this block. */
  byte data[ BLOCK_SIZE_LIMIT ];

  /** Length of the sequence of bytes. */
  unsigned short len;

  /** Sequence of replacement rules. */
  Replacement rlist[ BYTE_CODES - 1 ];

  /** Number of replaement rules for this block. */
  unsigned char rcount;
} Block;

#endif

/**
 * Writes the contents of the block to the buffer. 
 * Will add onto the previous contents of the buffer 
 * to ensure multiple blocks can be withing a single buffer. 
 * 
 * @param block A pointer to the Block to process
 * @param buf A pointer to the Buffer to add to
 */
void serializeBlock(Block *block, Buffer *buf);

/**
 * Writes the contents of the buffer to a block. 
 * Will continue where the previous buffer left off 
 * to support writing a buffer into multiple blocks. 
 * 
 * @param block A pointer to the Block to add to
 * @param buf A pointer to the Buffer to add to 
 */
bool deserializeBlock(Block *block, Buffer *buf);

/**
 * Substitutes 2 byte sequences that occur more than 3 times
 * to be represented with a single byte. Fills in the fields 
 * of Block by keeping track of the compressed size, number 
 * of replacement rules, and the replacement rules bytes. 
 * 
 * @param block Block to compress
 */
void compressBlock(Block *block);

/**
 * Takes the replacement codes from the given block to substitute
 * 2 bytes sequences with their given replacement code. Recalculates
 * the length of the block to account for growth from replacement.
 *  
 * @param block Block to uncompress
 */
bool uncompressBlock(Block *block);

/**
 * Compresses the uncompressed data in the source buffer to a destination
 * buffer that holds the compressed data. 
 * Calls the compressBlock function to process each block. 
 * Calls the serializeBlock function to transfer each block 
 * to the destination buffer. 
 * 
 * @param src A pointer to a Buffer to compress
 * @return A pointer to a Buffer that contains the compressed data
 */
Buffer *compressData(Buffer *src);

/**
 * Uncompresses the compressed data in the source buffer to a destination
 * buffer that holds the uncompressed data. 
 * Calls the uncompressBlock function to process each block. 
 * Calls the deserializeBlock function to transfer each block
 * to the destination buffer. 
 * 
 * @param src A pointer to a Buffer to uncompress
 * @return A pointer to a Buffer that contains uncompressed data
 */
Buffer *uncompressData(Buffer *src);