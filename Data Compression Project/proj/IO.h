/**
 * @file IO.h
 * @author Avery Murray (aemurra3)
 */

#ifndef IO_H
#define IO_H

#include <stdio.h>
#include <stdbool.h>
#include "buffer.h"

#endif

/**
 * Reads in a single line at a time from the given file stream. 
 * 
 * @param fp File strean to read from
 * @return A char pointer to the read in line
 */
char *readLine(FILE *fp);

/**
 * Reads in the entire contents of the given file to a Buffer. 
 * 
 * @param filename A constant char pointer to the filename to open
 * @return A pointer to a Buffer that contains all the data in the file
 */
Buffer *readFile(char const *filename);

/**
 * Writes the contents of the given Buffer's data to the given filename. 
 * 
 * @param filename File to open to write to 
 * @param buf A pointer to a Buffer to write to
 * @return True if the file was successfuly written to 
 *         False if the file could not be written to
 */
bool writeFile(char const *filename, Buffer *buf);