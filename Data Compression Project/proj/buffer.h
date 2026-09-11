/**
 * @file buffer.h
 * @author Avery Murray (aemurra3)
 * 
 * Declares the methods and structs used in buffer.c
 * to create and modify buffers. 
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <stdbool.h>

/** Type used to represent a byte. */
typedef unsigned char byte;

typedef struct {
  /** Dynamically allocated sequence of bytes stored in this buffer. */
  byte *data;

  /** Number of bytes stored in this buffer. */
  int len;

  /** Capacity of the data array. */
  int cap;
  
  /** Current position in the buffer, to simplify processing the
      contents of the buffer from front to back. */
  int pos;
} Buffer;

#endif

/**
 * Creates a Buffer by dynamically allocating memory
 * and setting int fields to 0.
 * 
 * @return A pointer to the new Buffer
 */
Buffer *makeBuffer();

/**
 * Frees the dynamic memory for a Buffer.
 * 
 * @param buf A pointer to the Buffer to free
 */
void freeBuffer(Buffer *buf);

/**
 * Appends a single value to the end of the data
 * array in the given buffer. Grows the memory of 
 * the data array if needed. 
 * 
 * @param buf A pointer to the Buffer
 * @param val A byte value to append to buf's data
 */
void appendByte(Buffer *buf, byte val);

/**
 * Appends a sequence of values to the end of the data
 * array in the given buffer. Grows the memory of 
 * the data array if needed.  
 * 
 * @param buf A pointer to the Buffer
 * @param seq A void pointer to the sequence of 
 *            bytes to add to data
 * @param n The number of bytes to add to data
 */
void appendBytes(Buffer *buf, void *seq, int n);

/**
 * Sets the value pointer to the next position in
 * buf's data array. 
 * 
 * @param buf A pointer to the Buffer
 * @param val A pointer to the next byte in data
 * @return Returns true if the next byte
 *             exists in buf's data and was set to val.
 *         Returns false if the next byte does not exist 
 *             in buf's data and val is uninitalized. 
 */
bool extractByte(Buffer *buf, byte *val);

/**
 * Sets the seq pointer to the next segment of 
 * bytes in buf's data array. 
 * 
 * @param buf A pointer to the Buffer
 * @param seq A void pointer to the sequence of 
 *            bytes in data
 * @param n The number of bytes to extract
 * @return Returns true if the n sequence of bytes exists
 *             in buf's data and was set to seq. 
 *         Returns false if the n sequence of
 *             bytes does not exist in buf's data
 *             and seq is uninitalized. 
 */
bool extractBytes(Buffer *buf, void *seq, int n);

