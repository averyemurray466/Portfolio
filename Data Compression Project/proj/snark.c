/**
 * @file snark.c
 * @author Avery Murray (aemurra3)
 * 
 * Calls the Archive functions to interpret user input from stdin and
 * from the command line. Implements two visitor functions to traverse 
 * Archive information. 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <setjmp.h>

#include "archive.h"
#include "error.h"

/** Minimum name length for showing the contents of an archive. */
#define MIN_NAME_LEN 20

/** Maximum file name length. */
#define MAX_FILE_NAME 100

/** Maximum test prompt length. */
#define MAX_TEST_PROMPT 10

/** Size of the array to check if there is user input after a file name. */
#define EXTRA_BYTE_INDEX 2

/** Number of user arguments for error. */
#define ARGUMENT_THRESHOLD 3

/** Number of expected user arguments. */
#define EXPECTED_NUM_ARGUMENTS 2

/** Factor to mod quote count by to ensure closing quotations. */
#define EVEN_QUOTE_CHECK 2

/**
 * Struct to save information about the number of files and the 
 * length of the longest file name. 
 */
typedef struct {
    /** Number of files in the Archive */
    int fileCount;

    /** Length of the longest filename in the Archive */
    int maxNameLength;
} ArchiveInfo;

/**
 * Loads and updates data in the ArchiveInfo to determine
 * the max filename length and the number of files in the Archive. 
 * 
 * @param rec A pointer to the FileRec to visit
 * @param context A pointer to the ArchiveInfo struct
 */
static void infoVisitor(FileRec *rec, void *context) 
{
    ArchiveInfo *ai = context;
    ai->fileCount++;
    int currLen = strlen(rec->name);
    if (currLen > ai->maxNameLength) {
        ai->maxNameLength = currLen;
    }

}

/**
 * Prints a usage error "usage: snark [-a archive-file] [-s script-file]"
 * to stderr if the incorrect number or order of input arguments are provided.
 */
static void usageError() 
{
    fprintf(stderr, "usage: snark [-a archive-file] [-s script-file]\n");
}

/**
 * Prints out each filename and the uncompressed and compressed 
 * buffer's size of each file in the Archive. 
 * 
 * @param rec A pointer to the FileRec to print 
 * @param context A pointer to the number of spaces to allocate for the 
 *               file name field
 */
static void printVisitor(FileRec *rec, void *context) 
{
    int nameLength = *(int *)context;

    printf("%s", rec->name);
    int padding = nameLength - strlen(rec->name);

    for (int i = 0; i < padding; i++) {
        putchar(' ');
    }
    printf(" %8d %8d\n", rec->raw->len, rec->comp->len);
}

