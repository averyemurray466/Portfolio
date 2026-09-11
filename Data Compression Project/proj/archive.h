/**
 * @file archive.h
 * @author Avery Murray (aemurra3)
 * 
 * Describes the methods in the archive.c class that can 
 * create and save Archives from files, remove Archives, 
 * and iterate through Archives. 
 */

#ifndef ARCHIVE_H
#define ARCHIVE_H

#include "IO.h"

typedef struct {
  /** Name and path of the file. */
  char *name;

  /** Raw, uncompressed copy of the file's contents. */
  Buffer *raw;

  /** Compessed copy of the file's contents. */
  Buffer *comp;
} FileRec;

/** Incomplete type for the archive. */
typedef struct ArchiveStruct Archive;

#endif

/**
 * Creates a new archive by dynamically allocated new memory
 * 
 * @return A pointer to the created Archive
 */
Archive *makeArchive();

/**
 * Frees the data in an Archive
 * 
 * @param arc A pointer to the Archive to free
 */
void freeArchive(Archive *arc);

/**
 * Loads in the data in the file to an Archive. 
 * Creates a FileRec and a FileNode for each file in the archive. 
 * Fills the data of each file into the fields of FileRec and FileNode. 
 * Generates the linked Archive list. 
 * If the given archive is empty "Can't open archive" is printed to stdout. 
 * If the given archive does not have a null terminated sequence for the filename
 * "Invalid archive" is printed to stdout. 
 * 
 * @param fname A constant char pointer to the name of the file to read
 * @return A pointer to the created Archive
 */
Archive *loadArchive(char const *fname);

/**
 * Reads in the given file from the filename and add the contents 
 * of the file into a FileNode and adds it to the Archive. 
 * If the archive already contains the given file name "Archive already contains file" is printed to stdout. 
 * If there is not data in the given file "Can't read file" is printed to stdout. 
 * 
 * @param arc A pointer to the Archive to add to 
 * @param fname A constant char pointer to the filename to open
 * @return True if the file was added to the Archive
 *         False if the file was not added to the Archive due to an error
 */
bool addArchive(Archive *arc, char const *fname);

/**
 * Removes the given file from the archive. 
 * If the file is not in the archive then false is returned and 
 * "Archive doesn't contain file" is printed to stdout. 
 * 
 * @param arc A pointer to the Archive to remove from 
 * @param fname A constant char pointer to the filename to remove 
 * @return True if the file was removed from the Archive
 *         False if the file was not removed from the Archive
 */
bool removeArchive(Archive *arc, char const *fname);

/**
 * Copies the uncompressed contents of the given file in the archive
 * to an output file to write to. 
 * If the given file name is not in the archive "Archive doesn't contain file" is printed to stdout. 
 * If the output file aready exists in the file system "Output file already exists" is printed to stdout. 
 * If there is no data in the file's uncompressed field "Can't create output file" is printed to stdout. 
 * If the data cannot be written to the output file "Cant create output file" is printed to stdout. 
 * 
 * @param arc A pointer to the Archive to extract from 
 * @param fname A constant char pointer to the filename to find and write to
 * @return True if the file was found and contents written to output file
 *         False if the file was not found or the contents were not written to output file
 */
bool extractArchive(Archive *arc, char const *fname);

/**
 * Saves the given archive files compressed data to the given output file. 
 * If the archive contents cannot be written to the output file an error message 
 * of "Can't create output file" is printed to stdout. 
 * 
 * @param arc A pointer to the Archive to save data from 
 * @param fname A constant char pointer to the filename to write to
 * @return True if the contents of the archive were written without failure
 *         False if the contents of the archive were not written properly
 */
bool saveArchive(Archive *arc, char const *fname);

/**
 * Iterates over all the entries in the archive according to the visitor function and context pointer. 
 * If the archive is empty an error message of "Archive is empty" is printed to stdout. 
 * 
 * @param arc A pointer to the Archive to iterate through
 * @param visitor() A pointer to the visitor function that take a FileRec and a context object to iterate
 *        through the archive in a specified manner
 * @param context a void pointer to an object to decide criteria for iterating through the archive
 */
void traverseArchive(Archive *arc, void visitor(FileRec *rec, void *context), void *context);