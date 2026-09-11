/**
 * @file archive.c
 * @author Avery Murray (aemurra3)
 * 
 * Implements the archive behaviors to access an archive contents 
 * and save and load the archive to and from files. 
 */

#include "archive.h"
#include "compress.h"
#include "error.h"
#include "IO.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

/** Number of bytes in an integer. */
#define INT_BYTES 4

/** Representation for a node on a linked list of FileRec objects. */
struct FileNodeStruct {
    /** FileRec struct stored inside this node (rather than by pointer). */
    FileRec rec;
  
    /** Pointer to the next node on the linked list. */
    struct FileNodeStruct *next;
};

/** Shorter name for FileNodeStruct */
typedef struct FileNodeStruct FileNode;

/** Representation of an archive, a linked list of FileNodes. */
struct ArchiveStruct {
    /** Head pointer of the head of a list of FileNodes. */
    FileNode *head;
};

/**
 * Checks if the given filename has a foward slash. 
 * Sets the error message to "Invalid file name".
 * 
 * @param fn A constant char pointer to the filename to check 
 * @return True if there is not foward slash present 
 *         False if there is a forward slash present
 */
static bool forwardSlashCheck(char const *fn) 
{
    // Check if the fname contains an forward slash
    if (strstr(fn, "/") != NULL) {
        setErrorMessage("Invalid file name");
        return false;
    }
    return true;
}

Archive *makeArchive() 
{
    Archive *archive = (Archive *)malloc(sizeof(Archive));
    archive->head = NULL;
    return archive;
}

void freeArchive(Archive *arc) 
{
    FileNode *curr = arc->head;
    while (curr) {
        FileNode *next = curr->next;
        free(curr->rec.name);
        freeBuffer(curr->rec.raw);
        freeBuffer(curr->rec.comp);
        free(curr);
        curr = next;
    }
    free(arc);
}

Archive *loadArchive(char const *fname) 
{
    // Create buffer and archive to load into
    Buffer *b = readFile(fname);
    if (b == NULL) {
        setErrorMessage("Can't open archive");
        return NULL;
    }
    byte currentByte;
    Archive *archive = makeArchive();
    // Loop through all the bytes in the buffer
    while (b->pos < b->len) {
        // Create a FileRec to load into
        int fileNameCount = 0;
        bool nullTermFound = false;
        FileRec currFileRec;
        currFileRec.raw = NULL;
        currFileRec.comp = NULL;
        // Reads the bytes in the buffer until null terminator
        while (extractByte(b, &currentByte)) {
            fileNameCount++;
            // Load the FileRec with the name of the file
            if (currentByte == '\0') {
                b->pos -= fileNameCount;
                currFileRec.name = malloc(fileNameCount);
                extractBytes(b, currFileRec.name, fileNameCount);
                nullTermFound = true;
                break;
            }
        }
        if (!nullTermFound) {
            freeArchive(archive);
            freeBuffer(b);
            setErrorMessage("Invalid archive");
            return NULL;
        }

        // Find the size of the compressed file contents
        int fileSize;
        extractBytes(b, &fileSize, INT_BYTES);
        currFileRec.comp = makeBuffer();

        // Load compressed file contents into a buffer
        byte *temp = (byte *)malloc(fileSize);
        extractBytes(b, temp, fileSize);

        // Add buffer to FileRec compressed field
        appendBytes(currFileRec.comp, temp, fileSize);
        currFileRec.comp->pos = 0;
        currFileRec.raw = uncompressData(currFileRec.comp);
        free(temp);

        // Create a new FileNode to contain newly created FileRec and add to the archive
        FileNode *currFileNode = malloc(sizeof(FileNode));
        currFileNode->rec = currFileRec;
        FileNode **currFN = &archive->head;
        while (*currFN && strcmp((*currFN)->rec.name, currFileRec.name) < 0) {
            currFN = &(*currFN)->next;
        }
        currFileNode->next = *currFN;
        *currFN = currFileNode;
  }
  freeBuffer(b);
  return archive;
}

