/**
 * @file IO.c
 * @author Avery Murray (aemurra3)
 * 
 * Implements IO functionality by reading users commands, 
 * and reading and writing the contents of a data file.
 * Uses the buffer file to help read and write to files.  
 */

#include "IO.h"
#include "buffer.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/** Factor to multiply the current capacity of the string by. */
#define INCREASE_CAPACITY_FACTOR 2

/** Initial capacity of the array to read the string in to. */
#define INITIAL_CAPACITY 10

/**
 * Checks if the given string is at capacity and
 * resizes the dynamically allocated array if needed.
 * 
 * @param str A pointer to a char pointer that contains the string
 * @param currCap A pointer to the capacity of the string 
 * @param minIncrease The minimum number of characters the new string must have
 */
static void resizeString(char **str, int *currCap, int minIncrease) 
{
    if (minIncrease + 1 > *currCap) {
       *currCap *= INCREASE_CAPACITY_FACTOR;
       *str = realloc(*str, *currCap * sizeof(byte)); 
    }
}

char *readLine(FILE *fp) 
{
    int cap = INITIAL_CAPACITY;
    char *string = (char *)malloc(cap * sizeof(byte));
    int c;
    int count = 0;
    while ((c = fgetc(fp)) != '\n' && c != EOF) {
        resizeString(&string, &cap, count + 1);
        *(string + count) = c;
        count++;
        *(string + count) = '\0';
    }
    
    // If we saw EOF returns NULL
    if (count == 0 && c == EOF) {
        free(string);
        return NULL;
    }
    string[count] = '\0';

    return string;
}

Buffer *readFile(char const *filename) 
{
    FILE *fn = fopen(filename, "rb");
    // If file cannot be opened returns NULL
    if (fn == NULL) {
        return NULL;
    }

    Buffer *b = makeBuffer();
    int c;
    while ((c = fgetc(fn)) != EOF) {
        appendByte(b, (byte)c);
    }
    fclose(fn);
    return b;
}

bool writeFile(char const *filename, Buffer *buf) 
{
    FILE *fn = fopen(filename, "wb");
    // If file cannot be opened returns false
    if (fn == NULL) {
        return false;
    }
    fwrite(buf->data, sizeof(byte), buf->len, fn);
    fclose(fn);
    return true;
}
