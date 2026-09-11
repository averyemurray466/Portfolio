/**
 * @file compress.c
 * @author Avery Murray (aemurra3)
 * 
 * Defines how to compress and decompress a block, 
 * how to convert buffer data to block data, and how 
 * to convert an uncompressed buffer to a compressed buffer
 * and vise versa. 
 */

#include "compress.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

/** Limit on the number of different byte values in an uncompressed block. */
#define BYTE_LIMIT 224

/** Minimum number of occurrences of a pair in order to create a rule
    to replace it. */
#define REPLACEMENT_THRESHOLD 3

/** Number of bytes for the length of a buffer. */
#define NUM_BYTES_LENGTH 2

/** Number of indexes to move forward when compressing/decompressing a byte.. */
#define FORWARD_CHECK_INDEX 2

/** Maximum number of byte codes in an uncompressed block. */
#define MAX_BYTE_CODES 224

/**
 * Constructs a 2D array to hold the counts of 
 * each byte code in the block.
 * 
 * @return hash A pointer to a integer pointer
 *         that hold the count of each byte
 */
static int **makeHash() 
{
    int **hash = (int **)malloc(sizeof(int*) * BYTE_CODES);
    for (int i = 0; i < BYTE_CODES; i++) {
        hash[i] = calloc(BYTE_CODES, sizeof(int));
    }
    return hash;
}

/**
 * Frees the memory of the hash array.
 * 
 * @param h A pointer to an integer pointer that 
 *          points to the hash array
 */
static void freeHash(int **h) 
{
    for (int i = 0; i < BYTE_CODES; i++) {
        free(h[i]);
    }
    free(h);
}

/**
 * Finds the lowest available byte to be used as a
 * replacement code in compressBlock.
 * 
 * @param a A boolean array to represent which byte codes are used
 * @param minIndex The previously found available byte
 * @return The index of the lowest available byte or -1 if all the 
 *         byte codes are used
 */
static int findReplacementByte(bool a[BYTE_CODES], int minIndex) 
{
    for (int i = minIndex; i < BYTE_CODES; i++) {
        if (a[i] == 0) {
            return i;
        }
    }
    return -1;
}

void serializeBlock(Block *block, Buffer *buf) 
{
    // Append the length 
    appendBytes(buf, &block->len, NUM_BYTES_LENGTH);

    // Append the data
    appendBytes(buf, block->data, block->len);
    
    // Append the number of replacement rules
    appendByte(buf, block->rcount);
    
    // Append each rules code
    for (int i = 0; i < block->rcount; i++) {
        appendByte(buf, block->rlist[i].code);
        appendByte(buf, block->rlist[i].first);
        appendByte(buf, block->rlist[i].second);
    }
}

bool deserializeBlock(Block *block, Buffer *buf) 
{
    // Read the length of the buffer
    unsigned short tempLen;
    if (!extractBytes(buf, &tempLen, NUM_BYTES_LENGTH)) { 
        return false;
    }
    block->len = tempLen;
    // Read the data in the buffer
    if (!extractBytes(buf, block->data, block->len)) {
        return false;
    }
    // Read the rule count
    byte tempRCount;
    if (!extractByte(buf, &tempRCount)) {
        return false;
    }
    block->rcount = tempRCount;
    // Read each rule
    for (int i = 0; i < block->rcount; i++) {
        if (!extractByte(buf, &block->rlist[i].code)) {
            return false;
        }
        if (!extractByte(buf, &block->rlist[i].first)) {
            return false;
        }
        if (!extractByte(buf, &block->rlist[i].second)) {
            return false;
        }            
    }
    return true;

}

