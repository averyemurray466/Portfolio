/**
 * @file error.c
 * @author Avery Murray (aemurra3)
 * 
 * Contains the implementation for setting error messages
 * for issues in program operation in the snark.c component
 * and in the user input. 
 */

#include "error.h"

#include <stdlib.h>
#include <string.h>

/** Stores a reference to the generate error message. */
static char *errorMessage;

bool setErrorMessage(char const *message) 
{
    if (message == NULL) {
        free(errorMessage);
        errorMessage = NULL;
        return true;
    }
    char *temp = realloc(errorMessage, strlen(message) + 1);
    errorMessage = temp;
    strcpy(errorMessage, message);
    return true;
}

char const *getErrorMessage() 
{
    if (errorMessage == NULL) { 
        char const *temp = NULL;
        return temp;
    }
    return errorMessage;
}