bool handleCommand(Archive *arc, char *line, jmp_buf *env) 
{
    char testPrompt[MAX_TEST_PROMPT];
    char fn [MAX_FILE_NAME];

    if (sscanf(line, "%9s", testPrompt) != 1) {
        return false;
    }

    // Comment line
    if (testPrompt[0] == '#') {
        return false;
    }
    
    if (strcmp(testPrompt, "add") == 0 || strcmp(testPrompt, "remove") == 0
       || strcmp(testPrompt, "extract") == 0 || strcmp(testPrompt, "save") == 0) {

        int n;
        //char check [EXTRA_BYTE_INDEX];
        n = sscanf(line, " %9s %99[^\n]", testPrompt, fn);
        if (n < EXPECTED_NUM_ARGUMENTS) {
            setErrorMessage("Invalid command");
            longjmp(*env, 1);
        }

        int quoteCount = 0;
        char fnCopy [MAX_FILE_NAME];
        int nameLength = strlen(fn);
        int j = 0;
        for (int i = 0; i < nameLength; i++) {
            if (fn[i] == '"') {
                quoteCount++;
            }
            else if (fn[i] == ' ' && quoteCount % EVEN_QUOTE_CHECK == 0) {
                for (int k = i; k < nameLength; k++) {
                    if (fn[k] != ' ') {
                        setErrorMessage("Invalid command");
                        longjmp(*env, 1);
                    }
                }
                break;
            }
            else {
                fnCopy[j++] = fn[i];
            }
        }
        fnCopy[j] = '\0';
        if (quoteCount % EVEN_QUOTE_CHECK != 0) {
            setErrorMessage("Unterminated quote");
            longjmp(*env, 1);
        }
        
        if (strcmp(testPrompt, "add") == 0) {
            if (!addArchive(arc, fnCopy)) {
                longjmp(*env, 1);
            }
            return false;
        }
        else if (strcmp(testPrompt, "remove") == 0) {
            if (!removeArchive(arc, fnCopy)) {
                longjmp(*env, 1);
            }
            return false;
        }
        else if (strcmp(testPrompt, "extract") == 0) {
            if (!extractArchive(arc, fnCopy)) {
                longjmp(*env, 1);
            }
            return false;
        }
        else if (strcmp(testPrompt, "save") == 0) {
            if (!saveArchive(arc, fnCopy)) {
                longjmp(*env, 1);
            }
            return false;
        }
    }
    
    // No files needed in prompt
    if (strcmp(testPrompt, "show") == 0) {
        ArchiveInfo archiveInfo = {.fileCount = 0, .maxNameLength = 0};
        traverseArchive(arc, infoVisitor, &archiveInfo);
        if (archiveInfo.fileCount == 0) {
            printf("Archive is empty\n");
            return false;
        }
        
        int nameLength = archiveInfo.maxNameLength;
        if (nameLength < MIN_NAME_LEN) {
            nameLength = MIN_NAME_LEN;
        }
        printf("File");
        int padding = nameLength - strlen("File");
        for (int i = 0; i < padding; i++) {
            putchar(' ');
        }
        printf(" %8s %8s\n", "orig", "comp");
        traverseArchive(arc, printVisitor, &nameLength);
        return false;
    }
    if (strcmp(testPrompt, "quit") == 0) {
        return true;
    }
    setErrorMessage("Invalid file name");
    longjmp(*env, 1);
    return false;
}

int main(int argc, char *argv[]) 
{

    // Handles reading in command line arguments 
    char *archiveFileName = NULL;
    char *scriptFileName = NULL;
    char *line;
    bool scriptMode = false;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0) {
            if (archiveFileName != NULL || i + 1 >= argc) {
                usageError();
                return EXIT_FAILURE;
            }
            archiveFileName = argv[++i];
        }
        
        else if (strcmp(argv[i], "-s") == 0) {
            if (scriptFileName != NULL || i + 1 > argc) {
                usageError();
                return EXIT_FAILURE;
            }
            scriptFileName = argv[++i];
            scriptMode = true;
        }
        else {
            usageError();
            return EXIT_FAILURE;
        }
        
    }

    // Handles creating an archive - if one is provided or none is provided
    Archive *arc;
    if (archiveFileName != NULL)  {
        arc = loadArchive(archiveFileName);
        if (arc == NULL) {
            fprintf(stderr, "%s\n", getErrorMessage());
            return EXIT_FAILURE;
        }
    }
    else {
        arc = makeArchive();
        if (arc == NULL) {
            fprintf(stderr, "%s\n", getErrorMessage());
            return EXIT_FAILURE;
        }
    }

    FILE *in;

    if (scriptMode) {
        in = fopen(scriptFileName, "r");
    }
    else {
        in = stdin;
    }

    bool quit = false;
    jmp_buf env;
    while (!quit) {
        
        int  j = setjmp(env);
        if (j != 0) {
            if (line != NULL) {
                free(line);
                line = NULL;
            }
            printf("%s\n", getErrorMessage());
            setErrorMessage(NULL);
        }

        if (!scriptMode) {
            printf("cmd> ");
        }

        line = readLine(in);
        if (line == NULL) {
            break;
        }
        if (!scriptMode) {
            printf("%s\n", line);
        }
        quit = handleCommand(arc, line, &env);
        free(line);
    }

    if (scriptMode && in != NULL) {
        fclose(in);
    }

    freeArchive(arc);
    return EXIT_SUCCESS;
}