void compressBlock(Block *block) 
{
    // Declare starting data trackers
    bool availableBytes[BYTE_CODES] = {0};
    int **hash = makeHash();
    int highestCountA = 0;
    int highestCountB = 0;
    int highestCount = 0;

    // Count the number of 2 byte sequences in the block
    // Indicate which byte codes are present in the block
    for (int i = 0; i + 1 < block->len; i++) {
        unsigned char ua = (unsigned char) block->data[i];
        unsigned char ub = (unsigned char) block->data[i + 1];
        int c = ++hash[ua][ub];
        availableBytes[ua] = true;
        availableBytes[ub] = true;
        if (c > highestCount || (c == highestCount && (ua < highestCountA ||
           (ua == highestCountA && ub < highestCountB)))) {
            highestCountA = ua;
            highestCountB = ub;
            highestCount = c;
        }
    }

    // Replace the highest 2 byte sequence with the lowest available byte
    // Create a replacement rule for each compressed sequence
    while (highestCount >= REPLACEMENT_THRESHOLD) {
        int minReplacementByte = findReplacementByte(availableBytes, 0);
        if (minReplacementByte < 0) {
            break;
        }
        availableBytes[minReplacementByte] = true;

        for (int i = 0; i + 1 < block->len; i++) {
            unsigned char ua = (unsigned char) block->data[i];
            unsigned char ub = (unsigned char) block->data[i + 1];
            if (ua == (unsigned char) highestCountA && ub == (unsigned char) highestCountB) {
                block->data[i] = (byte) minReplacementByte;
                memmove(&block->data[i + 1], &block->data[i + FORWARD_CHECK_INDEX], block->len - (i + FORWARD_CHECK_INDEX));
                block->len--;
                if ( i > 0) {
                    i--;
                }
            }
        }

        block->rlist[block->rcount].code = (byte) minReplacementByte;
        block->rlist[block->rcount].first = (byte) highestCountA;
        block->rlist[block->rcount].second = (byte) highestCountB;
        block->rcount++;

        // Recount the number of 2 byte sequences in the block
        for (int i = 0; i < BYTE_CODES; i++) {
            memset(hash[i], 0, BYTE_CODES*sizeof(int));
        }
        highestCount = 0;
        for (int i = 0; i + 1 < block->len; i++) {
            unsigned char ua = (unsigned char) block->data[i];
            unsigned char ub = (unsigned char) block->data[i + 1];
            int c = ++hash[ua][ub];
            if (c > highestCount || (c == highestCount && (ua < highestCountA ||
                                    (ua == highestCountA && ub < highestCountB)))) {
            highestCountA = ua;
            highestCountB = ub;
            highestCount = c;
            }
        }
    }

    freeHash(hash);
}

bool uncompressBlock(Block *block) 
{
    // Iterate through each replacement rule
    for (int i = block->rcount - 1; i >= 0; i--) {
        byte code = block->rlist[i].code;
        byte first = block->rlist[i].first;
        byte second = block->rlist[i].second;
        // Iterate thorugh the length of the block
        for (int i = 0; i < block->len; i++) {
            unsigned char ua = (unsigned char) block->data[i];
            // Check if the current char is equal to the current replacement byte
            if (ua == code) {
                if (block->len + 1 > BLOCK_SIZE_LIMIT) {
                    return false;
                }
                // Create a space for the replacement characters
                memmove(&block->data[i + FORWARD_CHECK_INDEX], &block->data[i + 1], block->len - (i + 1));
                block->data[i] = first;
                block->data[i + 1] = second;
                block->len++;
                i++;
            }
        }
    }
    return true;
}

Buffer *compressData(Buffer *src) 
{
    Buffer *dest = makeBuffer();

    // Iterate through each byte in data
    while (src->pos < src->len) {
        Block b;
        // Determine how many bytes will be in the block
        int remaining = src->len - src->pos;
        int maxCount;
        if (remaining - BLOCK_SIZE_LIMIT > 0) {
            maxCount = BLOCK_SIZE_LIMIT;
        }
        else {
            maxCount = remaining;
        }
        bool availableBytes[BYTE_CODES] = {0};
        int count = 0;
        int numUnique = 0;
        for (int i = 0; i < maxCount; i++) {
            unsigned char uc = (unsigned char) src->data[src->pos + i];
            if (!availableBytes[uc]) {
                if (numUnique == MAX_BYTE_CODES) {
                    break;
                }
                availableBytes[uc] = true;
                numUnique++;
                
            }
            count++;
        }
        
        // Put the bytes in the block
        extractBytes(src, b.data, count);
        b.len = count;
        b.rcount = 0;
        // Compress the block and put it in the destination buffer
        compressBlock(&b);
        serializeBlock(&b, dest);
    }
    return dest;
}

Buffer *uncompressData(Buffer *src) 
{
    Buffer *dest = makeBuffer();
    // Iterate through each byte in the data
    while (src->pos < src->len) {
        Block b; 
        // Put the compressed data into a block and uncompress
        if (!deserializeBlock(&b, src)) {
            return NULL;
        }
        if (!uncompressBlock(&b)) {
            return NULL;
        }
        // Add the bytes to the destination buffer
        appendBytes(dest, b.data, b.len);
    }
    return dest;
}