bool addArchive(Archive *arc, char const *fname) 
{
    if (!forwardSlashCheck(fname)) {
        return false;
    }
    
    // Check if the fname is already in the Archive
    FileNode **currFN = &arc->head;
    while (*currFN && strcmp((*currFN)->rec.name, fname) < 0) {
        currFN = &(*currFN)->next;
    }
    if (*currFN && strcmp((*currFN)->rec.name, fname) == 0 ) {
        setErrorMessage("Archive already contains file");
        return false;
    }

    // Read the data in the file to a buffer
    Buffer *b = readFile(fname);
    if (b == NULL) {
        setErrorMessage("Can't read file");
        return false;
    }

    // Create a FileRec to load into
    FileRec currFileRec;
    currFileRec.raw = b;
    currFileRec.comp = compressData(b);
    currFileRec.name = malloc(strlen(fname) + 1);
    strcpy(currFileRec.name, fname);

    // Create a new FileNode to contain newly created FileRec and add to the archive
    FileNode *currFileNode = malloc(sizeof(FileNode));
    currFileNode->rec = currFileRec;
    currFileNode->next = *currFN;
    *currFN = currFileNode;
    return true;
}

bool removeArchive(Archive *arc, char const *fname) 
{
    if (!forwardSlashCheck(fname)) {
        return false;
    }

    // Make a pointer to a pointer the current FileNode 
    FileNode **currFN = &arc->head;
    while (*currFN && strcmp(fname, (*currFN)->rec.name) != 0) {
        currFN = &(*currFN)->next;
    } 
    if (*currFN == NULL) {
        setErrorMessage("Archive doesn't contain file");
        return false;  
    }
    FileNode *n = *currFN;
    *currFN = n->next;
    free(n->rec.name);
    freeBuffer(n->rec.raw);
    freeBuffer(n->rec.comp);
    free(n);
    return true;
}

bool extractArchive(Archive *arc, char const *fname) 
{
    if (!forwardSlashCheck(fname)) {
        return false;
    }

    // Check if the fname is in the Archive
    FileNode *currFN = arc->head;
    while (currFN && strcmp(fname, currFN->rec.name) != 0) {
        currFN= currFN->next;
    } 
    if (currFN == NULL) {
        setErrorMessage("Archive doesn't contain file");
        return false;
    }

    // Check if output file already exists
    struct stat statbuf;
    if (stat(fname, &statbuf) == 0) {
        setErrorMessage("Output file already exists");
        return false;
    }

    if (currFN->rec.raw == NULL) {
        setErrorMessage("Can't create output file");
        return false;
    }

    if (!writeFile(fname, currFN->rec.raw)) {
        setErrorMessage("Can't create output file");
        return false;
    }

    return true;
}

bool saveArchive(Archive *arc, char const *fname) 
{
    Buffer *b = makeBuffer();
    FileNode **currFN = &arc->head;
    while (*currFN) {
        int nameLength = strlen((*currFN)->rec.name) + 1;
        appendBytes(b, (*currFN)->rec.name, nameLength);
        int fileLength = (*currFN)->rec.comp->len;
        appendBytes(b, &fileLength, INT_BYTES);
        appendBytes(b, (*currFN)->rec.comp->data, (*currFN)->rec.comp->len);
        currFN = &(*currFN)->next;
    } 
    if (!writeFile(fname, b)) {
        setErrorMessage("Can't create output file");
        freeBuffer(b);
        return false;
    }
    freeBuffer(b);
    return true;
}

void traverseArchive(Archive *arc, void visitor(FileRec *rec, void *context), void *context) 
{
    if (arc == NULL) {
        setErrorMessage("Archive is empty\n");
    }
    FileNode *currFN = arc->head;
    while (currFN) {
        visitor(&currFN->rec, context);
        currFN = currFN->next;
    }
}

