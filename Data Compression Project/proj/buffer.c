/**
 * @file buffer.c
 * @author Avery Murray (aemurra3)
 * 
 * Defines how to create a buffer, add bytes to a buffer, 
 * and how to extract bytes for a buffer.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "buffer.h"

/** How much the capacity of a string is multiplied by when increasing size. */
#define STRING_CAP_INCREASE_FACTOR 2

/** Initial capacity of a buffer's data. */
#define INITIAL_BUFFER_CAPACITY 5

/**
 * Checks if buf's data array is full and needs to grow
 * its capacity. If data is full the capacity is doubled. 
 * 
 * @param buf A pointer to the Buffer
 * @param size The new desired length of data
 */
static void checkArraySize(Buffer *buf, int size) {
    while (size > buf->cap) {
        buf->cap *= STRING_CAP_INCREASE_FACTOR;
        buf->data = realloc(buf->data, buf->cap * sizeof(byte));
    }
}

Buffer *makeBuffer() {
    Buffer *b = (Buffer *)malloc(sizeof(Buffer));
    b->cap = INITIAL_BUFFER_CAPACITY;
    b->len = 0;
    b->data = (byte *)malloc(b->cap * sizeof(byte));
    b->pos = 0;
    return b;
}

void freeBuffer(Buffer *buf) {
    free(buf->data);
    free(buf); 
}

void appendByte(Buffer *buf, byte val) {
    checkArraySize(buf, buf->len + 1);
    buf->data[buf->len++] = val;
}

void appendBytes(Buffer *buf, void *seq, int n) {
    byte *newSeq = (byte *) seq;
    int tempLen = buf->len + n;
    checkArraySize(buf, tempLen);
    memcpy(buf->data + buf->len, newSeq, n);
    buf->len = tempLen;
}

bool extractByte(Buffer *buf, byte *val) {
    if (buf->pos + 1 > buf->len) {
        return false;
    }
    *val = buf->data[buf->pos++];
    return true;
}

bool extractBytes(Buffer *buf, void *seq, int n) {
    byte *newSeq = (byte *) seq;
    if ((buf->pos + n) > buf->len) {
        return false;
    }
    memcpy(newSeq, buf->data + buf->pos, n);
    buf->pos += n;
    return true;
}

